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
#ifndef WINKEY_H
#define WINKEY_H
#include <QByteArray>
#include <QSettings>
#include <QString>
#include "defines.h"
#include <QSerialPort>

/*!
   Winkey support class
 */
class Winkey : public QObject
{
Q_OBJECT

public:
    Winkey(QSettings& s, QObject *parent = 0);
    ~Winkey();
    void loadbuff(QByteArray msg);
    bool isSending() const;
    void sendcw();
    void switchTransmit(int nrig);
    void setEchoMode(bool b);
    void setSpeed(int speed);
    bool winkeyIsOpen() const;

signals:
    void cwCanceled();
    void textSent(const QString& t,int);
    void version(int ver);
    void winkeyTx(bool, int);
    void winkeyError(const QString &);

public slots:
    void cancelcw();
    void openWinkey();

private slots:
    void receive();
    void receiveInit();

private:
    QSerialPort *winkeyPort;
    bool       echoMode;
    bool       ignoreEcho;
    bool       sending;
    bool       winkeyOpen;
    int        nchar;
    int        rigNum;
    int        txPtr;
    int        winkeySpeedPot;
    int        winkeyVersion;
    QByteArray sendBuff;
    QString sent;
    QSettings& settings;
    void closeWinkey();
    void openWinkey2();
    void processEcho(unsigned char byte);
};

#endif // WINKEY_H
