/*! Copyright 2010-2017 R. Torsten Clay N4OGW

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
#include "contest_arrl10.h"
#include "log.h"

/*! ARRL 10m contest */
ARRL10::ARRL10()
{
    setZoneMax(0);
    setZoneType(1); // this also chooses ARRL rather than CQ countries
    setVExch(true);
    dupeCheckingEveryBand = true;
    nExch                 = 2;
    logFieldPrefill       = new bool[nExch];
    for (int i = 0; i < nExch; i++) logFieldPrefill[i] = true;
    prefill               = false;
    finalExch             = new QByteArray[nExch];
    exchange_type         = new FieldTypes[nExch];
    exchange_type[0]      = RST;          // class
    exchange_type[1]      = State;        // state
    multFieldHighlight[0] = SQL_COL_RCV2; // state
    multFieldHighlight[1] = SQL_COL_CALL; // highlight call for DX
}

void ARRL10::setupContest(QByteArray MultFile[MMAX], const Cty *cty)
{
    if (MultFile[0].isEmpty()) {
        MultFile[0] = "arrl10.txt";
    }
    if (MultFile[1].isEmpty()) {
        MultFile[1] = "arrl_country";
    }
    readMultFile(MultFile, cty);
    zeroScore();
}

ARRL10::~ARRL10()
{
    delete[] logFieldPrefill;
    delete[] finalExch;
    delete[] exchange_type;
}

void ARRL10::addQso(Qso *qso)
{
    // must be on 10m for qso to count
    if (qso->band != BAND10) {
        qso->pts=0;
        for (int ii = 0; ii < MMAX; ii++) {
            qso->mult[ii]    = -1;
            qso->newmult[ii] = -1;
        }
        addQsoMult(qso);
        return;
    }
    // 2 pts phone, 4 pts cw
    switch (qso->mode) {
    case RIG_MODE_CW:
    case RIG_MODE_CWR:
        qso->pts = 4;
        break;
        case RIG_MODE_USB:
    case RIG_MODE_LSB:
    case RIG_MODE_AM:
    case RIG_MODE_FM:
        qso->pts = 2;
        break;
    default:
        qso->pts = 2;
        break;
    }
    if (qso->dupe || !qso->valid) qso->pts = 0;
    qsoPts += qso->pts;
    addQsoMult(qso);
}

/*!
   width in pixels of data fields shown */
int ARRL10::fieldWidth(int col) const
{
    switch (col) {
    case 0:
        return 35; // RST
        break;
    case 1:
        return 45;
        break; // qth/zone
    default:
        return 35;
    }
}

/*!
   number shown in column 0 is sequential qso number
 */
int ARRL10::numberField() const
{
    return(-1);
}

unsigned int ARRL10::rcvFieldShown() const
{
    return(1+2);  // show first and second fields
}

int ARRL10::Score() const
{
    return(qsoPts * (multsWorked[0][BAND10] + multsWorked[1][BAND10]));
}

unsigned int ARRL10::sntFieldShown() const
{
    return(0); // nothing shown
}

bool ARRL10::validateExchange(Qso *qso)
{
    if (!separateExchange(qso)) return(false);
    fillDefaultRST(qso);

    finalExch[1].clear();
    qso->rcv_exch[0].clear();
    qso->rcv_exch[1].clear();
    bool ok = false;

    for (int ii = 0; ii < MMAX; ii++) qso->mult[ii] = -1;

    qso->pts = 0;
    determineMultType(qso);

    // mobile ITU regions (RST) R1, (RST) R2, (RST) R3
    // not using validator in contest.cpp since here R1/R2/R3 are
    // multipliers
    if (qso->isMM) {
        for (int i = exchElement.size() - 1; i >= 0; i--) {
            int r = 0;
            if (exchElement[i] == "R1") r = 1;
            if (exchElement[i] == "R2") r = 2;
            if (exchElement[i] == "R3") r = 3;
            if (r) {
                // make this count as "domestic mult" regardless of actual country of callsign
                qso->mult[1]    = -1;
                qso->isamult[0] = true;
                break;
            }
        }
    }
    if (qso->isamult[0]) {
        // Domestic call: (RST) state
        ok = valExch_rst_state(0, qso->mult[0]);
    } else if (qso->isamult[1]) {
        // DX: (RST) #
        // two things entered; assume first is rst
        int inr = 0;
        if (exchElement.size() == 2) {
            finalExch[0] = exchElement[0];
            inr          = 1;
        }

        // serial number; must be convertable to int
        bool iok = false;
        exchElement[inr].toInt(&iok, 10);
        if (iok) {
            ok           = true;
            finalExch[1] = exchElement[inr];
        }
    }
    copyFinalExch(ok, qso);
    return(ok);
}
