/*! Copyright 2010-2023 R. Torsten Clay N4OGW

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
#include "udpreader.h"
#include "qso.h"
#include "wsjtxmessage.h"
#include <QByteArray>
#include <QColor>
#include <QDataStream>
#include <QDateTime>
#include <QDebug>
#include <QNetworkDatagram>
#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QString>
#include <QTime>

UDPReader::UDPReader(int rig, QSettings &cs, QObject *parent)
    : QObject(parent), settings(cs) {
  qRegisterMetaType<QAbstractSocket::SocketError>("socketerror");
  connect(&usocket, SIGNAL(error(QAbstractSocket::SocketError)), this,
          SLOT(tcpError(QAbstractSocket::SocketError)));
  isOpen = false;
  maxSchema = 2;
  _nrig = rig;
  _freq = 1800000.0;
  _band = 0;
  decayTime = 300;
  schema = 2;
  id.clear();
  wsjtxAddress = QHostAddress::LocalHost;
  wsjtxPort = 0;
  dbName = "WSJTX" + QString::number(_nrig + 1);
  QSqlDatabase::addDatabase("QSQLITE", dbName);
  QSqlDatabase db = QSqlDatabase::database(dbName);
  db.setDatabaseName(":memory:");
  if (db.open()) {
    QSqlQuery query(db);
    QString queryStr;
    queryStr.append("CREATE TABLE IF NOT EXISTS wsjtxcalls (`Call` TEXT "
                    "PRIMARY KEY NOT NULL, `Grid` TEXT, `Age` INT, "
                    "`SNR` INT,`Freq` INT,`RX` INT,`last` INT,`dupe` BOOLEAN, "
                    "`mult` BOOLEAN,`Seq` INT,`Msg` TEXT,"
                    "`Time` TEXT,`Dt` REAL,`Mode` TEXT,`Conf` BOOLEAN)");
    query.exec(queryStr);
    queryStr = "CREATE UNIQUE INDEX idx_call ON wsjtxcalls (call)";
    query.exec(queryStr);
    model = new QSqlTableModel(nullptr, db);
    model->setTable("wsjtxcalls");
    model->setEditStrategy(QSqlTableModel::OnFieldChange);
    model->select();
    model->setHeaderData(WSJTX_SQL_COL_CALL, Qt::Horizontal, "Call");
    model->setHeaderData(WSJTX_SQL_COL_GRID, Qt::Horizontal, "Grid");
    model->setHeaderData(WSJTX_SQL_COL_AGE, Qt::Horizontal, "Age");
    model->setHeaderData(WSJTX_SQL_COL_SNR, Qt::Horizontal, "SNR");
    model->setHeaderData(WSJTX_SQL_COL_FREQ, Qt::Horizontal, "Freq");
    model->setHeaderData(WSJTX_SQL_COL_RX, Qt::Horizontal, "RX");
    model->setHeaderData(WSJTX_SQL_COL_LAST, Qt::Horizontal, "Last");
    model->setHeaderData(WSJTX_SQL_COL_MULT, Qt::Horizontal, "Seq");
  } else {
    qDebug("ERROR: wsjtx call database could not be opened");
  }
}

UDPReader::~UDPReader() {
  delete model;
  QSqlDatabase::removeDatabase(dbName);
}

/*! close udp port
 */
void UDPReader::stop() {
  usocket.abort();
  if (usocket.state() != QAbstractSocket::UnconnectedState) {
    usocket.waitForDisconnected(1000);
  }
  isOpen = false;
}

void UDPReader::callClicked(const QModelIndex &index) {
  if (index.isValid()) {
    sendCall(index.model()
                 ->data(index.model()->index(index.row(), WSJTX_SQL_COL_CALL))
                 .toString()
                 .toLatin1());
  }
}

/*! open the udp port and start reading data
 */
void UDPReader::enable(bool b) {
  if (b) {
    if (isOpen) {
      usocket.abort();
    }
    if (!usocket.bind(
            settings.value(s_wsjtx_udp[_nrig], s_wsjtx_udp_def[_nrig]).toInt(),
            QAbstractSocket::ReuseAddressHint |
                QAbstractSocket::ShareAddress)) {
      emit(error("UDPsocket: UDP connection to wsjtx failed"));
      isOpen = false;
      return;
    }
    connect(&usocket, SIGNAL(readyRead()), this, SLOT(readDatagram()));
    isOpen = true;
  } else {
    usocket.abort();
    isOpen = false;
    disconnect(&usocket, SIGNAL(readyRead()), nullptr, nullptr);
  }
}

