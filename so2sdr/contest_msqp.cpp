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
#include "contest_msqp.h"
#include "log.h"

/*! Mississippi QSO Party */
MSQP::MSQP(QSettings &cs, QSettings &ss) : Contest(cs,ss)
{
    setVExch(true);
    dupeCheckingEveryBand = true;
    nExch                 = 2;
    logFieldPrefill       = new bool[nExch];
    for (int i = 0; i < nExch; i++) logFieldPrefill[i] = true;
    prefill               = false;
    finalExch             = new QByteArray[nExch];
    exchange_type         = new FieldTypes[nExch];
    exchange_type[0]=RST;
    exchange_type[1]=DMult;
    multFieldHighlight[0] = -1;
    setWithinState(true);
}

MSQP::~MSQP()
{
    delete[] logFieldPrefill;
    delete[] finalExch;
    delete[] exchange_type;
}

void MSQP::addQso(Qso *qso)
// determine qso point value, increase nqso, update score
// update mult count
{
    if (!qso->dupe && qso->valid && qso->band<=BAND420) {
        if (qso->modeType==CWType || qso->modeType==DigiType) {
            qso->pts = 2;
        } else if (qso->modeType==PhoneType) {
            qso->pts = 1;
        }
    } else {
        qso->pts = 0;
    }
    qsoPts += qso->pts;
    addQsoMult(qso);
}

/*! width in characters of data fields shown
 * */
int MSQP::fieldWidth(int col) const
{
    switch (col) {
    case 0: // RST
        return(4);
        break;
    case 1: //  mult
        return(5);
        break;
    default:
        return 4;
    }
}

/*!
   no qso number
 */
int MSQP::numberField() const
{
    return(-1);
}

unsigned int MSQP::rcvFieldShown() const
// 1 2=mult --> show
{
    return(2);
}

void MSQP::setupContest(QByteArray MultFile[MMAX], const Cty *cty)
{
    readMultFile(MultFile, cty);
    zeroScore();
}

unsigned int MSQP::sntFieldShown() const
{
    return(0); // show nothing
}

/*!
   MSQP exchange validator
 */
bool MSQP::validateExchange(Qso *qso)
{
    if (!separateExchange(qso)) return(false);

    qso->bandColumn=qso->band;
    for (int ii = 0; ii < MMAX; ii++) qso->mult[ii] = -1;

    // check prefix
    determineMultType(qso);

    // any number must be RST; take last one entered
    int nrField=-1;
    bool ok_part[2];
    ok_part[0]=false;
    ok_part[1]=false;
    for (int i=exchElement.size()-1;i>=0;i--) {
        bool ok=false;
        exchElement.at(i).toInt(&ok);
        if (ok) {
            nrField=i;
            ok_part[0]=true;
            break;
        }
    }
    if (nrField!=-1) {
        finalExch[0]=exchElement.at(nrField);
    } else {
        // default RST
        if (qso->modeType==CWType || qso->modeType==DigiType) {
            finalExch[0]="599";
        } else {
            finalExch[0]="59";
        }
        ok_part[0]=true;
    }

    // look for state or MS county

    // both MS and non-MS can work stations with county mults
    int m=-1;
    int multField=-1;
    if (qso->isamult[0]) {
        for (int i=exchElement.size()-1;i>=0;i--) {
            m=isAMult(exchElement.at(i),0);
            if (m!=-1) {
                ok_part[1]=true;
                qso->mult[0]=m;
                multField=i;
                break;
            }
        }
    } else if (withinState && qso->isamult[1]) {
        // MS stations: DXCC also mults; no need to check exchange here, copy last
        // logged string which is not a number
        for (int i=exchElement.size()-1;i>=0;i--) {
            bool ok=false;
            exchElement.at(i).toInt(&ok);
            if (!ok) {
                multField=i;
                ok_part[1]=true;
                break;
            }
        }
    }
    if (ok_part[1]) {
        finalExch[1]=exchElement.at(multField);
    }

    // only copy into log if exchange is validated
    if (ok_part[0] && ok_part[1]) {
        for (int i = 0; i < nExch; i++) {
            qso->rcv_exch[i] = finalExch[i];
        }
    }
    // if exchange is ok and a mobile, we need a mobile dupe check
    if (qso->isMobile && ok_part[0] && ok_part[1]) {
        emit(mobileDupeCheck(qso));
        if (!qso->dupe) {
            emit(clearDupe());
        }
    }
    return(ok_part[0] && ok_part[1]);
}

/*!
  Call to initialize withinState:

  b=true for station in state; false for stations outside
  */
void MSQP::setWithinState(bool b)
{
    withinState=b;
    if (b) {
        // MS stations
        multFieldHighlight[0] = SQL_COL_RCV2; // county, state, province
        multFieldHighlight[1] = SQL_COL_CALL; // for DXCC mults, highlight call
    } else {
        // outside MS
        multFieldHighlight[0] = SQL_COL_RCV2; // county
    }
}
