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
#include "contest_sprint.h"
#include "log.h"

/*! NCJ  NA Sprint */
Sprint::Sprint()
{
    setZoneType(1); // this also chooses ARRL rather than CQ countries
    setVExch(true);
    dupeCheckingEveryBand = true;
    nExch                 = 3;
    logFieldPrefill       = new bool[nExch];
    logFieldPrefill[0]    = true;
    logFieldPrefill[1]    = true;
    logFieldPrefill[2]    = true;
    prefill               = false;
    finalExch             = new QByteArray[nExch];
    exchange_type         = new FieldTypes[nExch];
    exchange_type[0]      = QsoNumber;    // qso number
    exchange_type[1]      = Name;         // name
    exchange_type[2]      = State;        // state or NA prefix
    multFieldHighlight[0] = SQL_COL_RCV3; // domestic mult field
    multFieldHighlight[1] = SQL_COL_RCV3; // NA country; uses same field
}

Sprint::~Sprint()
{
    delete[] logFieldPrefill;
    delete[] finalExch;
    delete[] exchange_type;
}

void Sprint::addQso(Qso *qso)
{
    if (!qso->dupe && qso->valid && qso->band<N_BANDS_SCORED) {
        qso->pts = 1;
    } else {
        qso->pts = 0;
    }
    qsoPts+=qso->pts;
    addQsoMult(qso);
}

int Sprint::fieldWidth(int col) const

// width in pixels of data fields shown
{
    switch (col) {
    case 0: return(33); break; // sent #
    case 1: return(33); break; // rcv #
    case 2: return(50); break; // rcv name
    case 3: return(33); break; // rcv qth
    case 4: return(33); break; // mult
    default: return(35);
    }
}

/*!
   Number shown in column 0 is the number sent
 */
int Sprint::numberField() const
{
    return(0);
}

unsigned int Sprint::rcvFieldShown() const

// 0 1=NR --> show
// 1 2=NAME --> show
// 2 4=STATE --> show
{
    return(1 + 2 + 4);  // show 3 fields
}

void Sprint::setupContest(QByteArray MultFile[MMAX], const Cty *cty)
{
    if (MultFile[0].isEmpty()) {
        MultFile[0] = "sprint.txt";
    }
    if (MultFile[1].isEmpty()) {
        MultFile[1] = "cont_na";
    }
    readMultFile(MultFile, cty);
    zeroScore();
}

unsigned int Sprint::sntFieldShown() const

// 0 1=NR   --> show
// 1 2=NAME
// 2 4=STATE
{
    return(1); // show qso # only
}

bool Sprint::validateExchange(Qso *qso)
{
    if (!separateExchange(qso)) return(false);
    bool ok = false;

    for (int ii = 0; ii < MMAX; ii++) qso->mult[ii] = -1;

    // get the exchange
    determineMultType(qso);

    if (qso->isamult[0]) {
        // Domestic call: # NAME STATE
        if (exchElement.size() < 3) return(false);
        ok = valExch_nr_name_state(0, qso->mult[0]);
    } else if (qso->isamult[1]) {
        // NA country
        ok           = valExch_nr_name();
        finalExch[2] = qso->PfxName;
    } else {
        // DX
        ok           = valExch_nr_name();
        finalExch[2] = "DX";
    }
    for (int i = 0; i < nExch; i++) {
        qso->rcv_exch[i] = finalExch[i];
    }

    return(ok);
}
