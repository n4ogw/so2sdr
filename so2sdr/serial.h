/*! Copyright 2010-2015 R. Torsten Clay N4OGW

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
#ifndef SERIAL_H
#define SERIAL_H
#include <QByteArray>
#include <QFile>
#include <QString>
#include <QList>
#include <QMap>
#include <QObject>
#include <QSettings>
#include <QTimer>
#include <QTimerEvent>
#include <QMutex>
#include "defines.h"

// using C rather than C++ bindings for hamlib because I don't
// want to have to deal with exceptions
#include <hamlib/rig.h>
#include <hamlib/riglist.h>

// how often to check for commands to be sent to radios
const int RIG_SEND_TIMER=20;

class hamlibModel
{
public:
    QByteArray model_name;
    int        model_nr;
    bool operator<(const hamlibModel &other) const;
};

class hamlibmfg
{
public:
    QByteArray         mfg_name;
    QList<hamlibModel> models;
    bool operator<(const hamlibmfg &other) const;
};

// this has to match the modes defined in hamlib rig.h, enum rmode_t
const int nModes=21;
const QString modes[nModes] = { "NONE", "AM",  "CW",  "USB", "LSB", "RTTY", "FM",  "WFM", "CWR", "RTTYR", "AMS",
                            "PKT",  "PKT", "PKT", "USB", "LSB", "FAX",  "SAM", "SAL", "SAH", "DSB" };

/*!
   Radio serial communications for one radio using Hamlib library.

   note that this class will reside in its own QThread
 */
class RigSerial : public QObject
{
Q_OBJECT

public:
    RigSerial(QSettings& s,QObject *parent = 0);
    ~RigSerial();
    void clearRIT(int nrig);
    ModeTypes getModeType(rmode_t mode) const;
    int getRigFreq(int nrig);
    QString hamlibModelName(int i, int indx) const;
    int hamlibNMfg() const;
    int hamlibNModels(int i) const;
    int hamlibModelIndex(int, int) const;
    void hamlibModelLookup(int, int&, int&) const;
    QString hamlibMfgName(int i) const;
    int ifFreq(int nrig);
    void initialize();
    rmode_t mode(int nrig) const;
    QString modeStr(int nrig) const;
    ModeTypes modeType(int nrig) const;
    void openRig();
    bool radioOpen(int nrig);
    void sendRaw(int nrig,QByteArray cmd);
    void stopSerial();

    static QList<hamlibmfg>        mfg;
    static QList<QByteArray>       mfgName;
signals:
    void radioError(const QString &);

public slots:
    void setPtt(int nrig,int state);
    void qsyExact(int nrig, int f);
    void setRigMode(int nrig, rmode_t m, pbwidth_t pb);
    void run();

protected:
    void timerEvent(QTimerEvent *event);

private:
    static int list_caps(const struct rig_caps *caps, void *data);

    void closeRig();
    bool                    clearRitFlag[NRIG];
    bool                    pttOnFlag[NRIG];
    bool                    pttOffFlag[NRIG];
    const struct confparams *confParamsIF[NRIG];
    const struct confparams *confParamsRIT[NRIG];
    int                     qsyFreq[NRIG];
    rmode_t                 chgMode[NRIG];
    pbwidth_t               passBW[NRIG];
    bool                    radioOK[NRIG];
    int                     rigFreq[NRIG];
    bool                    ritClear[NRIG];
    int                     model[NRIG];
    int                     ifFreq_[NRIG];
    rmode_t                 Mode[NRIG];
    QMutex                  mutex[NRIG];
    QMutex                  qsyMutex;
    RIG                     *rig[NRIG];
    int                     timerId[NRIG*2];
    QSettings&              settings;
};

#endif
