/*! Copyright 2010-2023 R. Torsten Clay N4OGW

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
#ifndef CWDAEMON_H
#define CWDAEMON_H

#include <QByteArray>
#include <QSettings>
#include <QString>
#include <QUdpSocket>

#include "defines.h"

/*!
   cwdaemon support class
 */
class Cwdaemon : public QObject {
  Q_OBJECT

public:
  Cwdaemon(QSettings &s, QObject *parent = nullptr);
  ~Cwdaemon();
  void loadbuff(QByteArray msg);
  bool isSending() const;
  void sendcw();
  void switchTransmit(int nrig);
  void setSpeed(int speed);
  void setUdpPort(int nrig, int p);
  bool isOpen(int nrig) const;

signals:
  void cwCanceled();
  void finished();
  void textSent(const QString &t, int);
  void version(int ver);
  void tx(bool, int);
  void error(const QString &);

public slots:
  void cancelcw();
  void open();

private slots:
  void receive1();
  void receive2();

private:
  bool sending;
  bool open_[NRIG];
  bool sendingCmd;
  int rigNum;
  int txRig;
  int udpPort[NRIG];
  QByteArray sendBuff;
  QString sent;
  QSettings &settings;
  QUdpSocket socket[NRIG];
  void close();
};

#endif // CWDAEMON_H
