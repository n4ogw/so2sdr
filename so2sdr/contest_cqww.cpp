/*! Copyright 2010-2024 R. Torsten Clay N4OGW

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
#include "contest_cqww.h"

/*! CQ WW contest */
CQWW::CQWW(QSettings &cs, QSettings &ss) : Contest(cs, ss) {
  setZoneMax(40);
  setZoneType(0);
  setVExch(true);
  dupeCheckingEveryBand = true;
  nExch = 2;
  logFieldPrefill = new bool[nExch];
  logFieldPrefill[0] = false;
  logFieldPrefill[1] = true;
  prefill = true;
  finalExch = new QByteArray[nExch];
  exchange_type = new FieldTypes[nExch];
  exchange_type[0] = RST;               // RST
  exchange_type[1] = Zone;              // zone
  multFieldHighlight[0] = SQL_COL_CALL; // new country: highlight call field
  multFieldHighlight[1] = SQL_COL_RCV2; // new zone: highlight zone column
}

CQWW::~CQWW() {
  delete[] logFieldPrefill;
  delete[] finalExch;
  delete[] exchange_type;
}

/*! CQWW addqso

   same country = 0 points
   [0]==1 stations in same continent
   [1]==2 stations within North America
   [2]==3 stations on different continents

   */
void CQWW::addQso(Qso *qso) {
  qso->pts = 0;
  for (int ii = 0; ii < MMAX; ii++)
    qso->newmult[ii] = -1;

  if (qso->isMM) {
    qso->pts = 3;
  } else if (qso->country == myCountry) {
    qso->pts = 0;
  } else if (myContinent == NA && qso->continent == NA) {
    qso->pts = 2;
  } else if (qso->continent == myContinent) {
    qso->pts = 1;
  } else {
    qso->pts = 3;
  }
  if (qso->dupe || !qso->valid || qso->band >= N_BANDS_HF)
    qso->pts = 0;
  qsoPts += qso->pts;
  addQsoMult(qso);
}

/*! width in characters of data fields shown
 * */
int CQWW::fieldWidth(int col) const {
  switch (col) {
  case 0: // RST
    return (4);
    break;
  case 1: // QTH/zone
    return (5);
    break;
  default:
    return (4);
  }
}

int CQWW::numberField() const { return (-1); }

unsigned int CQWW::rcvFieldShown() const {
  return (2); // show second field
}

void CQWW::setupContest(QByteArray MultFile[2], const Cty *cty) {
  if (MultFile[0].isEmpty()) {
    MultFile[0] = "cq_country";
  }
  if (MultFile[1].isEmpty()) {
    MultFile[1] = "cq_zone";
  }
  readMultFile(MultFile, cty);
  zeroScore();
}

unsigned int CQWW::sntFieldShown() const {
  return (0); // show no sent fields
}

bool CQWW::validateExchange(Qso *qso)

// mult1=CQ Country
// mult2=CQ zone
// exchange same for all : RST zone
{
  if (!separateExchange(qso))
    return (false);
  qso->bandColumn = qso->band;
  for (int ii = 0; ii < MMAX; ii++)
    qso->mult[ii] = -1;

  bool ok = false;

  fillDefaultRST(qso);
  // CW : look for non-default RST- will have three digits
  // Phone is trickier : no way to find a non-default RST. Just assume 59
  if (qso->modeType == CWType || qso->modeType == DigiType) {
    for (int i = exchElement.size() - 1; i >= 0; i--) {
      if (exchElement.at(i).size() == 3) {
        finalExch[0] = exchElement.at(i);
        break;
      }
    }
  }
  // take last non-three digit number as zone
  for (int i = exchElement.size() - 1; i >= 0; i--) {
    if (exchElement.at(i).size() != 3) {
      finalExch[1] = exchElement.at(i);
      break;
    }
  }

  int zone = finalExch[1].toInt(&ok);
  if (ok) {
    if (zone > 0 && zone < 41) {
      qso->zone = zone;
      qso->mult_name = finalExch[1];
      determineMultType(qso);
      // override for marine mobile stations: zone credit only
      if (qso->isMM) {
        qso->mult[0] = -1;
        qso->isamult[0] = false;
      }
      copyFinalExch(ok, qso);
    } else {
      ok = false;
    }
  }
  for (int i = 0; i < nExch; i++)
    finalExch[i].clear();
  return (ok);
}
