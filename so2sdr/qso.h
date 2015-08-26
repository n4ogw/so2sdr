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
#ifndef QSO_H
#define QSO_H
#include "defines.h"
#include <QByteArray>
#include <QDateTime>

/*!
  qso data structure
 */
class Qso
{
public:
    explicit Qso(int n = 1);
    ~Qso();
    void         setExchangeType(int, FieldTypes);

    bool       dupe;
    bool       isMM;
    bool       isMobile;
    bool       valid;
    int        band;
    int        freq;
    int        mult[MMAX];
    int        newmult[MMAX];
    int        nr;
    int        pts;
    QByteArray call;
    QByteArray *rcv_exch;
    QByteArray *snt_exch;
    QDateTime  time;
    rmode_t    mode;
    ModeTypes  modeType;

    bool         isamult[MMAX];
    Cont         continent;
    FieldTypes   *exchange_type;
    int          bearing;
    int          country;
    int          n_exchange;
    int          zone;
    QByteArray   country_name;
    QByteArray   exch;
    QByteArray   logInfo;
    QByteArray   mult_name;
    QByteArray   PfxName;
    QByteArray   prefill;
    QString      sun;
    unsigned int worked;
};

#endif // QSO_H
