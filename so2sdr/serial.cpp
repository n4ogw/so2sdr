/*! Copyright 2010-2020 R. Torsten Clay N4OGW

   This file is part of so2sdr.

    so2sdr is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    so2sdr is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with so2sdr.  If not, see <http://www.gnu.org/licenses/>.

 */
#include "defines.h"
#include "serial.h"
#include <stdio.h>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QHostAddress>
#include <QSettings>
#include <QString>
#include <QTcpSocket>
#include <QThread>
#include <QTimerEvent>
#include <QByteArray>

// need to define this internal hamlib function
extern "C" HAMLIB_EXPORT(int) write_block(hamlib_port_t *p, const char *txbuffer, size_t count);

// initialize statics
QList<hamlibmfg> RigSerial::mfg;
QList<QByteArray> RigSerial::mfgName;

/*! comparison for radio manufacturer class. Compare by name
 */
bool hamlibmfg::operator<(const hamlibmfg &other) const
{
    return (mfg_name<other.mfg_name);
}

/*! comparison for rig models within a manufacturer. Alphabetize by name
 */
bool hamlibModel::operator<(const hamlibModel &other) const
{
    return (model_name<other.model_name);
}


RigSerial::RigSerial(int nr,QString s)
{
    settingsFile=s;
    nrig=nr;

    // turn off all the debug messages coming from hamlib
    rig_set_debug_level(RIG_DEBUG_NONE);

    // load all backends and step through them. list_caps function defined below.
    // only radio 1 will do this, since one static copy is kept for both radios
    if (nr==0) {
        rig_load_all_backends();
        rig_list_foreach(list_caps,nullptr);

        // sort list by manufacturer name
        std::sort(mfg.begin(),mfg.end());
        std::sort(mfgName.begin(),mfgName.end());

        // sort list of rigs for each manuacturer
        for (int i=0;i<mfg.size();i++) {
            std::sort(mfg[i].models.begin(),mfg[i].models.end());
        }
    }
    settings=nullptr;
    for (int i=0;i<nRigSerialTimers;i++) timerId[i]=0;
    rig=nullptr;
    socket=nullptr;
    pttOffFlag=    false;
    pttOnFlag=     false;
    clearRitFlag = 0;
    qsyFreq      = 0;
    chgMode      = RIG_MODE_NONE;
    radioOK      = false;
    ifFreq_      = 0;
    passBW      = 0;
    Mode=RIG_MODE_NONE;
    rigFreq=0;
    model=1;
    lock.unlock();
    pttMutex.unlock();
    lock.unlock();
}

/*! static function passed to rig_list_foreach
 *
 * see hamlib examples rigctl.c
 */
int RigSerial::list_caps(const struct rig_caps *caps, void *data)
{
    Q_UNUSED(data)

    hamlibModel newrig;
    newrig.model_name = caps->model_name;
    newrig.model_nr   = caps->rig_model;
    if (!mfgName.contains(caps->mfg_name)) {
        mfgName.append(caps->mfg_name);

        hamlibmfg newmfg;
        newmfg.mfg_name  = caps->mfg_name;
        newmfg.models.clear();
        newmfg.models.append(newrig);
        mfg.append(newmfg);
    } else {
        int indx=0;
        for (int j = 0; j < mfg.size(); j++) {
            if (mfg.at(j).mfg_name == caps->mfg_name) {
                indx = j;
                break;
            }
        }
        mfg[indx].models.append(newrig);
    }
    return -1;
}


/*! figure out the manufacturer and model index for combo boxes
   given the hamlib model number
 */
void RigSerial::hamlibModelLookup(int hamlib_nr, int &indx_mfg, int &indx_model) const
{
    // not the fastest way to search...
    bool found = false;
    for (int i = 0; i < mfg.size(); i++) {
        for (int j = 0; j < mfg.at(i).models.size(); j++) {
            if (hamlib_nr == mfg.at(i).models.at(j).model_nr) {
                found      = true;
                indx_mfg   = i;
                indx_model = j;
                break;
            }
        }
        if (found) break;
    }
    if (!found) {
        indx_mfg   = 0;
        indx_model = 0;
    }
}

/*! number of radio manufacturers defined in hamlib

 */
int RigSerial::hamlibNMfg() const
{
    return(mfg.size());
}

