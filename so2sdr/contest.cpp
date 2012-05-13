/*! Copyright 2010-2012 R. Torsten Clay N4OGW

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
#include <QDebug>
#include <QDir>
#include <QSettings>
#include "cty.h"
#include "contest.h"
#include "hamlib/rotator.h"

Contest::Contest()
{
    dataDirectory.clear();
    myGrid.clear();
    myLat = 0.0;
    myLon = 0.0;
    for (int ii = 0; ii < MMAX; ii++) {
        mults[ii].clear();
        _nMults[ii] = 0;
        qsoTypeCntry[ii].clear();
        qsoTypeStr[ii].clear();
        for (int i = 0; i <= N_BANDS; i++) {
            multsWorked[ii][i] = 0;
            multWorked[ii][i].clear();
        }
    }
    score.clear();
    nextCall.clear();
    _zoneType = 0;
    _zoneMax  = 40;
    _vExch    = false;
}

void Contest::initialize(QSettings *ss,QSettings *cs,const Cty *cty)
{
    settings=cs;
    stnSettings=ss;
    setGrid();
    QByteArray tmp[MMAX];
    tmp[0]=settings->value(c_multfile1,c_multfile1_def).toString().toAscii();
    tmp[1]=settings->value(c_multfile2,c_multfile2_def).toString().toAscii();
    readMultFile(tmp, cty);
    zeroScore();
}

QByteArray Contest::contestName() const
{
    return(_contestName);
}

void Contest::setContestName(QByteArray s)
{
    _contestName=s;
}

void Contest::setDataDirectory(QString d)
{
    dataDirectory = d;
}

Contest::~Contest()
{
    for (int j = 0; j < MMAX; j++) {
        for (int i = 0; i < mults[j].size(); i++) {
            delete (mults[j][i]);
        }
        mults[j].clear();
    }
    for (int i = 0; i < score.size(); i++)
        delete (score[i]);
}

/*!
   Updates mult counters for added qso
    */
void Contest::addQsoMult(Qso *qso)
{
    scoreRecord* newrec = new scoreRecord;
    newrec->pts        = qso->pts;
    newrec->dupe       = qso->dupe;
    newrec->mult[0]    = qso->mult[0];
    newrec->mult[1]    = qso->mult[1];
    newrec->newmult[0] = -1;
    newrec->newmult[1] = -1;
    newrec->valid      = qso->valid;
    // qsos marked invalid ignored
    if (!qso->valid) {
        score.append(newrec);
        return;
    }
    for (int i = 0; i <= N_BANDS; i++) {
        // mults worked counted per-band
        // last index N_BANDS is for total number of mults regardless of band
        for (int j = 0; j < MMAX; j++) {
            multsWorked[j][i] = 0;
        }
    }
    bool new_m[MMAX]  = { false, false };
    bool new_bm[MMAX] = { false, false };
    int  b;
    if (settings->value(c_multsband,c_multsband_def).toBool()) {
        b = qso->band;
    } else {
        b = N_BANDS;
    }
    for (int ii = 0; ii < MMAX; ii++) {
        // unique callsigns, special mults. Check to see if this is
        // a completely new mult, and if so add it to the lists
        if (qso->isamult[ii] && (multType[ii] == Uniques || multType[ii] == Special || multType[ii] == Prefix)) {
            QByteArray tmp;
            if (multType[ii] == Uniques) {
                tmp = qso->call;
            } else {
                tmp = qso->mult_name;
            }

            // assume mult list is sorted in this case
            bool newmult = true;
            /*!
               can make search more efficient here
             */
            for (int j = 0; j < mults[ii].size(); j++) {
                if (mults[ii][j]->name == tmp) {
                    newmult = false;
                    break;
                }
            }
            if (newmult) {
                DomMult* mult = new DomMult;
                mult->hasAltNames = false;
                mult->name        = tmp;
                mult->isamult     = true;
                // insert so list is ordered
                int indx = mults[ii].size();
                for (int j = 0; j < mults[ii].size(); j++) {
                    if (tmp < mults[ii][j]->name) {
                        indx = j;
                        break;
                    }
                }
                qso->mult[ii]    = indx;
                qso->isamult[ii] = true;

                if (indx != mults[ii].size()) {
                    for (int k = 0; k < score.size(); k++) {
                        if (score.at(k)->mult[ii] >= indx) {
                            score[k]->mult[ii]++;
                        }
                    }
                }

                mults[ii].insert(indx, mult);

                // must insert extra element in other arrays
                for (int j = 0; j <= N_BANDS; j++) {
                    multWorked[ii][j].insert(indx, false);
                }
                _nMults[ii] = mults[ii].size();
            }
        }

        // count number of worked mults BEFORE this qso
        int sz[N_BANDS + 1];
        for (int i = 0; i <= N_BANDS; i++) {
            sz[i] = multWorked[ii][i].size();
        }
        // for all-band mults
        for (int j = 0; j < sz[N_BANDS]; j++) {
            if (!mults[ii].at(j)->isamult) continue;
            if (multWorked[ii][N_BANDS].at(j)) {
                multsWorked[ii][N_BANDS]++;
            }
        }
        // band-by-band mults
        for (int b = 0; b < N_BANDS_SCORED; b++) {
            for (int j = 0; j < sz[b]; j++) {
                if (!mults[ii].at(j)->isamult) continue;
                if (multWorked[ii][b].at(j)) {
                    multsWorked[ii][b]++;
                }
            }
        }
    }
    // add new mult
    for (int ii = 0; ii < MMAX; ii++) {
        if (_nMults[ii] == 0 || !qso->isamult[ii]) continue;

        if (qso->mult[ii] != -1) {
            if (!multWorked[ii][qso->band][qso->mult[ii]] && mults[ii][qso->mult[ii]]->isamult) {
                new_bm[ii] = true;
                multsWorked[ii][qso->band]++;
                multWorked[ii][qso->band][qso->mult[ii]] = true;
            }
            // all-band mults
            if (!multWorked[ii][N_BANDS][qso->mult[ii]] && mults[ii][qso->mult[ii]]->isamult) {
                new_m[ii] = true;
                multsWorked[ii][N_BANDS]++;
                multWorked[ii][N_BANDS][qso->mult[ii]] = true;
            }
        }
    }
    qso->newmult[0] = -1;
    qso->newmult[1] = -1;
    if (settings->value(c_multsband,c_multsband_def).toBool()) {
        // mults count per-band
        for (int ii = 0; ii < MMAX; ii++) {
            if (new_bm[ii]) {
                qso->newmult[ii] = multFieldHighlight[ii];
            }
        }
    } else {
        for (int ii = 0; ii < MMAX; ii++) {
            if (new_m[ii]) {
                qso->newmult[ii] = multFieldHighlight[ii];
            }
        }
    }
    // update score record
    newrec->mult[0]    = qso->mult[0];
    newrec->mult[1]    = qso->mult[1];
    newrec->newmult[0] = qso->newmult[0];
    newrec->newmult[1] = qso->newmult[1];
    score.append(newrec);
}

