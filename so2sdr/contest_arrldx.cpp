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
#include "contest_arrldx.h"
#include "log.h"

/*! ARRL DX contest
   b=true : qth is US/VE
   b=false: qth is DX
 */
ARRLDX::ARRLDX(bool b, QSettings &cs, QSettings &ss) : Contest(cs,ss)
{
    setZoneType(1); // this also chooses ARRL rather than CQ countries
    usVe = b;       // true: station is US/VE false: DX
    setVExch(true);
    dupeCheckingEveryBand = true;
    nExch                 = 2;
    logFieldPrefill       = new bool[nExch];
    logFieldPrefill[0]    = true;
    logFieldPrefill[1]    = true;
    prefill               = false;
    finalExch             = new QByteArray[nExch];
    exchange_type         = new FieldTypes[nExch];
    exchange_type[0]      = RST; // class
    if (usVe) {
        // US/VE calls
        exchange_type[1]      = State;        // State
        multFieldHighlight[0] = SQL_COL_CALL; // highlight call for DX
    } else {
        // DX
        exchange_type[1]      = General;      // power
        multFieldHighlight[0] = SQL_COL_RCV2; // highlight state/province
    }
}

ARRLDX::~ARRLDX()
{
    delete[] logFieldPrefill;
    delete[] finalExch;
    delete[] exchange_type;
}

/*! width in characters of data fields shown
 * */
int ARRLDX::fieldWidth(int col) const
{
    switch (col) {
    case 0: // RST
        return(4);
        break;
    case 1: // QTH/zone
        return(5);
        break;
    default:
        return(4);
    }
}

void ARRLDX::addQso(Qso *qso)
{
    if (qso->dupe || !qso->valid || qso->band>=N_BANDS_HF) {
        qso->pts=0;
    }
    qsoPts += qso->pts;
    addQsoMult(qso);
}

/*! no prefills- can't predict from callsign
 */
QByteArray ARRLDX::prefillExchange(Qso *qso)
{
    Q_UNUSED(qso);
    return("");
}

/*!
   number shown in column 0 is sequential qso number
 */
int ARRLDX::numberField() const
{
    return(-1);
}

unsigned int ARRLDX::rcvFieldShown() const
{
    return(1+2);  // show first and second fields
}

void ARRLDX::setupContest(QByteArray MultFile[MMAX], const Cty *cty)
{
    if (usVe) {
        if (MultFile[0].isEmpty()) {
            MultFile[0] = "arrl_country";
        }
    } else {
        if (MultFile[0].isEmpty()) {
            MultFile[0] = "arrldx.txt";
        }
    }
    readMultFile(MultFile, cty);
    zeroScore();
}

unsigned int ARRLDX::sntFieldShown() const
{
    return(0); // nothing shown
}

bool ARRLDX::validateExchange(Qso *qso)
{
    if (!separateExchange(qso)) return(false);
    qso->bandColumn=qso->band;
    fillDefaultRST(qso);
    finalExch[1].clear();
    qso->rcv_exch[0].clear();
    qso->rcv_exch[1].clear();
    bool ok = false;

    for (int ii = 0; ii < MMAX; ii++) qso->mult[ii] = -1;

    // get the exchange
    determineMultType(qso);

    qso->pts = 0;

    // for US/VE stations
    if (usVe) {
        // mobile stations: /MM /AM qso credit but not a mult
        if (qso->isMM) {
            qso->mult[0]    = -1;
            qso->isamult[0] = true;
        }
        if (qso->isamult[0]) {
            ok = valExch_rst_name(qso);
            if (ok) {
                qso->pts = 3;
            }
        }
    } else {
        // DX stations
        if (qso->isamult[0]) {
            ok = valExch_rst_state(0, qso->mult[0], qso);
            if (ok) {
                qso->pts = 3;
            }
        }
    }
    if (qso->dupe) qso->pts = 0;
    copyFinalExch(ok, qso);
    return(ok);
}