/*! if b=true only show non-dupe calls;
 *  if b=false show all calls including dupes
 */
void UDPReader::setDupeDisplay(bool b) {
  if (b) {
    model->setFilter("DUPE = false");
  } else {
    model->setFilter("");
  }
}

/*! clear all calls from list
 */
void UDPReader::clearCalls() {
  QSqlQuery query(QSqlDatabase::database(dbName));
  query.exec("DELETE from wsjtxcalls");
  model->select();
  while (model->canFetchMore()) {
    model->fetchMore();
  }
  // clear wsjtx colors by sending invalid QColor
  QColor invalid;
  highlightCall("", invalid, invalid);
}

/*!
 * UDPReader::decayCalls
 *
 *  updates age of calls, removes those with age > decayTime
 *  also update dupe status and multiplier status for VHF contest or WW Digi
 */
void UDPReader::decayCalls() {
  QSqlQuery query(QSqlDatabase::database(dbName));
  query.exec("SELECT * FROM wsjtxcalls");
  qint64 currentTime = QDateTime::currentDateTimeUtc().toSecsSinceEpoch();
  QStringList updates;
  updates.clear();
  while (query.next()) {
    // update age
    qint64 age = currentTime - query.value(WSJTX_SQL_COL_LAST).toLongLong();
    if (age < decayTime) {
      updates.append("UPDATE wsjtxcalls SET age = " + QString::number(age) +
                     " WHERE call LIKE '" +
                     query.value(WSJTX_SQL_COL_CALL).toString() + "';");
    } else {
      updates.append("DELETE FROM wsjtxcalls WHERE call LIKE '" +
                     query.value(WSJTX_SQL_COL_CALL).toString() + "';");
      continue;
    }
    // update dupe and mult status
    Qso qso;
    qso.call = query.value(WSJTX_SQL_COL_CALL).toByteArray();
    qso.band = _band;
    qso.modeType = DigiType;
    qso.nr = _nrig;
    qso.freq = _freq;
    qso.mult_name = query.value(WSJTX_SQL_COL_GRID).toByteArray();
    qso.valid = true;
    qso.isamult[0] = true;
    bool oldDupe = query.value(WSJTX_SQL_COL_DUPE).toBool();
    bool oldMult = query.value(WSJTX_SQL_COL_MULT).toBool();
    emit dupeCheck(&qso);
    if (qso.dupe != oldDupe || qso.isnewmult[0] != oldMult) {
      if (qso.dupe) {
        highlightCall(qso.call, Qt::white, DUPE_COLOR);
      } else if (qso.isnewmult[0]) {
        highlightCall(qso.call, Qt::white, Qt::cyan);
      } else {
        highlightCall(qso.call, Qt::white, Qt::black);
      }
      updates.append(
          "UPDATE wsjtxcalls set dupe = " +
          ((qso.dupe == true) ? QString("true") : QString("false")) +
          ", mult = " +
          ((qso.isnewmult[0] == true) ? QString("true") : QString("false")) +
          " WHERE call like '" + qso.call + "';");
    }
  }
  for (int row = 0; row < updates.size(); ++row) {
    query.exec(updates.at(row));
  }
  model->select();
  while (model->canFetchMore()) {
    model->fetchMore();
  }
}

/*! update dupe and mult status in database
 *
 * currently only grid square mults are supported, and only on VHF+ bands
 *
 */
void UDPReader::redupe() {
  QStringList updates;
  QSqlQuery query(QSqlDatabase::database(dbName));
  query.exec("UPDATE wsjtxcalls set dupe = false, mult = false");
  model->select();
  while (model->canFetchMore()) {
    model->fetchMore();
  }
  query.clear();
  query.exec("SELECT * FROM wsjtxcalls");
  while (query.next()) {
    Qso qso;
    qso.call = query.value(WSJTX_SQL_COL_CALL).toByteArray();
    qso.band = _band;
    qso.modeType = DigiType;
    qso.nr = _nrig;
    bool oldDupe = query.value(WSJTX_SQL_COL_DUPE).toBool();
    bool oldMult = query.value(WSJTX_SQL_COL_MULT).toBool();
    qso.freq = _freq;
    qso.valid = true;
    qso.mult_name = query.value(WSJTX_SQL_COL_GRID).toByteArray();
    qso.isamult[0] = true;
    emit(dupeCheck(&qso));
    if (qso.dupe != oldDupe || qso.isnewmult[0] != oldMult) {
      if (qso.dupe) {
        highlightCall(qso.call, Qt::white, DUPE_COLOR);
      } else if (qso.isnewmult[0]) {
        highlightCall(qso.call, Qt::white, Qt::cyan);
      } else {
        highlightCall(qso.call, Qt::white, Qt::black);
      }
      updates.append(
          "UPDATE wsjtxcalls set dupe = " +
          ((qso.dupe == true) ? QString("true") : QString("false")) +
          ", mult = " +
          ((qso.isnewmult[0] == true) ? QString("true") : QString("false")) +
          " WHERE call like '" + qso.call + "';");
    }
  }
  model->select();
  while (model->canFetchMore()) {
    model->fetchMore();
  }
  for (int row = 0; row < updates.size(); ++row) {
    query.exec(updates.at(row));
  }
  model->select();
  while (model->canFetchMore()) {
    model->fetchMore();
  }
}