/*! dupe status for qso at row "row"
 */
bool Contest::dupe(int row) const
{
    return(score.at(row)->dupe);
}

/*! return multiplier index number for qso on row "row" of the log;
   ii is the type of multiplier (0 or 1) */
int Contest::mult(int row, int ii) const
{
    if (row < score.size()) {
        return(score.at(row)->mult[ii]);
    } else {
        return(-1);
    }
}

/*! returns newmult flag for qso at row "row", mult type ii (0 or 1)
 */
int Contest::newMult(int row, int ii) const
{
    if (row < score.size()) {
        return(score.at(row)->newmult[ii]);
    } else {
        return(-1);
    }
}

/*! qso points for qso at row "row".

   If qso is a dupe, this should have been set to 0
 */
int Contest::points(int row) const
{
    return(score.at(row)->pts);
}

/*! set grid square
 */
void Contest::setGrid()
{
    QByteArray s=stnSettings->value(s_grid,s_grid_def).toByteArray();
    // check to see if this is a valid grid square
    if (s.size() != 4 || s.at(0) < 'A' || s.at(0) > 'R' ||
        s.at(1) < 'A' || s.at(1) > 'R' ||
        s.at(2) < '0' || s.at(2) > '9' ||
        s.at(3) < '0' || s.at(3) > '9') {
        myGrid.clear();
    } else {
        myGrid = s;
        locator2longlat(&myLon, &myLat, s.data());
        myLon *= -1.0;
    }
}


/*!
   Add a new multiplier country (by index)
 */
void Contest::addQsoTypeCntry(int i, int ii)
{
    qsoTypeCntry[ii].append(i);
}


/*!
   Add a new qso/mult type
 */
void Contest::addQsoType(const QByteArray str, const int ii)
{
    qsoTypeStr[ii].append(str);
}

QList<QByteArray> &Contest::qsoType(int ii)
{
    return(qsoTypeStr[ii]);
}

/*! re-count mults
 */
void Contest::count_mults()
{
    for (int ii = 0; ii < MMAX; ii++) {
        for (int i = 0; i <= N_BANDS; i++) {
            multsWorked[ii][i] = 0;
        }
    }
    for (int ii = 0; ii < MMAX; ii++) {
        for (int j = 0; j < N_BANDS_SCORED; j++) {
            for (int i = 0; i < _nMults[ii]; i++) {
                if (multWorked[ii][j].at(i)) multsWorked[ii][j]++;
            }
        }
        // all-band mults
        for (int i = 0; i < _nMults[ii]; i++) {
            if (multWorked[ii][N_BANDS].at(i)) multsWorked[ii][N_BANDS]++;
        }
    }
}

/*!
   Tries to fill in the multiplier given only the callsign for this qso

   Does not overwrite mult info determined elsewhere
 */
void Contest::guessMult(Qso *qso) const
{
    for (int ii = 0; ii < MMAX; ii++) {
        // mult info already known
        if (qso->mult[ii] != -1) continue;

        switch (multType[ii]) {
        case ARRLCountry:
        case CQCountry:
        case ContNA:
        case ContSA:
        case ContEU:
        case ContAF:
        case ContAS:
        case ContOC:
            qso->mult[ii] = qso->country;
            break;
        case CQZone:
        case ITUZone:
            qso->mult[ii] = qso->zone - 1;
            break;
        default:
            break;
        }
    }
}

/*!
   Returns unsigned integer worked  with bits set to indicate what bands mult1 and mult2
   are worked on
 */
