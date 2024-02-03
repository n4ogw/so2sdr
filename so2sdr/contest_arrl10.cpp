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
#include "contest_arrl10.h"

/*! ARRL 10m contest */
ARRL10::ARRL10(QSettings &cs, QSettings &ss) : Contest(cs, ss) {
  setZoneMax(0);
  setZoneType(1); // this also chooses ARRL rather than CQ countries
  setVExch(true);
  dupeCheckingEveryBand = true;
  nExch = 2;
  logFieldPrefill = new bool[nExch];
  logFieldPrefill[0] = false;
  logFieldPrefill[1] = true;
  prefill = false;
  finalExch = new QByteArray[nExch];
  exchange_type = new FieldTypes[nExch];
  exchange_type[0] = RST;               // class
  exchange_type[1] = State;             // state
  multFieldHighlight[0] = SQL_COL_RCV2; // state
  multFieldHighlight[1] = SQL_COL_CALL; // highlight call for DX
}

/* default labels for bands in score summary */
QString ARRL10::bandLabel(int i) const {
  switch (i) {
  case 0:
    return "";
    break;
  case 1:
    return "";
    break;
  case 2:
    return "";
    break;
  case 3:
    return "";
    break;
  case 4:
    return "10CW";
    break;
  case 5:
    return "10SSB";
    break;
  default:
    return "";
  }
}

bool ARRL10::bandLabelEnable(int i) const {
  switch (i) {
  case 4:
  case 5:
    return true;
    break;
  default:
    return false;
  }
}

int ARRL10::nMultsColumn(int col, int ii) const {
  switch (col) {
  case 4:
    return multsWorked[ii][CWType][5];
    break;
  case 5:
    return multsWorked[ii][PhoneType][5];
    break;
  default:
    return 0;
    break;
  }
}

void ARRL10::setupContest(QByteArray MultFile[MMAX], const Cty *cty) {
  if (MultFile[0].isEmpty()) {
    MultFile[0] = "arrl10.txt";
  }
  if (MultFile[1].isEmpty()) {
    MultFile[1] = "arrl_country";
  }
  readMultFile(MultFile, cty);
  zeroScore();
}

ARRL10::~ARRL10() {
  delete[] logFieldPrefill;
  delete[] finalExch;
  delete[] exchange_type;
}

void ARRL10::addQso(Qso *qso) {
  // must be on 10m for qso to count
  if (qso->band != BAND10) {
    qso->pts = 0;
    for (int ii = 0; ii < MMAX; ii++) {
      qso->mult[ii] = -1;
      qso->newmult[ii] = -1;
    }
    addQsoMult(qso);
    return;
  }
  // 2 pts phone, 4 pts cw
  switch (qso->mode) {
  case RIG_MODE_CW:
  case RIG_MODE_CWR:
    qso->pts = 4;
    break;
  case RIG_MODE_USB:
  case RIG_MODE_LSB:
  case RIG_MODE_AM:
  case RIG_MODE_FM:
    qso->pts = 2;
    break;
  default:
    qso->pts = 2;
    break;
  }
  if (qso->dupe || !qso->valid)
    qso->pts = 0;
  qsoPts += qso->pts;
  addQsoMult(qso);
}

/*! width in characters of data fields shown
 * */
int ARRL10::fieldWidth(int col) const {
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

/*! ARRL10 uses non-default columns;
 * column 5 = CW qsos
 * column 6 = SSB qsos
 */
int ARRL10::highlightBand(int b, ModeTypes modeType) const {
  if (b == BAND10) {
    switch (modeType) {
    case CWType:
      return BAND15;
      break;
    case PhoneType:
      return BAND10;
      break;
    default:
      return -1;
    }
  } else {
    return -1;
  }
}

/*!
   number shown in column 0 is sequential qso number
 */
int ARRL10::numberField() const { return (-1); }

unsigned int ARRL10::rcvFieldShown() const {
  return (2); // show second exchange field (ST or #)
}

int ARRL10::Score() const {
  return (qsoPts *
          (multsWorked[0][CWType][BAND10] + multsWorked[1][CWType][BAND10] +
           multsWorked[0][PhoneType][BAND10] +
           multsWorked[1][PhoneType][BAND10]));
}

unsigned int ARRL10::sntFieldShown() const {
  return (0); // nothing shown
}

bool ARRL10::validateExchange(Qso *qso) {
  if (!separateExchange(qso))
    return (false);
  fillDefaultRST(qso);
  if (qso->modeType == CWType) {
    qso->bandColumn = BAND15;
  } else if (qso->modeType == PhoneType) {
    qso->bandColumn = BAND10;
  }
  finalExch[1].clear();
  qso->rcv_exch[0].clear();
  qso->rcv_exch[1].clear();
  bool ok = false;

  for (int ii = 0; ii < MMAX; ii++)
    qso->mult[ii] = -1;

  qso->pts = 0;
  determineMultType(qso);

  // mobile ITU regions (RST) R1, (RST) R2, (RST) R3
  // not using validator in contest.cpp since here R1/R2/R3 are
  // multipliers
  if (qso->isMM) {
    for (int i = exchElement.size() - 1; i >= 0; i--) {
      int r = 0;
      if (exchElement[i] == "R1")
        r = 1;
      if (exchElement[i] == "R2")
        r = 2;
      if (exchElement[i] == "R3")
        r = 3;
      if (r) {
        // make this count as "domestic mult" regardless of actual country of
        // callsign
        qso->mult[1] = -1;
        qso->isamult[0] = true;
        break;
      }
    }
  }
  if (qso->isamult[0]) {
    // Domestic call: (RST) state
    ok = valExch_rst_state(0, qso->mult[0], qso);
  } else if (qso->isamult[1]) {
    // DX: (RST) #
    // two things entered; assume first is rst
    int inr = 0;
    if (exchElement.size() == 2) {
      finalExch[0] = exchElement[0];
      inr = 1;
    }

    // serial number; must be convertable to int
    bool iok = false;
    exchElement[inr].toInt(&iok, 10);
    if (iok) {
      ok = true;
      finalExch[1] = exchElement[inr];
    }
  }
  copyFinalExch(ok, qso);
  return (ok);
}

/*!
   Returns unsigned integer worked  with bits set to indicate what bands mult1
   and mult2 are worked on

   ARRL 10M contest: CW mult status in 5th band slot ([4]); SSB mult
   status in 6th band slot ([5])
 */
void ARRL10::workedMults(Qso *qso, unsigned int worked[MMAX]) const {
  for (int ii = 0; ii < settings.value(c_nmulttypes, c_nmulttypes_def).toInt();
       ii++) {
    worked[ii] = 0;
    if (qso->mult[ii] != -1 && qso->mult[ii] < _nMults[ii]) {
      worked[ii] += multWorked[ii][CWType][5][qso->mult[ii]] * bits[4];
      worked[ii] += multWorked[ii][PhoneType][5][qso->mult[ii]] * bits[5];
    }
  }
}

/*!
   Returns unsigned integer worked  with bits set to indicate what bands qso is
   are worked on

   ARRL 10M contest: CW status in 5th band slot ([4]); SSB
   status in 6th band slot ([5])
 */
void ARRL10::workedQso(ModeTypes m, int band, unsigned int &worked) const {
  Q_UNUSED(band)

  switch (m) {
  case CWType:
    worked |= bits[4];
    break;
  case PhoneType:
    worked |= bits[5];
    break;
  default:
    break;
  }
}
