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
#include <QDebug>
#include <QSettings>
#include "defines.h"
#include "otrsp.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

OTRSP::OTRSP(QSettings *s, QObject *parent) : QObject(parent)
{
    // load from settings file
    settings=s;
    OTRSPPort     = new QextSerialPort(settings->value(s_otrsp_device,s_otrsp_device_def).toString(), QextSerialPort::EventDriven);
    deviceName.clear();
    OTRSPOpen     = false;
    stereo        = false;
}

OTRSP::~OTRSP()
{
    closeOTRSP();
    delete OTRSPPort;
}

/*!
   returns true if OTRSP has been opened successfully
 */
bool OTRSP::OTRSPIsOpen() const
{
    return(OTRSPOpen);
}

/*!
   Slot triggered when data is available at port

   nothing implemented yet- only send data to port
 */
void OTRSP::receive()
{
    // unsigned char wkbyte;
    //int           n = OTRSPPort->read((char *) &wkbyte, 1);
}


/*!
 * \brief OTRSP::switchRadios
 *switches both RX and TX focus to other radio
 */
void OTRSP::switchRadios(int nr)
{
    if (!OTRSPOpen || nr<0 || nr>1) return;

    char radnr[2]={'1','2'};
    const char rx[3]="RX";
    const char tx[3]="TX";
    const char cr=0x0d;
    OTRSPPort->write(rx,2);
    OTRSPPort->write(&radnr[nr],1);
    OTRSPPort->write(&cr,1);

    OTRSPPort->write(tx,2);
    OTRSPPort->write(&radnr[nr],1);
    OTRSPPort->write(&cr,1);
}

/*!
 * \brief OTRSP::toggleStereo
 * \param nr the active radio
 * Toggles stereo receive mode. OTRSP needs parameter nr because it
 * can't remember how to get out of stereo split mode.
 */
void OTRSP::toggleStereo(int nr)
{
    if (!OTRSPOpen) return;
    if (stereo) {
        switchRadios(nr);
        stereo=false;
    } else {
        const char cmd[5]="RX1S";
        const char cr=0x0d;
        OTRSPPort->write(cmd,5);
        OTRSPPort->write(&cr,1);
        stereo=true;
    }
}

/*!
   open OTRSP device
 */
void OTRSP::openOTRSP()
{
    // in case we are re-starting OTRSP
    if (OTRSPPort->isOpen()) {
        closeOTRSP();
        OTRSPOpen = false;
    }
    OTRSPPort->setPortName(settings->value(s_otrsp_device,s_otrsp_device_def).toString());

    // currently only 9600N81
    OTRSPPort->setBaudRate(BAUD9600);
    OTRSPPort->setFlowControl(FLOW_OFF);
    OTRSPPort->setParity(PAR_NONE);
    OTRSPPort->setDataBits(DATA_8);
    OTRSPPort->setStopBits(STOP_1);

    OTRSPPort->setTimeout(500);
    OTRSPPort->open(QIODevice::ReadWrite);
    OTRSPPort->setRts(0);
    OTRSPPort->setDtr(0);
    OTRSPPort->flush();

    // have to wait a while at various places
#ifdef Q_OS_LINUX
    usleep(100000);
#endif
#ifdef Q_OS_WIN
    Sleep(100);
#endif

    if (!OTRSPPort->isOpen()) {
        OTRSPOpen = false;
        return;
    }
/*
    // get device name
    char buff[64]="?NAME";
    const char cr=0x0d;
    OTRSPPort->write(buff, 5);
    OTRSPPort->write(&cr,1);
#ifdef Q_OS_LINUX
    usleep(100000);
#endif
#ifdef Q_OS_WIN
    Sleep(100);
#endif

    // read response from OTRSP
    int n = OTRSPPort->bytesAvailable();
    if (n > 64) n = 64;
    qDebug("%d bytes available",n);
    OTRSPPort->read(buff, n);

    deviceName=buff;
    qDebug("OTRSP deviceName=%s",deviceName.data());
  */
  OTRSPOpen = true;

}

void OTRSP::closeOTRSP()
{
    OTRSPPort->close();
}