void Contest::workedMults(Qso *qso, unsigned int worked[MMAX]) const
{
    for (int ii = 0; ii < MMAX; ii++) worked[ii] = 0;
    if (settings->value(c_multsband,c_multsband_def).toBool()) {
        for (int ii = 0; ii < MMAX; ii++) {
            for (int i = 0; i < N_BANDS_SCORED; i++) {
                if (qso->mult[ii] != -1 && qso->mult[ii] < _nMults[ii]) {
                    worked[ii] += multWorked[ii][i][qso->mult[ii]] * bits[i];
                }
            }
        }
    } else {
        // all-band mults
        for (int ii = 0; ii < MMAX; ii++) {
            if (qso->mult[ii] != -1 && qso->mult[ii] < _nMults[ii]) {
                if (multWorked[ii][N_BANDS][qso->mult[ii]]) worked[ii] = 1 + 2 + 4 + 8 + 16 + 32;
            }
        }
    }
}

/*!
   read/setup mult lists. Possible options:
        filename,arrl_country,cq_country,prefix,cq_zone,itu_zone
 */
void Contest::readMultFile(QByteArray filename[MMAX], const Cty *cty)
{
    QDir::setCurrent(dataDirectory);
    for (int ii = 0; ii < MMAX; ii++) multFile[ii] = filename[ii];
    Qso* tmpqso = new Qso(1);
    for (int ii = 0; ii < MMAX; ii++) {
        isMultCntry[ii].clear();

        for (int i = 0; i < qsoTypeStr[ii].size(); i++) {
            if (qsoTypeStr[ii][i].at(0) == '!') {
                isMultCntry[ii].append(false);
                int s = qsoTypeStr[ii][i].length();
                qsoTypeStr[ii][i] = qsoTypeStr[ii][i].right(s - 1);
            } else {
                isMultCntry[ii].append(true);
            }
            tmpqso->call = qsoTypeStr[ii][i];
            bool b;
            int  pfx = cty->idPfx(tmpqso, b);
            addQsoTypeCntry(pfx, ii);
        }

        if (multFile[ii].toLower() == "none" || multFile[ii].isEmpty()) {
            _nMults[ii]  = 0;
            multType[ii] = None;
            continue;
        }
        for (int i = 0; i <= N_BANDS; i++) {
            multWorked[ii][i].clear();
        }

        // special cases
        if (multFile[ii].toLower() == "arrl_country") {
            selectCountries(ii, cty, ALL);
            multType[ii] = ARRLCountry;
        } else if (multFile[ii].toLower() == "cq_country") {
            selectCountries(ii, cty, ALL);
            multType[ii] = CQCountry;
        } else if (multFile[ii].toLower() == "cont_na") {
            selectCountries(ii, cty, NA);
            multType[ii] = ContNA;
        } else if (multFile[ii].toLower() == "cont_sa") {
            selectCountries(ii, cty, SA);
            multType[ii] = ContSA;
        }   else if (multFile[ii].toLower() == "cont_eu") {
            selectCountries(ii, cty, EU);
            multType[ii] = ContEU;
        } else if (multFile[ii].toLower() == "cont_as") {
            selectCountries(ii, cty, AS);
            multType[ii] = ContAS;
        } else if (multFile[ii].toLower() == "cont_af") {
            selectCountries(ii, cty, AF);
            multType[ii] = ContAF;
        } else if (multFile[ii].toLower() == "cont_oc") {
            selectCountries(ii, cty, OC);
            multType[ii] = ContOC;
        } else if (multFile[ii].toLower() == "prefix") {
            // prefixes (WPX). Number of mults initially=0
            _nMults[ii]  = 0;
            multType[ii] = Prefix;
        } else if (multFile[ii].toLower() == "special") {
            _nMults[ii]  = 0;
            multType[ii] = Special;
        } else if (multFile[ii].toLower() == "uniques") {
            // unique callsigns
            _nMults[ii]  = 0;
            multType[ii] = Uniques;
        } else if (multFile[ii].toLower() == "cq_zone") {
            multType[ii] = CQZone;
            _nMults[ii]  = _zoneMax;
            for (int i = 0; i < _nMults[ii]; i++) {
                DomMult* mult = new DomMult;
                mult->hasAltNames = false;
                mult->isamult     = true;
                mult->name        = QByteArray::number(i + 1);
                // make all zones display as two digits
                if (mult->name.size()==1) {
                    mult->name="0"+mult->name;
                }
                mults[ii].append(mult);
            }
        } else if (multFile[ii].toLower() == "itu_zone") {
            multType[ii] = ITUZone;
            _nMults[ii]  = _zoneMax;
            for (int i = 0; i < _nMults[ii]; i++) {
                DomMult* mult = new DomMult;
                mult->hasAltNames = false;
                mult->name        = QByteArray::number(i + 1);
                // make all zones display as two digits
                if (mult->name.size()==1) {
                    mult->name="0"+mult->name;
                }
                mult->isamult     = true;
                mults[ii].append(mult);
            }
        } else {
            // read file of mults
            multType[ii] = File;
            QFile file(multFile[ii]);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                qDebug("couldn't open file <%s>", multFile[ii].data());
                _nMults[ii] = 0;
                continue;
            }
            while (!file.atEnd()) {
                QByteArray buffer;
                buffer = file.readLine();
                buffer = buffer.trimmed();
                if (buffer.mid(0, 1) == "#") continue;  // #==comment
                DomMult* mult = new DomMult;

                mult->isamult = true;

                // check for alternate names
                if (buffer.contains(";")) {
                    mult->hasAltNames = true;
                    int i1 = buffer.indexOf(";", 0);
                    mult->name = buffer.mid(0, i1).trimmed().toUpper();
                    while (i1 != -1) {
                        int i3;
                        int i2 = buffer.indexOf(";", i1 + 1);
                        i3 = i2;
                        if (i3 == -1) {
                            i3 = buffer.size();
                        }
                        QByteArray tmp = buffer.mid(i1 + 1, i3 - i1 - 1);
                        i1 = i2;
                        mult->alt_names.append(tmp);
                    }
                    mults[ii].append(mult);
                } else {
                    // no alternate name
                    mult->hasAltNames = false;
                    mult->name        = buffer.trimmed().toUpper();
                    mults[ii].append(mult);
                }
            }
            file.close();
            _nMults[ii] = mults[ii].size();
        }
        for (int i = 0; i <= N_BANDS; i++) {
            for (int j = 0; j < _nMults[ii]; j++) {
                multWorked[ii][i].append(false);
            }
        }
    }
    delete tmpqso;
}