QString RigSerial::hamlibMfgName(int i) const
{
    if (i >= 0 && i < mfg.size()) {
        return(mfg.at(i).mfg_name);
    } else {
        return("");
    }
}

QString RigSerial::hamlibModelName(int i, int indx) const
{
    if (i >= 0 && i < mfg.size()) {
        if (indx >= 0 && indx < mfg.at(i).models.size()) {
            return(mfg.at(i).models.at(indx).model_name);
        }
    }
    return("");
}

int RigSerial::hamlibNModels(int i) const
{
    if (i >= 0 && i < mfg.size()) {
        return(mfg.at(i).models.size());
    } else {
        return(0);
    }
}

/*! returns hamlib model number

   indx1=index in mfg combo box
   indx2=index in model combo box
 */
int RigSerial::hamlibModelIndex(int indx1, int indx2) const
{
    if (indx1 >= 0 && indx1 < mfg.size()) {
        if (indx2 >= 0 && indx2 < mfg.at(indx1).models.size()) {
            return(mfg.at(indx1).models.at(indx2).model_nr);
        }
    }
    return(0);
}

RigSerial::~RigSerial()
{
    closeRig();
    rig_cleanup(rig);
    if (socket) delete socket;
    if (settings) delete settings;
}


/*!
   Called when thread started
 */
void RigSerial::run()
{
    if (!settings) settings=new QSettings(settingsFile,QSettings::IniFormat);
    openRig();
    openSocket();
    timerId[0] = startTimer(settings->value(s_radios_poll[nrig],500).toInt());
    timerId[1] = startTimer(RIG_SEND_TIMER);
}

/*! stop timers
 */
void RigSerial::stopSerial()
{
    for (int i = 0; i < nRigSerialTimers; i++) {
        killTimer(timerId[i]);
    }
}

/*! set ptt on or off
 * state=0 : off  state=1 : on
 */
void RigSerial::setPtt(int state)
{
    pttMutex.lock();
    if (state) {
        pttOnFlag=true;
    } else {
        pttOffFlag=true;
    }
    pttMutex.unlock();
}

/*!
   process timer events
 */
