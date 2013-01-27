/*! Copyright 2010-2013 R. Torsten Clay N4OGW

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
#include <QSettings>
#include <QString>
#include <QTimerEvent>
#include <QByteArray>
#ifdef Q_OS_WIN
#include <windows.h>
#endif

// list of backends in hamlib riglist.h
static struct rig_backend_list {
    rig_model_t model;
    const char  *backend;
} rig_backend_list[] = RIG_BACKEND_LIST;


RigSerial::RigSerial(QSettings& s,QObject *parent) : QObject(parent),settings(s)
{
    cancelled = false;
}

/*! slot called to abort initializeHamlib
 */
void RigSerial::cancelHamlib()
{
    cancelled = true;
}

/*! queries hamlib to generate a list of all supported rigs

   dir should be directory with user data. Used to save/load hamlib model list
 */
bool RigSerial::initializeHamlib(QString dir)
{
    // turn off all the debug crud coming from hamlib
    rig_set_debug_level(RIG_DEBUG_NONE);

    if (!loadHamlibList(dir)) {
        mfgName.clear();
        mfg.clear();

        // wish hamlib had an faster way to query the possible rig numbers...
        // it is very slow on Windows
        RIG *rig;
        int n = 1;
        while (rig_backend_list[n].model != 0) {
            n++;
        }
        emit(maxBackends(n - 1));
        int cnt = 0;
        for (int i = 0; i < n; i++) {
            if (cancelled) {
                return(false);
            }

            // not sure what happens in hamlib when there are more than 100 of a given model...
            for (int j = 0; j < 100; j++) {
                int m = RIG_MAKE_MODEL(i, j);
                rig = 0;
                rig = rig_init(m);
                if (rig) {
                    int         indx = 0;
                    hamlibModel newrig;
                    newrig.model_name = rig->caps->model_name;
                    newrig.model_nr   = m;
                    if (!mfgName.contains(rig->caps->mfg_name)) {
                        mfgName.append(rig->caps->mfg_name);

                        hamlibmfg newmfg;
                        newmfg.mfg_name  = rig->caps->mfg_name;
                        newmfg.mfg_index = i;
                        newmfg.index     = cnt++;
                        newmfg.models.clear();
                        newmfg.models.append(newrig);
                        mfg.append(newmfg);
                    } else {
                        for (int j = 0; j < mfg.size(); j++) {
                            if (mfg.at(j).mfg_name == rig->caps->mfg_name) {
                                indx = j;
                                break;
                            }
                        }
                        mfg[indx].models.append(newrig);
                    }
                    rig_close(rig);
                    rig_cleanup(rig);
                }
            }
            emit(backendsDone(i));
        }
        saveHamlibList(dir);
    }
    return(true);
}

/*! save the list of manufacturers and rig models queried from hamlib

   dir is the directory to save hamlib.dat in
 */
void RigSerial::saveHamlibList(QString dir)
{
    QDir  directory;
    directory.setCurrent(dir);
    QFile file("hamlib.dat");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }
#ifdef Q_OS_WIN
    file.write("<" + QByteArray(so2sdr_hamlib_version) + ">\n");
#endif
#ifdef Q_OS_LINUX
    file.write("<" + QByteArray(hamlib_version) + ">\n");
#endif
    for (int i = 0; i < mfgName.size(); i++) {
        file.write("<" + mfgName.at(i) + ">\n");
    }
    file.write("#\n");
    for (int i = 0; i < mfg.size(); i++) {
        file.write(mfg.at(i).mfg_name + ";" + QByteArray::number(mfg.at(i).index) +
                   ";" + QByteArray::number(mfg.at(i).mfg_index) + "\n");
        for (int j = 0; j < mfg.at(i).models.size(); j++) {
            file.write(mfg.at(i).models.at(j).model_name + ";" +
                       QByteArray::number(mfg.at(i).models.at(j).model_nr) + "\n");
        }
        file.write("#\n");
    }
    file.close();
}

/*! load the list of manufacturers and rig models queried from hamlib

   dir is the directory containing hamlib.dat

   note: this data needs to match the internal data of the version of hamlib being used!
    Returns true if matches and load was successful
 */
bool RigSerial::loadHamlibList(QString dir)
{
    QDir  directory;
    directory.setCurrent(dir);
    QFile file("hamlib.dat");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return(false);
    }

    // check version
#ifdef Q_OS_WIN
    QByteArray hv(so2sdr_hamlib_version);
#endif
#ifdef Q_OS_LINUX
    QByteArray hv(hamlib_version);
