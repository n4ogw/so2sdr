/*! Copyright 2010-2025 R. Torsten Clay N4OGW

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

#include "contest_wwdigi.h"
#include "hamlib/rotator.h"
#include <math.h>

WWDigi::WWDigi(QSettings &cs, QSettings &ss) : Contest(cs, ss) {
  setVExch(false);
  dupeCheckingEveryBand = true;
  nExch = 1;
  logFieldPrefill = new bool[nExch];
  logFieldPrefill[0] = true;
  prefill = false;
  finalExch = new QByteArray[nExch];
  for (int i = 0; i < nExch; i++)
    finalExch[i].clear();
  exchange_type = new FieldTypes[nExch];
  exchange_type[0] = Grid;
  multFieldHighlight[0] = SQL_COL_RCV1;
}

WWDigi::~WWDigi() {
  delete[] logFieldPrefill;
  delete[] finalExch;
  delete[] exchange_type;
}

void WWDigi::setupContest(QByteArray MultFile[MMAX], const Cty *cty) {
  Q_UNUSED(MultFile)
  Q_UNUSED(cty)
  zeroScore();
}

bool WWDigi::validateExchange(Qso *qso) {
  if (!separateExchange(qso))
    return false;
  qso->bandColumn = qso->band;
  bool ok = true;

  // check to see if this is a valid grid square
  if (exchElement.at(0).size() != 4 || exchElement.at(0).at(0) < 'A' ||
      exchElement.at(0).at(0) > 'R' || exchElement.at(0).at(1) < 'A' ||
      exchElement.at(0).at(1) > 'R' || exchElement.at(0).at(2) < '0' ||
      exchElement.at(0).at(2) > '9' || exchElement.at(0).at(3) < '0' ||
      exchElement.at(0).at(3) > '9') {
    ok = false;
  } else {
    finalExch[0] = exchElement[0];
    qso->rcv_exch[0] = finalExch[0];
    if (exchElement.at(0) == "ZZ00") {
      // grid entered as "ZZ00" for stations not sending a grid. Counts as 1
      // point?
      qso->mult_name.clear();
      qso->isamult[0] = false;
      qso->pts = 1;
    } else {
      qso->isamult[0] = true;
      qso->mult_name = exchElement.at(0).left(2);
    }
  }
  // get index of this grid field mult (if already worked)
  qso->mult[0] = -1;
  multIndx(qso);

  // calculate bearing and distance
  if (myGrid.size()) {
    if (myGrid == qso->rcv_exch[0]) {
      qso->bearing = 0;
      qso->distance = 0; // same grid, give distance as 0
      qso->pts = 1;
    } else {
      double lat, lon;
      locator2longlat(&lon, &lat, qso->rcv_exch[0].data());
      lon *= -1.0;
      double dist, head;
      if (qrb(myLon * -1.0, myLat, lon * -1.0, lat, &dist, &head) == RIG_OK) {
        qso->bearing = qRound(head);
        qso->distance = qRound(dist);
      }
      qso->pts = qso->distance / 3000 + 1;
    }
  }
  return ok;
}

QVariant WWDigi::columnName(int c) const {
  switch (c) {
  case SQL_COL_RCV1:
    return (QVariant("Grid"));
    break;
  }
  return Contest::columnName(c);
}

void WWDigi::addQso(Qso *qso) {
  if (qso->dupe || !qso->valid)
    qso->pts = 0;
  qsoPts += qso->pts;
  addQsoMult(qso);
}

/*! width in characters of data fields shown
 * */
int WWDigi::fieldWidth(int col) const {
  switch (col) {
  case 0: // grid
    return 5;
  default:
    return 4;
  }
}
