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
#ifndef WINKEY_H
#define WINKEY_H
#include <QByteArray>
#include <QSettings>
#include <QString>
#include "defines.h"
#include <qextserialport.h>

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
    void setSpeed(int speed);
    bool winkeyIsOpen() const;

/*! @todo make this private; can connect signals instead of referring directly to winkeyPort
 */
    QextSerialPort *winkeyPort;

signals:
    void version(int ver);
    void winkeyTx(bool, int);

public slots:
    void cancelcw();
    void openWinkey();

private slots:
    void receive();

private:
    bool       sending;
    bool       winkeyOpen;
    int        nchar;
    int        rigNum;
    int        winkeySpeedPot;
    int        winkeyVersion;
    QByteArray sendBuff;
    QSettings& settings;
    void closeWinkey();
};

#endif // WINKEY_H
