/*! Copyright 2010-2016 R. Torsten Clay N4OGW

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
#include <QDir>
#include <QFile>
#include <QHostAddress>
#include <QSettings>
#include <QString>
#include <QTcpSocket>
#include <QThread>
#include <QTimerEvent>
#include <QByteArray>
#ifdef Q_OS_WIN
#include <windows.h>
#endif

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


RigSerial::RigSerial(QString s)
{
    settingsFile=s;

    // turn off all the debug crud coming from hamlib
    rig_set_debug_level(RIG_DEBUG_NONE);

    // load all backends and step through them. list_caps function defined below.
    rig_load_all_backends();
    rig_list_foreach(list_caps,NULL);

    // sort list by manufacturer name
    qSort(mfg.begin(),mfg.end());
    qSort(mfgName.begin(),mfgName.end());

    // sort list of rigs for each manuacturer
    for (int i=0;i<mfg.size();i++) {
        qSort(mfg[i].models.begin(),mfg[i].models.end());
    }
    settings=0;
    for (int i=0;i<nRigSerialTimers;i++) timerId[i]=0;
    for (int i = 0; i < NRIG; i++) {
        rig[i]=0;
        socket[i]=0;
        pttOffFlag[i]=    false;
        pttOnFlag[i]=     false;
        clearRitFlag[i] = 0;
        qsyFreq[i]      = 0;
        chgMode[i]      = RIG_MODE_NONE;
        radioOK[i]      = false;
        ifFreq_[i]      = 0;
        passBW[i]       = 0;
        Mode[i]=RIG_MODE_NONE;
        rigFreq[i]=0;
        model[i]=1;
    }
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
    rig_cleanup(rig[0]);
    rig_cleanup(rig[1]);
    if (socket[0]) delete socket[0];
    if (socket[1]) delete socket[1];
    delete settings;
}


/*!
   Called when thread started
 */
