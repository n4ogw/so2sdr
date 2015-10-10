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
#include "contest_paqp.h"
#include "log.h"

QList<QString> PAQP::EPA_counties = QList<QString>()
        << "ADA" << "BER" << "BRA" << "BUX" << "CAR" << "CHE" << "COL"
        << "CUM" << "DAU" << "DCO" << "JUN" << "LAC" << "LAN" << "LEB"
        << "LEH" << "LUZ" << "LYC" << "MGY" << "MOE" << "MTR" << "NHA"
        << "NUM" << "PER" << "PHI" << "PIK" << "SCH" << "SNY" << "SUL"
        << "SUS" << "TIO" << "UNI" << "WAY" << "WYO" << "YOR";

PAQP::PAQP()
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
    exchange_type[1]      = DMult;      // County/Section
    multFieldHighlight[0] = SQL_COL_RCV2; // PA counties
    multFieldHighlight[1] = SQL_COL_RCV2; // Sections
    withinState = true;
}

PAQP::~PAQP()
{
    delete[] logFieldPrefill;
    delete[] finalExch;
    delete[] exchange_type;
}

QVariant PAQP::columnName(int c) const
{
    switch (c) {
    case SQL_COL_RCV1: return QVariant("#");break;
    case SQL_COL_RCV2: return QVariant("Mult");break;
    }
    return Contest::columnName(c);
}

void PAQP::addQso(Qso *qso)

// determine qso point value, increase nqso, update score
// update mult count
{
    if (!qso->dupe && qso->valid && qso->band<N_BANDS_SCORED) {
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

int PAQP::fieldWidth(int col) const

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
int PAQP::numberField() const
{
    return(0);
}

unsigned int PAQP::rcvFieldShown() const

// 0 1=NR --> show
// 1 2=mult --> show
{
    return(1 + 2);  // show 2 fields
}

void PAQP::setupContest(QByteArray MultFile[MMAX], const Cty *cty)
{
    readMultFile(MultFile, cty);
    zeroScore();
}

unsigned int PAQP::sntFieldShown() const

// 0 1=NR   --> show
{
    return(1); // show qso # only
}

/*!
   PAQP exchange validator

   mult 0: PA counties
   mult 1: ARRL/Canadian Sections
 */
bool PAQP::validateExchange(Qso *qso)
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

    // look for section or PA county

    // both PA and non-PA have county mults
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
        // special case: PA stations get the "WPA" or "EPA" mult for PA county worked
        if (withinState) {
            if (ok_part[1]) {
                if (EPA_counties.contains(exchElement.at(multField)))
                       qso->mult[1]=0; // EPA
                else
                       qso->mult[1]=1; // WPA
            }
        }
    }
    // non-PA only works PA
    if (!withinState) {
        if (!ok_part[1]) return(false);
    }

    // PA also gets mults for sections; can also work DX

    // check first for DX:
    if (!qso->isamult[1]) {
        finalExch[1]="DX";
        ok_part[1]=true;
    } else {
        for (int i=exchElement.size()-1;i>=0;i--) {
            // ignore any 'WPA' or 'EPA' entered here
            if (exchElement.at(i)=="EPA" || exchElement.at(i)=="WPA") continue;

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
void PAQP::setWithinState(bool b)
{
    withinState=b;
}
