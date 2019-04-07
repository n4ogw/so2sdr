/*! Copyright 2010-2019 R. Torsten Clay N4OGW

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
#include "qso.h"

/*!
  n is number of exchange fields
  */
Qso::Qso(int n)
{
    rcv_exch = new QByteArray[n];
    snt_exch = new QByteArray[n];
    for (int i = 0; i < n; i++) {
        rcv_exch[i].clear();
        snt_exch[i].clear();
    }
    isMM = false;
    isMobile = false;
    isRover = false;
    dupe = false;
    valid = false;
    pts  = 0;
    call.clear();
    for (int ii = 0; ii < MMAX; ii++) {
        mult[ii]    = -1;
        newmult[ii] = -1;
    }
    mode = RIG_MODE_CW;
    modeType = CWType;
    band = 0;
    bandColumn=0;
    freq = 0.0;
    zone=0;
    distance=-1;
    nr=0;
    n_exchange    = n;
    exchange_type = new FieldTypes[n_exchange];
    for (int i = 0; i < n_exchange; i++) exchange_type[i] = General;
    exch.clear();
    country_name.clear();
    mult_name.clear();
    PfxName.clear();
    prefill.clear();
    sun.clear();
    worked = 0;
    for (int ii = 0; ii < MMAX; ii++) {
        isamult[ii] = false;
    }
}

Qso::~Qso()
{
    delete [] rcv_exch;
    delete [] snt_exch;
    delete[] exchange_type;
}

void Qso::clear()
{
    for (int i = 0; i < n_exchange; i++) {
        rcv_exch[i].clear();
        snt_exch[i].clear();
    }
    isMM = false;
    isMobile = false;
    isRover = false;
    dupe = false;
    valid = false;
    pts  = 0;
    for (int ii = 0; ii < MMAX; ii++) {
        mult[ii]    = -1;
        newmult[ii] = -1;
        isamult[ii] = false;
    }
    zone=0;
    band=0;
    bandColumn=0;
    nr=0;
    call.clear();
    exch.clear();
    country_name.clear();
    mult_name.clear();
    PfxName.clear();
    prefill.clear();
    sun.clear();
    worked=0;
}

void Qso::setExchangeType(int i, FieldTypes f)
{
    if (i >= 0 && i < n_exchange) {
        exchange_type[i] = f;
    }
}
