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
#ifndef SIGNAL_H
#define SIGNAL_H

#include "defines.h"

class Signal
{
public:
    Signal();

    void clear();
    bool active;
    int  cnt;
    int  f;
    int  fcq;
    int  n;
    int  space;
    long fsum;
};
Q_DECLARE_TYPEINFO(Signal, Q_MOVABLE_TYPE);

class CalibSignal
{
public:
    CalibSignal();

    double    gain, phase;
    double    z[2];
    double    zsum[2];
    long long n;
};
Q_DECLARE_TYPEINFO(CalibSignal, Q_MOVABLE_TYPE);

#endif // SIGNAL_H