void RigSerial::run()
{
    if (!settings) settings=new QSettings(settingsFile,QSettings::IniFormat);
    openRig();
    openSocket();
    timerId[0] = startTimer(settings->value(s_radios_poll[0],500).toInt());
    timerId[1] = startTimer(settings->value(s_radios_poll[1],500).toInt());
    timerId[2] = startTimer(RIG_SEND_TIMER);
    timerId[3] = startTimer(RIG_SEND_TIMER);

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
void RigSerial::setPtt(int nrig, int state)
{
    mutex[nrig].lock();
    if (state) {
        pttOnFlag[nrig]=true;
    } else {
        pttOffFlag[nrig]=true;
    }
    mutex[nrig].unlock();
}

/*!
   process timer events
 */
void RigSerial::timerEvent(QTimerEvent *event)
{
    for (int i = 0; i < NRIG; i++) {

        // using rigctld for this radio

        if (settings->value(s_radios_rigctld_enable[i],s_radios_rigctld_enable_def[i]).toBool()) {

            if (socket[i]) {
                if (!socket[i]->isOpen()) continue;
            }

            if (event->timerId() == timerId[i]) {
                socket[i]->write(";\\get_freq\n");
                socket[i]->write(";\\get_mode\n");
                if (model[i]==229) {
                    socket[i]->write(";\\get_level ifctr\n");
                }
            }

            if (event->timerId() == timerId[i + 2]) {
                mutex[i].lock();

                // clear RIT if flag set
                // behavior of hamlib over serial and via rigctld is different. At least with hamlib/rigctld
                // v 3.0.1, set_rit 0 does not turn off RIT like docs say.
                if (clearRitFlag[i]) {
                    socket[i]->write(";\\set_rit 0\n");
                    clearRitFlag[i] = false;
                }

                if (pttOnFlag[i]) {
                    socket[i]->write(";\\set_ptt 1\n");
                    pttOnFlag[i]=false;
                }

                if (pttOffFlag[i]) {
                    socket[i]->write(";\\set_ptt 0\n");
                    pttOffFlag[i]=false;
                }

                if (qsyFreq[i]) {
                    QByteArray cmd=";\\set_freq "+QByteArray::number(qsyFreq[i])+"\n";
                    socket[i]->write(cmd);
                    rigFreq[i] = qsyFreq[i];
                    qsyFreq[i] = 0;
                }
                if ((chgMode[i] > RIG_MODE_NONE) && (chgMode[i] < RIG_MODE_TESTS_MAX)) {
                    QByteArray width=QByteArray::number((int)passBW[i])+"\n";
                    switch (chgMode[i]) {
                    case RIG_MODE_CW:
                        socket[i]->write(";\\set_mode CW "+width);
                        Mode[i]=RIG_MODE_CW;
                        break;
                    case RIG_MODE_CWR:
                        socket[i]->write(";\\set_mode CWR "+width);
                        Mode[i]=RIG_MODE_CWR;
                        break;
                    case RIG_MODE_USB:
                        socket[i]->write(";\\set_mode USB "+width);
                        Mode[i]=RIG_MODE_USB;
                        break;
                    case RIG_MODE_LSB:
                        socket[i]->write(";\\set_mode LSB "+width);
                        Mode[i]=RIG_MODE_LSB;
                        break;
                    case RIG_MODE_RTTY:
                        socket[i]->write(";\\set_mode RTTY "+width);
                        Mode[i]=RIG_MODE_RTTY;
                        break;
                    case RIG_MODE_RTTYR:
                        socket[i]->write(";\\set_mode RTTYR "+width);
                        Mode[i]=RIG_MODE_RTTYR;
                        break;
                    case RIG_MODE_FM:
                        socket[i]->write(";\\set_mode FM "+width);
                        Mode[i]=RIG_MODE_FM;
                        break;
                    case RIG_MODE_AM:
                        socket[i]->write(";\\set_mode AM "+width);
                        Mode[i]=RIG_MODE_AM;
                        break;
                    case RIG_MODE_AMS:
                        socket[i]->write(";\\set_mode AMS "+width);
                        Mode[i]=RIG_MODE_AMS;
                        break;
                    case RIG_MODE_DSB:
                        socket[i]->write(";\\set_mode DSB "+width);
                        Mode[i]=RIG_MODE_DSB;
                        break;
                    default:
                        break;
                    }
                    chgMode[i]=RIG_MODE_NONE;
                }
                mutex[i].unlock();
            }
        } else {
            // using hamlib over serial port

            // timers for sending data to radios (change freq, RIT clear,...)
            // timerID[2] and timerId[3] are polled more frequently than the regular
            // radio poll timer
            if (radioOK[i] && event->timerId() == timerId[i + 2]) {
                // set PTT on
                mutex[i].lock();
                if (pttOnFlag[i]) {
                    rig_set_ptt(rig[i],RIG_VFO_CURR,RIG_PTT_ON);
                    pttOnFlag[i]=false;
                }
                mutex[i].unlock();

                // set PTT off
                mutex[i].lock();
                if (pttOffFlag[i]) {
                    rig_set_ptt(rig[i],RIG_VFO_CURR,RIG_PTT_OFF);
                    pttOffFlag[i]=false;
                }
                mutex[i].unlock();

                // clear RIT if flag set
                mutex[i].lock();
                if (clearRitFlag[i]) {
                    if (confParamsRIT[i]) {
                        value_t val;
                        val.i = 0;
                        rig_set_ext_level(rig[i], RIG_VFO_CURR, confParamsRIT[i]->token, val);
                    } else {
                        // not preferred, as this turns RIT off completely
                        rig_set_rit(rig[i], RIG_VFO_CURR, 0);
                    }
                    clearRitFlag[i] = false;
                }
                mutex[i].unlock();

                // qsy radio
                mutex[i].lock();
                if (qsyFreq[i]) {
                    int status = rig_set_freq(rig[i], RIG_VFO_CURR, qsyFreq[i]);
                    if (status == RIG_OK) {
                        rigFreq[i] = qsyFreq[i];
                    }
                    qsyFreq[i] = 0;
                }

                mutex[i].unlock();

                // change mode on radio
                mutex[i].lock();
                if ((chgMode[i] > RIG_MODE_NONE) && (chgMode[i] < RIG_MODE_TESTS_MAX)) {
                    int status = rig_set_mode(rig[i], RIG_VFO_CURR, chgMode[i], passBW[i]);
                    if (status == RIG_OK) {
                        Mode[i] = chgMode[i];
                    }
                    chgMode[i] = RIG_MODE_NONE; // reset for next change
                }

                mutex[i].unlock();
            }

            // poll frequency, mode, and IF freq from radio
            if (radioOK[i] && event->timerId() == timerId[i]) {
                freq_t freq;

                int    status = rig_get_freq(rig[i], RIG_VFO_CURR, &freq);
                if (status == RIG_OK) {
                    int ff = Hz(freq);
                    if (ff != 0) rigFreq[i] = ff;
                }
                rmode_t   m;
                pbwidth_t width;
                status = rig_get_mode(rig[i], RIG_VFO_CURR, &m, &width);
                if (status == RIG_OK) {
                    Mode[i] = m;
                }

                // if using K3, get IF center frequency
                if (model[i] == 229) {
                    value_t val;
                    int     status = rig_get_ext_level(rig[i], RIG_VFO_CURR, confParamsIF[i]->token, &val);
                    if (status == RIG_OK) {
                        mutex[i].lock();
                        ifFreq_[i] = (int) (val.f - 8210000.0);
                        mutex[i].unlock();
                    }
                }
            }
        }
    }
}

/*!
   clear RIT
 */
void RigSerial::clearRIT(int nrig)
{
    mutex[nrig].lock();
    clearRitFlag[nrig] = true;

    mutex[nrig].unlock();
}

/*! returns true if radio opened successfully
 */
bool RigSerial::radioOpen(int nrig)
{
    mutex[nrig].lock();
    bool b = radioOK[nrig];

    mutex[nrig].unlock();
    return(b);
}

/*! returns current radio frequency in Hz
 */
int RigSerial::getRigFreq(int nrig)
{
    mutex[nrig].lock();
    int f = rigFreq[nrig];

    mutex[nrig].unlock();
    return(f);
}

/*! returns current radio IF frequency (K3 only, others return 0)
 */
int RigSerial::ifFreq(int nrig)
{
    mutex[nrig].lock();
    int f = ifFreq_[nrig];
    mutex[nrig].unlock();
    return(f);
}

/*! returns current radio mode

   rmode_t defined in hamlib
 */
rmode_t RigSerial::mode(int nrig) const
{
    return Mode[nrig];
}

/*! return string describing the current mode for
 radio nrig
 */
QString RigSerial::modeStr(int nrig) const
{
    switch (Mode[nrig]) {
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

/*! convert mode to ModeType
*/
ModeTypes RigSerial::getModeType(rmode_t mode) const
{
    switch (mode) {
    case RIG_MODE_NONE: return CWType;
    case RIG_MODE_AM: return PhoneType;
    case RIG_MODE_CW: return CWType;
    case RIG_MODE_USB: return PhoneType;
    case RIG_MODE_LSB: return PhoneType;
    case RIG_MODE_RTTY: return DigiType;
    case RIG_MODE_FM: return PhoneType;
    case RIG_MODE_WFM: return PhoneType;
    case RIG_MODE_CWR: return CWType;
    case RIG_MODE_RTTYR: return DigiType;
    case RIG_MODE_AMS: return PhoneType;
    case RIG_MODE_PKTLSB: return DigiType;
    case RIG_MODE_PKTUSB: return DigiType;
    case RIG_MODE_PKTFM: return DigiType;
    case RIG_MODE_ECSSUSB: return PhoneType;
    case RIG_MODE_ECSSLSB: return PhoneType;
    case RIG_MODE_FAX: return DigiType;
    case RIG_MODE_SAM: return PhoneType;
    case RIG_MODE_SAL: return PhoneType;
    case RIG_MODE_SAH: return PhoneType;
    case RIG_MODE_DSB: return PhoneType;
    default:return CWType;
    }
}


/*! return current type of mode: CW, PHONE, DIGI
 */
ModeTypes RigSerial::modeType(int nrig) const
{
    return getModeType(Mode[nrig]);
}

/*! initialize TcpSocket for rigctld
 */
void RigSerial::openSocket()
{
    for (int i=0;i<NRIG;i++) {
        if (socket[i]) {
            if (socket[i]->isOpen()) socket[i]->close();
            disconnect(socket[i],0,0,0);
        }
        if (settings->value(s_radios_rigctld_enable[i],s_radios_rigctld_enable_def[i]).toBool()) {
            radioOK[i]=true;
            if (!socket[i]) socket[i]=new QTcpSocket();

            // QHostAddress doesn't understand "localhost"
            if (settings->value(s_radios_rigctld_ip[i],s_radios_rigctld_ip_def[i]).toString().simplified()=="localhost") {
                socket[i]->connectToHost(QHostAddress::LocalHost,
                                         settings->value(s_radios_rigctld_port[i],s_radios_rigctld_port_def[i]).toInt());
            } else {
                socket[i]->connectToHost(QHostAddress(settings->value(s_radios_rigctld_ip[i],s_radios_rigctld_ip_def[i]).toString()).toString(),
                                         settings->value(s_radios_rigctld_port[i],s_radios_rigctld_port_def[i]).toInt());
            }

            switch (i) {
            case 0:
                connect(socket[0],SIGNAL(readyRead()),this,SLOT(rxSocket1()));
                connect(socket[0],SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(tcpError1(QAbstractSocket::SocketError)));
                break;
            case 1:
                connect(socket[1],SIGNAL(readyRead()),this,SLOT(rxSocket2()));
                connect(socket[1],SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(tcpError2(QAbstractSocket::SocketError)));
                break;
            }
        }
    }
}

/*! initialize radio
 *
 * this should run in the new thread
 */
void RigSerial::openRig()
{
    for (int i = 0; i < NRIG; i++) {
        pttOffFlag[i]=    false;
        pttOnFlag[i]=     false;
        clearRitFlag[i] = 0;
        qsyFreq[i]      = 0;
        chgMode[i]      = RIG_MODE_NONE;
        radioOK[i]      = false;
        ifFreq_[i]      = 0;
        model[i]=settings->value(s_radios_rig[i],RIG_MODEL_DUMMY).toInt();
        if (rig[i]) {
            rig_close(rig[i]);
            rig_cleanup(rig[i]);
        }
        rig[i] = rig_init(model[i]);
        token_t t = rig_token_lookup(rig[i], "rig_pathname");
        rig_set_conf(rig[i],t,settings->value(s_radios_port[i],defaultSerialPort[i]).toString().toLatin1().data());
        rig[i]->state.rigport.parm.serial.rate=settings->value(s_radios_baud[i],s_radios_baud_def[i]).toInt();
    }

    // default starting freq/mode if communications to rigs
    // initially fails (or rig==RIG_MODEL_DUMMY)
    if (rigFreq[0]==0) rigFreq[0] = 14000000;
    if (rigFreq[1]==0) rigFreq[1] = 7000000;
    if (Mode[0]==RIG_MODE_NONE)   Mode[0] = RIG_MODE_CW;
    if (Mode[1]==RIG_MODE_NONE)   Mode[1] = RIG_MODE_CW;

    for (int i = 0; i < NRIG; i++) {
        int r = rig_open(rig[i]);
        if (model[i] == RIG_MODEL_DUMMY) {
            rig_set_freq(rig[i], RIG_VFO_A, rigFreq[i]);
            rig_set_vfo(rig[i], RIG_VFO_A);
            rig_set_mode(rig[i], RIG_VFO_A, Mode[i], RIG_PASSBAND_NORMAL);
        }
        if (r == RIG_OK) {
            radioOK[i] = true;

            // get ext params struct for K3 in order to get IF center freq
            if (model[i] == 229) {
                confParamsIF[i] = rig_ext_lookup(rig[i], "ifctr");
            }

            // ext param for RIT clear
            confParamsRIT[i] = rig_ext_lookup(rig[i], "ritclr");
        } else {
            emit(radioError("ERROR: radio "+QString::number(i+1)+" could not be opened"));
            radioOK[i] = false;
            rig_close(rig[i]);
        }
    }
}

/*! qsy radio
   f=freq in Hz (no checking done)
 */
void RigSerial::qsyExact(int nrig, int f)
{
    mutex[nrig].lock();
    qsyFreq[nrig] = f;
    mutex[nrig].unlock();
}

/*! Set radio mode
 *
 * m = Hamlib mode, no checking is done
 * pb = Passband width in Hz
 */
void RigSerial::setRigMode(int nrig, rmode_t m, pbwidth_t pb)
{
    mutex[nrig].lock();
    chgMode[nrig] = m;
    passBW[nrig] = pb;
    mutex[nrig].unlock();
}

/*! shut down radio interface
 */
void RigSerial::closeRig()
{
    rig_close(rig[0]);
    rig_close(rig[1]);
    radioOK[0] = false;
    radioOK[1] = false;
}

/*! send a raw byte string to the radio: careful, there is no checking here!
 */
void RigSerial::sendRaw(int nrig, QByteArray cmd)
{
    if (!cmd.contains('<')) {
        write_block(&rig[nrig]->state.rigport,cmd.data(),cmd.size());
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
        write_block(&rig[nrig]->state.rigport,data.data(),data.size());
    }
}

/*! slot called from readyRead of tcp socket for rigctld radio 1
 */
void RigSerial::rxSocket1()
{
    rxSocket(0);
}

/*! slot called from readyRead of tcp socket for rigctld radio 2
 */
void RigSerial::rxSocket2()
{
    rxSocket(1);
}

/*!
 * \brief RigSerial::rxSocket
 * \param nrig
 *
 * Parse data coming from rigctld
 */
void RigSerial::rxSocket(int nrig)
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

    radioOK[nrig]=true;
    mutex[nrig].lock();
    while (socket[nrig]->bytesAvailable()) {
        QByteArray data=socket[nrig]->readAll();
        QList<QByteArray> cmdList=data.split(';');
        bool ok;
        int f;
        double iff;
        for (int i = 0; i < nCmdNames; i++) {
            if (cmdList.at(0)==cmdNames[i]) {
                switch (i) {
                case 0: // get_freq. Response looks like "get_freq:;Frequency: 28009360;RPRT 0"
                    cmdList[1].remove(0,10);
                    f=cmdList[1].toInt(&ok);
                    if (ok) rigFreq[nrig]=f;
                    break;
                case 1: // get_mode. Response looks like "get_mode:;Mode: CW;Passband: 600;RPRT 0"
                    cmdList[1].remove(0,5);
                    cmdList[1]=cmdList[1].simplified();
                    for (int j=0;j<nModeNames;j++) {
                        if (cmdList.at(1)==modeNames[j]) {
                            Mode[nrig]=modes[j];
                            break;
                        }
                    }
                    break;
                case 2: // get IF center (K3). Response looks like "get_level: ifctr;8212940.000000\nRPRT 0"
                    cmdList[1].truncate(14);
                    iff=cmdList.at(1).toDouble(&ok);
                    if (ok) {
                        ifFreq_[nrig] = (int) (iff - 8210000.0);
                    }
                    break;
                }
            }
        }
    }
    mutex[nrig].unlock();
}

/*! Slot called on error of tcpsocket of radio 1 (rigctld) */
void RigSerial::tcpError1(QAbstractSocket::SocketError e)
{
    radioOK[0]=false;
    emit(radioError("ERROR: Rigctld radio 1: "+ socket[0]->errorString().toLatin1()));
}

/*! Slot called on error of tcpsocket of radio 2 (rigctld) */
void RigSerial::tcpError2(QAbstractSocket::SocketError e)
{
    radioOK[1]=false;
    emit(radioError("ERROR: Rigctld radio 2: "+ socket[1]->errorString().toLatin1()));
}