/*!
   Determines if qso fits in mult categories 1 or 2

   This may be called more than once as additional information (country, exchange, ...) is added to qso

 */
void Contest::determineMultType(Qso *qso)
{
    for (int ii = 0; ii < MMAX; ii++) qso->isamult[ii] = false;
    int  i, sz;
    bool ok;
    for (int ii = 0; ii < MMAX; ii++) {
        switch (multType[ii]) {
        case File:
            // if no country list given, apply list to all
            sz = qsoTypeCntry[ii].size();
            if (sz == 0) {
                qso->isamult[ii] = true;
                break;
            } else {
                if (qso->country == -1) break;

                // country should match one on list in order to use this mult file
                for (i = 0; i < sz; i++) {
                    if (isMultCntry[ii].at(i)) {
                        if (qso->country == qsoTypeCntry[ii].at(i)) {
                            qso->isamult[ii] = true;
                            break;
                        }
                    } else {
                        if (qso->country != qsoTypeCntry[ii].at(i)) {
                            qso->isamult[ii] = true;
                            break;
                        }
                    }
                }
            }
            break;
        case Uniques:

            // unique callsigns. Anything is ok
            qso->isamult[ii] = true;
            break;
        case ARRLCountry: case CQCountry:
            if (qso->country == -1) break;

            // check for excluded countries
            qso->isamult[ii] = true;
            qso->mult[ii]    = qso->country;
            for (i = 0; i < qsoTypeCntry[ii].size(); i++) {
                if (qso->country == qsoTypeCntry[ii][i] && !isMultCntry[ii][i]) {
                    qso->isamult[ii] = false;
                    qso->mult[ii]    = -1;
                    break;
                }
            }
            break;
        case ContNA:
            if (qso->country == -1) break;

            // must match NA continent
            if (qso->continent == NA) {
                // must not be an excluded country
                ok = true;
                for (i = 0; i < qsoTypeCntry[ii].size(); i++) {
                    if (!isMultCntry[ii][i] && qso->country == qsoTypeCntry[ii][i]) {
                        ok = false;
                    }
                }
                if (ok) {
                    qso->isamult[ii] = true;
                    qso->mult[ii]    = qso->country;
                } else {
                    qso->mult[ii] = -1;
                }
            }
            break;
        case ContSA:
            if (qso->country == -1) break;

            // must match SA continent
            if (qso->continent == SA) {
                // must not be an excluded country
                ok = true;
                for (i = 0; i < qsoTypeCntry[ii].size(); i++) {
                    if (!isMultCntry[ii][i] && qso->country == qsoTypeCntry[ii][i]) {
                        ok = false;
                    }
                }
                if (ok) {
                    qso->isamult[ii] = true;
                    qso->mult[ii]    = qso->country;
                } else {
                    qso->mult[ii] = -1;
                }
            }
            break;
        case ContEU:
            if (qso->country == -1) break;

            // must match EU continent
            if (qso->continent == EU) {
                // must not be an excluded country
                ok = true;
                for (i = 0; i < qsoTypeCntry[ii].size(); i++) {
                    if (!isMultCntry[ii][i] && qso->country == qsoTypeCntry[ii][i]) {
                        ok = false;
                    }
                }
                if (ok) {
                    qso->isamult[ii] = true;
                    qso->mult[ii]    = qso->country;
                } else {
                    qso->mult[ii] = -1;
                }
            }
            break;
        case ContAF:
            if (qso->country == -1) break;

            // must match AF continent
            if (qso->continent == AF) {
                // must not be an excluded country
                ok = true;
                for (i = 0; i < qsoTypeCntry[ii].size(); i++) {
                    if (!isMultCntry[ii][i] && qso->country == qsoTypeCntry[ii][i]) {
                        ok = false;
                    }
                }
                if (ok) {
                    qso->isamult[ii] = true;
                    qso->mult[ii]    = qso->country;
                } else {
                    qso->mult[ii] = -1;
                }
            }
            break;
        case ContAS:
            if (qso->country == -1) break;

            // must match AS continent
            if (qso->continent == AS) {
                // must not be an excluded country
                ok = true;
                for (i = 0; i < qsoTypeCntry[ii].size(); i++) {
                    if (!isMultCntry[ii][i] && qso->country == qsoTypeCntry[ii][i]) {
                        ok = false;
                    }
                }
                if (ok) {
                    qso->isamult[ii] = true;
                    qso->mult[ii]    = qso->country;
                } else {
                    qso->mult[ii] = -1;
                }
            }
            break;
        case ContOC:
            if (qso->country == -1) break;

            // must match OC continent
            if (qso->continent == OC) {
                // must not be an excluded country
                ok = true;
                for (i = 0; i < qsoTypeCntry[ii].size(); i++) {
                    if (!isMultCntry[ii][i] && qso->country == qsoTypeCntry[ii][i]) {
                        ok = false;
                    }
                }
                if (ok) {
                    qso->isamult[ii] = true;
                    qso->mult[ii]    = qso->country;
                } else {
                    qso->mult[ii] = -1;
                }
            }
            break;
        case CQZone:
        case ITUZone:

            // if exchange not known yet, assume call has a zone
            if (qso->mult_name == "") {;
                                       qso->isamult[ii] = true;

                                       // fill in zone if known already from call
                                       // subtract 1 since mult list indexed from 0
                                       if (qso->zone) qso->mult[ii] = qso->zone - 1;
                                       break; }
            ok = false;
            i  = qso->mult_name.toInt(&ok, 10);
            if (ok) {
                qso->isamult[ii] = true;
                qso->mult[ii]    = i - 1;
            } else {
                qso->mult[ii] = -1;
            }
            break;
        case Prefix:

            // any call has a prefix
            qso->isamult[ii] = true;
            break;
        case Special:

            // is it already in the list of mults?
            for (int j = 0; j < mults[ii].size(); j++) {
                if (qso->mult_name == mults[ii][j]->name) {
                    qso->isamult[ii] = true;
                    qso->mult[ii]    = j;
                    break;
                }
            }

            // if exchange is non-numeric, it might be a special
            if (qso->mult_name.isEmpty()) {
                break;
            }
            i = qso->mult_name.toInt(&ok, 10);
            if (!ok) {
                qso->isamult[ii] = true;
            }
            break;
        case None:
            break;
        }
    }
}

