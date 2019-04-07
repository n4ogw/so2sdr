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
#include "contest_arrl160.h"
#include "log.h"

/*! ARRL 160m contest
b=true: station in USA/VE
 =false:  DX*/
ARRL160::ARRL160(bool b, QSettings &cs, QSettings &ss) : Contest(cs,ss)
{
    usVe=b;
    setZoneMax(0);
    setZoneType(1); // this also chooses ARRL rather than CQ countries
    setVExch(true);
    dupeCheckingEveryBand = true;
    nExch                 = 2;
    logFieldPrefill       = new bool[nExch];
    logFieldPrefill[0]    = true;
    logFieldPrefill[1]    = true;
    prefill               = true;
    finalExch             = new QByteArray[nExch];
    exchange_type         = new FieldTypes[nExch];
    exchange_type[0]      = RST;          // class
    exchange_type[1]      = ARRLSection;  // section
    multFieldHighlight[0] = SQL_COL_RCV2; // section
    multFieldHighlight[1] = SQL_COL_CALL; // highlight call for DX
}

ARRL160::~ARRL160()
{
    delete[] logFieldPrefill;
    delete[] finalExch;
    delete[] exchange_type;
}

/* default labels for bands in score summary */
QString ARRL160::bandLabel(int i) const
{
    switch (i) {
    case 0: return "160CW";break;
    default: return "";
    }
}

bool ARRL160::bandLabelEnable(int i) const
{
    switch (i) {
    case 0: return true;
    default: return false;
    }
}

void ARRL160::addQso(Qso *qso)
{
    if (qso->dupe || !qso->valid) {
        qso->pts=0;
    }
    // must be on 160m for qso to count
    if (qso->band != BAND160) {
        qso->pts=0;
        for (int ii = 0; ii < MMAX; ii++) {
            qso->mult[ii]    = -1;
            qso->newmult[ii] = -1;
        }
    }
    qsoPts += qso->pts;
    addQsoMult(qso);
}


/*! width in characters of data fields shown
 * */
int ARRL160::fieldWidth(int col) const
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

/*!
   number shown in column 0 is sequential qso number
 */
int ARRL160::numberField() const
{
    return(-1);
}

/*! prefill exchange for DX stations == country prefix
 */
QByteArray ARRL160::prefillExchange(Qso *qso)
{
    qso->mult[0]    = -1;
    qso->mult[1]    = -1;
    qso->isamult[0] = false;
    qso->isamult[1] = false;
    determineMultType(qso);
    if (qso->isamult[1] && !qso->isamult[0]) {
        return(qso->PfxName);
    } else {
        return("");
    }
}

unsigned int ARRL160::rcvFieldShown() const
{
    return(1+2);  // show first and second fields
}

int ARRL160::Score() const
{
    return(qsoPts * (multsWorked[0][CWType][BAND160] + multsWorked[1][CWType][BAND160]));
}

void ARRL160::setupContest(QByteArray MultFile[MMAX], const Cty *cty)
{
    if (MultFile[0].isEmpty()) {
        MultFile[0] = "arrl.txt";
    }
    readMultFile(MultFile, cty);
    zeroScore();
}

unsigned int ARRL160::sntFieldShown() const
{
    return(0); // nothing shown
}

bool ARRL160::validateExchange(Qso *qso)
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

    // mobile ITU regions (RST) R1, (RST) R2, (RST) R3
    // these are ok for US/VE stations to work
    if (qso->isMM && usVe) {
        if ((ok = valExch_mm(qso))) {
            qso->pts        = 5;
            qso->mult[0]    = -1;
            qso->mult[1]    = -1;
            qso->isamult[0] = false;
            qso->isamult[1] = false;
        }
    }

    if (!qso->isMM) {
        if (qso->isamult[0]) {
            // Domestic call: (RST) SEC
            ok = valExch_rst_state(0, qso->mult[0], qso);
            if (ok) {
                qso->pts = 2;
            }
        } else if (qso->isamult[1]) {
            // DX: (RST)
            // check for rst
            bool iok = false;
            int  r;
            r = exchElement.at(0).toInt(&iok, 10);
            if (iok && r >= 111 && r <= 599) {
                finalExch[0] = exchElement[0];
            }

            // record country prefix
            finalExch[1] = qso->PfxName;
            qso->pts     = 5;
            ok           = true;
        }
    }
    if (qso->dupe) qso->pts = 0;
    copyFinalExch(ok, qso);
    return(ok);
}

