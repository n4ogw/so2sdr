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
#ifndef AFEDRI_H
#define AFEDRI_H

#include <QAbstractSocket>
#include <QObject>
#include <QString>
#include "network.h"
#include "sdr-ip.h"
#include "defines.h"
#include "spectrum.h"

#define TARGET_NAME "AFEDRI SDR Network"
#define SERIAL_NUMBER "AN000102"
#define IF_VERSION      0x101
#define BOOT_CODE_VER   0x000
#define APP_FW_VER      0x112
#define HW_VER          0x000
#define PRODUCT_ID 0x03524453L

#define MAX_UDP_SIZE 1044

class Afedri : public NetworkSDR
{
    Q_OBJECT
public:
    Afedri(QString settingsFile,QObject *parent = 0);
    ~Afedri();

public slots:
    void stop();
    void initialize();

private slots:
    void readDatagram();
    void readTcp();

private:
    void set_broadcast_flag(bool);
    void set_freq(unsigned long frequency, int channel);
    void set_multichannel_mode(int channel);
    void set_sample_rate(unsigned long sample_rate);
};

#endif // AFEDRI_H
