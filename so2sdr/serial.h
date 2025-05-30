/*! Copyright 2010-2025 R. Torsten Clay N4OGW

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
#include "defines.h"
#include "utils.h"
#include <QAbstractSocket>
#include <QByteArray>
#include <QList>
#include <QMutex>
#include <QObject>
#include <QReadWriteLock>
#include <QString>
#include <QTimer>
#include <QTimerEvent>

// using C rather than C++ bindings for hamlib because I don't
// want to have to deal with exceptions
#include <hamlib/rig.h>
#include <hamlib/riglist.h>

// how often to check for commands to be sent to radios
// Note: setting this to zero will cause 100% CPU when using hamlib dummy rig
const int RIG_SEND_TIMER = 2;

class hamlibModel {
public:
  QByteArray model_name;
  int model_nr;
  bool operator<(const hamlibModel &other) const;
};

class hamlibmfg {
public:
  QByteArray mfg_name;
  QList<hamlibModel> models;
  bool operator<(const hamlibmfg &other) const;
};

class QSettings;
class QTcpSocket;

/*!
   Radio serial communications for both radios using Hamlib library.

   note that this class will run in its own QThread
 */
class RigSerial : public QObject {
  Q_OBJECT

public:
  RigSerial(int, QString);
  ~RigSerial();
  int band() const;
  QString bandName();
  void clearRIT();
  double getRigFreq();
  QString hamlibModelName(int i, int indx) const;
  int hamlibNMfg() const;
  int hamlibNModels(int i) const;
  int hamlibModelIndex(int, int) const;
  void hamlibModelLookup(int, int &, int &) const;
  QString hamlibMfgName(int i) const;
  int ifFreq();
  rmode_t mode();
  QString modeStr();
  ModeTypes modeType();
  bool radioOpen();
  void sendRaw(QByteArray cmd);

  static QList<hamlibmfg> mfg;
  static QList<QByteArray> mfgName;
signals:
  void radioError(const QString &);

public slots:
  void setPtt(int state);
  void qsyExact(double f);
  void setRigMode(rmode_t m);
  void run();
  void stopSerial();

protected:
  void timerEvent(QTimerEvent *event);

private slots:
  void rxSocket();
  void tcpError(QAbstractSocket::SocketError e);

private:
  static int list_caps(const struct rig_caps *caps, void *data);

  // number of timers
  const static int nRigSerialTimers = 2;

  void closeRig();
  void openRig();
  void openSocket();

  bool clearRitFlag;
  bool pttOnFlag;
  bool pttOffFlag;
  const struct confparams *confParamsIF;
  bool hasIFext;
  const struct confparams *confParamsRIT;
  double qsyFreq;
  rmode_t chgMode;
  pbwidth_t passBW;
  bool radioOK;
  double rigFreq;
  int model;
  int ifOffset;
  int nrig;
  rmode_t Mode;
  QMutex pttMutex;
  QReadWriteLock lock;
  RIG *rig;
  int timerId[nRigSerialTimers];
  QSettings *settings;
  QString settingsFile;
  QTcpSocket *socket;
};

#endif
