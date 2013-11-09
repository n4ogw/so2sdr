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
#ifndef MICROHAM_H
#define MICROHAM_H

#include <QSettings>
#include <QObject>
#include "defines.h"
#include <qextserialport.h>

/*
 * Support for microHam Control Protocol enabled devices
 *
 * -Commands: FT1, FT2, FR1, FR2, and FRS commands
 * -does not receive any data from the device, only sends out commands
 * -connection settings fixed at 9600N81
 * -tested on MK2R/+ device (NO3M)
 */

class MicroHam : public QObject
{
Q_OBJECT

public:
    MicroHam(QSettings& s, QObject *parent = 0);
    ~MicroHam();
    bool MicroHamIsOpen() const;
    void switchAudio(int nr);
    void toggleStereo(int nr);
    void switchTransmit(int nr);
    bool stereoActive() const;

public slots:
    void openMicroHam();

private:
    bool       MicroHamOpen;
    bool       stereo;
    QextSerialPort *MicroHamPort;
    QSettings&  settings;
    void closeMicroHam();
};

#endif // MICROHAM_H
