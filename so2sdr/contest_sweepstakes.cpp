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
#include "contest_sweepstakes.h"
#include "log.h"

/*! ARRL Sweepstakes */
Sweepstakes::Sweepstakes(QSettings &cs, QSettings &ss) : Contest(cs,ss)
{
    setVExch(true);
    dupeCheckingEveryBand = false;
    nExch                 = 4;
    logFieldPrefill       = new bool[nExch];
    for (int i = 0; i < nExch; i++) logFieldPrefill[i] = true;
    prefill               = false;
    finalExch             = new QByteArray[nExch];
    exchange_type         = new FieldTypes[nExch];
    exchange_type[0]      = QsoNumber;    // qso number
    exchange_type[1]      = General;      // prec
    exchange_type[2]      = Number;       // check
    exchange_type[3]      = ARRLSection;  // ARRL section
    multFieldHighlight[0] = SQL_COL_RCV4; // domestic mult field
    multFieldHighlight[1] = -1;
}

Sweepstakes::~Sweepstakes()
{
    delete[] logFieldPrefill;
    delete[] finalExch;
    delete[] exchange_type;
}

QVariant Sweepstakes::columnName(int c) const
{
    switch (c) {
    case SQL_COL_RCV1:return(QVariant("#"));break;
    case SQL_COL_RCV2:return(QVariant("Pr"));break;
    case SQL_COL_RCV3:return(QVariant("Ck"));break;
    }
    return Contest::columnName(c);
}

void Sweepstakes::addQso(Qso *qso)

// determine qso point value, increase nqso, update score
// update mult count
{
    if (!qso->dupe && qso->valid && qso->band<N_BANDS_HF) {
        qso->pts = 2;
    } else {
        qso->pts = 0;
    }
    qsoPts += qso->pts;
    addQsoMult(qso);
}

/*! width in characters of data fields shown
 * */
int Sweepstakes::fieldWidth(int col) const
{
    switch (col) {
    case 0: // sent #
        return(4);
        break;
    case 1: // rcv #
        return(4);
        break;
    case 2: // prec
        return(3);
        break;
    case 3: // check
        return(3);
        break;
    case 4: // section
        return(4);
        break;
    default:
        return(4);
    }
}

/*!
   Number shown in column 0 is the number sent
 */
int Sweepstakes::numberField() const
{
    return(0);
}

unsigned int Sweepstakes::rcvFieldShown() const

// 0 1=NR --> show
// 1 2=prec --> show
// 2 4=check --> show
// 3 8=section --> show
{
    return(1 + 2 + 4 + 8);  // show 4 fields
}

void Sweepstakes::setupContest(QByteArray MultFile[MMAX], const Cty *cty)
{
    if (MultFile[0].isEmpty()) {
        MultFile[0] = "arrl.txt";
    }
    readMultFile(MultFile, cty);
    zeroScore();
}

unsigned int Sweepstakes::sntFieldShown() const

// 0 1=NR   --> show
{
    return(1); // show qso # only
}

/*!
   sweepstakes exchange validator
 */