/*! Read datagrams from the udp socket
 */
void UDPReader::readDatagram() {
  while (usocket.hasPendingDatagrams()) {
    QNetworkDatagram datagram = usocket.receiveDatagram();
    processDatagram(datagram);
  }
}

/*! process wsjtx datagrams
 *  See file NetworkMessage.hpp in wsjt-x source code for info on format
 */
void UDPReader::processDatagram(QNetworkDatagram datagram) {
  wsjtxAddress = datagram.senderAddress();
  if (datagram.senderPort() >= 0) {
    wsjtxPort = static_cast<quint16>(datagram.senderPort());
  }
  QDataStream in(datagram.data());
  quint32 i1;
  in >> i1 >> schema;
  if (i1 == magic) {
    if (schema <= 1) {
      in.setVersion(QDataStream::Qt_5_0); // Qt schema version
    }
#if QT_VERSION >= 0x050200
    else if (schema <= 2) {
      in.setVersion(QDataStream::Qt_5_2); // Qt schema version
    }
#endif
#if QT_VERSION >= 0x050400
    else if (schema <= 3) {
      in.setVersion(QDataStream::Qt_5_4); // Qt schema version
    }
#endif
    else {
      return;
    }
    quint32 key;
    in >> key;
    switch (key) {
    case 0: // heartbeat
    {
      in >> wsjtxId >> maxSchema >> wsjtxVersion >> wsjtxRevision;
      break;
    }
    case 1: // status
      break;
    case 2: // decode
    {
      in >> id;
      bool newDecode;
      in >> newDecode;
      QTime time;
      in >> time;
      // if decode is older than max age, ignore it
      if (time.secsTo(QTime::currentTime()) > decayTime)
        break;
      qint32 snr;
      in >> snr;
      double dt;
      in >> dt;
      quint32 delta;
      in >> delta;
      QByteArray mode, msg;
      bool conf;
      in >> mode >> msg >> conf;
      // figure out which sequence
      int s = time.second();
      int seq = 1;
      switch (mode.at(0)) {
      case '~': // FT8
        // 1st = 0,30 sec
        // 2nd = 15,45 sec
        if (!(s % 30)) {
          seq = 1;
        } else {
          seq = 2;
        }
        break;
      case '+': // FT4
        // 1st = 0,15,30,45
        if (!(s % 15)) {
          seq = 1;
        } else {
          seq = 2;
        }
        break;
      case '&': // MSK144
        // default 1st=0,30
        // sequence time is user configurable so this can fail
        if (!(s % 15)) {
          seq = 1;
        } else {
          seq = 2;
        }
        break;
      case '#':
      case ':':
      case '$':
      case '@': // JT4,JT9,JT65, QRA64
        // 60 sec sequence
        int m = time.minute();
        if (!(m % 2)) {
          seq = 1;
        } else {
          seq = 2;
        }
        break;
      }
      QByteArray call, grid;
      call.clear();
      grid.clear();
      if (decode(msg, call, grid)) {
        int rx = 1;
        Qso qso;
        qso.call = call;
        qso.band = _band;
        qso.modeType = DigiType;
        qso.nr = _nrig;
        qso.dupe = false;
        qso.valid = true;
        qso.freq = _freq;
        qso.isamult[0] = true;
        qso.isnewmult[0] = false;
        QSqlQuery query(QSqlDatabase::database(dbName));
        query.exec("SELECT * FROM wsjtxcalls where call LIKE '" + call + "'");
        QByteArray oldGrid;
        while (query.next()) {
          // call already in list; increment rx count
          rx = query.value(WSJTX_SQL_COL_RX).toInt();
          rx++;
          oldGrid = query.value(WSJTX_SQL_COL_GRID).toByteArray();
        }
        if (grid.isEmpty() && !oldGrid.isEmpty()) {
          grid = oldGrid;
        }
        qso.mult_name = grid;
        emit dupeCheck(&qso);
        if (qso.dupe) {
          highlightCall(qso.call, Qt::white, DUPE_COLOR);
        } else if (qso.isnewmult[0]) {
          highlightCall(qso.call, Qt::white, Qt::cyan);
        } else {
          highlightCall(qso.call, Qt::white, Qt::black);
        }
        query.clear();
        query.prepare("REPLACE INTO wsjtxcalls "
                      "(call,snr,grid,freq,rx,age,last,dupe,mult,seq,msg,time,"
                      "dt,mode,conf)"
                      "VALUES "
                      "(:call,:snr,:grid,:freq,:rx,:age,:last,:dupe,:mult,:seq,"
                      ":msg,:time,:dt,:mode,:conf)");
        query.bindValue(":call", call);
        query.bindValue(":grid", grid);
        query.bindValue(":snr", snr);
        query.bindValue(":freq", delta);
        query.bindValue(":rx", rx);
        query.bindValue(":age", 0);
        query.bindValue(":dupe", qso.dupe);
        query.bindValue(":seq", seq);
        query.bindValue(":mult", qso.isnewmult[0] || qso.isnewmult[1]);
        query.bindValue(":msg", msg);
        query.bindValue(":time", time.toString());
        query.bindValue(":dt", dt);
        query.bindValue(":mode", mode);
        query.bindValue(":conf", conf);
        // use current time rather than time from wsjtx; prevents problems when
        // day changes
        query.bindValue(":last",
                        QDateTime::currentDateTimeUtc().toSecsSinceEpoch());
        query.exec();
        model->select();
        while (model->canFetchMore()) {
          model->fetchMore();
        }
      }
      break;
    }
    case 3: // decodes cleared
      break;
    case 5: // qso logged
    {
      Qso qso;
      QByteArray id, grid;
      in >> id >> qso.time >> qso.call >> grid;
      quint64 f;
      in >> f;
      qso.nr = _nrig;
      qso.freq = _freq;
      qso.band = _band;
      if (qso.band != _band) {
        qDebug("Warning: wsjtx and radio are set to different bands!");
      }
      QByteArray repSent, repRecv, txPower, comments, name, opCall, myCall,
          myGrid;
      QDateTime timeOn;
      in >> qso.adifMode >> repSent >> repRecv >> txPower >> comments >> name >>
          timeOn >> opCall;
      in >> myCall >> myGrid;
      in >> qso.snt_exch[0] >> qso.exch;
      qso.mult_name = grid;
      if (qso.exch.isEmpty()) {
        qso.exch = grid;
      }
      qso.dupe = true;
      emit wsjtxQso(&qso);
      // mark call as dupe
      QSqlQuery query(QSqlDatabase::database(dbName));
      QString queryStr =
          "UPDATE wsjtxcalls SET dupe= true, mult = false WHERE call LIKE '" +
          qso.call + "'";
      query.exec(queryStr);
      model->select();
      while (model->canFetchMore()) {
        model->fetchMore();
      }
      break;
    }
    case 6: // shutting down
      break;
    case 10: // WSPR decode
      break;
    case 11: // logged ADIF
      break;
    }
  }
}

