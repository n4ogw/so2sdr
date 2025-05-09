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
#ifndef LOG_H
#define LOG_H
#include "contest.h"
#include "cty.h"
#include "defines.h"
#include "detailededit.h"
#include "logdelegate.h"
#include "logedit.h"
#include "qso.h"
#include <QDateTime>
#include <QEvent>
#include <QFile>
#include <QList>
#include <QObject>
#include <QSettings>

/*!
   class defining log database structure and related functions
 */
class Log : public QObject {
  Q_OBJECT

public:
  Log(QSettings &cs, QSettings &s, QObject *parent);
  ~Log();

  void addQso(Qso *qso);
  QString bandLabel(int i) const;
  bool bandLabelEnable(int i) const;
  int columnCount(int col) const;
  QVariant columnName(int c) const;
  ContestType contestType() const;
  Cty *ctyPtr() const;
  QWidget *currentEditor() const;
  logDelegate *delegate() const;
  bool detailIsVisible() const;
  bool dupeCheckingByBand() const;
  void editLogDetail(QModelIndex index);
  QString exchangeName(int) const;
  FieldTypes exchType(int i) const;
  bool exportADIF(QFile *) const;
  void exportCabrillo(QFile *, QString call, QString, QString, QString,
                      QString) const;
  int fieldWidth(int col) const;
  bool gridMults() const;
  void guessMult(Qso *qso) const;
  bool hasPrefill() const;
  int highlightBand(int b, ModeTypes modeType = CWType) const;
  int idPfx(Qso *qso, bool &qsy) const;
  void importCabrillo(QString cabFile);
  void initializeContest();
  void isDupe(Qso *qso, bool DupeCheckingEveryBand, bool FillWorked) const;
  bool isEditing() const;
  int lastNr() const;
  bool logSearch(QByteArray searchFrag);
  void logSearchClear();
  tableModel *mod();
  QString mySunTimes() const;
  QByteArray neededMultName(int ii, int band, int i, bool &needed_band,
                            bool &needed) const;
  QByteArray neededMultNameMode(int ii, int band, ModeTypes mode, int i,
                                bool &needed_band, bool &needed) const;
  int nExch() const;
  ModeTypes nextModeType(ModeTypes m) const;
  bool newCall(QByteArray &s) const;
  int nMults(int ii) const;
  int nMultsBWorked(int ii, int band) const;
  int nMultsColumn(int col, int ii) const;
  int nQso(int band) const;
  QString offTime(int minOffTime, QDateTime start, QDateTime end);
  bool openLogFile(QString fname);
  void postEditorEvent(QEvent *event);
  QByteArray prefillExchange(Qso *qso);
  unsigned int rcvFieldShown() const;
  QSqlRecord record(QModelIndex index);
  void rescore();
  int rowCount() const;
  int score() const;
  void searchPartial(Qso *qso, const QByteArray &part, QList<QByteArray> &calls,
                     QList<unsigned int> &worked, QList<int> &mult1,
                     QList<int> &mult2);
  void selectContest();
  void setContinent(Cont);
  void setCountry(int);
  void setLatLon(double la, double lo);
  void setMyZone(int zone);
  void setZoneType(int);
  bool showQsoPtsField() const;
  unsigned int sntFieldShown() const;
  void startDetailedEdit();
  void updateHistory();
  bool validateExchange(Qso *qso);
  bool validMult(int ii, int j) {
    if (contest) {
      return contest->validMult(ii, j);
    } else
      return false;
  }

  void workedMults(Qso *qso, unsigned int worked[MMAX]) const;
  void zeroScore();
  int zoneType() const;

signals:
  void addQsoHistory(const Qso *qso);
  void clearDupe();
  void dataChanged(QModelIndex, QModelIndex);
  void errorMessage(QString);
  void logEditDone(QSqlRecord, QSqlRecord);
  void multByBandEnabled(bool);
  void progressCnt(int);
  void progressMax(int);
  void setExch1(const QString &);
  void setExch2(const QString &);
  void setExch3(const QString &);
  void setExch4(const QString &);
  void startLogEdit();
  void update();

public slots:
  void mobileDupeCheck(Qso *qso);
  void setOrigRecord(QModelIndex index);
  void updateRecord(QSqlRecord &r);

private slots:
  void startQsoEditRow(QModelIndex index);
  void startDetailedQsoEditRow(QModelIndex index);
  void finishEdit(int row, QSqlRecord &r);

private:
  logDelegate *logdel;
  bool logSearchFlag;
  Contest *contest;
  Cty *cty;
  DetailedEdit *detail;
  double lat;
  double lon;
  int qsoCnt[N_BANDS];
  QList<int> searchList;
  QSettings &csettings;
  QSettings &settings;
  QSqlRecord origEditRecord;
  tableModel *model;
};

#endif
