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
#ifndef CONTEST_H
#define CONTEST_H
#include <QObject>
#include <QByteArray>
#include <QSettings>
#include <QString>
#include <QFile>
#include <QList>
#include "cty.h"
#include "defines.h"
#include "qso.h"

/*! structure recording point and multiplier information for
   scoring each qso */
typedef struct scoreRecord
{
    bool dupe;
    bool valid;
    int  pts;
    int  mult[MMAX];
    int  newmult[MMAX];
} scoreRecord;
Q_DECLARE_TYPEINFO(scoreRecord, Q_PRIMITIVE_TYPE);


/*!
   Contest: defines rules, scoring, exchange for generic contests
 */
class Contest : public QObject
{
Q_OBJECT

public:
    Contest();
    virtual ~Contest();

    void addQsoMult(Qso *qso);
    void addQsoType(QByteArray str, int ii);
    virtual void addQso(Qso *qso) = 0;
    QByteArray contestName() const;
    virtual QString cabrilloName() const
    {
        return("");
    }
    void copyFinalExch(bool validated, Qso *qso);
    void determineMultType(Qso *qso);
    bool dupe(int row) const;
    bool dupeCheckingByBand() const;
    FieldTypes exchType(int) const;
    virtual int fieldWidth(int col) const = 0;
    void guessMult(Qso *qso) const;
    bool hasPrefill() const;
    void initialize(QSettings *ss,QSettings *cs,const Cty *cty);
    int isAMult(QByteArray exch, int ii) const;
    bool logPrefill(int n) const;
    int myZone() const;
    QByteArray neededMultName(int ii, int band, int i, bool &needed_band, bool &needed) const;
    bool newCall(QByteArray &) const;
    int mult(int row, int ii) const;
    void multIndx(Qso *qso) const;
    int newMult(int row, int ii) const;
    int nExchange() const;
    ModeTypes nextModeType(ModeTypes m) const;
    int nMults(int ii) const;
    int nMultsWorked() const;
    int nMultsBWorked(int ii, int band) const;
    virtual int numberField() const = 0;
    int points(int row) const;
    virtual QByteArray prefillExchange(Qso *qso);
    QList<QByteArray> &qsoType(int ii);
    virtual unsigned int rcvFieldShown() const = 0;
    void readMultFile(QByteArray filename[MMAX], const Cty * cty);
    int rstField() const;
    virtual int Score() const;
    void setContestName(QByteArray s);
    void setContinent(Cont);
    void setCountry(int);
    void setDataDirectory(QString);
    virtual void setupContest(QByteArray MultFile[MMAX], const Cty * cty) = 0;
    void setMyZone(int);
    void setVExch(bool);
    void setZoneMax(int);
    void setZoneType(int);
    virtual unsigned int sntFieldShown() const = 0;
    bool valid(int row) const;
    virtual bool validateExchange(Qso *qso)    = 0;
    bool vExch() const;
    void workedMults(Qso * qso, unsigned int worked[MMAX]) const;
    void zeroScore();
    int zoneMax() const;
    int zoneType() const;
signals:
    void mobileDupeCheck(Qso *qso);
    void clearDupe();

protected:
    bool                 availableModeTypes[NModeTypes];
    bool                 dupeCheckingEveryBand;
    bool                 *logFieldPrefill;
    bool                 prefill;
    bool                 _vExch;
    Cont                 myContinent;
    double               myLat;
    double               myLon;
    FieldTypes           *exchange_type;
    int                  multFieldHighlight[MAX_EXCH_FIELDS];
    int                  multsWorked[MMAX][N_BANDS + 1];
    int                  myCountry;
    int                  _myZone;
    int                  nExch;
    int                  _nMults[MMAX];
    int                  qsoPts;
    int                  _zoneMax;
    int                  _zoneType;
    MultTypeDef          multType[MMAX];
    QByteArray           _contestName;
    QByteArray           *finalExch;
    QByteArray           multFile[MMAX];
    QByteArray           myGrid;
    QByteArray           nextCall;
    QList<bool>          isMultCntry[MMAX];
    QList<bool>          multWorked[MMAX][N_BANDS + 1];
    QList<DomMult *>     mults[MMAX];
    QList<int>           qsoTypeCntry[MMAX];
    QList<QByteArray>    exchElement;
    QList<QByteArray>    qsoTypeStr[MMAX];
    QList<scoreRecord *> score;
    QString              dataDirectory;
    QSettings            *settings;
    QSettings            *stnSettings;

    void addQsoTypeCntry(int i, int ii);
    void count_mults();
    void fillDefaultRST(Qso *qso) const;
    void setGrid();
    void selectCountries(int ii, const Cty *cty, Cont cont);
    bool separateExchange(Qso *qso);
    bool valExch_mm(Qso *qso);
    bool valExch_rst_state(int ii, int &mult_indx);
    bool valExch_rst_zone(int ii, int &mult_indx);
    bool valExch_nr_name_state(int ii, int &mult_indx);
    bool valExch_name_state(int ii, int &mult_indx);
    bool valExch_nr_name();
    bool valExch_rst_nr(Qso *qso);
    bool valExch_rst_name(Qso *qso);
};

#endif // CONTEST_H