/*! decode wsjt-x received messages. Extracts call and grid of station sending
 * msg
 *
 * returns true if at least call could be decoded
 * will fail for station in grid "RR73"
 *
 * Currently works for general operating and NA VHF contest exchanges
 *
 * @todo These will need revising for special contests in wsjtx (RTTY Roundup,
 * Field Day, etc) In that case, may need to get special exchange mode from
 * wsjtx and adjust decoding to match
 */
bool UDPReader::decode(QByteArray msg, QByteArray &call, QByteArray &grid) {
  bool ret = false;
  QList<QByteArray> list = msg.split(' ');
  // signal report  XXXXX XXXXX R+05
  // grid unknown, just return call
  if (list.size() == 3 &&
      (list.last().contains('+') || list.last().contains('-'))) {
    call = list.at(list.size() - 2);
    grid.clear();
    ret = true;
  } else if (list.size() >= 3 && list.last().size() == 4 &&
             list.last() != "RR73") {
    // look for last element being grid square
    // check to see if this is a valid grid square. This fails in the (unlikely)
    // case there is someone in grid RR73!
    if ((list.last().at(0) >= 'A' && list.last().at(0) <= 'R') &&
        (list.last().at(1) >= 'A' && list.last().at(1) <= 'R') &&
        (list.last().at(2) >= '0' && list.last().at(2) <= '9') &&
        (list.last().at(3) >= '0' && list.last().at(3) <= '9')) {
      call = list.at(list.size() - 2);
      // XXXX XXXX R GRID in NA Contest mode?
      if (list.size() == 4 && call == "R") {
        call = list.at(1);
      }
      grid = list.last();
      ret = true;
    }
  } else if (list.size() == 3 &&
             (list.last() == "73" || list.last() == "RR73" ||
              list.last() == "RRR")) {
    // station sending 73, RR73, RRR; grid unknown, just return call
    call = list.at(list.size() - 2);
    grid.clear();
    ret = true;
  }
  if (call == "<...>") {
    call.clear();
    grid.clear();
    ret = false;
  }
  if (call.contains('<')) {
    call.replace('<', "");
  }
  if (call.contains('>')) {
    call.replace('>', "");
  }
  if (grid.size() != 4)
    grid.clear();
  return ret;
}

