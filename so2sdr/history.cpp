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

#include "history.h"
#include "defines.h"
#include "utils.h"
#include <QFileInfo>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QString>
#include <QVariant>

History::History(QSettings &s, QObject *parent)
    : QObject(parent), csettings(s) {
  QSqlDatabase::addDatabase("QSQLITE", "HISTORY");
  isopen = false;
}

History::~History() { QSqlDatabase::removeDatabase("HISTORY"); }

/*!
 * \brief History::addQso update history database from qso
 * \param qso
 * \param newqso
 */
void History::addQso(const Qso *qso) {
  QString query;
  query.append("INSERT OR REPLACE INTO history "
               "(Call,General,DMult,Name,State,ARRLSection,Grid,Number,Zone) ");
  query.append("SELECT new.Call, old.General, old.DMult, old.Name, old.State, "
               "old.ARRLSection, old.Grid, old.Number, old.Zone ");
  query.append("FROM ( SELECT '" + QVariant(qso->call).toString() +
               "' AS Call");
  for (int i = 0; i < qso->n_exchange; i++) {
    switch (qso->exchange_type[i]) {
    case General:
      query.replace(QString("old.General"), QString("new.General"));
      query.append(", '" + QVariant(qso->rcv_exch[i]).toString() +
                   "' AS General");
      break;
    case DMult:
      query.replace(QString("old.DMult"), QString("new.DMult"));
      query.append(", '" + QVariant(qso->rcv_exch[i]).toString() +
                   "' AS DMult");
      break;
    case Name:
      query.replace(QString("old.Name"), QString("new.Name"));
      query.append(", '" + QVariant(qso->rcv_exch[i]).toString() + "' AS Name");
      break;
    case State:
      query.replace(QString("old.State"), QString("new.State"));
      query.append(", '" + QVariant(qso->rcv_exch[i]).toString() +
                   "' AS State");
      break;
    case ARRLSection:
      query.replace(QString("old.ARRLSection"), QString("new.ARRLSection"));
      query.append(", '" + QVariant(qso->rcv_exch[i]).toString() +
                   "' AS ARRLSection");
      break;
    case Grid:
      query.replace(QString("old.Grid"), QString("new.Grid"));
      query.append(", '" + QVariant(qso->rcv_exch[i]).toString() + "' AS Grid");
      break;
    case Number:
      query.replace(QString("old.Number"), QString("new.Number"));
      query.append(", '" + QVariant(qso->rcv_exch[i]).toString() +
                   "' AS Number");
      break;
    case Zone:
      query.replace(QString("old.Zone"), QString("new.Zone"));
      query.append(", '" + QVariant(qso->rcv_exch[i]).toString() + "' AS Zone");
      break;
    default:
      break;
    }
  }
  query.append(") AS new LEFT JOIN ( SELECT "
               "Call,General,DMult,Name,State,ARRLSection,Grid,Number,Zone "
               "FROM history ) AS old ON new.Call = old.Call");
  QSqlQuery h(QSqlDatabase::database("HISTORY"));
  h.exec(query);
  query.clear();
}

/*!
 * \brief History::fillExchange copy exchange from history database to Qso
 * prefill field \param qso current qso \param part partial callsign
 */
void History::fillExchange(Qso *qso, const QByteArray &part) {
  QSqlQueryModel h;
  QString query;
  query.append("SELECT Call");
  for (int i = 0; i < qso->n_exchange; i++) {

    switch (qso->exchange_type[i]) {
    case General:
      query.append(",General");
      break;
    case DMult:
      query.append(",DMult");
      break;
    case Name:
      query.append(",Name");
      break;
    case State:
      query.append(",State");
      break;
    case ARRLSection:
      query.append(",ARRLSection");
      break;
    case Grid:
      query.append(",Grid");
      break;
    case Number:
      query.append(",Number");
      break;
    case Zone:
      query.append(",Zone");
      break;
    default:
      break;
    }
  }
  query.append(" FROM history where Call = '" + part + "'");
  h.setQuery(query, QSqlDatabase::database("HISTORY"));
  if (h.rowCount()) {
    for (int i = 1; i < h.columnCount(); i++) { // skip call field
      qso->prefill.append(h.record(0).value(i).toString().toLatin1());
      if (i < (h.columnCount() - 1)) {
        qso->prefill.append(" ");
      }
    }
  }
  h.clear();
}

bool History::isOpen() { return isopen; }

/*! initialize the history database. If historyfile is not found, it will be
 * created.
 *
 */
void History::startHistory() {
  QSqlDatabase history = QSqlDatabase::database("HISTORY");
  QString filename =
      csettings.value(c_historyfile, c_historyfile_def).toString();
  QFileInfo info(userDirectory() + "/" + filename);
  if (info.exists()) {
    history.setDatabaseName(userDirectory() + "/" + filename);
    if (!history.open()) {
      emit message("ERROR: can't open history file " + userDirectory() + "/" +
                       filename,
                   3000);
    } else {
      isopen = true;
      emit message(
          "History file " + userDirectory() + "/" + filename + " loaded", 3000);
    }
  } else { // create new history file
    history.setDatabaseName(userDirectory() + "/" + filename);
    if (!history.open()) {
      emit message("ERROR: can't open history file " + userDirectory() + "/" +
                       filename,
                   3000);
    } else {
      isopen = true;
      QSqlQuery h(history);
      QString query;
      query.append(
          "CREATE TABLE IF NOT EXISTS history (`Call` TEXT PRIMARY KEY, "
          "`General` TEXT, `DMult` TEXT, `Name` TEXT, `State` TEXT, "
          "`ARRLSection` TEXT, `Grid` TEXT, `Number` TEXT, `Zone` TEXT)");
      h.exec(query);
      query.clear();
      query.append("CREATE UNIQUE INDEX `call_idx` ON history (`Call`)");
      h.exec(query);
      query.clear();
      emit message("INFO: History file " + userDirectory() + "/" + filename +
                       " created",
                   3000);
    }
  }
}
