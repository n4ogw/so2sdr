/*! Copyright 2010-2020 R. Torsten Clay N4OGW

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
#include "contest_iaru.h"
#include "log.h"

/*! IARU contest */
IARU::IARU(QSettings &cs, QSettings &ss) : Contest(cs,ss)
{
    setZoneMax(90);
    setZoneType(1);
    setVExch(true);
    dupeCheckingEveryBand = true;
    nExch                 = 2;
    logFieldPrefill       = new bool[nExch];
    logFieldPrefill[0]    = false;
    logFieldPrefill[1]    = true;
    prefill               = true;
    finalExch             = new QByteArray[nExch];
    exchange_type         = new FieldTypes[nExch];
    exchange_type[0]      = RST;
    exchange_type[1]      = Zone;
    multFieldHighlight[0] = SQL_COL_RCV2;  // zone field
    multFieldHighlight[1] = SQL_COL_RCV2;  // new HQ
}

IARU::~IARU()
{
    delete[] logFieldPrefill;
    delete[] finalExch;
    delete[] exchange_type;
}

/*!
   returns total number of mults worked
   this overrides the function in Contest
*/
int IARU::nMultsWorked() const
{
    int n = 0;
    for (int ii = 0; ii < MMAX; ii++) {
        for (int k=0;k<NModeTypes;k++) {
            for (int i = 0; i < N_BANDS_HF; i++) {
                n += multsWorked[ii][k][i];
            }
        }
    }
    return n;
}


/*! IARU add qso

     [0]==2 stations in same country
     [1]==5 stations in different country but same continent, HQ or IARU officials
     [2]=10 stations in different continents
 */
void IARU::addQso(Qso *qso)
{
    if (!qso->dupe && qso->valid && qso->band<N_BANDS_HF) {
        if (qso->zone == _myZone || qso->isamult[1]) {
            // same zone, HQ, IARU official : 1 pt
            qso->pts = 1;
        } else if (qso->continent == myContinent) {
            qso->pts = 3;
        } else {
            qso->pts = 5;
        }
    } else {
        qso->pts = 0;
    }
    qsoPts+=qso->pts;
    addQsoMult(qso);
}

/*! width in characters of data fields shown
 * */
int IARU::fieldWidth(int col) const
{
    switch (col) {
    case 0: // RST
        return 4;
    case 1: //  zone/HQ
        return 5;
    default:
        return 4;
    }
}

int IARU::numberField() const
{
    return -1;
}

unsigned int IARU::rcvFieldShown() const
{
    return 2;  // show second field
}

void IARU::setupContest(QByteArray MultFile[MMAX], const Cty *cty)
{
    zeroScore();
    if (MultFile[0].isEmpty()) {
        MultFile[0] = "itu_zone";
    }
    if (MultFile[1].isEmpty()) {
        MultFile[1] = "iaru.txt";
    }
    _nMults[0] = 90;
    readMultFile(MultFile, cty);
}

bool IARU::validateExchange(Qso *qso)
{
    if (!separateExchange(qso)) return(false);
    bool ok = false;
    qso->bandColumn=qso->band;
    for (int ii = 0; ii < MMAX; ii++) qso->mult[ii] = -1;

    determineMultType(qso);

    // auto-fill rst
    if (qso->modeType == CWType || qso->modeType == DigiType) {
        finalExch[0] = "599";
    } else {
        finalExch[0] = "59";
    }
    // is it a HQ or official?
    qso->isamult[1] = valExch_rst_state(1, qso->mult[1], qso);

    if (qso->isamult[1]) {
        // HQ's don't count for zone mult
        qso->isamult[0] = false;
        qso->mult[0]    = -1;
        qso->zone       = 0;
    } else {
        // not a HQ : (RST) + zone
        // CW : look for non-default RST- will have three digits
        // Phone is trickier : no way to find a non-default RST. Just assume 59
        if (qso->modeType == CWType || qso->modeType == DigiType) {
            for (int i=exchElement.size()-1;i>=0;i--) {
                if (exchElement.at(i).size()==3) {
                    finalExch[0] = exchElement.at(i);
                    break;
                }
            }
        }
        // take last non-three digit number as zone
        for (int i=exchElement.size()-1;i>=0;i--) {
            if (exchElement.at(i).size()!=3) {
                finalExch[1] = exchElement.at(i);
                break;
            }
        }
        // parse zone. ok is true if conversion to integer succeeds
        qso->zone = finalExch[1].toInt(&ok, 10);
        if (ok && qso->zone > 0 && qso->zone <= zoneMax()) {
            qso->isamult[0] = true;
            qso->mult[0]    = qso->zone - 1; // important: update zone with what was logged!
        } else {
            qso->isamult[0] = false;
            qso->zone       = 0;
            qso->mult[0]    = -1;
            qso->continent  = ALL;
        }
    }
    qso->mult_name = finalExch[1];
    ok             = qso->isamult[0] || qso->isamult[1];

    for (int i = 0; i < nExch; i++) {
        qso->rcv_exch[i] = finalExch[i];
    }

    // dupes get zero pts
    if (qso->dupe) {
        qso->pts = 0;
        return ok;
    }

    // unknown mults: assign 1 point; it may be a HQ station not on list
    if (qso->isamult[0] == false && qso->isamult[1] == false) {
        qso->pts = 1;
        return(ok);
    }
    if (qso->zone == _myZone || qso->isamult[1]) {
        // same zone, HQ, IARU official : 1 pt
        qso->pts = 1;
    } else if (qso->continent == myContinent) {
        qso->pts = 3;
    } else {
        qso->pts = 5;
    }
    for (int i=0;i<nExch;i++) finalExch[i].clear();
    return ok;
}
