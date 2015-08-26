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
#ifndef BANDMAPINTERFACE_H
#define BANDMAPINTERFACE_H

#include <QObject>
#include <QProcess>
#include <QSettings>
#include <QString>
#include <QAbstractSocket>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QXmlStreamReader>
#include "defines.h"
#include "../so2sdr-bandmap/bandmap-tcp.h"
#include "bandmapentry.h"

class BandmapInterface : public QObject
{
    Q_OBJECT
public:
    explicit BandmapInterface(QSettings &s,QObject *parent = 0);
    ~BandmapInterface();
    void removeSpot(int nr,const BandmapEntry &spot);
    void addSpot(int nr,const BandmapEntry &spot);
    void connectTcp();
    int currentBand(int nr) const;
    bool bandmapon(int nr) const;
    void findFreq(int nr);
    void nextFreq(int nr,bool higher);
    void setInvert(int nr,bool b);
    void setFreqLimits(int nr,int flow,int fhigh);
    void setAddOffset(int f, int nr);
    void syncCalls(int nr,QList<BandmapEntry> &spotList);

signals:
    void bandmap1state(bool);
    void bandmap2state(bool);
    void qsy(int,int);
    void removeCall(QByteArray,int);
    void sendMsg(const QString &msg);

public slots:
    void bandmapSetFreq(int f,int nr);
    void closeBandmap(int nr);
    void showBandmap(int nr,int checkboxState);
    void setBandmapTxStatus(bool b, int nr);

private slots:
    void launchBandmap1State(QProcess::ProcessState state);
    void launchBandmap2State(QProcess::ProcessState state);
    void udpRead();
    void socketError0(QAbstractSocket::SocketError);
    void socketError1(QAbstractSocket::SocketError);
    void showBandmapProcessError(int nr,QProcess::ProcessError);
    void launchShowBandmapProcessError1(QProcess::ProcessError);
    void launchShowBandmapProcessError2(QProcess::ProcessError);
    void launchTcpSocketStateChange1(QAbstractSocket::SocketState);
    void launchTcpSocketStateChange2(QAbstractSocket::SocketState);
    void tcpSocketStateChange(int nr,QAbstractSocket::SocketState);

private:
    bool                 bandmapOn[NRIG];
    bool                 bandmapAvailable[NRIG];
    QSettings            &settings;
    QString              ipAddress[NRIG];
    char                 cmd[NRIG];
    int                  cmdLen[NRIG];
    int                  port[NRIG];
    int                  band[NRIG];
    QProcess             bandmapProcess[NRIG];
    QTcpSocket           socket[NRIG];
    QUdpSocket           socketUdp;
    QXmlStreamReader     xmlReader;

    void socketError(int nr,QAbstractSocket::SocketError err);
    void setBandmapState(int nr, QProcess::ProcessState state);
    void xmlParse();
};

#endif // BANDMAPINTERFACE_H