/*! called on udp port error.
 * @todo display this in an error box
 */
void UDPReader::tcpError(QAbstractSocket::SocketError err) {
  Q_UNUSED(err)
  QString errstring = "UDPReader: " + usocket.errorString();
  qDebug("%s", errstring.toLatin1().data());
}

/*! send a call from a double-clicked row to wsjtx, and generate messages
 */
void UDPReader::sendCall(QByteArray call) {
  if (id.isEmpty() || wsjtxPort == 0)
    return;
  QSqlQuery query(QSqlDatabase::database(dbName));
  query.exec("SELECT * FROM wsjtxcalls where call LIKE '" + call + "'");
  if (query.next()) {
    QByteArray data;
    WsjtxMessage out(&data, QIODevice::WriteOnly);
    out.setSchema(schema);
    out.startMessage(Reply);
    out << QTime::fromString(query.value(WSJTX_SQL_COL_TIME).toString())
        << query.value(WSJTX_SQL_COL_SNR).toInt()
        << query.value(WSJTX_SQL_COL_DT).toDouble()
        << query.value(WSJTX_SQL_COL_FREQ).toUInt()
        << query.value(WSJTX_SQL_COL_MODE).toByteArray()
        << query.value(WSJTX_SQL_COL_MSG).toByteArray()
        << static_cast<bool>(false) << static_cast<quint8>(0)
        << query.value(WSJTX_SQL_COL_CONF).toBool();
    usocket.writeDatagram(data, wsjtxAddress, wsjtxPort);
  }
}

/*! clear color highlights in wsjtx
 */
void UDPReader::clearColors() {
  if (id.isEmpty() || wsjtxPort == 0)
    return;
  QByteArray data;
  WsjtxMessage out(&data, QIODevice::WriteOnly);
  out.setSchema(schema);
  out.startMessage(HighlightCallsign);
  QColor invalid;
  out << QByteArray("") << invalid << invalid << static_cast<bool>(false);
  usocket.writeDatagram(data, wsjtxAddress, wsjtxPort);
}

/*! color highlight a call in wsjtx
 */
void UDPReader::highlightCall(QByteArray call, QColor bg, QColor fg) {
  if (id.isEmpty() || wsjtxPort == 0)
    return;
  QByteArray data;
  WsjtxMessage out(&data, QIODevice::WriteOnly);
  out.setSchema(schema);
  out.startMessage(HighlightCallsign);
  out << call << bg << fg << static_cast<bool>(false);
  usocket.writeDatagram(data, wsjtxAddress, wsjtxPort);
}

/*! sends wsjtx command to resend all decodes in Band Activity window
 */
void UDPReader::replay() {
  if (id.isEmpty() || wsjtxPort == 0)
    return;
  QByteArray data;
  WsjtxMessage out(&data, QIODevice::WriteOnly);
  out.setSchema(schema);
  out.startMessage(Replay);
  usocket.writeDatagram(data, wsjtxAddress, wsjtxPort);
}

/*! sends wsjtx command to resend all decodes in Band Activity window
 */
void UDPReader::clearWsjtxCalls() {
  if (id.isEmpty() || wsjtxPort == 0)
    return;
  QByteArray data;
  WsjtxMessage out(&data, QIODevice::WriteOnly);
  out.setSchema(schema);
  out.startMessage(Clear);
  out << static_cast<quint8>(2);
  usocket.writeDatagram(data, wsjtxAddress, wsjtxPort);
}
