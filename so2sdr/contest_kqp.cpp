/*! Copyright 2010-2016 R. Torsten Clay N4OGW

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
#include "contest_kqp.h"
#include "log.h"

/*! Kansas QSO Party */
KQP::KQP()
{
    setVExch(true);
    dupeCheckingEveryBand = true;
    nExch                 = 2;
    logFieldPrefill       = new bool[nExch];
    for (int i = 0; i < nExch; i++) logFieldPrefill[i] = true;
    prefill               = false;
    finalExch             = new QByteArray[nExch];
    exchange_type         = new FieldTypes[nExch];
    exchange_type[0]      = RST;
    exchange_type[1]      = DMult;
    multFieldHighlight[0] = -1;
    multFieldHighlight[1] = SQL_COL_RCV2; // kqp_ks.txt
    withinState = true;
}

KQP::~KQP()
{
    delete[] logFieldPrefill;
    delete[] finalExch;
    delete[] exchange_type;
}

void KQP::addQso(Qso *qso)

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

int KQP::fieldWidth(int col) const

// width in pixels of data fields shown
{
    switch (col) {
    case 0: return(37); break; // rcv RST
    case 1: return(45); break; // mult
    default: return(37);
    }
}

/*!
   no qso number
 */
int KQP::numberField() const
{
    return(-1);
}

unsigned int KQP::rcvFieldShown() const

// 0 1=RST --> show
// 1 2=mult --> show
{
    return(1 + 2);  // show 2 fields
}

void KQP::setupContest(QByteArray MultFile[MMAX], const Cty *cty)
{
    readMultFile(MultFile, cty);
    zeroScore();
}

unsigned int KQP::sntFieldShown() const

// 0 1=NR   --> show
{
    return(1); // show qso # only
}

/*!
   KQP exchange validator
 */
bool KQP::validateExchange(Qso *qso)
{
    if (!separateExchange(qso)) return(false);

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

    // look for state or CA county

    // both KS and non-KS can work stations with county mults
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
        // special case: KS stations get the "KS" mult for any KS county worked
        if (withinState) {
            if (ok_part[1]) {
                qso->isamult[1]=true;
               // qso->isamult[0]=false;
                qso->mult[1]=0;
               // qso->mult[0]=-1; // don't count as a county mult
                qso->newmult[0]=false;
            }
        }
    }
    // non-KS only works KS
    if (withinState) {
        // KS station mults
        // check first for DX:
        for (int i=exchElement.size()-1;i>=0;i--) {
            // ignore any 'KS' entered here
            if (exchElement.at(i)=="KS") continue;

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

        // only copy into log if exchange is validated
        if (ok_part[0] && ok_part[1]) {
            for (int i = 0; i < nExch; i++) {
                qso->rcv_exch[i] = finalExch[i];
            }
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
void KQP::setWithinState(bool b)
{
    withinState=b;
}