#endif
    QByteArray temp = file.readLine(40);
    int        i1   = temp.indexOf(">");
    if (i1 != -1) {
        temp = temp.mid(1, i1 - 1);
    } else {
        return(false);
    }
    if (temp != hv || file.atEnd()) return(false);

    // list of manufacturer names
    mfgName.clear();
    do {
        if (file.atEnd()) return(false);
        temp = file.readLine(40);
        if (temp.startsWith('#')) break;
        int i1 = temp.indexOf(">");
        if (i1 != -1) {
            temp = temp.mid(1, i1 - 1);
        } else {
            return(false);
        }
        mfgName.append(temp);
    } while (true);

    // rig list
    mfg.clear();
    do {
        temp = file.readLine(40);
        hamlibmfg         x;
        QList<QByteArray> temp2 = temp.split(';');
        if (temp2.size() != 3) return(false);
        x.mfg_name = temp2.at(0);
        bool ok = true;
        x.index = temp2.at(1).toInt(&ok, 10);
        if (!ok) return(false);
        temp2[2]    = temp2[2].simplified();
        x.mfg_index = temp2.at(2).toInt(&ok, 10);
        if (!ok) return(false);
        mfg.append(x);
        do {
            temp = file.readLine(40);
            if (temp.startsWith('#')) break;
            QList<QByteArray> temp3 = temp.split(';');
            if (temp3.size() != 2) return(false);
            hamlibModel       z;
            z.model_name = temp3.at(0);
            temp3[1]     = temp3[1].simplified();
            z.model_nr   = temp3.at(1).toInt(&ok, 10);
            if (!ok) return(false);
            mfg.last().models.append(z);
        } while (true);
    } while (!file.atEnd());
    file.close();
    return(true);
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

/*! @todo Handle return codes from hamlib, emitting error code to main thread */
void RigSerial::initialize()
{
    for (int i = 0; i < NRIG; i++) {
        clearRitFlag[i] = 0;
        qsyFreq[i]      = 0;
        chgMode[i]      = RIG_MODE_NONE;
        radioOK[i]      = false;
        ifFreq_[i]      = 0;
        model[i]=settings.value(s_radios_rig[i],RIG_MODEL_DUMMY).toInt();
        rig[i] = rig_init(model[i]);
        token_t t = rig_token_lookup(rig[i], "rig_pathname");
        rig_set_conf(rig[i],t,settings.value(s_radios_port[i],defaultSerialPort[i]).toString().toAscii().data());
        // set baud rate
        rig[i]->state.rigport.parm.serial.rate=settings.value(s_radios_baud[i],s_radios_baud_def[i]).toInt();
    }

    // default starting freq/mode if communications to rigs
    // initially fails (or rig==RIG_MODEL_DUMMY)
    rigFreq[0] = 14000000;
    rigFreq[1] = 7000000;
    Mode[0]    = RIG_MODE_CW;
    Mode[1]    = RIG_MODE_CW;
}

RigSerial::~RigSerial()
{
    closeRig();
}


/*!
   start timers for each rig
 */
void RigSerial::run()
{
    timerId[0] = startTimer(settings.value(s_radios_poll[0],500).toInt());
    timerId[1] = startTimer(settings.value(s_radios_poll[1],500).toInt());
    timerId[2] = startTimer(RIG_SEND_TIMER);
    timerId[3] = startTimer(RIG_SEND_TIMER);
}

/*! stop timers
 */
void RigSerial::stopSerial()
{
    for (int i = 0; i < (NRIG + 2); i++) {
        killTimer(timerId[i]);
    }
}

/*!
   process timer events
 */
void RigSerial::timerEvent(QTimerEvent *event)
{
    for (int i = 0; i < NRIG; i++) {
        // timers for sending data to radios (change freq, RIT clear,...)
        // timerID[2] and timerId[3] are polled more frequently than the regular
        // radio poll timer
        if (radioOK[i] && event->timerId() == timerId[i + 2]) {
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

/*!
   clear RIT
 */
void RigSerial::clearRIT(int nrig)
{
    mutex[nrig].lock();
    clearRitFlag[nrig] = true;

    // rig_set_rit(rig[nrig],RIG_VFO_CURR,0);
    mutex[nrig].unlock();
}

/*! returns true if radio opened successfully
 */
bool RigSerial::radioOpen(int nrig)
{
    // mutex[nrig].lock();
    bool b = radioOK[nrig];

    // mutex[nrig].unlock();
    return(b);
}

/*! returns current radio frequency in Hz
 */
int RigSerial::getRigFreq(int nrig)
{
    // mutex[nrig].lock();
    int f = rigFreq[nrig];

    // mutex[nrig].unlock();
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

/*! initialize radio
 */
void RigSerial::openRig()
{
    for (int i = 0; i < NRIG; i++) {
        int r = rig_open(rig[i]);
        if (model[i] == RIG_MODEL_DUMMY) {
            rig_set_freq(rig[i], RIG_VFO_A, rigFreq[i]);
            rig_set_vfo(rig[i], RIG_VFO_A);
            rig_set_mode(rig[i], RIG_VFO_A, RIG_MODE_CW, RIG_PASSBAND_NORMAL);
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
            qDebug("radio %d not ok status=%d", i + 1, r);
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
    // not checking return code here
    rig_close(rig[0]);
    rig_close(rig[1]);
    radioOK[0] = false;
    radioOK[1] = false;
}