bool Sweepstakes::validateExchange(Qso *qso)
{
    if (!separateExchange(qso)) return(false);

    qso->bandColumn=qso->band;
    for (int ii = 0; ii < MMAX; ii++) qso->mult[ii] = -1;

    // check prefix
    determineMultType(qso);

    bool ok_part[4];
    ok_part[0]   = false; // qso #
    ok_part[1]   = false; // prec
    ok_part[2]   = false; // check
    ok_part[3]   = false; // section

    qso->mult[1] = -1;
    // don't check non-US/VE calls
    if (qso->isamult[0]) {
        // # PREC CK SECTION; 4 elements minimum
        // could have #PREC  together; 3 elements minimum
        if (exchElement.size() < 3) return(false);

        bool *used = new bool[exchElement.size()];
        for (int i = 0; i < exchElement.size(); i++) used[i] = false;

        // take last entered section name
        for (int i = exchElement.size() - 1; i >= 0; i--) {
            int m;
            if ((m = isAMult(exchElement.at(i), 0)) != -1) {
                if (!ok_part[3]) {
                    finalExch[3] = exchElement.at(i);
                    qso->mult[0] = m;
                    ok_part[3]   = true;
                    used[i]      = true;
                } else {
                    used[i] = true; // mark other entries matching section as used
                }
            }
        }

        // look for precedence attached to qso number (eq 213A). If so,
        // move precedence to a separate array element in exchElement

        // start search from end of entered data
        for (int i = exchElement.size() - 1; i >= 0; i--) {
            QByteArray last = exchElement.at(i);
            last = last.right(1);
            if (last == "Q" || last == "A" || last == "B" ||
                last == "U" || last == "M" || last == "S") {
                QByteArray tmp = exchElement.at(i);
                tmp.chop(1);
                if (!tmp.isEmpty()) {
                    bool ok = false;
                    int nr=tmp.toInt(&ok, 10);
                    Q_UNUSED(nr);
                    if (ok) {
                        used[i]=true;
                        if (!ok_part[0]) {
                            finalExch[0]=tmp;
                            ok_part[0]=true;
                        }
                        if (!ok_part[1]) {
                            ok_part[1]=true;
                            finalExch[1]=last;
                        }
                    }
                } else if (exchElement.at(i).size()==1) {
                    // matched just prec
                    used[i]=true;
                    if (!ok_part[1]) {
                        ok_part[1]=true;
                        finalExch[1]=last;
                    }
                }
            }
        }

        // qso number: take last number which is not 2 digits or is a single digit
        for (int i = exchElement.size() - 1; i >= 0; i--) {
            if (used[i]) continue;
            bool nrok = false;
            int  nr   = exchElement.at(i).toInt(&nrok, 10);
            if (nrok && nr > 99) {
                // >99: QSO number
                finalExch[0] = exchElement.at(i);
                used[i]      = true;
                ok_part[0]   = true;
                break;
            } else if (nrok && nr<10 && (exchElement.at(i).size()==1)) {
                // single digit: must also be qso number
                finalExch[0] = exchElement.at(i);
                used[i]      = true;
                ok_part[0]   = true;
                break;
            }
        }
        // check: take last two-digit number
        for (int i = exchElement.size() - 1; i >= 0; i--) {
            if (used[i]) continue;
            bool nrok = false;
            int  nr   = exchElement.at(i).toInt(&nrok, 10);
            Q_UNUSED(nr);
            if (nrok && (exchElement.at(i).size()==2)) {
                ok_part[2]   = true;
                used[i]      = true;
                finalExch[2] = exchElement.at(i);
                break;
            }
        }

        // if still don't have a qso number, take the first number on the line
        if (!ok_part[0]) {
            for (int i = 0; i < exchElement.size(); i++) {
                if (used[i]) continue;

                bool ok;
                int nr = exchElement.at(i).toInt(&ok, 10);
                Q_UNUSED(nr);
                if (ok && !ok_part[0]) {
                    finalExch[0] = exchElement.at(i);
                    ok_part[0]=true;
                    used[i]=true;
                } else if (ok) {
                    used[i]      = true;
                }
            }
        }
        // try to update callsign if there is a non-identified string
        if (ok_part[0] & ok_part[1] & ok_part[2] & ok_part[3]) {
            for (int i = exchElement.size() - 1; i >= 0; i--) {
                if (!used[i]) {
                    bool ok;
                    int nr = exchElement.at(i).toInt(&ok, 10);
                    Q_UNUSED(nr);
                    if (!ok) {
                        qso->call = exchElement.at(i);
                    }
                    break;
                }
            }
        }
        delete[] used;
    } else {
        // not US/VE call, can't be logged
        return(false);
    }

    // only copy into log if exchange is validated
    if (ok_part[0] & ok_part[1] & ok_part[2] & ok_part[3]) {
        for (int i = 0; i < nExch; i++) {
            qso->rcv_exch[i] = finalExch[i];
        }
    }

    return(ok_part[0] & ok_part[1] & ok_part[2] & ok_part[3]);
}
