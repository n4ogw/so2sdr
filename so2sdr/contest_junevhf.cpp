/*! Copyright 2010-2019 R. Torsten Clay N4OGW

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


JuneVHF::JuneVHF(QSettings &cs, QSettings &ss) : Contest(cs,ss)
{
    setVExch(false);
    dupeCheckingEveryBand = true;
    nExch                 = 1;
    logFieldPrefill       = new bool[nExch];
    logFieldPrefill[0]    = true;
    prefill               = false;
    finalExch             = new QByteArray[nExch];
    exchange_type         = new FieldTypes[nExch];
    exchange_type[0]      = Grid;
    multFieldHighlight[0] = SQL_COL_RCV1;
}

void JuneVHF::setupContest(QByteArray MultFile[MMAX], const Cty * cty)
{
    Q_UNUSED(MultFile)
    Q_UNUSED(cty)
    zeroScore();
}

/*! ARRL June VHF uses non-default columns; these are the
 * lowest 6 VHF/UHF bands. For higher bands, returns the highest one.
 */
int JuneVHF::highlightBand(int b,ModeTypes modeType) const
{
    Q_UNUSED(modeType)

    if (b<BAND6) return -1;
    switch (b) {
    case (BAND6): return 0;break;
    case (BAND2): return 1;break;
    case (BAND222): return 2;break;
    case (BAND420): return 3;break;
    case (BAND902): return 4;break;
    case (BAND1240): return 5;break;
    default: return 5;
    }
}

/* default labels for bands in score summary */
QString JuneVHF::bandLabel(int i) const
{
    switch (i) {
    case 0: return "6";break;
    case 1: return "2";break;
    case 2: return "222";break;
    case 3: return "432";break;
    case 4: return "902";break;
    case 5: return "1296";break;
    default: return "";
    }
}

bool JuneVHF::bandLabelEnable(int i) const
{
    Q_UNUSED(i)
    return true;
}

int JuneVHF::nMultsColumn(int col,int ii) const
{
    switch (col) {
    case 0:
        return multsWorked[ii][CWType][BAND6];
        break;
    case 1:
        return multsWorked[ii][CWType][BAND2];
        break;
    case 2:
        return multsWorked[ii][CWType][BAND222];
        break;
    case 3:
        return multsWorked[ii][CWType][BAND420];
        break;
    case 4:
        return multsWorked[ii][CWType][BAND902];
        break;
    case 5:
        return multsWorked[ii][CWType][BAND1240];
        break;
    default:
        return 0;
    }
}


bool JuneVHF::validateExchange(Qso *qso)
{
    if (!separateExchange(qso)) return(false);

    bool ok=true;

    // these bands are displayed on the screen totals
    switch (qso->band) {
    case (BAND6): qso->bandColumn=0;break;
    case (BAND2): qso->bandColumn=1;break;
    case (BAND222): qso->bandColumn=2;break;
    case (BAND420): qso->bandColumn=3;break;
    case (BAND902): qso->bandColumn=4;break;
    case (BAND1240): qso->bandColumn=5;break;
    }
    if (qso->band>BAND1240) qso->bandColumn=5;
    qso->isamult[0]=true;

    // check to see if this is a valid grid square
    if (exchElement.at(0).size() != 4 || exchElement.at(0).at(0) < 'A' || exchElement.at(0).at(0) > 'R' ||
        exchElement.at(0).at(1) < 'A' || exchElement.at(0).at(1) > 'R' ||
        exchElement.at(0).at(2) < '0' || exchElement.at(0).at(2) > '9' ||
        exchElement.at(0).at(3) < '0' || exchElement.at(0).at(3) > '9') {
        ok = false;
    } else {
        finalExch[0]     = exchElement[0];
        qso->rcv_exch[0] = finalExch[0];
        qso->mult_name = exchElement[0];
    }
    // get index of this grid mult (if already worked)
    qso->mult[0]=-1;
    multIndx(qso);

    // calculate bearing and distance
    if (myGrid.size()) {
        if (myGrid==qso->rcv_exch[0]) {
            qso->bearing=0;
            qso->distance=0; // same grid, give distance as 0
        } else {
            double lat, lon;
            locator2longlat(&lon, &lat, qso->rcv_exch[0].data());
            lon *= -1.0;
            double dist, head;
            if (qrb(myLon * -1.0, myLat, lon * -1.0, lat, &dist, &head) == RIG_OK) {
                qso->bearing=qRound(head);
                qso->distance=qRound(dist);
            }
        }
    }
    // if exchange is ok and this is a rover, we need a mobile dupe check
    if (qso->isRover && ok) {
        emit(mobileDupeCheck(qso));
        if (!qso->dupe) {
            emit(clearDupe());
        }
    }
    return(ok);
}

QVariant JuneVHF::columnName(int c) const
{
    switch (c) {
    case SQL_COL_RCV1:return(QVariant("Grid"));break;
    }
    return Contest::columnName(c);
}

void JuneVHF::addQso(Qso *qso)
{
    // not on VHF/UHF, does not count
    if (qso->band < BAND6 ) {
        qso->pts = 0;
        addQsoMult(qso);
        return;
    }
    // qso points
    switch (qso->band) {
    case (BAND6): case (BAND2):
        qso->pts=1;
        break;
    case (BAND420): case (BAND222):
        qso->pts=2;
        break;
    case (BAND902): case (BAND1240):
        qso->pts=3;
        break;
    default:
        qso->pts=4;
    }

    if (qso->dupe || !qso->valid) qso->pts = 0;
    qsoPts += qso->pts;
    qso->isamult[0] = true;
    addQsoMult(qso);
}

/*! width in characters of data fields shown
 * */
int JuneVHF::fieldWidth(int col) const
{
    switch (col) {
    case 0: // grid
        return(5);
        break;
    default:
        return 4;
    }
}