void RigSerial::timerEvent(QTimerEvent *event)
{
    // using rigctld for this radio
    if (settings->value(s_radios_rigctld_enable[nrig],s_radios_rigctld_enable_def[nrig]).toBool() &&
            socket->isOpen()) {

        if (event->timerId() == timerId[0]) {
            socket->write(";\\get_freq\n");
            socket->write(";\\get_mode\n");
            if (model==229) {
                socket->write(";\\get_level ifctr\n");
            }
        }
        if (event->timerId() == timerId[1]) {
            // clear RIT if flag set
            // behavior of hamlib over serial and via rigctld is different. At least with hamlib/rigctld
            // v 3.0.1, set_rit 0 does not turn off RIT like docs say.
            lock.lockForWrite();
            if (clearRitFlag) {
                clearRitFlag = false;
                lock.unlock();
                socket->write(";\\set_rit 0\n");
            }
            lock.unlock();
            pttMutex.lock();
            if (pttOnFlag) {
                pttOnFlag=false;
                pttMutex.unlock();
                socket->write(";\\set_ptt 1\n");
            }
            pttMutex.unlock();

            pttMutex.lock();
            if (pttOffFlag) {
                pttOffFlag=false;
                pttMutex.unlock();
                socket->write(";\\set_ptt 0\n");
            }
            pttMutex.unlock();

            lock.lockForWrite();
            if (qAbs(qsyFreq)>0.0) {
                rigFreq = qsyFreq;
                qsyFreq = 0;
                lock.unlock();
                QByteArray cmd=";\\set_freq "+QByteArray::number(rigFreq)+"\n";
                socket->write(cmd);
            }
            lock.lockForWrite();
            if ((chgMode > RIG_MODE_NONE) && (chgMode < RIG_MODE_TESTS_MAX)) {
                int j=chgMode;
                chgMode=RIG_MODE_NONE;
                lock.unlock();
                QByteArray width=QByteArray::number(static_cast<int>(passBW))+"\n";
                switch (j) {
                case RIG_MODE_CW:
                    socket->write(";\\set_mode CW "+width);
                    Mode=RIG_MODE_CW;
                    break;
                case RIG_MODE_CWR:
                    socket->write(";\\set_mode CWR "+width);
                    Mode=RIG_MODE_CWR;
                    break;
                case RIG_MODE_USB:
                    socket->write(";\\set_mode USB "+width);
                    Mode=RIG_MODE_USB;
                    break;
                case RIG_MODE_LSB:
                    socket->write(";\\set_mode LSB "+width);
                    Mode=RIG_MODE_LSB;
                    break;
                case RIG_MODE_RTTY:
                    socket->write(";\\set_mode RTTY "+width);
                    Mode=RIG_MODE_RTTY;
                    break;
                case RIG_MODE_RTTYR:
                    socket->write(";\\set_mode RTTYR "+width);
                    Mode=RIG_MODE_RTTYR;
                    break;
                case RIG_MODE_FM:
                    socket->write(";\\set_mode FM "+width);
                    Mode=RIG_MODE_FM;
                    break;
                case RIG_MODE_AM:
                    socket->write(";\\set_mode AM "+width);
                    Mode=RIG_MODE_AM;
                    break;
                case RIG_MODE_AMS:
                    socket->write(";\\set_mode AMS "+width);
                    Mode=RIG_MODE_AMS;
                    break;
                case RIG_MODE_DSB:
                    socket->write(";\\set_mode DSB "+width);
                    Mode=RIG_MODE_DSB;
                    break;
                default:
                    break;
                }
            }
            lock.unlock();
        }
    } else {
        // using hamlib over serial port

        // timers for sending data to radios (change freq, RIT clear,...)
        // timerID[1] is polled more frequently than the regular
        // radio poll timer
        if (radioOK && event->timerId() == timerId[1]) {
            // set PTT on
            pttMutex.lock();
            if (pttOnFlag) {
                pttOnFlag=false;
                pttMutex.unlock();
                rig_set_ptt(rig,RIG_VFO_CURR,RIG_PTT_ON);
            }
            pttMutex.unlock();

            // set PTT off
            pttMutex.lock();
            if (pttOffFlag) {
                pttOffFlag=false;
                pttMutex.unlock();
                rig_set_ptt(rig,RIG_VFO_CURR,RIG_PTT_OFF);
            }
            pttMutex.unlock();

            // clear RIT if flag set
            lock.lockForWrite();
            if (clearRitFlag) {
                clearRitFlag=false;
                lock.unlock();
                if (confParamsRIT) {
                    value_t val;
                    val.i = 0;
                    rig_set_ext_level(rig, RIG_VFO_CURR, confParamsRIT->token, val);
                } else {
                    // not preferred, as this turns RIT off completely
                    rig_set_rit(rig, RIG_VFO_CURR, 0);
                }
            }
            lock.unlock();

            // qsy radio
            lock.lockForWrite();
            if (qAbs(qsyFreq)>0.0) {
                double f=qsyFreq;
                qsyFreq=0;
                lock.unlock();
                int status = rig_set_freq(rig, RIG_VFO_CURR, f);
                if (status == RIG_OK) {
                    rigFreq = f;
                }
            }
            lock.unlock();

            // change mode on radio
            lock.lockForWrite();
            if ((chgMode > RIG_MODE_NONE) && (chgMode < RIG_MODE_TESTS_MAX)) {
                int status = rig_set_mode(rig, RIG_VFO_CURR, chgMode, passBW);
                if (status == RIG_OK) {
                    Mode = chgMode;
                }
                chgMode = RIG_MODE_NONE; // reset for next change
            }
            lock.unlock();
        }

        // poll frequency, mode, and IF freq from radio
        if (radioOK && event->timerId() == timerId[0]) {
            freq_t freq;

            int    status = rig_get_freq(rig, RIG_VFO_CURR, &freq);
            if (status == RIG_OK) {
                double ff = Hz(freq);
                if (ff != 0.0) rigFreq = ff;
            }

            rmode_t   m;
            pbwidth_t width;
            status = rig_get_mode(rig, RIG_VFO_CURR, &m, &width);
            if (status == RIG_OK) {
                Mode = m;
            }

            // if using K3, get IF center frequency
            if (model == 229) {
                value_t val;
                int     status = rig_get_ext_level(rig, RIG_VFO_CURR, confParamsIF->token, &val);
                if (status == RIG_OK) {
                    // no mutex protection since ifFreq is read-only from outside class
                    ifFreq_ = qRound(val.f - 8210000.0);
                }
            }
        }
    }
}

