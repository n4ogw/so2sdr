/*! Copyright 2010-2022 R. Torsten Clay N4OGW

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
#include "contest_stew.h"
#include "hamlib/rotator.h"
#include <math.h>

/*! Stew Perry topband challenge */
Stew::Stew(QSettings &cs, QSettings &ss) : Contest(cs,ss)
{
    setVExch(false);
    dupeCheckingEveryBand = true;
    nExch                 = 1;
    logFieldPrefill       = new bool[nExch];
    logFieldPrefill[0]    = true;
    prefill               = false;
    finalExch             = new QByteArray[nExch];
    exchange_type         = new FieldTypes[nExch];
    exchange_type[0]      = Grid; // grid square
    multFieldHighlight[0] = -1;   // no mults to highlight
    multFieldHighlight[1] = -1;
}

Stew::~Stew()
{
    delete[] logFieldPrefill;
    delete[] finalExch;
    delete[] exchange_type;
}

/*! determines qso points based on grid squares
 */
void Stew::addQso(Qso *qso)
{
    if (qso->band != BAND160) {
        qso->pts = 0;
        addQsoMult(qso);
        return; // not on 160, does not count
    }

    // qso points
    if (myGrid.size()) {
        double lat, lon;
        locator2longlat(&lon, &lat, qso->rcv_exch[0].data());
        lon *= -1.0;
        double dist, head;
        if (qrb(myLon * -1.0, myLat, lon * -1.0, lat, &dist, &head) != RIG_OK) {
            dist = 0.0;
        }
        qso->pts = ceil(dist / 500.0);
        if (qso->pts <= 0) qso->pts = 1;
    } else {
        // grid entered incorrectly
        qso->pts = 1;
    }
    if (qso->dupe || !qso->valid) qso->pts = 0;
    qsoPts += qso->pts;
    addQsoMult(qso);
}

/*! width in characters of data fields shown
 * */
int Stew::fieldWidth(int col) const
{
    switch (col) {
    case 0: // grid
        return 5;
    default:
        return 5;
    }
}

int Stew::numberField() const
{
    return -1;
}

QByteArray Stew::prefillExchange(Qso *qso)
{
    Q_UNUSED(qso)

    // nothing prefilled
    return("");
}

unsigned int Stew::rcvFieldShown() const
{
    return 1;
}

/*!
   only count qso's on 160m in score. No mults, just qso points
 */
int Stew::Score() const
{
    return(qsoPts);
}

void Stew::setupContest(QByteArray MultFile[MMAX], const Cty *cty)
{
    Q_UNUSED(MultFile)
    Q_UNUSED(cty)
    zeroScore();
}

/*! no mults, any 4-char grid is acceptable for exchange
 */
bool Stew::validateExchange(Qso *qso)
{
    if (!separateExchange(qso)) return(false);

    bool ok = true;
    const int idx = exchElement.size()-1;
    qso->bandColumn=qso->band;

    // check to see if the last entered is a valid grid square
    if (exchElement.at(idx).size() != 4 || exchElement.at(idx).at(0) < 'A' || exchElement.at(idx).at(0) > 'R' ||
        exchElement.at(idx).at(1) < 'A' || exchElement.at(idx).at(1) > 'R' ||
        exchElement.at(idx).at(2) < '0' || exchElement.at(idx).at(2) > '9' ||
        exchElement.at(idx).at(3) < '0' || exchElement.at(idx).at(3) > '9') {
        ok = false;
    } else {
        finalExch[0]     = exchElement[idx];
        qso->rcv_exch[0] = finalExch[0];
    }
    return ok;
}

