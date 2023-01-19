/*! Copyright 2010-2023 R. Torsten Clay N4OGW

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

#include "contest_junevhf.h"
#include "hamlib/rotator.h"
#include <math.h>

JuneVHF::JuneVHF(QSettings &cs, QSettings &ss) : Contest(cs, ss) {
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

JuneVHF::~JuneVHF() {
  delete[] logFieldPrefill;
  delete[] finalExch;
  delete[] exchange_type;
}

void JuneVHF::setupContest(QByteArray MultFile[MMAX], const Cty *cty) {
  Q_UNUSED(MultFile)
  Q_UNUSED(cty)
  zeroScore();
}

/*! ARRL June VHF uses non-default columns; these are the
 * lowest 6 VHF/UHF bands. For higher bands, returns the highest one.
 */
int JuneVHF::highlightBand(int b, ModeTypes modeType) const {
  Q_UNUSED(modeType)

  if (b < BAND6)
    return -1;
  switch (b) {
  case (BAND6):
    return 0;
  case (BAND2):
    return 1;
  case (BAND222):
    return 2;
  case (BAND420):
    return 3;
  case (BAND902):
    return 4;
  case (BAND1240):
    return 5;
  default:
    return 5;
  }
}

/* default labels for bands in score summary */
QString JuneVHF::bandLabel(int i) const {
  switch (i) {
  case 0:
    return "6";
  case 1:
    return "2";
  case 2:
    return "222";
  case 3:
    return "432";
  case 4:
    return "902";
  case 5:
    return "1296";
  default:
    return "";
  }
}

bool JuneVHF::bandLabelEnable(int i) const {
  Q_UNUSED(i)
  return true;
}

int JuneVHF::nMultsColumn(int col, int ii) const {
  switch (col) {
  case 0:
    return multsWorked[ii][CWType][BAND6];
  case 1:
    return multsWorked[ii][CWType][BAND2];
  case 2:
    return multsWorked[ii][CWType][BAND222];
  case 3:
    return multsWorked[ii][CWType][BAND420];
  case 4:
    return multsWorked[ii][CWType][BAND902];
  case 5:
    return multsWorked[ii][CWType][BAND1240];
  default:
    return 0;
  }
}

bool JuneVHF::validateExchange(Qso *qso) {
  if (!separateExchange(qso))
    return false;

  bool ok = true;
  // these bands are displayed on the screen totals
  switch (qso->band) {
  case (BAND6):
    qso->bandColumn = 0;
    qso->isamult[0] = true;
    break;
  case (BAND2):
    qso->bandColumn = 1;
    qso->isamult[0] = true;
    break;
  case (BAND222):
    qso->bandColumn = 2;
    qso->isamult[0] = true;
    break;
  case (BAND420):
    qso->bandColumn = 3;
    qso->isamult[0] = true;
    break;
  case (BAND902):
    qso->bandColumn = 4;
    qso->isamult[0] = true;
    break;
  case (BAND1240):
  case (BAND2200):
  case (BAND3300):
  case (BAND5650):
  case (BAND10000):
  case (BAND24000):
  case (BAND47000):
  case (BAND76000):
  case (BAND122000):
  case (BAND134000):
  case (BAND241000):
    qso->bandColumn = 5;
    qso->isamult[0] = true;
    break;
  default: // not on VHF/UHF
    qso->isamult[0] = false;
    qso->bandColumn = -1;
  }

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
    qso->mult_name = exchElement[0];
  }
  // get index of this grid mult (if already worked)
  qso->mult[0] = -1;
  multIndx(qso);

  // calculate bearing and distance
  if (myGrid.size()) {
    if (myGrid == qso->rcv_exch[0]) {
      qso->bearing = 0;
      qso->distance = 0; // same grid, give distance as 0
    } else {
      double lat, lon;
      locator2longlat(&lon, &lat, qso->rcv_exch[0].data());
      lon *= -1.0;
      double dist, head;
      if (qrb(myLon * -1.0, myLat, lon * -1.0, lat, &dist, &head) == RIG_OK) {
        qso->bearing = qRound(head);
        qso->distance = qRound(dist);
      }
    }
  }
  // if exchange is ok and this is a rover, we need a mobile dupe check
  if (qso->isRover && ok) {
    emit mobileDupeCheck(qso);
    if (!qso->dupe) {
      emit clearDupe();
    }
  }
  return ok;
}

QVariant JuneVHF::columnName(int c) const {
  switch (c) {
  case SQL_COL_RCV1:
    return (QVariant("Grid"));
    break;
  }
  return Contest::columnName(c);
}

void JuneVHF::addQso(Qso *qso) {
  // qso points
  switch (qso->band) {
  case (BAND6):
  case (BAND2):
    qso->pts = 1;
    qso->isamult[0] = true;
    break;
  case (BAND420):
  case (BAND222):
    qso->pts = 2;
    qso->isamult[0] = true;
    break;
  case (BAND902):
  case (BAND1240):
    qso->pts = 3;
    qso->isamult[0] = true;
    break;
  case (BAND2300):
  case (BAND3300):
  case (BAND5650):
  case (BAND10000):
  case (BAND24000):
  case (BAND47000):
  case (BAND76000):
  case (BAND122000):
  case (BAND134000):
  case (BAND241000):
    qso->pts = 4;
    qso->isamult[0] = true;
    break;
  default:
    // wrong band
    qso->pts = 0;
    qso->isamult[0] = false;
  }

  if (qso->dupe || !qso->valid)
    qso->pts = 0;
  qsoPts += qso->pts;
  addQsoMult(qso);
}

/*! width in characters of data fields shown
 * */
int JuneVHF::fieldWidth(int col) const {
  switch (col) {
  case 0: // grid
    return 5;
  default:
    return 4;
  }
}