/*!
   clear RIT
 */
void RigSerial::clearRIT()
{
    lock.lockForWrite();
    clearRitFlag = true;
    lock.unlock();
}

/*! returns true if radio opened successfully
 */
bool RigSerial::radioOpen()
{
    lock.lockForRead();
    bool b = radioOK;
    lock.unlock();
    return(b);
}

/*! returns current radio frequency in Hz
 */
double RigSerial::getRigFreq()
{
    lock.lockForRead();
    double f = rigFreq;
    lock.unlock();
    return(f);
}

/*! returns current radio IF frequency (K3 only, others return 0)
 */
int RigSerial::ifFreq()
{
    lock.lockForRead();
    int f=ifFreq_;
    lock.unlock();
    return f;
}

/*! returns current radio mode

   rmode_t defined in hamlib
 */
rmode_t RigSerial::mode()
{
    lock.lockForRead();
    rmode_t m=Mode;
    lock.unlock();
    return m;
}

/*! return string describing the current mode for
 radio nrig
 */
QString RigSerial::modeStr()
{
    lock.lockForRead();
    rmode_t m=Mode;
    lock.unlock();
    switch (m) {
    case RIG_MODE_NONE: return modes[0];
    case RIG_MODE_AM: return modes[1];
    case RIG_MODE_CW: return modes[2];
    case RIG_MODE_USB: return modes[3];
    case RIG_MODE_LSB: return modes[4];
    case RIG_MODE_RTTY: return modes[5];
    case RIG_MODE_FM: return modes[6];
    case RIG_MODE_WFM: return modes[7];
    case RIG_MODE_CWR: return modes[8];
    case RIG_MODE_RTTYR: return modes[9];
    case RIG_MODE_AMS: return modes[10];
    case RIG_MODE_PKTLSB: return modes[11];
    case RIG_MODE_PKTUSB: return modes[12];
    case RIG_MODE_PKTFM: return modes[13];
    case RIG_MODE_ECSSUSB: return modes[14];
    case RIG_MODE_ECSSLSB: return modes[15];
    case RIG_MODE_FAX: return modes[16];
    case RIG_MODE_SAM: return modes[17];
    case RIG_MODE_SAL: return modes[18];
    case RIG_MODE_SAH: return modes[19];
    case RIG_MODE_DSB: return modes[20];
    default:return modes[0];
    }
}


/*! return current type of mode: CW, PHONE, DIGI
 * @todo this does not handle digi modes correctly - there is no way to identify these from hamlib
 */
ModeTypes RigSerial::modeType()
{
    lock.lockForRead();
    rmode_t m=Mode;
    lock.unlock();
    return getModeType(m);
}

/*! initialize TcpSocket for rigctld
 */
void RigSerial::openSocket()
{
    if (socket) {
        if (socket->isOpen()) socket->close();
        disconnect(socket,nullptr,nullptr,nullptr);
    }
    if (settings->value(s_radios_rigctld_enable[nrig],s_radios_rigctld_enable_def[nrig]).toBool()) {
        radioOK=true;
        if (!socket) socket=new QTcpSocket();

        // QHostAddress doesn't understand "localhost"
        if (settings->value(s_radios_rigctld_ip[nrig],s_radios_rigctld_ip_def[nrig]).toString().simplified()=="localhost") {
            socket->connectToHost(QHostAddress::LocalHost,
                                  settings->value(s_radios_rigctld_port[nrig],s_radios_rigctld_port_def[nrig]).toInt());
        } else {
            socket->connectToHost(QHostAddress(settings->value(s_radios_rigctld_ip[nrig],s_radios_rigctld_ip_def[nrig]).toString()).toString(),
                                  settings->value(s_radios_rigctld_port[nrig],s_radios_rigctld_port_def[nrig]).toInt());
        }
        connect(socket,SIGNAL(readyRead()),this,SLOT(rxSocket()));
        connect(socket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(tcpError(QAbstractSocket::SocketError)));
    }
}

