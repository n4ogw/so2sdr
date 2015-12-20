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
#include <QSettings>
#include "defines.h"
#include "otrsp.h"
#include <QSerialPortInfo>

OTRSP::OTRSP(QSettings& s, QObject *parent) : QObject(parent),settings(s)
{
    QSerialPortInfo info(settings.value(s_otrsp_device,s_otrsp_device_def).toString());
    OTRSPPort = new QSerialPort(info);
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

void OTRSP::sendCommand(QByteArray command)
{
    if (!OTRSPOpen) return;
    OTRSPPort->write(command, command.length());
}

/*!
 * \brief OTRSP::switchRadios
 * switches both RX and TX focus to other radio
 */
void OTRSP::switchAudio(int nr)
{
    if (!OTRSPOpen || nr<0 || nr>1) return;

    const char rxcmd[2][5]={"RX1\r","RX2\r"};

    // if in stereo mode, don't switch RX
    if (!stereo) {
        OTRSPPort->write(rxcmd[nr],4);
    }

}

void OTRSP::switchTransmit(int nr)
{
    if (!OTRSPOpen || nr<0 || nr>1) return;
    const char txcmd[2][5]={"TX1\r","TX2\r"};

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
        stereo=false;
        switchAudio(nr);
    } else {
        const char cmd[6]="RX1S\r";
        OTRSPPort->write(cmd,5);
        stereo=true;
    }
}

bool OTRSP::stereoActive () const
{
    return stereo;
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
    OTRSPPort->setPortName(settings.value(s_otrsp_device,s_otrsp_device_def).toString());

    // currently only 9600N81
    OTRSPPort->setBaudRate(QSerialPort::Baud9600);
    OTRSPPort->setFlowControl(QSerialPort::NoFlowControl);
    OTRSPPort->setParity(QSerialPort::NoParity);
    OTRSPPort->setDataBits(QSerialPort::Data8);
    OTRSPPort->setStopBits(QSerialPort::OneStop);

    OTRSPPort->open(QIODevice::ReadWrite);

    if (!OTRSPPort->isOpen()) {
        OTRSPOpen = false;
        emit(otrspError("ERROR: could not open otrsp device"));
        return;
    }
    OTRSPPort->setRequestToSend(false);
    OTRSPPort->setDataTerminalReady(false);
    OTRSPOpen = true;
}

void OTRSP::closeOTRSP()
{
    OTRSPPort->close();
}


