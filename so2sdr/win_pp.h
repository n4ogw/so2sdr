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
#ifndef WIN_PP_H
#define WIN_PP_H

// Parallel port access under Windows using inpout32.dll
// see http://www.logix4u.net/inpout32.htm

#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include <qDebug>
#include <QSettings>
#include "defines.h"

// prototype (function typedef) for DLL function Inp32:

typedef short (_stdcall * inpfuncPtr)(short portaddr);
typedef void (_stdcall * oupfuncPtr)(short portaddr, short datum);

// bit to pin mapping
// Bit:     7   6   5   4   3   2   1   0
// Base (Data port)     Pin:    9   8   7   6   5   4   3   2
// Base+1 (Status port)     Pin:    ~11     10  12  13  15
// Base+2 (Control port)    Pin:                ~17     16  ~14     ~1

class ParallelPort : public QObject
{
Q_OBJECT

public:
    ParallelPort(QSettings& s);
    ~ParallelPort();
    void switchAudio(int r);
    void toggleStereoPin();

signals:
    void parallelPortError(const QString &);

public slots:
    void initialize();

private:
    void PinLow(int p);
    void PinHigh(int p);

	HINSTANCE hLib;

// After successful initialization, these 2 variables
// will contain function pointers.
//
    inpfuncPtr inp32fp;
    oupfuncPtr oup32fp;
    bool       initSuccess;
    bool stereoPinStatus;
// Wrapper functions for the function pointers
// - call these functions to perform I/O.

    short  Inp32(short portaddr);
    void  Out32(short portaddr, short datum);
    unsigned long port;
    QSettings& settings;
};
#endif // WIN_PP_H