/*!
   Field number containing RST. returns -1 if no rst
 */
int Contest::rstField() const
{
    for (int i = 0; i < nExch; i++) {
        if (exchange_type[i] == RST) return(i);
    }
    return(-1);
}

void Contest::selectCountries(int ii, const Cty *cty, Cont cont)
{
    _nMults[ii] = cty->nCountries();
    Qso tmpqso(1);
    for (int i = 0; i < _nMults[ii]; i++) {
        DomMult* mult = new DomMult;
        mult->hasAltNames = false;
        mult->name        = cty->pfxName(i);
        tmpqso.call       = mult->name;
        bool b;
        cty->idPfx(&tmpqso, b);
        Cont continent = tmpqso.continent;

        if (cont == continent || cont == ALL) {
            mult->isamult = true;
        } else {
            mult->isamult = false;
        }
        for (int j = 0; j < qsoTypeCntry[ii].size(); j++) {
            if (i == qsoTypeCntry[ii][j] && !isMultCntry[ii][j]) {
                mult->isamult = false;
            }
        }
        mults[ii].append(mult);
    }
}

/*! returns true if there is a default exchange that can be prefilled (zone, country, etc) */
bool Contest::hasPrefill() const
{
    return(prefill);
}

/*! returns true if this log field get filled in for later
   qso's
   example: name, state, ... these should get prefilled
         qso number: this doesn't, since it changes
 */
bool Contest::logPrefill(int n) const
{
    if (n >= 0 && n < nExch) return(logFieldPrefill[n]);
    else return(false);
}

/*! default exchange prefill for CQWW contests- returns zone

   other contests should define something more specific
 */
QByteArray Contest::prefillExchange(Qso *qso)
{
    if (qso->zone!=0) {
        return(QByteArray::number(qso->zone));
    } else {
        return "";
    }
}

/*! returns the type of each element in the contest exchange
 */
FieldTypes Contest::exchType(int indx) const
{
    return(exchange_type[indx]);
}

/*! true: stations can be worked on multiple bands
    false:stations can only be worked once

    see setmultsByBand

   @todo for SSB, will need to check also if per-mode qsos are ok*/
bool Contest::dupeCheckingByBand() const
{
    return(dupeCheckingEveryBand);
}

/*!
   returns total number of mults worked
 */
int Contest::nMultsWorked() const
{
    if (settings->value(c_multsband,c_multsband_def).toBool()) {
        int n = 0;
        for (int ii = 0; ii < MMAX; ii++) {
            for (int i = 0; i < N_BANDS_SCORED; i++) {
                n += multsWorked[ii][i];
            }
        }
        return(n);
    } else {
        // all-band mults
        return(multsWorked[0][N_BANDS] + multsWorked[1][N_BANDS]);
    }
}

/*!
   number of mults on a single band
 */
int Contest::nMultsBWorked(int ii, int band) const
{
    return(multsWorked[ii][band]);
}


/*!
   return name of mult i, needed=true if it is still needed
 */
QByteArray Contest::neededMultName(int ii, int band, int i, bool &needed_band, bool &needed) const
{
    needed      = false;
    needed_band = false;

    if (i < _nMults[ii]) {
        if (!mults[ii][i]->isamult) {
            return("");
        }
        if (multWorked[ii][band][i]) {
            needed_band = true;
        }
        if (multWorked[ii][N_BANDS][i]) {
            needed = true;
        }
        return(mults[ii][i]->name);
    } else {
        return("");
    }
}

/*!
   Number of elements in exchange
 */
int Contest::nExchange() const
{
    return(nExch);
}

/*! this station's zone (CQ or ITU)
 */
int Contest::myZone() const
{
    return(_myZone);
}

/*!
   Returns number of mults of category ii
 */
int Contest::nMults(int ii) const
{
    return(_nMults[ii]);
}

/*!
   Determines if exch is a valid mult. returns -1 if not,
   otherwise an index for the mult
 */
