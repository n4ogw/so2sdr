/*! Copyright 2010-2025 R. Torsten Clay N4OGW

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
#include "cty.h"
#include "defines.h"
#include "qso.h"
#include <QByteArray>
#include <QFile>
#include <QList>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QVariant>

/*! structure recording point and multiplier information for
   scoring each qso */
typedef struct scoreRecord {
  bool dupe;
  bool valid;
  int pts;
  int mult[MMAX];
  int newmult[MMAX];
  ModeTypes modeType;
} scoreRecord;
Q_DECLARE_TYPEINFO(scoreRecord, Q_PRIMITIVE_TYPE);

/*!
   Contest: defines rules, scoring, exchange for generic contests
 */
class Contest : public QObject {
  Q_OBJECT

public:
  Contest(QSettings &cs, QSettings &ss);
  virtual ~Contest();

  void addQsoMult(Qso *qso);
  void addQsoType(const QByteArray &str, int ii);
  virtual void addQso(Qso *qso) = 0;
  virtual QString bandLabel(int i) const;
  virtual bool bandLabelEnable(int i) const;
  QByteArray contestName() const;
  int columnCount(int col) const;
  virtual QVariant columnName(int c) const;
  virtual ContestType contestType() const = 0;
  void copyFinalExch(bool validated, Qso *qso);
  void determineMultType(Qso *qso);
  bool dupe(int row) const;
  bool dupeCheckingByBand() const;
  QString exchangeName(int) const;
  FieldTypes exchType(int) const;
  virtual int fieldWidth(int col) const = 0;
  void guessMult(Qso *qso) const;
  bool gridMults() const;
  bool hasPrefill() const;
  virtual bool hasWorked() const;
  virtual int highlightBand(int b, ModeTypes modeType = CWType) const;
  void initialize(const Cty *cty);
  int isAMult(QByteArray exch, int ii) const;
  bool logPrefill(int n) const;
  int myZone() const;
  QByteArray neededMultName(int ii, int band, int i, bool &needed_band,
                            bool &needed) const;
  QByteArray neededMultNameMode(int ii, int band, ModeTypes mode, int i,
                                bool &needed_band, bool &needed) const;
  bool newCall(QByteArray &) const;
  int mult(int row, int ii) const;
  void multIndx(Qso *qso) const;
  MultTypeDef contestMultType(int ii) { return multType[ii]; }
  int newMult(int row, int ii) const;
  int nExchange() const;
  ModeTypes nextModeType(ModeTypes m) const;
  int nMults(int ii) const;
  virtual int nMultsColumn(int col, int ii) const;
  virtual int nMultsWorked() const;
  int nMultsBWorked(int ii, int band) const;
  int nMultsBMWorked(int ii, int band, int mode) const;
  virtual int numberField() const = 0;
  int points(int row) const;
  virtual QByteArray prefillExchange(Qso *qso);
  QList<QByteArray> &qsoType(int ii);
  virtual unsigned int rcvFieldShown() const = 0;
  void readMultFile(QByteArray filename[MMAX], const Cty *cty);
  virtual int rstField() const { return -1; }
  virtual int Score() const;
  void setContestName(const QByteArray &s);
  void setContinent(Cont);
  void setCountry(int);
  virtual void setupContest(QByteArray MultFile[MMAX], const Cty *cty) = 0;
  void setMyZone(int);
  void setVExch(bool);
  void setZoneMax(int);
  void setZoneType(int);
  virtual bool showQsoPtsField() const = 0;
  virtual unsigned int sntFieldShown() const { return 0; }
  bool valid(int row) const;
  bool validMult(int ii, int j) {
    return mults[ii].at(j)->isamult;
  }
  virtual bool validateExchange(Qso *qso) = 0;
  bool vExch() const;
  virtual void workedMults(Qso *qso, unsigned int worked[MMAX]) const;
  virtual void workedQso(ModeTypes m, int band, unsigned int &worked) const;
  void zeroScore();
  int zoneMax() const;
  int zoneType() const;
signals:
  void mobileDupeCheck(Qso *qso);
  void clearDupe();

protected:
  bool availableModeTypes[NModeTypes];
  bool dupeCheckingEveryBand;
  bool *logFieldPrefill;
  bool prefill;
  bool _vExch;
  Cont myContinent;
  double myLat;
  double myLon;
  FieldTypes *exchange_type;
  int multFieldHighlight[MAX_EXCH_FIELDS];
  int multsWorked[MMAX][NModeTypes][N_BANDS + 1];
  int myCountry;
  int _myZone;
  int nExch;
  int _nMults[MMAX];
  int qsoCnt[6];
  int qsoPts;
  int _zoneMax;
  int _zoneType;
  MultTypeDef multType[MMAX];
  QByteArray _contestName;
  QByteArray *finalExch;
  QByteArray multFile[MMAX];
  QByteArray myGrid;
  QByteArray nextCall;
  QList<bool> isMultCntry[MMAX];
  QList<bool> multWorked[MMAX][NModeTypes][N_BANDS + 1];
  QList<DomMult *> mults[MMAX];
  QList<int> qsoTypeCntry[MMAX];
  QList<QByteArray> exchElement;
  QList<QByteArray> qsoTypeStr[MMAX];
  QList<scoreRecord *> score;
  QSettings &settings;
  QSettings &stnSettings;
  QString exchName[MAX_EXCH_FIELDS];

  void addQsoTypeCntry(int i, int ii);
  void count_mults();
  void fillDefaultRST(Qso *qso) const;
  void setGrid();
  void selectCountries(int ii, const Cty *cty, Cont cont);
  bool separateExchange(Qso *qso);
  bool valExch_grid(Qso *qso);
  bool valExch_mm(Qso *qso);
  bool valExch_rst_state(int ii, int &mult_indx, Qso *qso);
  bool valExch_rst_zone(int ii, int &mult_indx);
  bool valExch_nr_name_state(int ii, int &mult_indx);
  bool valExch_name_state(int ii, int &mult_indx);
  bool valExch_nr_name();
  bool valExch_rst_nr(Qso *qso);
  bool valExch_rst_name(Qso *qso);
};

#endif // CONTEST_H
