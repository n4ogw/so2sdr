/*! Copyright 2010-2018 R. Torsten Clay N4OGW

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
#include "contest_cq160.h"
#include "log.h"

/*! CQ 160m contest */
CQ160::CQ160(QSettings &cs, QSettings &ss) : Contest(cs,ss)
{
    setZoneMax(40);
    setZoneType(0);
    setVExch(true);
    dupeCheckingEveryBand = true;
    nExch                 = 2;
    logFieldPrefill       = new bool[nExch];
    logFieldPrefill[0]    = true;
    logFieldPrefill[1]    = true;
    prefill               = true;
    finalExch             = new QByteArray[nExch];
    exchange_type         = new FieldTypes[nExch];
    exchange_type[0]      = RST;          // signal report
    exchange_type[1]      = DMult;        //
    multFieldHighlight[0] = SQL_COL_RCV2; // new state/province ; highlight mult field
    multFieldHighlight[1] = SQL_COL_CALL; // new country; highlight callsign field
}

CQ160::~CQ160()
{
    delete[] logFieldPrefill;
    delete[] finalExch;
    delete[] exchange_type;
}

/* default labels for bands in score summary */
QString CQ160::bandLabel(int i) const
{
    switch (i) {
    case 0: return "160CW";break;
    default: return "";
    }
}

bool CQ160::bandLabelEnable(int i) const
{
    switch (i) {
    case 0: return true;
    default: return false;
    }
}

/*! add qso
   determine qso point value, increase nqso, update score
   update mult count

   qso point values:
   [0]==2 stations in same country
   [1]==5 stations in different country but same continent
   [2]=10 stations in different continents */
void CQ160::addQso(Qso *qso)
{
    // not on 160, does not count
    if (qso->band != BAND160) {
        qso->pts = 0;
        for (int ii = 0; ii < MMAX; ii++) {
            qso->mult[ii]    = -1;
            qso->newmult[ii] = -1;
        }
        addQsoMult(qso);
        return;
    }
    if (qso->dupe || !qso->valid) {
        qso->pts=0;
    } else if (qso->country == myCountry) {
        qso->pts = 2;
    } else if (qso->continent == myContinent || qso->isMM) {
        qso->pts = 5;
    } else {
        qso->pts = 10;
    }
    qsoPts+=qso->pts;
    addQsoMult(qso);
}

int CQ160::fieldWidth(int col) const

// width in pixels of data fields shown
{
    switch (col) {
    case 0:
        return 35; // RST
        break;
    case 1:
        return 45; //  qth/zone
        break;
    default:
        return 35;
    }
}

int CQ160::numberField() const
{
    return(-1);
}

// for non W/VE, return CQ zone
QByteArray CQ160::prefillExchange(Qso *qso)
{
    determineMultType(qso);
    if (qso->isMM) return "";
    if (!qso->isamult[0] && qso->zone!=0) {
        return(QByteArray::number(qso->zone));
    } else {
        return("");
    }
}

unsigned int CQ160::rcvFieldShown() const
{
    return(1+2);  // show first and second fields
}

/*!
   only count qso's on 160m in score
 */
int CQ160::Score() const
{
    return(qsoPts * (multsWorked[0][CWType][BAND160] + multsWorked[1][CWType][BAND160]));
}

void CQ160::setupContest(QByteArray MultFile[MMAX], const Cty *cty)
{
    if (MultFile[0].isEmpty()) {
        MultFile[0] = "cq160.txt";
    }
    readMultFile(MultFile, cty);
    zeroScore();
}

unsigned int CQ160::sntFieldShown() const
{
    return(0);  // show no sent fields
}

bool CQ160::validateExchange(Qso *qso)

// mult1=US/VE state
// mult2=ARRL DXCC
{
    if (!separateExchange(qso)) return(false);
    bool ok = false;
    qso->bandColumn=qso->band;
    for (int ii = 0; ii < MMAX; ii++) qso->mult[ii] = -1;

    // get the exchange
    determineMultType(qso);

    if (qso->isMM) {
        // /MM stations 5 points, not a multipler
        ok              = true;
        qso->mult[0]    = -1;
        qso->mult[1]    = -1;
        qso->isamult[0] = false;
        qso->isamult[1] = false;
        if (exchElement.size()==2) {
            finalExch[0]=exchElement[0]; // rst  entered
            finalExch[1]=exchElement[1]; // presumably R1, R2, R3
        } else {
            fillDefaultRST(qso);
            finalExch[1] = exchElement[0]; // presumably R1, R2, R3
        }
    } else if (qso->isamult[0]) {
        // Domestic call: RST STATE
        if (exchElement.size() < 1) return(false);
        ok = valExch_rst_state(0, qso->mult[0], qso);
    } else if (qso->isamult[1]) {
        // DX: sends zone, but mult is country
        if (exchElement.size() == 2) {
            finalExch[0] = exchElement[0]; // rst
            finalExch[1] = exchElement[1]; // zone
        } else {
            fillDefaultRST(qso);
            finalExch[1] = exchElement[0]; // zone
        }
        ok = true;
    }
    copyFinalExch(ok, qso);
    return(ok);
}
