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
#include "contest_wpx.h"

/*! CQ WPX contest */
WPX::WPX(QSettings &cs, QSettings &ss) : Contest(cs, ss) {
  setVExch(true);
  setZoneMax(40);
  setZoneType(0);
  dupeCheckingEveryBand = true;
  nExch = 2;
  logFieldPrefill = new bool[nExch];
  logFieldPrefill[0] = true;
  logFieldPrefill[1] = true;
  prefill = false;
  finalExch = new QByteArray[nExch];
  exchange_type = new FieldTypes[nExch];
  exchange_type[0] = RST;
  exchange_type[1] = QsoNumber;
  multFieldHighlight[0] = SQL_COL_CALL; // new prefix: callsign field
  multFieldHighlight[1] = -1;
}

WPX::~WPX() {
  delete[] logFieldPrefill;
  delete[] finalExch;
  delete[] exchange_type;
}

QVariant WPX::columnName(int c) const {
  switch (c) {
  case SQL_COL_SNT2:
    return (QVariant("Sent#"));
    break;
  }
  return Contest::columnName(c);
}

/*! WPX add qso

    // qso point values
    // [0]==1 stations in same country
    // [1]==1 stations in different country but same continent  10/15/20
    // [2]==2 stations in different country but same continent  40/80/160
    // [3]==2 stations in different country but same continent  10/15/20 (NA)
    // [4]==4 stations in different country but same continent  40/80/160 (NA)
    // [5]==3 different continent 10/15/20
    // [6]==6 different continent 40/80/160
 */
void WPX::addQso(Qso *qso) {
  // determine point value
  if (!qso->dupe && qso->valid && qso->band < N_BANDS_HF) {
    if (qso->country == myCountry) {
      qso->pts = 1;
    } else if (qso->continent == myContinent && qso->continent == NA) {
      // 2/4 points within NA
      if (qso->band < 3) {
        qso->pts = 4;
      } else {
        qso->pts = 2;
      }
    } else if (qso->continent == myContinent) {
      // 1/2 points within other continents
      if (qso->band < 3) {
        qso->pts = 2;
      } else {
        qso->pts = 1;
      }
    } else {
      // different continents
      if (qso->band < 3) {
        qso->pts = 6;
      } else {
        qso->pts = 3;
      }
    }
  } else {
    qso->pts = 0;
  }
  qsoPts += qso->pts;
  addQsoMult(qso);
}

/*! width in characters of data fields shown
 * */
int WPX::fieldWidth(int col) const {
  switch (col) {
  case 0: // sent #
    return 5;
  case 1: //  RST
    return 4;
  case 2: // rcv #
    return 5;
  default:
    return 4;
  }
}

int WPX::numberField() const { return 1; }

unsigned int WPX::rcvFieldShown() const {
  return 2; // show second field
}

void WPX::setupContest(QByteArray MultFile[MMAX], const Cty *cty) {
  if (MultFile[0].isEmpty()) {
    MultFile[0] = "wpx";
  }
  if (MultFile[1].isEmpty()) {
    MultFile[1] = "none";
  }
  readMultFile(MultFile, cty);
  zeroScore();
  _nMults[0] = 0;
  _nMults[1] = 0;
}

// 0 1=RST
// 1 2=qso # --> show
unsigned int WPX::sntFieldShown() const {
  return 2; // show sent #
}

bool WPX::validateExchange(Qso *qso) {
  if (!separateExchange(qso))
    return (false);
  qso->isamult[0] = true;
  qso->isamult[1] = false;
  qso->bandColumn = qso->band;

  // RST NR
  bool ok = valExch_rst_nr(qso);

  for (int i = 0; i < nExch; i++) {
    qso->rcv_exch[i] = finalExch[i];
  }

  // determine prefix for mult
  wpxPrefix(qso->call, qso->mult_name);
  return ok;
}

void WPX::wpxPrefix(QByteArray call, QByteArray &pfx)

// determines prefix from call
// may not correctly handle callsigns with two /'s: will assume pfx before FIRST
// / is correct
{
  const char digits[10] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'};
  const int nIgnorePfx = 8;
  const QByteArray ignorePfx[nIgnorePfx] = {"P",  "M",  "QRP", "AE",
                                            "AG", "KT", "MM",  "AM"};
  bool changeDigit = false;
  QByteArray portPfx = "";
  pfx = "";

  // is it a portable?
  if (call.contains("/")) {
    int j = call.indexOf("/");
    QByteArray tmp1 = call.mid(0, j);
    QByteArray tmp2 = call.mid(j + 1, call.length() - j - 1);

    // the longer one is probably the call, the other the port pfx
    // tmp1/tmp2
    if (tmp2.length() > tmp1.length()) {
      portPfx = tmp1;
      call = call.right(call.length() - tmp1.length() - 1);
    } else {
      portPfx = tmp2;
      call.chop(tmp2.length() + 1);
    }

    // certain portable prefixes do not count as new pfx
    bool ignore = false;
    for (int i = 0; i < nIgnorePfx; i++) {
      if (portPfx == ignorePfx[i]) {
        portPfx.clear();
        ignore = true;
        break;
      }
    }
    if (!ignore) {
      // is there a digit in the portable pfx?
      bool isdigit = false;
      for (int i = 0; i < 10; i++) {
        if (portPfx.contains(digits[i])) {
          isdigit = true;
          break;
        }
      }
      if (portPfx.length() == 1) {

        if (isdigit) {
          // if the port Pfx is 1 character and a digit, it will modify the main
          // pfx
          changeDigit = true;
        } else {
          // single character pfx: must add '0' to form pfx
          portPfx = portPfx + "0";
        }
      }
    }
  }

  // is there a digit in the call? Find LAST digit
  int j = -1;
  for (int i = 0; i < 10; i++) {
    for (int k = 0; k < call.length(); k++) {
      if ((call.at(k) == digits[i]) && k > j) {
        j = k;
      }
    }
  }
  if (j != -1) {
    // take up to the last digit in the call
    pfx = call.mid(0, j + 1);
  } else {
    // take first 2 letters and add zero
    pfx = call.mid(0, 2) + "0";
  }

  // apply change from portable digit
  if (changeDigit) {
    pfx.chop(1);
    pfx = pfx + portPfx;
  } else if (portPfx.length() > 1) {
    pfx = portPfx;
  }
}