int Contest::isAMult(QByteArray exch, int ii) const
{
    if (multType[ii]==CQZone || multType[ii]==ITUZone) {
        /* match zones by number, not string */
        for (int i = 0; i < _nMults[ii]; i++) {
            bool ok=false;
            int eZone=exch.toInt(&ok);
            if (ok && eZone==(i+1)) return(i);
        }
    } else {
        for (int i = 0; i < _nMults[ii]; i++) {
            if (exch == mults[ii].at(i)->name) {
                return(i);
            }

            // check alt names
            if (mults[ii].at(i)->hasAltNames) {
                for (int j = 0; j < mults[ii].at(i)->alt_names.size(); j++) {
                    if (exch == mults[ii].at(i)->alt_names[j]) {
                        return(i);
                    }
                }
            }
        }
    }
    return(-1);
}

/*!
   Returns callsign entered as next call in exchange window (begins with "/")
 */
bool Contest::newCall(QByteArray &b) const
{
    b = nextCall;
    if (nextCall.isEmpty()) {
        return(false);
    } else {
        return(true);
    }
}

/*!
   Total score
 */
int Contest::Score() const
{
    if (settings->value(c_multsband,c_multsband_def).toBool()) {
        int n = 0;
        for (int ii = 0; ii < MMAX; ii++) {
            for (int i = 0; i < N_BANDS_SCORED; i++) {
                n += multsWorked[ii][i];
            }
        }
        return(qsoPts * n);
    } else {
        // all-band mults
        int n = (multsWorked[0][N_BANDS] + multsWorked[1][N_BANDS]);
        return(qsoPts * n);
    }
}

/*!
   Separate initial entered exchange into pieces
 */
bool Contest::separateExchange(Qso *qso)
{
    // clean up
    QByteArray exchange = qso->exch;
    exchange = exchange.simplified();
    exchange = exchange.toUpper();
    if (exchange.size() == 0) {
        exchElement.clear();
        return(false);  // nothing to do!
    }
    // find exchange elements separated by space
    int nel = 0;
    int i1  = 0;
    int i2  = exchange.indexOf(" ");
    exchElement.clear();
    while (i2 != -1) {
        exchElement.append(exchange.mid(i1, i2 - i1));
        nel++;
        i1 = i2 + 1;
        i2 = exchange.indexOf(" ", i1);
    }

    // last (or only) piece
    exchElement.append(exchange.mid(i1, exchange.size() - i1));
    nel++;

    // put first nExch or non-null pieces into qso received info. This will be used
    // in the case the exchange can't be validated, but the qso
    // is forcibly logged (ctrl+enter)
    int n = nExch;
    if (exchElement.size() < n) n = exchElement.size();
    for (int i = 0; i < n; i++) {
        qso->rcv_exch[i] = exchElement[i];
    }

    // check for next callsign (begins with "/")
    i1 = -1;
    nextCall.clear();
    for (int i = 0; i < exchElement.size(); i++) {
        if (exchElement.at(i).left(1) == "/") {
            nextCall = exchElement.at(i);
            nextCall = nextCall.right(nextCall.size() - 1);
            exchElement.removeAt(i);
        }
    }
    return(true);
}

void Contest::setContinent(Cont i)
{
    myContinent = i;
}

void Contest::setCountry(int i)
{
    myCountry = i;
}

void Contest::setMyZone(int i)
{
    _myZone = i;
}

/*! fill in default RS(T) based on mode
   assumed to be first element in exchange */
void Contest::fillDefaultRST(Qso *qso) const
{
    if (qso->mode == RIG_MODE_CW || qso->mode == RIG_MODE_CWR ||
        qso->mode == RIG_MODE_RTTY || qso->mode == RIG_MODE_RTTYR) {
        finalExch[0] = "599";
    } else {
        finalExch[0] = "59";
    }
}

/*! Exchange: RST + R1/R2/R3 (for /MM stations)
 */
bool Contest::valExch_mm(Qso *qso)
{
    Q_UNUSED(qso);
    bool ok = false;

    // look for ITU region
    for (int i = exchElement.size() - 1; i >= 0; i--) {
        if (exchElement[i] == "R1" ||
            exchElement[i] == "R2" ||
            exchElement[i] == "R3") {
            finalExch[1] = exchElement[i];
            ok           = true;
            break;
        }
    }

    // check for possible RST
    for (int i = exchElement.size() - 1; i >= 0; i--) {
        bool nrok = false;
        int  rst;
        if ((rst = exchElement[i].toInt(&nrok, 10))) {
            if (rst >= 111 && rst <= 599) {
                finalExch[0] = exchElement[i];
                break;
            }
        }
    }
    return(ok);
}

/*!
   Exchange: RST + multiplier

      ii = mult list
      mult_indx = returned mult number, -1 if no match
 */
