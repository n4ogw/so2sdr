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
#ifndef UDPREADER_H
#define UDPREADER_H

#include "qso.h"
#include "utils.h"
#include <QByteArray>
#include <QColor>
#include <QDataStream>
#include <QHostAddress>
#include <QIODevice>
#include <QModelIndex>
#include <QNetworkDatagram>
#include <QObject>
#include <QSettings>
#include <QSqlTableModel>
#include <QString>
#include <QUdpSocket>

const quint32 magic = 0xadbccbda;

const int WSJTX_SQL_COL_CALL = 0;
const int WSJTX_SQL_COL_GRID = 1;
const int WSJTX_SQL_COL_AGE = 2;
const int WSJTX_SQL_COL_SNR = 3;
const int WSJTX_SQL_COL_FREQ = 4;
const int WSJTX_SQL_COL_RX = 5;
const int WSJTX_SQL_COL_LAST = 6;
const int WSJTX_SQL_COL_DUPE = 7;
const int WSJTX_SQL_COL_MULT = 8;
const int WSJTX_SQL_COL_SEQ = 9;
const int WSJTX_SQL_COL_MSG = 10;
const int WSJTX_SQL_COL_TIME = 11;
const int WSJTX_SQL_COL_DT = 12;
const int WSJTX_SQL_COL_MODE = 13;
const int WSJTX_SQL_COL_CONF = 14;
const int WSJTX_SQL_COL_NCOL = 15;

// from wsjtx NetworkMessage.hpp
enum WsjtxMessageType {
  Heartbeat,
  Status,
  Decode,
  Clear,
  Reply,
  QSOLogged,
  Close,
  Replay,
  HaltTx,
  FreeText,
  WSPRDecode,
  Location,
  LoggedADIF,
  HighlightCallsign,
  SwitchConfiguration,
  Configure,
  maximum_message_type_
};

// interface for external UDP log broadcasts (WSJTX)
class UDPReader : public QObject {
  Q_OBJECT
public:
  explicit UDPReader(int rig, QSettings &cs, QObject *parent = nullptr);
  bool isEnabled() const { return isOpen; }
  int nrig() const { return _nrig; }
  void redupe();
  void setDupeDisplay(bool);
  void setFreq(double f) {
    _freq = f;
    _band = getBand(_freq);
  }
  ~UDPReader();
  QSqlTableModel *tableModel() const { return model; }

signals:
  void dupeCheck(Qso *qso);
  void error(const QString &);
  void wsjtxQso(Qso *);

public slots:
  void callClicked(const QModelIndex &index);
  void clearCalls();
  void clearWsjtxCalls();
  void decayCalls();
  void enable(bool);
  void replay();
  void stop();

private slots:
  void readDatagram();
  void tcpError(QAbstractSocket::SocketError err);

private:
  bool isOpen;
  double _freq;
  int _band;
  int _nrig;
  int decayTime;
  quint32 schema;
  QByteArray id;
  QByteArray wsjtxId;
  QByteArray wsjtxVersion;
  QByteArray wsjtxRevision;
  quint32 maxSchema;
  QUdpSocket usocket;
  QSettings &settings;
  QSqlTableModel *model;
  QString dbName;
  QHostAddress wsjtxAddress;
  quint16 wsjtxPort;

  void clearColors();
  bool decode(QByteArray msg, QByteArray &call, QByteArray &grid);
  void highlightCall(const QByteArray &call, QColor bg, QColor fg);
  void processDatagram(QNetworkDatagram datagram);
  void sendCall(const QByteArray &call);
};

#endif // UDPREADER_H
