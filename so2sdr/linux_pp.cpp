/*! Copyright 2010-2021 R. Torsten Clay N4OGW

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
#include <QErrorMessage>
#include <QString>
#include "defines.h"
#include "linux_pp.h"
#include <sys/param.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <grp.h>

#include <fcntl.h>
#include <linux/parport.h>
#include <linux/ppdev.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

/*!
 * \brief ParallelPort::ParallelPort Parallel port access under Linux
 * \param s
 */
ParallelPort::ParallelPort(QSettings& s):settings(s)
{
    parallelFD  = -1;
    initialized = false;
    stereoPinStatus=false;
}

ParallelPort::~ParallelPort()
{
    if (parallelFD != -1) {
        int err = ioctl(parallelFD, PPRELEASE);
        if (err == -1) {
            QString tmp = "Error releasing parallel port";
            emit(parallelPortError(tmp));
            return;
        }
        err = close(parallelFD);
        if (err == -1) {
            QString tmp = "Error closing parallel port";
            emit(parallelPortError(tmp));
            return;
        }
    }
}

/*!
   Note that user must be in lp group to access /dev/parport

   Will attempt to drop privleges after getting port access.
 */
void ParallelPort::initialize()
{
    QString port= settings.value(s_radios_pport,defaultParallelPort).toString();
    if (parallelFD != -1) {
        int err = ioctl(parallelFD, PPRELEASE);
        if (err == -1) {
            QString tmp = "Error releasing " + port;
            emit(parallelPortError(tmp));
            return;
        }
        err = close(parallelFD);
        if (err == -1) {
            QString tmp = "Error closing " + port;
            emit(parallelPortError(tmp));
            return;
        }
    }
    parallelFD = open(port.toLatin1().data(), O_RDWR);
    if (parallelFD == -1) {
        QString tmp = "Can't open " + port;
        emit(parallelPortError(tmp));
        return;
    }
    int err = ioctl(parallelFD, PPCLAIM);
    if (err == -1) {
        QString tmp = "ERROR: Can't claim " + port;
        emit(parallelPortError(tmp));
        return;
    }
    int mode = IEEE1284_MODE_COMPAT;
    if (ioctl(parallelFD, PPNEGOT, &mode)) {
        QString tmp = "ERROR: can't set " + port + " to IEEE1284 compat mode";
        emit(parallelPortError(tmp));
        return;
    }

    initialized = true;
}

/*!
   set a pin  p=(2:9) low
 */
void ParallelPort::PinLow(const int p)
{
    if (!initialized || (p < 2) || (p > 9)) return;

    char data = 0;
    int  err  = ioctl(parallelFD, PPRDATA, &data); // read port data
    if (err == -1) {
        QString tmp = "ERROR: Can't read from " +  settings.value(s_radios_pport,defaultParallelPort).toString();
        emit(parallelPortError(tmp));
        return;
    }
    char b = 1 << (p - 2);
    if (data & b) {
        data -= b;
        err   = ioctl(parallelFD, PPWDATA, &data);
        if (err == -1) {
            QString tmp = "ERROR: Can't write to " +  settings.value(s_radios_pport,defaultParallelPort).toString();
            emit(parallelPortError(tmp));
        }
    }
}

/*!
   set a pin  p=(2:9) high
 */
void ParallelPort::PinHigh(const int p)
{
    if (!initialized || (p < 2) || (p > 9)) return;

    char data = 0;
    int  err  = ioctl(parallelFD, PPRDATA, &data); // read port data
    if (err == -1) {
        QString tmp = "ERROR: Can't read from " +  settings.value(s_radios_pport,defaultParallelPort).toString();
        emit(parallelPortError(tmp));
        return;
    }
    char b = 1 << (p - 2);
    if (!(data & b)) {
        data += b;
        err   = ioctl(parallelFD, PPWDATA, &data);
        if (err == -1) {
            QString tmp = "ERROR: Can't write to " +  settings.value(s_radios_pport,defaultParallelPort).toString();
            emit(parallelPortError(tmp));
        }
    }
}

/*!
  switch audio to radio r
  */
void ParallelPort::switchAudio(int r)
{
    int radioPin=settings.value(s_radios_focus,defaultParallelPortAudioPin).toInt();
    bool invert=settings.value(s_radios_focusinvert,false).toBool();
    if (r==0) {
        if (invert) {
            PinHigh(radioPin);
        } else {
            PinLow(radioPin);
        }
    } else {
        if (invert) {
            PinLow(radioPin);
        } else {
            PinHigh(radioPin);
        }
    }
}

// switch ptt routing
void ParallelPort::switchTransmit(int r)
{
    int txPin=settings.value(s_radios_txfocus,defaultParallelPortTxPin).toInt();
    bool invert=settings.value(s_radios_txfocusinvert,false).toBool();
    if (r==0) {
        if (invert) {
            PinHigh(txPin);
        } else {
            PinLow(txPin);
        }
    } else {
        if (invert) {
            PinLow(txPin);
        } else {
            PinHigh(txPin);
        }
    }
}

/*!
  toggle pin stereoPin on parallel port
  */
void ParallelPort::toggleStereoPin()
{
    int stereoPin=settings.value(s_radios_stereo,defaultParallelPortStereoPin).toInt();
    if (stereoPinStatus) {
        PinLow(stereoPin);
        stereoPinStatus = false;
    } else {
        PinHigh(stereoPin);
        stereoPinStatus = true;
    }
}

bool ParallelPort::stereoActive() const
{
    return stereoPinStatus;
}