bool Contest::valExch_rst_state(int ii, int &mult_indx)
{
    if (exchElement.size() == 0) return(false);

    finalExch[0] = "599";   // default received report
    finalExch[1] = "";
    bool ok[2] = { false, false };

    // look for RST
    int *nr_indx = new int[exchElement.size()];
    for (int i = 0; i < exchElement.size(); i++) {
        if (exchElement[i].toInt()) {
            nr_indx[i] = 1;
        } else {
            nr_indx[i] = 0;
        }
    }

    // if only one element in exchange, it must be the state
    int k;
    if (exchElement.size() == 1) {
        k = isAMult(exchElement[0], ii);
        if (k != -1) {
            mult_indx    = k;
            finalExch[1] = mults[ii][mult_indx]->name;
            ok[1]        = true;
        }
    } else {
        for (int i = exchElement.size() - 1; i >= 0; i--) {
            if (!ok[0] && nr_indx[i]) {
                finalExch[0] = exchElement[i];
                ok[0]        = true;
            }
            if (!ok[1] && !nr_indx[i] && (k = isAMult(exchElement[i], ii)) != -1) {
                mult_indx    = k;
                finalExch[1] = mults[ii][mult_indx]->name;
                ok[1]        = true;
            }
        }
    }
    delete[] nr_indx;
    return(ok[1]); // RST does not have to be entered
}

/*!
   Exchange: # name multiplier

   ii = index of mult file with state mults
   mult_indx = returned mult number
 */
bool Contest::valExch_nr_name_state(int ii, int &mult_indx)
{
    if (exchElement.size() < 3) return(false);  // need at least 3 pieces

    finalExch[0] = "";
    finalExch[1] = "";
    finalExch[2] = "";
    bool ok[3] = { false, false, false };

    // look for number
    int cnt_nr   = 0;
    int *nr_indx = new int[exchElement.size()];
    int *d_indx  = new int[exchElement.size()];
    for (int i = 0; i < exchElement.size(); i++) {
        if (exchElement[i].toInt()) {
            nr_indx[i] = 1;
            cnt_nr++;
        } else {
            nr_indx[i] = 0;
        }
        d_indx[i] = isAMult(exchElement[i], ii);
    }

    // not enough non-numeric fields
    if ((exchElement.size() - cnt_nr) < 2) {
        delete[] nr_indx;
        delete[] d_indx;
        return(false);
    }

    int k;
    int dm_indx = exchElement.size() - 1;
    for (int i = exchElement.size() - 1; i >= 0; i--) {
        // number: take LAST number element in exchange
        // (in case it was corrected)
        if (!ok[0] && nr_indx[i]) {
            finalExch[0] = exchElement[i];
            ok[0]        = true;
        }

        // try to match anything not # or DMult to name
        if (!ok[1] && !nr_indx[i] && d_indx[i] == -1) {
            finalExch[1] = exchElement[i];
            ok[1]        = true;
        }
    }

    // match Dom Mult
    for (int i = exchElement.size() - 1; i >= 0; i--) {
        if (!ok[2] && !nr_indx[i] && d_indx[i] != -1) {
            mult_indx    = d_indx[i];
            finalExch[2] = mults[ii][mult_indx]->name;
            ok[2]        = true;
            dm_indx      = i;
            break;
        }
    }

    // name must match the DMult list; take the next non-nr element before the mult
    if (!ok[1]) {
        for (k = dm_indx - 1; k >= 0; k--) {
            if (!nr_indx[k]) {
                finalExch[1] = exchElement[k];
                ok[1]        = true;
                break;
            }
        }
    }
    delete[] nr_indx;
    delete[] d_indx;
    return(ok[0] && ok[1] && ok[2]);
}

/*!
   Exchange: name DomMult

 */
bool Contest::valExch_name_state(int ii, int &mult_indx)
{
    int s = exchElement.size();
    if (s < 2) return(false);

    finalExch[0].clear();
    finalExch[1].clear();

    int *mult = new int[s];

    // try to match each element to mult list
    int nmatch = 0;
    for (int i = 0; i < s; i++) {
        mult[i] = isAMult(exchElement[i], ii);
        if (mult[i] != -1) nmatch++;
    }

    // only one match- that must be the mult
    if (nmatch == 1) {
        for (int i = 0; i < s; i++) {
            if (mult[i] != -1) {
                mult_indx    = mult[i];
                finalExch[1] = exchElement[i];
                break;
            }
        }

        // last non-mult must be the name
        for (int i = (s - 1); i >= 0; i--) {
            if (mult[i] == -1) {
                finalExch[0] = exchElement[i];
                break;
            }
        }
    } else {
        // more than one elment matches the mult list
        // take the last one as the mult
        for (int i = (s - 1); i >= 0; i--) {
            if (mult[i] != -1) {
                mult_indx    = mult[i];
                finalExch[1] = exchElement[i];
                break;
            }
        }

        // if something doesn't match the mult list, take as name
        // last non-mult must be the name
        for (int i = (s - 1); i >= 0; i--) {
            if (mult[i] == -1) {
                finalExch[0] = exchElement[i];
                break;
            }
        }

        // in case name also matches the mult list, just take position 0 as name
        if (finalExch[0].isEmpty()) {
            finalExch[0] = exchElement[0];
        }
    }

    delete[] mult;

    if (!finalExch[0].isEmpty() && !finalExch[1].isEmpty()) {
        return(true);
    } else {
        return(false);
    }
}


/*!
   Exchange: nr name

 */
bool Contest::valExch_nr_name()
{
    if (exchElement.size() < 2) return(false);  // need at least 2 pieces

    finalExch[0] = "";
    finalExch[1] = "";
    bool ok[2] = { false, false };

    // look for number
    int *nr_indx = new int[exchElement.size()];
    for (int i = 0; i < exchElement.size(); i++) {
        if (exchElement[i].toInt()) {
            nr_indx[i] = 1;
        } else {
            nr_indx[i] = 0;
        }
    }
    for (int i = exchElement.size() - 1; i >= 0; i--) {
        // number: take LAST number element in exchange
        // (in case it was corrected)
        if (!ok[0] && nr_indx[i]) {
            finalExch[0] = exchElement[i];
            ok[0]        = true;
            break;
        }
    }
    for (int i = 0; i < exchElement.size(); i++) {
        // try to match anything not # to name
        // start from 0(take first one) since some contests have third Pfx/country field
        if (!ok[1] && !nr_indx[i]) {
            finalExch[1] = exchElement[i];
            ok[1]        = true;
            break;
        }
    }
    delete[] nr_indx;
    return(ok[0] && ok[1]);
}

