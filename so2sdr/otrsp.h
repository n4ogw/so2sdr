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
#ifndef OTRSP_H
#define OTRSP_H
#include <QSettings>
#include "defines.h"
#include <qextserialport.h>

/*
 * Support for OTRSP (Open Two Radio Switching Protocol) devices
 *
 * -limited support at present: only uses RX1, RX2, TX1, TX2, and RX1S commands
 * -does not receive any data from the device, only sends out commands
 * -connection settings fixed at 9600N81
 * -tested on SO2RDUINO device (N4OGW
 */

class OTRSP : public QObject
{
Q_OBJECT

public:
    OTRSP(QSettings& s, QObject *parent = 0);
    ~OTRSP();
    bool OTRSPIsOpen() const;
    void switchAudio(int nr);
    void toggleStereo(int nr); 
    void switchTransmit(int nr);
    bool stereoActive() const;

public slots:
    void openOTRSP();

private:
    bool       OTRSPOpen;
    bool       stereo;
    QextSerialPort *OTRSPPort;
    QSettings&  settings;
    void closeOTRSP();
};

#endif // OTRSP_H
