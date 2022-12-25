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
#ifndef BANDMAPINTERFACE_H
#define BANDMAPINTERFACE_H

#include "../so2sdr-bandmap/bandmap-tcp.h"
#include "bandmapentry.h"
#include "defines.h"
#include <QAbstractSocket>
#include <QObject>
#include <QProcess>
#include <QSettings>
#include <QString>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QXmlStreamReader>

class BandmapInterface : public QObject {
  Q_OBJECT
public:
  explicit BandmapInterface(QSettings &s, QObject *parent = nullptr);
  ~BandmapInterface();
  void removeSpot(int nr, const BandmapEntry &spot);
  void removeSpotFreq(int nr, const BandmapEntry &spot);
  void addSpot(int nr, const BandmapEntry &spot);
  void connectTcp();
  int currentBand(int nr) const;
  bool bandmapon(int nr) const;
  void findFreq(int nr);
  void nextFreq(int nr, bool higher);
  void setInvert(int nr, bool b);
  void setFreqLimits(int nr, double flow, double fhigh);
  void setAddOffset(double f, int nr);
  void syncCalls(int nr, QList<BandmapEntry> &spotList);
  unsigned long winId(int nr) const { return winid[nr]; }

signals:
  void bandmap1state(bool);
  void bandmap2state(bool);
  void qsy1(double);
  void qsy2(double);
  void removeCall(double, int);
  void sendMsg(const QString &msg);

public slots:
  void bandmapSetFreq(double f, int nr);
  void closeBandmap(int nr);
  void showBandmap(int nr, bool state);
  void setBandmapTxStatus(bool b, int nr);

private slots:
  void launchBandmap1State(QProcess::ProcessState state);
  void launchBandmap2State(QProcess::ProcessState state);
  void udpRead();
  void socketError0(QAbstractSocket::SocketError);
  void socketError1(QAbstractSocket::SocketError);
  void showBandmapProcessError(int nr, QProcess::ProcessError);
  void launchShowBandmapProcessError1(QProcess::ProcessError);
  void launchShowBandmapProcessError2(QProcess::ProcessError);
  void launchTcpSocketStateChange1(QAbstractSocket::SocketState);
  void launchTcpSocketStateChange2(QAbstractSocket::SocketState);
  void tcpSocketStateChange(int nr, QAbstractSocket::SocketState);

private:
  bool bandmapOn[NRIG];
  bool bandmapAvailable[NRIG];
  QSettings &settings;
  QString ipAddress[NRIG];
  char cmd[NRIG];
  int cmdLen[NRIG];
  int port[NRIG];
  int band[NRIG];
  unsigned long winid[NRIG];
  QProcess bandmapProcess[NRIG];
  QTcpSocket socket[NRIG];
  QUdpSocket socketUdp;
  QXmlStreamReader xmlReader;

  void socketError(int nr, QAbstractSocket::SocketError err);
  void setBandmapState(int nr, QProcess::ProcessState state);
  void xmlParse();
};

#endif // BANDMAPINTERFACE_H
