/*! Copyright 2010-2021 R. Torsten Clay N4OGW

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
#include "utils.h"

Contest::Contest(QSettings &cs,QSettings &ss) : settings(cs),stnSettings(ss)
{
    myGrid.clear();
    myLat = 0.0;
    myLon = 0.0;
    dupeCheckingEveryBand=true;
    prefill=false;
    logFieldPrefill=nullptr;
    myContinent=NA;
    myCountry=0;
    _myZone=0;
    nExch=0;
    qsoPts=0;
    exchange_type=nullptr;
    for (int i=0;i<MAX_EXCH_FIELDS;i++) multFieldHighlight[i]=-1;
    for (int i=0;i< NModeTypes;i++) availableModeTypes[i]=true;
    for (int ii = 0; ii < MMAX; ii++) {
        multType[ii]=None;
        mults[ii].clear();
        _nMults[ii] = 0;
        qsoTypeCntry[ii].clear();
        qsoTypeStr[ii].clear();
        for (int k=0;k<NModeTypes;k++) {
            for (int i = 0; i <= N_BANDS; i++) {
                multsWorked[ii][k][i] = 0;
                multWorked[ii][k][i].clear();
            }
        }

    }
    score.clear();
    nextCall.clear();
    _zoneType = 0;
    _zoneMax  = 40;
    _vExch    = false;
    for (int i=0;i<6;i++) qsoCnt[i]=0;
}

/* default labels for bands in score summary */
QString Contest::bandLabel(int i) const
{
    switch (i) {
    case 0: return "160";
    case 1: return "80";
    case 2: return "40";
    case 3: return "20";
    case 4: return "15";
    case 5: return "10";
    default: return "";
    }
}

bool Contest::bandLabelEnable(int i) const
{
    Q_UNUSED(i)
    return true;
}

bool Contest::gridMults() const
{
    if (multType[0]==Grids) return true;
    else return false;
}

/* mapping for band to displayed band slot on main Ui. In most
 * cases this is 1:1, but for some contests (ARRL10 for example),
 * bands are displayed in non-standard positions. Those contests should
 * redefine this function.
 */
int Contest::highlightBand(int b,ModeTypes modeType) const
{
    Q_UNUSED(modeType)
    return b;
}

int Contest::columnCount(int col) const
{
    if (col>=0 && col<6) return qsoCnt[col];
    else return 0;
}

void Contest::initialize(const Cty *cty)
{
    setGrid();
    QByteArray tmp[MMAX];
    tmp[0]=settings.value(c_multfile1,c_multfile1_def).toString().toLatin1();
    tmp[1]=settings.value(c_multfile2,c_multfile2_def).toString().toLatin1();
    availableModeTypes[0]=settings.value(c_multimode_cw,c_multimode_cw_def).toBool();
    availableModeTypes[1]=settings.value(c_multimode_phone,c_multimode_phone_def).toBool();
    availableModeTypes[2]=settings.value(c_multimode_digital,c_multimode_digital_def).toBool();
    exchName[0]=settings.value(c_exchname1,c_exchname1_def).toString();
    exchName[1]=settings.value(c_exchname2,c_exchname2_def).toString();
    exchName[2]=settings.value(c_exchname3,c_exchname3_def).toString();
    exchName[3]=settings.value(c_exchname4,c_exchname4_def).toString();
    readMultFile(tmp, cty);
    zeroScore();
}

QByteArray Contest::contestName() const
{
    return(_contestName);
}

QString Contest::exchangeName(int i) const
{
    return exchName[i];
}

