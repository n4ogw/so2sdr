/*! Copyright 2010-2014 R. Torsten Clay N4OGW

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
#include "microham.h"

/*!
 *  MicroHam Control Protocol (MCP)
 *
 *  Tested MK2R / mhuxd (git) Linux
 *  Windows uRouter untested
 *
 */

MicroHam::MicroHam(QSettings& s, QObject *parent) : QObject(parent),settings(s)
{
    MicroHamPort = new QextSerialPort(settings.value(s_microham_device,s_microham_device_def).toString(), QextSerialPort::EventDriven);
    MicroHamOpen = false;
    stereo       = false;
}

MicroHam::~MicroHam()
{
    closeMicroHam();
    delete MicroHamPort;
}

bool MicroHam::MicroHamIsOpen() const
{
    return(MicroHamOpen);
}

void MicroHam::sendCommand(QByteArray command)
{
    if (!MicroHamOpen) return;
    MicroHamPort->write(command, command.length());
}

void MicroHam::switchAudio(int nr)
{
    if (!MicroHamOpen || nr<0 || nr>1) return;

    const char rxcmd[2][5]={"FR1\r","FR2\r"};

    // if in stereo mode, don't switch RX
    if (!stereo) {
        MicroHamPort->write(rxcmd[nr],4);
    }

}

void MicroHam::switchTransmit(int nr)
{
    if (!MicroHamOpen || nr<0 || nr>1) return;
    const char txcmd[2][5]={"FT1\r","FT2\r"};

    MicroHamPort->write(txcmd[nr],4);
}

void MicroHam::toggleStereo(int nr)
{
    if (!MicroHamOpen || nr<0 || nr>1) return;
    if (stereo) {
        stereo=false;
        this->switchAudio(nr);
    } else {
        const char cmd[5]="FRS\r";
        MicroHamPort->write(cmd,4);
        stereo=true;
    }
}

bool MicroHam::stereoActive () const
{
    return stereo;
}

/*!
   open MicroHam device
 */
void MicroHam::openMicroHam()
{
    if (MicroHamPort->isOpen()) {
        closeMicroHam();
        MicroHamOpen = false;
    }
    MicroHamPort->setPortName(settings.value(s_microham_device,s_microham_device_def).toString());

    // 9600N81
    MicroHamPort->setBaudRate(BAUD9600);
    MicroHamPort->setFlowControl(FLOW_OFF);
    MicroHamPort->setParity(PAR_NONE);
    MicroHamPort->setDataBits(DATA_8);
    MicroHamPort->setStopBits(STOP_1);

    MicroHamPort->setTimeout(500);
    MicroHamPort->open(QIODevice::ReadWrite);
    MicroHamPort->setRts(0);
    MicroHamPort->setDtr(0);
    MicroHamPort->flush();

    if (!MicroHamPort->isOpen()) {
        MicroHamOpen = false;
        qDebug("ERROR: could not open MicroHam device");
        return;
    }
    MicroHamOpen = true;
}

void MicroHam::closeMicroHam()
{
    MicroHamPort->close();
}
