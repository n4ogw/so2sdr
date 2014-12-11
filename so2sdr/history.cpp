/*! Copyright 2010-2015 R. Torsten Clay N4OGW

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

#include "defines.h"
#include "log.h"
#include "utils.h"
#include "history.h"
#include <QFileInfo>
#include <QString>
#include <QVariant>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlQueryModel>
#include <QSettings>

History::History(QSettings& s,QObject *parent) :
    QObject(parent),csettings(s)
{
}

History::~History()
{
    stopHistory();
}

/*!
 * \brief History::addQso update history database from qso
 * \param qso
 * \param newqso
 */
void History::addQso(const Qso *qso)
{
    if (!history.isOpen()) return;

    QString query;
    query.append("INSERT OR REPLACE INTO history (Call,General,DMult,Name,State,ARRLSection,Grid,Number) ");
    query.append("SELECT new.Call, old.General, old.DMult, old.Name, old.State, old.ARRLSection, old.Grid, old.Number ");
    query.append("FROM ( SELECT '" + QVariant(qso->call).toString() + "' AS Call");
    for (int i = 0; i < qso->n_exchange; i++) {
        switch (qso->exchange_type[i]) {
            case General:
                query.replace(QString("old.General"), QString("new.General"));
                query.append(", '" + QVariant(qso->rcv_exch[i]).toString() + "' AS General");
                break;
            case DMult:
                query.replace(QString("old.DMult"), QString("new.DMult"));
                query.append(", '" + QVariant(qso->rcv_exch[i]).toString() + "' AS DMult");
                break;
            case Name:
                query.replace(QString("old.Name"), QString("new.Name"));
                query.append(", '" + QVariant(qso->rcv_exch[i]).toString() + "' AS Name");
                break;
            case State:
                query.replace(QString("old.State"), QString("new.State"));
                query.append(", '" + QVariant(qso->rcv_exch[i]).toString() + "' AS State");
                break;
            case ARRLSection:
                query.replace(QString("old.ARRLSection"), QString("new.ARRLSection"));
                query.append(", '" + QVariant(qso->rcv_exch[i]).toString() + "' AS ARRLSection");
                break;
            case Grid:
                query.replace(QString("old.Grid"), QString("new.Grid"));
                query.append(", '" + QVariant(qso->rcv_exch[i]).toString() + "' AS Grid");
                break;
            case Number:
                query.replace(QString("old.Number"), QString("new.Number"));
                query.append(", '" + QVariant(qso->rcv_exch[i]).toString() + "' AS Number");
                break;
            default:
                break;
        }
    }
    query.append(") AS new LEFT JOIN ( SELECT Call,General,DMult,Name,State,ARRLSection,Grid,Number FROM history ) AS old ON new.Call = old.Call");
    QSqlQuery h(history);
    h.exec(query);
    query.clear();
}

/*!
 * \brief History::fillExchange copy exchange from history database to Qso prefill field
 * \param qso current qso
 * \param part partial callsign
 */
void History::fillExchange(Qso *qso,QByteArray part)
{
    if (!history.isOpen()) return;
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
        default:
            break;
        }
    }
    query.append(" FROM history where Call = '" + part + "'");
    h.setQuery(query, history);
    if (h.rowCount()) {
        for (int i = 1; i < h.columnCount(); i++) { // skip call field
            qso->prefill.append(h.record(0).value(i).toString().toAscii());
            if (i < (h.columnCount() - 1)) {
                qso->prefill.append(" ");
            }
        }
    }
    h.clear();
}

bool History::isOpen()
{
    return history.open();
}

/*! initialize the history database. If historyfile is not found, it will be created.
 *
 */
void History::startHistory()
{
    if (history.isOpen()) {
        history.close();
        history = QSqlDatabase();
        QSqlDatabase::removeDatabase("HISTORY");
    }
    QString filename=csettings.value(c_historyfile,c_historyfile_def).toString();
    QFileInfo info(userDirectory() + "/" + filename);
    if (info.exists()) {
        history = QSqlDatabase::addDatabase("QSQLITE", "HISTORY");
        history.setDatabaseName(userDirectory() + "/" + filename);
        if (!history.open()) {
            history = QSqlDatabase();
            QSqlDatabase::removeDatabase("HISTORY");
            emit(message("ERROR: can't open history file " + userDirectory() + "/" + filename,3000));
        } else {
            emit(message("History file " + userDirectory() + "/" + filename + " loaded",3000));
        }
    } else { // create new history file
        history = QSqlDatabase::addDatabase("QSQLITE", "HISTORY");
        history.setDatabaseName(userDirectory() + "/" + filename);
        history.open();

        QSqlQuery h(history);
        QString query;
        query.append("CREATE TABLE IF NOT EXISTS history (`Call` TEXT PRIMARY KEY, `General` TEXT, `DMult` TEXT, `Name` TEXT, `State` TEXT, `ARRLSection` TEXT, `Grid` TEXT, `Number` TEXT)");
        h.exec(query);
        query.clear();
        query.append("CREATE UNIQUE INDEX `call_idx` ON history (`Call`)");
        h.exec(query);
        query.clear();
        emit(message("INFO: History file " + userDirectory() + "/" + filename + " created",3000));
    }
}

void History::stopHistory()
{
    if (history.isOpen()) {
        history.close();
        history = QSqlDatabase();
        QSqlDatabase::removeDatabase("HISTORY");
    }
}