/*! initialize radio
 *
 * this should run in the new thread
 */
void RigSerial::openRig()
{
    pttOffFlag=    false;
    pttOnFlag=     false;
    clearRitFlag = 0;
    qsyFreq      = 0;
    chgMode      = RIG_MODE_NONE;
    radioOK      = false;
    ifFreq_      = 0;
    model=settings->value(s_radios_rig[nrig],RIG_MODEL_DUMMY).toInt();
    if (rig) {
        rig_close(rig);
        rig_cleanup(rig);
    }
    rig = rig_init(model);
    token_t t = rig_token_lookup(rig, "rig_pathname");
    rig_set_conf(rig,t,settings->value(s_radios_port[nrig],defaultSerialPort[nrig]).toString().toLatin1().data());
    rig->state.rigport.parm.serial.rate=settings->value(s_radios_baud[nrig],s_radios_baud_def[nrig]).toInt();

    t = rig_token_lookup(rig,"ptt_type");
    token_t t_rts = rig_token_lookup(rig,"rts_state");
    token_t t_dtr = rig_token_lookup(rig,"dtr_state");

    // hamlib messes with the RTS/DTR lines when initializing a rig. If we are going
    // to use these for PTT (which is NOT done through hamlib but is in so2r.cpp), first turn
    // them off here so they don't get stuck on.
    switch (settings->value(s_radios_ptt_type[nrig],s_radios_ptt_type_def).toInt()) {
    case 1:
        rig_set_conf(rig,t_rts,"OFF");
        break;
    case 2:
        rig_set_conf(rig,t_dtr,"OFF");
        break;
    }

    // default starting freq/mode if communications to rigs
    // initially fails (or rig==RIG_MODEL_DUMMY)
    if (rigFreq<1.0 && nrig==0) rigFreq = 14000000;
    if (rigFreq<1.0 && nrig==1) rigFreq = 7000000;
    if (Mode==RIG_MODE_NONE)   Mode = RIG_MODE_CW;

    int r = rig_open(rig);
    if (model == RIG_MODEL_DUMMY) {
        rig_set_freq(rig, RIG_VFO_A, rigFreq);
        rig_set_vfo(rig, RIG_VFO_A);
        rig_set_mode(rig, RIG_VFO_A, Mode, RIG_PASSBAND_NORMAL);
    }
    if (r == RIG_OK) {
        radioOK = true;
        // get ext params struct for K3 in order to get IF center freq
        if (model == 229) {
            confParamsIF = rig_ext_lookup(rig, "ifctr");
        }

        // ext param for RIT clear
        confParamsRIT = rig_ext_lookup(rig, "ritclr");
    } else {
        emit(radioError("ERROR: radio "+QString::number(nrig+1)+" could not be opened"));
        radioOK = false;
        rig_close(rig);
    }
}

/*! qsy radio
   f=freq in Hz (no checking done)
 */
void RigSerial::qsyExact(double f)
{
    lock.lockForWrite();
    qsyFreq = f;
    lock.unlock();
}

/*! Set radio mode
 *
 * m = Hamlib mode, no checking is done
 * pb = Passband width in Hz
 */
void RigSerial::setRigMode(rmode_t m, pbwidth_t pb)
{
    lock.lockForWrite();
    chgMode = m;
    passBW = pb;
    lock.unlock();
}

/*! shut down radio interface
 */
void RigSerial::closeRig()
{
    rig_close(rig);
    radioOK = false;
}

/*! send a raw byte string to the radio: careful, there is no checking here!
 */
void RigSerial::sendRaw(QByteArray cmd)
{
    if (!cmd.contains('<')) {
        write_block(&rig->state.rigport,cmd.data(),cmd.size());
    } else {
        // numbers inside "< >" will be interpreted as hexadecimal bytes
        QByteArray data;
        data.clear();
        int i0=0;
        do {
            int i1=cmd.indexOf("<",i0);
            if (i1!=-1) {
                data=data+cmd.mid(i0,i1-i0);
            } else {
                break;
            }
            int i2=cmd.indexOf(">",i1);
            if (i2==-1 || (i2-i1)!=3) break;
            QByteArray hex=cmd.mid(i1+1,i2-i1-1);
            bool ok=false;
            char c=hex.toInt(&ok,16);
            if (ok) {
                data=data+c;
            }
            i0=i2+1;
        } while (i0<cmd.size());
        if ((i0+1)<cmd.size()) {
            data=data+cmd.mid(i0,-1);
        }
        write_block(&rig->state.rigport,data.data(),data.size());
    }
}

