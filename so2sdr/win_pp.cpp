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
#include <QString>
#include "win_pp.h"


// Wrapper functions for the function pointers
// - call these functions to perform I/O.
// there does not seem to be a way to check if inp32 or out32 complete successfully?
//
short ParallelPort::Inp32(short portaddr)
{
    return((inp32fp) (portaddr));
}

void ParallelPort::Out32(short portaddr, short datum)
{
    (oup32fp) (portaddr, datum);
}

ParallelPort::ParallelPort(QSettings &s) : settings(s)
{
    port        = defaultParallelPort.toULong();
    initialized = false;
    stereoPinStatus=false;
    initSuccess = false;
	stereoPinStatus=false;
    hLib        = LoadLibraryA("InpOut32.DLL");

    if (hLib == NULL) {
        QString tmp = "LoadLibrary inpout32.dll failed";
        emit(parallelPortError(tmp));
        return;
    }

    // get the address of the function

    inp32fp = (inpfuncPtr) GetProcAddress(hLib, "Inp32");
    if (inp32fp == NULL) {
        QString tmp = "GetProcAddress for Inp32 Failed.";
        emit(parallelPortError(tmp));
        return;
    }

    oup32fp = (oupfuncPtr) GetProcAddress(hLib, "Out32");
    if (oup32fp == NULL) {
        QString tmp = "GetProcAddress for Oup32 Failed.";
        emit(parallelPortError(tmp));
        return;
    }
    initSuccess = true;
}

ParallelPort::~ParallelPort()
{
    FreeLibrary(hLib);
}

void ParallelPort::PinLow(const int p)

// set pin p (2:9) low
{
    if (!initSuccess) return;
    if ((p < 2) | (p > 9)) return;

    short x = Inp32(port);
    short b = 1 << (p - 2);
    if (x & b) {
        x -= b;
        Out32(port, x);
    }
}

void ParallelPort::PinHigh(const int p)

// set pin (2:9) high
{
    if (!initSuccess) return;
    if ((p < 2) | (p > 9)) return;

    short x = Inp32(port);
    short b = 1 << (p - 2);
    if (!(x & b)) {
        x += b;
        Out32(port, x);
    }
}


void ParallelPort::initialize()
{
    bool ok=false;
	QString pstring=settings.value(s_radios_pport,defaultParallelPort).toString();
    port= pstring.toULong(&ok,0);
    if (!ok) {
        port = defaultParallelPort.toULong();
        QString tmp = "port <" + pstring + "> invalid, using port "+defaultParallelPort;
        emit(parallelPortError(tmp));
    }
	initialized=true;
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
