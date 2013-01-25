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

OTRSP::OTRSP(QSettings *s, QObject *parent) : QObject(parent)
{
    settings=s;
    OTRSPPort     = new QextSerialPort(settings->value(s_otrsp_device,s_otrsp_device_def).toString(), QextSerialPort::EventDriven);
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
 * \brief OTRSP::switchRadios
 * switches both RX and TX focus to other radio
 */
void OTRSP::switchRadios(int nr)
{
    if (!OTRSPOpen || nr<0 || nr>1) return;

    const char rxcmd[2][5]={"RX1\r","RX2\r"};
    const char txcmd[2][5]={"TX1\r","TX2\r"};
    OTRSPPort->write(rxcmd[nr],4);
    OTRSPPort->write(txcmd[nr],4);
}

/*!
 * \brief OTRSP::toggleStereo
 * \param nr the active radio
 * Toggles stereo receive mode. OTRSP needs parameter nr because it
 * can't remember how to get out of stereo split mode.
 */
void OTRSP::toggleStereo(int nr)
{
    if (!OTRSPOpen || nr<0 || nr>1) return;
    if (stereo) {
        switchRadios(nr);
        stereo=false;
    } else {
        const char cmd[6]="RX1S\r";
        OTRSPPort->write(cmd,5);
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

    if (!OTRSPPort->isOpen()) {
        OTRSPOpen = false;
        qDebug("ERROR: could not open otrsp device");
        return;
    }
    OTRSPOpen = true;
}

void OTRSP::closeOTRSP()
{
    OTRSPPort->close();
}


