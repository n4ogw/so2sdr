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
#include "contest_cwops.h"

/*! CW Ops mini-contest */
Cwops::Cwops(QSettings &cs, QSettings &ss) : Contest(cs, ss) {
  setVExch(false);
  dupeCheckingEveryBand = true;
  nExch = 2;
  logFieldPrefill = new bool[nExch];
  logFieldPrefill[0] = true;
  logFieldPrefill[1] = true;
  prefill = false;
  finalExch = new QByteArray[nExch];
  exchange_type = new FieldTypes[nExch];
  exchange_type[0] = Name;    // name
  exchange_type[1] = General; // Member # or State/QTH
  multFieldHighlight[0] = -1;
  multFieldHighlight[1] = -1;
}

Cwops::~Cwops() {
  delete[] logFieldPrefill;
  delete[] finalExch;
  delete[] exchange_type;
}

void Cwops::addQso(Qso *qso) {
  if (!qso->dupe && qso->valid && qso->band < N_BANDS_HF) {
    qso->pts = 1;
  } else {
    qso->pts = 0;
  }
  qsoPts += qso->pts;
  addQsoMult(qso);
}

/*! width in characters of data fields shown
 * */
int Cwops::fieldWidth(int col) const {
  switch (col) {
  case 0: // name
    return 8;
  case 1: // QTH
    return 4;
  default:
    return 4;
  }
}

/*!
   Number shown in column 0 is sequential
 */
int Cwops::numberField() const { return -1; }

unsigned int Cwops::rcvFieldShown() const

// 0 1=NAME --> show
// 1 2=#/QTH --> show
{
  return (1 + 2);
}

void Cwops::setupContest(QByteArray MultFile[MMAX], const Cty *cty) {
  if (MultFile[0].isEmpty()) {
    MultFile[0] = "uniques";
  }
  readMultFile(MultFile, cty);
  zeroScore();
}

bool Cwops::validateExchange(Qso *qso) {
  if (!separateExchange(qso))
    return (false);
  qso->bandColumn = qso->band;
  for (int ii = 0; ii < MMAX; ii++)
    qso->mult[ii] = -1;

  determineMultType(qso);

  // need exactly two pieces (NAME,NR/QTH) in exchange
  if (exchElement.size() != 2)
    return (false);

  // no validation, just take two elements as logged
  qso->rcv_exch[0] = exchElement[0];
  qso->rcv_exch[1] = exchElement[1];
  return (true);
}
