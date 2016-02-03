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
#include "contest_dxped.h"
#include "log.h"

/*! General QSO logger
   //
   // simply records what is entered in the exchange line
   //
   // tracks DXCC and CQ zones
 */
Dxped::Dxped()
{
    setVExch(true);
    dupeCheckingEveryBand = true;
    nExch                 = 2;
    logFieldPrefill       = new bool[nExch];
    logFieldPrefill[0]    = true;
    logFieldPrefill[1]    = false;
    prefill               = true;
    finalExch             = new QByteArray[nExch];
    exchange_type         = new FieldTypes[nExch];
    exchange_type[0]      = RST;          // RST
    exchange_type[1]      = General;      // comment
    multFieldHighlight[0] = SQL_COL_CALL; // new country: highlight call field
    multFieldHighlight[1] = -1;
}

Dxped::~Dxped()
{
    delete[] logFieldPrefill;
    delete[] finalExch;
    delete[] exchange_type;
}

QByteArray Dxped::prefillExchange(Qso *qso)
{
    if (qso->mode == RIG_MODE_CW || qso->mode == RIG_MODE_CWR ||
            qso->mode == RIG_MODE_RTTY || qso->mode == RIG_MODE_RTTYR) {
        return "599";
    } else {
        return "59";
    }
}

void Dxped::addQso(Qso *qso)
{
    // make each qso 1 pt in order to count qso's
    qso->pts = 1;
    addQsoMult(qso);
}

int Dxped::fieldWidth(int col) const

// width in pixels of data fields shown
{
    switch (col) {
    case 0:
        return 35;  // RST
        break;
    case 1:
        return 120; // comment
        break;
    default:
        return 35;
    }
}

int Dxped::numberField() const
{
    return(-1);
}

unsigned int Dxped::rcvFieldShown() const

// show RST + comment
{
    return(3);
}

int Dxped::Score() const
{
    return(0);
}

void Dxped::setupContest(QByteArray MultFile[2], const Cty *cty)
{
    if (MultFile[0].isEmpty()) {
        MultFile[0] = "arrl_country";
    }
    readMultFile(MultFile, cty);
    zeroScore();
}

unsigned int Dxped::sntFieldShown() const
{
    return(0);  // show no sent fields
}

bool Dxped::validateExchange(Qso *qso)
{
    Q_UNUSED(qso);
    separateExchange(qso);
    determineMultType(qso);

    // auto-fill RST 599/59
    if (qso->mode == RIG_MODE_CW || qso->mode == RIG_MODE_CWR ||
        qso->mode == RIG_MODE_RTTY || qso->mode == RIG_MODE_RTTYR) {
        finalExch[0] = "599";
    } else {
        finalExch[0] = "59";
    }
    finalExch[1] = "";
    if (exchElement.isEmpty()) {
        copyFinalExch(true, qso);
        return(true);
    }

    // check to see if first element is a number, assume this is RST
    bool ok  = false;
    int  rst = exchElement[0].toInt(&ok, 10);
    if (ok && (rst >= 111 && rst <= 599)) {
        finalExch[0] = exchElement[0];
    } else {
        finalExch[1] = exchElement[0];
    }

    // add any other stuff entered
    for (int i = 1; i < exchElement.size(); i++) {
        finalExch[1] = finalExch[1] + " " + exchElement[i];
    }
    copyFinalExch(true, qso);

    return(true);
}