// Default column names displayed above log window. Not all
// of these are used.
QVariant Contest::columnName(int c) const
{
    switch (c) {
    case SQL_COL_NR:return QVariant("#");
    case SQL_COL_TIME:return QVariant("Time");
    case SQL_COL_FREQ:return QVariant("Freq");
    case SQL_COL_CALL:return QVariant("Call");
    case SQL_COL_BAND:return QVariant("Band");
    case SQL_COL_ADIF_MODE:return QVariant("Mode");
    case SQL_COL_DATE:return QVariant("Date");
    case SQL_COL_VALID:return QVariant("");
    case SQL_COL_PTS:return QVariant("Pts");
    case SQL_COL_RCV1:
        if (nExch>0) {
            return QVariant(FieldTypesNames[exchange_type[0]]);
        } else {
            return QVariant("");
        }
    case SQL_COL_RCV2:
        if (nExch>1) {
            return QVariant(FieldTypesNames[exchange_type[1]]);
        } else {
            return QVariant("");
        }
    case SQL_COL_RCV3:
        if (nExch>2) {
            return QVariant(FieldTypesNames[exchange_type[2]]);
        } else {
            return QVariant("");
        }
    case SQL_COL_RCV4:
        if (nExch>3) {
            return QVariant(FieldTypesNames[exchange_type[3]]);
        } else {
            return QVariant("");
        }
    case SQL_COL_SNT1:
        if (nExch>0) {
            return QVariant("Sent1");
        } else {
            return QVariant("");
        }
    case SQL_COL_SNT2:
        if (nExch>1) {
            return QVariant("Sent2");
        } else {
            return QVariant("");
        }
    case SQL_COL_SNT3:
        if (nExch>2) {
            return QVariant("Sent3");
        } else {
            return QVariant("");
        }
    case SQL_COL_SNT4:
        if (nExch>3) {
            return QVariant("Sent4");
        } else {
            return QVariant("");
        }
    default: return QVariant("");
    }
}

