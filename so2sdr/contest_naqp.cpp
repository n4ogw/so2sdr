/*! Copyright 2010-2022 R. Torsten Clay N4OGW

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
#include "contest_naqp.h"
#include "log.h"

/*! North American QSO Party (NAQP) */
Naqp::Naqp(QSettings &cs, QSettings &ss) : Contest(cs,ss)
{
    setZoneType(1); // this also chooses ARRL rather than CQ countries
    setVExch(false);
    dupeCheckingEveryBand = true;
    nExch                 = 2;
    logFieldPrefill       = new bool[nExch];
    logFieldPrefill[0]    = true;
    logFieldPrefill[1]    = true;
    prefill               = false;
    finalExch             = new QByteArray[nExch];
    exchange_type         = new FieldTypes[nExch];
    exchange_type[0]      = Name;         // name
    exchange_type[1]      = State;        // qth
    multFieldHighlight[0] = SQL_COL_RCV2; // domestic mult field
    multFieldHighlight[1] = SQL_COL_RCV2; // NA country; uses same field
}

Naqp::~Naqp()
{
    delete[] logFieldPrefill;
    delete[] finalExch;
    delete[] exchange_type;
}

void Naqp::addQso(Qso *qso)
{
    // all qsos are 1 point
    if (!qso->dupe && qso->valid && qso->band<N_BANDS_HF) {
        qso->pts = 1;
        qsoPts++;
    } else {
        qso->pts = 0;
    }
    addQsoMult(qso);
}

/*! width in characters of data fields shown
 * */
int Naqp::fieldWidth(int col) const
{
    switch (col) {
    case 0: // name
        return(6);
        break;
    case 1: //  qth
        return(4);
        break;
    default:
        return 4;
    }
}

/*!
   number shown in column 0 is sequential qso number
 */
int Naqp::numberField() const
{
    return(-1);
}

unsigned int Naqp::rcvFieldShown() const

// 0 1=NAME --> show
// 1 2=STATE --> show
{
    return(1 + 2);  // show 2
}

void Naqp::setupContest(QByteArray MultFile[MMAX], const Cty *cty)
{
    if (MultFile[0].isEmpty()) {
        MultFile[0] = "naqp.txt";
    }
    if (MultFile[1].isEmpty()) {
        MultFile[1] = "cont_na";
    }
    readMultFile(MultFile, cty);
    zeroScore();
}

bool Naqp::validateExchange(Qso *qso)
{
    if (!separateExchange(qso)) return(false);
    bool ok = false;

    qso->bandColumn=qso->band;
    for (int ii = 0; ii < MMAX; ii++) qso->mult[ii] = -1;

    // get the exchange
    determineMultType(qso);

    if (qso->isamult[0]) {
        // Domestic call: NAME STATE
        if (exchElement.size() < 2) return(false);
        ok = valExch_name_state(0, qso->mult[0]);
    } else if (qso->isamult[1]) {
        // NA country. mult already known from call
        if (!exchElement[0].isEmpty()) {
            ok           = true;
            finalExch[0] = exchElement[0];
            finalExch[1] = qso->PfxName;
        }
    } else {
        // DX
        if (!exchElement[0].isEmpty()) {
            ok           = true;
            finalExch[0] = exchElement[0];
            finalExch[1] = "DX";
        }
    }
    for (int i = 0; i < nExch; i++) {
        qso->rcv_exch[i] = finalExch[i];
    }
    return(ok);
}
