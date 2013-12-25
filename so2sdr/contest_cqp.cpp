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
#include "contest_cqp.h"
#include "log.h"

/*! California QSO Party */
CQP::CQP()
{
    setVExch(true);
    dupeCheckingEveryBand = true;
    nExch                 = 2;
    logFieldPrefill       = new bool[nExch];
    for (int i = 0; i < nExch; i++) logFieldPrefill[i] = true;
    prefill               = false;
    finalExch             = new QByteArray[nExch];
    exchange_type         = new FieldTypes[nExch];
    exchange_type[0]      = QsoNumber;    // qso number
    exchange_type[1]      = DMult;      // County/State
    multFieldHighlight[0] = SQL_COL_RCV2; // CA counties
    multFieldHighlight[1] = SQL_COL_RCV2; // states
    withinState = true;
}

CQP::~CQP()
{
    delete[] logFieldPrefill;
    delete[] finalExch;
    delete[] exchange_type;
}

void CQP::addQso(Qso *qso)

// determine qso point value, increase nqso, update score
// update mult count
{
    if (!qso->dupe && qso->valid && qso->band<N_BANDS_SCORED) {
        if (qso->modeType==CWType) {
            qso->pts = 3;
        } else if (qso->modeType==PhoneType) {
            qso->pts = 2;
        }
    } else {
        qso->pts = 0;
    }
    qsoPts += qso->pts;
    addQsoMult(qso);
}

int CQP::fieldWidth(int col) const

// width in pixels of data fields shown
{
    switch (col) {
    case 0: return(37); break; // sent #
    case 1: return(37); break; // rcv #
    case 2: return(45); break; // mult
    default: return(35);
    }
}

/*!
   Number shown in column 0 is the number sent
 */
int CQP::numberField() const
{
    return(0);
}

unsigned int CQP::rcvFieldShown() const

// 0 1=NR --> show
// 1 2=mult --> show
{
    return(1 + 2);  // show 2 fields
}

void CQP::setupContest(QByteArray MultFile[MMAX], const Cty *cty)
{
    readMultFile(MultFile, cty);
    zeroScore();
}

unsigned int CQP::sntFieldShown() const

// 0 1=NR   --> show
{
    return(1); // show qso # only
}

/*!
   CQP exchange validator

   mult 0: CA counties
   mult 1: states/prov
 */
bool CQP::validateExchange(Qso *qso)
{
    if (!separateExchange(qso)) return(false);

    for (int ii = 0; ii < MMAX; ii++) qso->mult[ii] = -1;

    // check prefix
    determineMultType(qso);

    // any number must be qso number; take last one entered
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
    }

    // look for state or CA county

    // both CA and non-CA have county mults
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
        // special case: CA stations get the "CA" mult for any CA county worked
        if (withinState) {
            if (ok_part[1]) {
                qso->mult[1]=0;
            }
        }
    }
    // non-CA only works CA
    if (!withinState) {
        if (!ok_part[1]) return(false);
    }

    // CA also gets mults for states; can also work DX

    // check first for DX:
    if (!qso->isamult[1]) {
        finalExch[1]="DX";
        ok_part[1]=true;
    } else {
        for (int i=exchElement.size()-1;i>=0;i--) {
            // ignore any 'CA' entered here
            if (exchElement.at(i)=="CA") continue;

            m=isAMult(exchElement.at(i),1);
            if (m!=-1) {
                ok_part[1]=true;
                qso->mult[1]=m;
                multField=i;
            break;
            }
        }
        if (ok_part[1]) {
            finalExch[1]=exchElement.at(multField);
        }
    }

    // only copy into log if exchange is validated
    if (ok_part[0] & ok_part[1]) {
        for (int i = 0; i < nExch; i++) {
            qso->rcv_exch[i] = finalExch[i];
        }
    }
    // if exchange is ok and a mobile, we need a mobile dupe check
    if (qso->isMobile & ok_part[0] & ok_part[1]) {
        emit(mobileDupeCheck(qso));
        if (!qso->dupe) {
            emit(clearDupe());
        }
    }
    return(ok_part[0] & ok_part[1]);
}

/*!
  Call to initialize withinState:

  b=true for station in state; false for stations outside
  */
void CQP::setWithinState(bool b)
{
    withinState=b;
}