void Contest::setContestName(QByteArray s)
{
    _contestName=s;
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
  Determine mult index. Currently only implemented for Uniques, Special, Grids, GridFields, and Prefix mults

  also checks to see if this is a new mult
  */
void Contest::multIndx(Qso *qso) const
{
    for (int ii = 0; ii < settings.value(c_nmulttypes,c_nmulttypes_def).toInt(); ii++) {
        // unique callsigns, special mults. Check to see if this is
        // a completely new mult, and if so add it to the lists
        if (qso->isamult[ii] && (multType[ii] == Uniques || multType[ii] == Special || multType[ii] == Prefix ||
                                 multType[ii] == Grids || multType[ii] == GridFields)) {
            QByteArray tmp;
            if (multType[ii] == Uniques) {
                tmp = qso->call;
            } else {
                tmp = qso->mult_name;
            }

            if (tmp.isEmpty()) continue;

            // search for the mult
            bool newmult = true;
            int j;
            for (j = 0; j < mults[ii].size(); j++) {
                if (mults[ii][j]->name == tmp) {
                    newmult = false;
                    break;
                }
            }
            if (newmult) {
                qso->mult[ii]=-1;
                qso->isnewmult[ii]=true;
            } else {
                qso->isnewmult[ii]=false;
                qso->mult[ii]=j;
            }
        }
    }
    /* recheck newmult status for contests where mults count per-band
       if mults do not count per-mode, store them all in the CW slot */
    if (settings.value(c_multsband,c_multsband_def).toBool()) {
        for (int ii = 0; ii < settings.value(c_nmulttypes,c_nmulttypes_def).toInt(); ii++) {
            if (qso->isnewmult[ii]) continue;

            mode_t mode=CWType;
            if (settings.value(c_multsmode,c_multsmode_def).toBool()) mode=qso->modeType;
            qso->isnewmult[ii]=!multWorked[ii][mode][qso->band][qso->mult[ii]];
        }
    }
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
    newrec->modeType   = qso->modeType;

    // qsos marked invalid ignored
    if (!qso->valid) {
        score.append(newrec);
        return;
    }
    // if mults do not count per-mode, store them all in the CW slot
    mode_t mode=CWType;
    if (settings.value(c_multsmode,c_multsmode_def).toBool()) mode=qso->modeType;

    // counter for screen display of qsos
    if (!qso->dupe && qso->bandColumn>=0 && qso->bandColumn<6) qsoCnt[qso->bandColumn]++;

    // mults worked counted per-band
    // last index N_BANDS is for total number of mults regardless of band
    for (int j = 0; j < settings.value(c_nmulttypes,c_nmulttypes_def).toInt(); j++) {
        for (int k = 0; k < NModeTypes; k++) {
            for (int i = 0; i <= N_BANDS; i++) {
                multsWorked[j][k][i] = 0;
            }
        }
    }
    bool new_m[MMAX]  = { false, false };
    bool new_bm[MMAX] = { false, false };
    for (int ii = 0; ii < settings.value(c_nmulttypes,c_nmulttypes_def).toInt(); ii++) {
        // unique callsigns, special mults. Check to see if this is
        // a completely new mult, and if so add it to the lists
        if (qso->isamult[ii] && (multType[ii] == Uniques || multType[ii] == Special || multType[ii] == Prefix ||
                                 multType[ii] == Grids || multType[ii] == GridFields)) {
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
                    multWorked[ii][mode][j].insert(indx, false);
                }
                _nMults[ii] = mults[ii].size();
            }
        }
        // count number of worked mults BEFORE this qso
        int sz[NModeTypes][N_BANDS + 1];
        for (int k=0;k<NModeTypes;k++) {
            if (k>0 && !settings.value(c_multsmode,c_multsmode_def).toBool()) break;
            for (int i = 0; i <= N_BANDS; i++) {
                sz[k][i] = multWorked[ii][k][i].size();
            }
        }
        // for all-band mults: stored in (last+1) band array element
        for (int k=0;k < NModeTypes; k++) {
            if (k>0 && !settings.value(c_multsmode,c_multsmode_def).toBool()) break;
            for (int j = 0; j < sz[k][N_BANDS]; j++) {
                if (!mults[ii].at(j)->isamult) continue;
                if (multWorked[ii][k][N_BANDS].at(j)) {
                    multsWorked[ii][k][N_BANDS]++;
                }
            }
        }
        // band-by-band mults
        for (int k = 0; k < NModeTypes; k++) {
            if (k>0 && !settings.value(c_multsmode,c_multsmode_def).toBool()) break;
            for (int b = 0; b < N_BANDS; b++) {
                for (int j = 0; j < sz[k][b]; j++) {
                    if (!mults[ii].at(j)->isamult) continue;
                    if (multWorked[ii][k][b].at(j)) {
                        multsWorked[ii][k][b]++;
                    }
                }
            }
        }
    }
    // add new mult
    for (int ii = 0; ii < settings.value(c_nmulttypes,c_nmulttypes_def).toInt(); ii++) {
        if (_nMults[ii] == 0 || !qso->isamult[ii]) continue;
        if (qso->mult[ii] != -1) {
            if (!multWorked[ii][mode][qso->band][qso->mult[ii]] && mults[ii][qso->mult[ii]]->isamult) {
                new_bm[ii] = true;
                multsWorked[ii][mode][qso->band]++;
                multWorked[ii][mode][qso->band][qso->mult[ii]] = true;
            }
            // all-band mults
            if (!multWorked[ii][mode][N_BANDS][qso->mult[ii]] && mults[ii][qso->mult[ii]]->isamult) {
                new_m[ii] = true;
                multsWorked[ii][mode][N_BANDS]++;
                multWorked[ii][mode][N_BANDS][qso->mult[ii]] = true;
            }
        }
    }
    qso->newmult[0] = -1;
    qso->newmult[1] = -1;
    if (settings.value(c_multsband,c_multsband_def).toBool()) {
        // mults count per-band
        for (int ii = 0; ii < settings.value(c_nmulttypes,c_nmulttypes_def).toInt(); ii++) {
            if (new_bm[ii]) {
                qso->newmult[ii] = multFieldHighlight[ii];
            }
        }
    } else {
        for (int ii = 0; ii < settings.value(c_nmulttypes,c_nmulttypes_def).toInt(); ii++) {
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
    if (row < score.size()) {
        return(score.at(row)->dupe);
    } else {
        return false;
    }
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
    if (row < score.size()) {
        return(score.at(row)->pts);
    } else {
        return 0;
    }
}

/*! set grid square
 */
void Contest::setGrid()
{
    QByteArray s=stnSettings.value(s_grid,s_grid_def).toByteArray();
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
    for (int ii = 0; ii < settings.value(c_nmulttypes,c_nmulttypes_def).toInt(); ii++) {
        for (int k=0;k<NModeTypes;k++) {
            if (k>0 && !settings.value(c_multsmode,c_multsmode_def).toBool()) break;
            for (int i = 0; i <= N_BANDS; i++) {
                multsWorked[ii][k][i] = 0;
            }
        }
    }
    for (int ii = 0; ii < settings.value(c_nmulttypes,c_nmulttypes_def).toInt(); ii++) {
        for (int k=0;k<NModeTypes;k++) {
            if (k>0 && !settings.value(c_multsmode,c_multsmode_def).toBool()) break;
            for (int j = 0; j < N_BANDS; j++) {
                for (int i = 0; i < _nMults[ii]; i++) {
                    if (multWorked[ii][k][j].at(i)) multsWorked[ii][k][j]++;
                }
            }
        }
        // all-band mults
        for (int k=0;k<NModeTypes;k++) {
            if (k>0 && !settings.value(c_multsmode,c_multsmode_def).toBool()) break;
            for (int i = 0; i < _nMults[ii]; i++) {
                if (multWorked[ii][k][N_BANDS].at(i)) multsWorked[ii][k][N_BANDS]++;
            }
        }
    }
}

/*!
   Tries to fill in the multiplier given only the callsign for this qso

   Does not overwrite mult info determined elsewhere
 */
void Contest::guessMult(Qso *qso) const
{
    for (int ii = 0; ii < settings.value(c_nmulttypes,c_nmulttypes_def).toInt(); ii++) {
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
    for (int ii = 0; ii < settings.value(c_nmulttypes,c_nmulttypes_def).toInt(); ii++) worked[ii] = 0;

    // option 1: per-mode mults
    // this currently only applies to the ARRL 10M contest; the code below is only for this
    // special case. @todo fix for general case
    if (settings.value(c_multsmode,c_multsmode_def).toBool()) {
        for (int ii = 0; ii < settings.value(c_nmulttypes,c_nmulttypes_def).toInt(); ii++) {
            if (qso->mult[ii] != -1 && qso->mult[ii] < _nMults[ii]) {
                worked[ii] += multWorked[ii][CWType][5][qso->mult[ii]] * bits[4];
                worked[ii] += multWorked[ii][PhoneType][5][qso->mult[ii]] * bits[5];
            }
        }
        return;
    }
    // option 2: mults count per-band but not per-mode. Here the mult status is stored internally in the CW modetype slot
    if (settings.value(c_multsband,c_multsband_def).toBool()) {
        for (int ii = 0; ii < settings.value(c_nmulttypes,c_nmulttypes_def).toInt(); ii++) {
            for (int i = 0; i < N_BANDS; i++) {
                if (qso->mult[ii] != -1 && qso->mult[ii] < _nMults[ii]) {
                    worked[ii] += multWorked[ii][CWType][i][qso->mult[ii]] * bits[i];
                }
            }
        }
    } else {
        // option 3: mults count once on all bands (Sweepstakes, qso parties, etc) on any mode. Here
        // the mult status is stored in the N_BANDS slot. Note that this is only set up for HF contests.
        for (int ii = 0; ii < settings.value(c_nmulttypes,c_nmulttypes_def).toInt(); ii++) {
            if (qso->mult[ii] != -1 && qso->mult[ii] < _nMults[ii]) {
                if (multWorked[ii][CWType][N_BANDS][qso->mult[ii]] ||
                        multWorked[ii][PhoneType][N_BANDS][qso->mult[ii]] ||
                        multWorked[ii][DigiType][N_BANDS][qso->mult[ii]]) worked[ii] = 1 + 2 + 4 + 8 + 16 + 32;
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
    QDir::setCurrent(dataDirectory());
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
        for (int k=0;k<NModeTypes;k++) {
            for (int i = 0; i <= N_BANDS; i++) {
                multWorked[ii][k][i].clear();
            }
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
        } else if (multFile[ii].toLower() == "grid") {
            _nMults[ii] = 0;
            multType[ii] = Grids;
        } else if (multFile[ii].toLower() == "gridfield") {
            _nMults[ii] = 0;
            multType[ii] = GridFields;
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
        for (int k=0;k<NModeTypes;k++) {
            for (int i = 0; i <= N_BANDS; i++) {
                for (int j = 0; j < _nMults[ii]; j++) {
                    multWorked[ii][k][i].append(false);
                }
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
    for (int ii = 0; ii < settings.value(c_nmulttypes,c_nmulttypes_def).toInt(); ii++) qso->isamult[ii] = false;
    int  i, sz;
    bool ok;
    for (int ii = 0; ii < settings.value(c_nmulttypes,c_nmulttypes_def).toInt(); ii++) {
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
        case Uniques: case Grids: case GridFields:
            // unique callsigns, grids, grid fields. Anything is ok
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
            if (qso->mult_name == "") {
                qso->isamult[ii] = true;

                // fill in zone if known already from call
                // subtract 1 since mult list indexed from 0
                if (qso->zone) qso->mult[ii] = qso->zone - 1;
                break;
            }
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
    if (logFieldPrefill && n >= 0 && n < nExch) return(logFieldPrefill[n]);
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
    */
bool Contest::dupeCheckingByBand() const
{
    return(dupeCheckingEveryBand);
}

/*!
   returns total number of mults worked
*/
int Contest::nMultsWorked() const
{
    if (settings.value(c_multsband,c_multsband_def).toBool()) {
        int n = 0;
        for (int ii = 0; ii < settings.value(c_nmulttypes,c_nmulttypes_def).toInt(); ii++) {
            for (int k=0;k<NModeTypes;k++) {
                if (k>0 && !settings.value(c_multsmode,c_multsmode_def).toBool()) break;
                for (int i = 0; i < N_BANDS; i++) {
                    n += multsWorked[ii][k][i];
                }
            }
        }
        return(n);
    } else {
        // all-band mults
        int tot=0;
        for (int k=0;k<NModeTypes;k++) {
            if (k>0 && !settings.value(c_multsmode,c_multsmode_def).toBool()) break;
            tot+=multsWorked[0][k][N_BANDS];
            tot+=multsWorked[1][k][N_BANDS];
        }
        return(tot);
    }
}

/*!
   number of mults on a single band
*/
int Contest::nMultsBWorked(int ii, int band) const
{
    int tot=0;
    for (int k=0;k<NModeTypes;k++) {
        if (k>0 && !settings.value(c_multsmode,c_multsmode_def).toBool()) break;
        tot+=multsWorked[ii][k][band];
    }
    return tot;
}

/*!
   number of mults on a single band, mode
*/
int Contest::nMultsBMWorked(int ii, int band,int mode) const
{
    return multsWorked[ii][mode][band];
}

/*!
  number of mults in column col. By default this
  just returns the number of mults in the band with the same number. Some contests
  redefine this
  */
int Contest::nMultsColumn(int col,int ii) const
{
    return nMultsBWorked(ii,col);
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
        for (int k=0;k<NModeTypes;k++) {
            if (k>0 && !settings.value(c_multsmode,c_multsmode_def).toBool()) break;
            if (multWorked[ii][k][band][i]) {
                needed_band = true;
            }
        }
        for (int k=0;k<NModeTypes;k++) {
            if (k>0 && !settings.value(c_multsmode,c_multsmode_def).toBool()) break;
            if (multWorked[ii][k][N_BANDS][i]) {
                needed = true;
            }
        }
        return(mults[ii][i]->name);
    } else {
        return("");
    }
}

/*!
   return name of mult i, needed=true if it is still needed, with specific mode
  */
QByteArray Contest::neededMultNameMode(int ii, int band, ModeTypes mode,int i, bool &needed_band, bool &needed) const
{
    needed      = false;
    needed_band = false;

    if (i < _nMults[ii]) {
        if (!mults[ii][i]->isamult) {
            return("");
        }
        if (multWorked[ii][mode][band][i]) {
            needed_band = true;
        }
        if (multWorked[ii][mode][N_BANDS][i]) {
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
    if (settings.value(c_multsband,c_multsband_def).toBool()) {
        // per-band mults
        int n = 0;
        for (int ii = 0; ii < settings.value(c_nmulttypes,c_nmulttypes_def).toInt(); ii++) {
            switch (ii) {
            case 0:
                if (settings.value(c_mult1_displayonly,c_mult1_displayonly_def).toBool())
                    continue;
                break;
            case 1:
                if (settings.value(c_mult2_displayonly,c_mult2_displayonly_def).toBool())
                    continue;
                break;
            }
            for (int k=0;k<NModeTypes;k++) {
                if (k>0 && !settings.value(c_multsmode,c_multsmode_def).toBool()) break;
                for (int i = 0; i < N_BANDS; i++) {
                    n += multsWorked[ii][k][i];
                }
            }
        }
        return(qsoPts * n);
    } else {
        // all-band mults
        int n=0;
        if (!settings.value(c_mult1_displayonly,c_mult1_displayonly_def).toBool()) {
            for (int k=0;k<NModeTypes;k++) {
                if (k>0 && !settings.value(c_multsmode,c_multsmode_def).toBool()) break;
                n += multsWorked[0][k][N_BANDS];
            }
        }
        if (!settings.value(c_mult2_displayonly,c_mult2_displayonly_def).toBool()) {
            for (int k=0;k<NModeTypes;k++) {
                if (k>0 && !settings.value(c_multsmode,c_multsmode_def).toBool()) break;
                n += multsWorked[1][k][N_BANDS];
            }
        }
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
    Q_UNUSED(qso)
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
bool Contest::valExch_rst_state(int ii, int &mult_indx,Qso *qso)
{
    if (exchElement.size() == 0) return(false);

    int rstSize=3;
    if (qso->modeType==CWType || qso->modeType==DigiType) {
        finalExch[0] = "599";
    } else {
        finalExch[0] = "59";
        rstSize=2;
    }
    finalExch[1] = "";
    bool ok[2] = { false, false };

    // look for RST
    int *nr_indx = new int[exchElement.size()];
    for (int i = 0; i < exchElement.size(); i++) {
        if (exchElement[i].toInt() && exchElement[i].length()==rstSize) {
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
    return ok[1]; // RST does not have to be entered
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
        for (int k = dm_indx - 1; k >= 0; k--) {
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
   Exchange: 4 character grid square
 */
bool Contest::valExch_grid(Qso *qso)
{
    Q_UNUSED(qso)

    if (exchElement.size() == 0) return false;

    bool ok=true;
    // check to see if this is a valid grid square
    if (exchElement.at(0).size() != 4 || exchElement.at(0).at(0) < 'A' || exchElement.at(0).at(0) > 'R' ||
        exchElement.at(0).at(1) < 'A' || exchElement.at(0).at(1) > 'R' ||
        exchElement.at(0).at(2) < '0' || exchElement.at(0).at(2) > '9' ||
        exchElement.at(0).at(3) < '0' || exchElement.at(0).at(3) > '9') {
        ok = false;
    } else {
        finalExch[0]     = exchElement[0];
    }
    return ok;
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
    if (row < score.size()) {
        return score.at(row)->valid;
    } else {
        return true;
    }
}

/*!
   zeros out score and mult count
 */
void Contest::zeroScore()
{
    qsoPts = 0;
    for (int ii = 0; ii < settings.value(c_nmulttypes,c_nmulttypes_def).toInt(); ii++) {
        for (int kk=0;kk<NModeTypes;kk++) {
            for (int j = 0; j <= N_BANDS; j++) {
                for (int k = 0; k < multWorked[ii][kk][j].size(); k++) {
                    multWorked[ii][kk][j][k] = false;
                }
                multsWorked[ii][kk][j] = 0;
            }
        }

        // in these cases, the number of mults depends on the contents of the log
        if (multType[ii] == Uniques || multType[ii] == Special || multType[ii] == Prefix) {
            mults[ii].clear();
            for (int jj = 0; jj < settings.value(c_nmulttypes,c_nmulttypes_def).toInt(); jj++) {
                for (int k=0;k<NModeTypes;k++) {
                    for (int j = 0; j <= N_BANDS; j++) {
                        multWorked[jj][k][j].clear();
                    }
                }
            }
        }
    }
    for (int i = 0; i < score.size(); i++) {
        delete (score[i]);
    }
    for (int i=0;i<6;i++) qsoCnt[i]=0;
    score.clear();
}

/*!
 *For multimode contests, returns the next mode type
 *allowed in current contest
 */
ModeTypes Contest::nextModeType(ModeTypes m) const
{
    if (settings.value(c_multimode,c_multimode_def).toBool()) {
        int i=m;
        i=(i+1) % NModeTypes;
        while (i!=m) {
            if (availableModeTypes[i]) {
                return(static_cast<ModeTypes>(i));
            } else {
                i=(i+1)% NModeTypes;
            }
        }
    }
    return m;
}