/*!
   Exchange: RST+Zone

   returns zone number in mult_indx

 */
bool Contest::valExch_rst_zone(int ii, int &mult_indx)
{
    if (exchElement.size() == 0) return(false);

    finalExch[0] = "599";   // default received report
    finalExch[1] = "";

    int zoneField = 0;
    if (exchElement.size() == 2) {
        // explicit RST entered
        finalExch[0] = exchElement[0];
        finalExch[1] = exchElement[1];
        zoneField    = 1;
    }
    mult_indx = isAMult(exchElement[zoneField], ii);
    if (mult_indx == -1) {
        return(false);
    }
    return(true);
}

/*!
   Exchange: RST + serial #
 */
bool Contest::valExch_rst_nr(Qso *qso)
{
    if (exchElement.size() == 0) return(false);

    for (int i = 0; i < exchElement.size(); i++) {
        // an element isn't a number
        if (exchElement[i].toInt() == 0) return(false);
    }

    // auto-fill rst
    if (qso->mode == RIG_MODE_CW || qso->mode == RIG_MODE_CWR ||
        qso->mode == RIG_MODE_RTTY || qso->mode == RIG_MODE_RTTYR) {
        finalExch[0] = "599";
    } else {
        finalExch[0] = "59";
    }
    finalExch[1] = "";

    // only one number: assume RST=599
    if (exchElement.size() == 1) {
        finalExch[1] = exchElement[0];
    } else if (exchElement.size() == 2) {
        finalExch[0] = exchElement[0];
        finalExch[1] = exchElement[1];
    } else {
        finalExch[0] = exchElement[0];
        finalExch[1] = exchElement.last();
    }
    return(true);
}

/*!
   Exchange: RST + name (or other general item)
 */
bool Contest::valExch_rst_name(Qso *qso)
{
    if (exchElement.isEmpty()) return(false);

    // auto-fill rst
    if (qso->mode == RIG_MODE_CW || qso->mode == RIG_MODE_CWR ||
        qso->mode == RIG_MODE_RTTY || qso->mode == RIG_MODE_RTTYR) {
        finalExch[0] = "599";
    } else {
        finalExch[0] = "59";
    }
    finalExch[1] = "";

    // only one element; assume RST=599/59
    if (exchElement.size() == 1) {
        finalExch[1] = exchElement[0];
    } else if (exchElement.size() == 2) {
        // two elements: first=RST if it is numeric and in correct range
        bool ok  = false;
        int  rst = exchElement[0].toInt(&ok, 10);
        if (ok && (rst >= 111 && rst <= 599)) {
            finalExch[0] = exchElement[0];
        }
        finalExch[1] = exchElement[1];
    } else {
        // take last one to be name
        finalExch[1] = exchElement.last();
    }
    return(true);
}

/*! maximum legal zone (ie 40 for CQ)
 */
int Contest::zoneMax() const
{
    return(_zoneMax);
}

void Contest::setZoneMax(int i)
{
    _zoneMax = i;
}

/*! 0 = cq zones
    1 = itu zones
 */
void Contest::setZoneType(int i)
{
    _zoneType = i;
}

int Contest::zoneType() const
{
    return(_zoneType);
}

/*! true:  contest exchange is variable (like qso number).
    false: contest exchange is fixed. Can fill cabrillo sent exchange fields with identical information
 */
bool Contest::vExch() const
{
    return(_vExch);
}

void Contest::setVExch(bool b)
{
    _vExch = b;
}

/*! copy validated exchange. If validation failed, copy
   exchange directly from entered data with no checks */
void Contest::copyFinalExch(bool validated, Qso *qso)
{
    if (validated) {
        for (int i = 0; i < nExch; i++) {
            qso->rcv_exch[i] = finalExch[i];
        }
    } else {
        for (int i = 0; i < nExch; i++) {
            if (i == exchElement.size()) break;
            qso->rcv_exch[i] = exchElement.at(i);
        }
    }
}

/*!
  returns valid flag:
     true: qso included in final (cabrillo) log
     false: qso will not be in final log
     */
bool Contest::valid(int row) const
{
    return score.at(row)->valid;
}

/*!
   zeros out score and mult count
 */
void Contest::zeroScore()
{
    qsoPts = 0;
    for (int ii = 0; ii < MMAX; ii++) {
        for (int j = 0; j <= N_BANDS; j++) {
            for (int k = 0; k < _nMults[ii]; k++) {
                multWorked[ii][j][k] = false;
            }
            multsWorked[ii][j] = 0;
        }

        // in these cases, the number of mults depends on the contents of the log
        if (multType[ii] == Uniques || multType[ii] == Special || multType[ii] == Prefix) {
            mults[ii].clear();
            for (int ii = 0; ii < MMAX; ii++) {
                for (int j = 0; j <= N_BANDS; j++) {
                    multWorked[ii][j].clear();
                }
            }
        }
    }
    for (int i = 0; i < score.size(); i++) {
        delete (score[i]);
    }
    score.clear();
}