/*!
 * \brief RigSerial::rxSocket
 * \param nrig
 *
 * Parse data coming from rigctld
 */
void RigSerial::rxSocket()
{
    const QByteArray cmdNames[] = { "get_freq:",
                                    "get_mode:",
                                    "get_level: ifctr"};
    const int nCmdNames=3;
    const QByteArray modeNames[]= { "USB",
                                    "LSB",
                                    "CW",
                                    "CWR",
                                    "RTTY",
                                    "RTTYR",
                                    "AM",
                                    "FM",
                                    "WFM",
                                    "AMS",
                                    "PKTLSB",
                                    "PKTUSB",
                                    "PKTFM",
                                    "ECSSUSB",
                                    "ECSSLSB",
                                    "FAX",
                                    "SAM",
                                    "SAL",
                                    "SAH",
                                    "DSB"};
    const int nModeNames=20;
    // Note: PKTLSB and PKTUSB replaced by RTTY and RTTYR. rigctld v 3.0.1 returns PKT when K3 is
    // in AFSK data mode ??!
    const rmode_t modes[]={ RIG_MODE_USB,RIG_MODE_LSB,RIG_MODE_CW,RIG_MODE_CWR,RIG_MODE_RTTY,RIG_MODE_RTTYR,
                            RIG_MODE_AM,RIG_MODE_FM,RIG_MODE_WFM,RIG_MODE_AMS,RIG_MODE_RTTY,RIG_MODE_RTTYR,
                            RIG_MODE_PKTFM,RIG_MODE_ECSSUSB,RIG_MODE_ECSSLSB,RIG_MODE_FAX,RIG_MODE_SAM,
                            RIG_MODE_SAL,RIG_MODE_SAH,RIG_MODE_DSB};

    radioOK=true;
    lock.lockForWrite();
    while (socket->bytesAvailable()) {
        QByteArray data=socket->readAll();
        QList<QByteArray> cmdList=data.split(';');
        bool ok;
        double f;
        double iff;
        for (int i = 0; i < nCmdNames; i++) {
            if (cmdList.at(0)==cmdNames[i]) {
                switch (i) {
                case 0: // get_freq. Response looks like "get_freq:;Frequency: 28009360;RPRT 0"
                    cmdList[1].remove(0,10);
                    f=cmdList[1].toDouble(&ok);
                    if (ok) rigFreq=f;
                    break;
                case 1: // get_mode. Response looks like "get_mode:;Mode: CW;Passband: 600;RPRT 0"
                    cmdList[1].remove(0,5);
                    cmdList[1]=cmdList[1].simplified();
                    for (int j=0;j<nModeNames;j++) {
                        if (cmdList.at(1)==modeNames[j]) {
                            Mode=modes[j];
                            break;
                        }
                    }
                    break;
                case 2: // get IF center (K3). Response looks like "get_level: ifctr;8212940.000000\nRPRT 0"
                    cmdList[1].truncate(14);
                    iff=cmdList.at(1).toDouble(&ok);
                    if (ok) {
                        ifFreq_ = qRound(iff - 8210000.0);
                    }
                    break;
                }
            }
        }
    }
    lock.unlock();
}

/*! Slot called on error of tcpsocket of radio 1 (rigctld) */
void RigSerial::tcpError(QAbstractSocket::SocketError e)
{
    Q_UNUSED(e)
    radioOK=false;
    emit(radioError("ERROR: Rigctld radio "+QString::number(nrig+1)+" "+ socket->errorString().toLatin1()));
}


int RigSerial::band() const
{
    // in case caught in the middle of a qsy
    double f=qsyFreq;
    if (qAbs(f)>0.0) {
        return getBand(f);
    } else {
        return getBand(rigFreq);
    }
}

QString RigSerial::bandName()
{
    double f=qsyFreq;
    if (qAbs(f)>0) {
        return bandNames[getBand(f)];
    } else {
        return bandNames[getBand(rigFreq)];
    }
}
