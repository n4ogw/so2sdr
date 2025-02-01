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
#include "cwdaemon.h"
#include "defines.h"
#include <QDebug>
#include <QNetworkDatagram>
#include <QSettings>
#include <QTimer>

Cwdaemon::Cwdaemon(QSettings &s, QObject *parent)
    : QObject(parent), settings(s) {
  sendBuff.clear();
  sent.clear();
  sending = false;
  open_[0] = false;
  open_[1] = false;
  sendingCmd = false;
  txRig = 0;
  rigNum = 0;
  udpPort[0] = 6789;
  udpPort[1] = 6790;
}

Cwdaemon::~Cwdaemon() {
  cancelcw();
  close();
}

/*!
   returns true if CWdaemon is present
 */
bool Cwdaemon::isOpen(int nrig) const { return (open_[nrig]); }

/*!
   returns true if sending cw
 */
bool Cwdaemon::isSending() const { return (sending); }

/*!
   Slot triggered when data is available at port
 */
void Cwdaemon::receive1() {
  while (socket[0].hasPendingDatagrams()) {
    QNetworkDatagram datagram = socket[0].receiveDatagram();
    sending = false;
    // received h indicating message finished playing
    if (datagram.data().at(0) == 'h') {
      open_[0] = true;
      emit tx(false, txRig);
      QTimer::singleShot(queueDelay, this, SIGNAL(finished()));
    }
  }
}
/*!
   Slot triggered when data is available at port
 */
void Cwdaemon::receive2() {
  while (socket[1].hasPendingDatagrams()) {
    QNetworkDatagram datagram = socket[1].receiveDatagram();
    sending = false;
    // received h indicating message finished playing
    if (datagram.data().at(0) == 'h') {
      open_[1] = true;
      emit tx(false, txRig);
      QTimer::singleShot(queueDelay, this, SIGNAL(finished()));
    }
  }
}

/*!
   load a message into buffer
 */
void Cwdaemon::loadbuff(QByteArray msg) {
  if (open_[rigNum]) {
    sendBuff.append(msg);
    sent = QString::fromLatin1(sendBuff);

    sent.remove('|'); // remove half spaces
  } else {
    // if cwdaemon not open, just echo back the sent text as if it had been sent
    QString tmp = QString::number(rigNum + 1) + ":" + QString::fromLatin1(msg);
    tmp.remove('|');
    emit textSent(tmp, 3600);
  }
}

/*!
   Slot to start sending cw
 */
void Cwdaemon::sendcw() {
  const char buff[2] = {0x1b, 0x68}; // <ESC> h
  socket->writeDatagram(
      buff, 2, QHostAddress::LocalHost,
      settings.value(s_cwdaemon_udp[rigNum], s_cwdaemon_udp_def[rigNum])
          .toInt());
  socket->writeDatagram(
      sendBuff.data(), sendBuff.size(), QHostAddress::LocalHost,
      settings.value(s_cwdaemon_udp[rigNum], s_cwdaemon_udp_def[rigNum])
          .toInt());
  sendBuff.clear();
  sending = true;
  txRig = rigNum;
  emit tx(true, rigNum);
}

void Cwdaemon::setUdpPort(int nrig, int p) { udpPort[nrig] = p; }

/*!
   cancel sending
 */
void Cwdaemon::cancelcw() {
  if (!open_[rigNum])
    return;

  char cmd[2] = {0x1b, 0x34};
  socket->writeDatagram(
      cmd, 2, QHostAddress::LocalHost,
      settings.value(s_cwdaemon_udp[rigNum], s_cwdaemon_udp_def[rigNum])
          .toInt());
  emit cwCanceled();
}

/*!
   set speed directly in WPM
 */
void Cwdaemon::setSpeed(int speed) {
  if (!open_[rigNum])
    return;

  char cmd[2] = {0x1b, 0x32};
  QByteArray s = QByteArray(cmd) + QByteArray::number(speed);
  socket->writeDatagram(
      s, s.size(), QHostAddress::LocalHost,
      settings.value(s_cwdaemon_udp[rigNum], s_cwdaemon_udp_def[rigNum])
          .toInt());
}

/*!
   choose radio to send on
 */
void Cwdaemon::switchTransmit(int nrig) { rigNum = nrig; }

/*!
  set up connections to cwdaemon
 */
void Cwdaemon::open() {
  open_[0] = false;
  open_[1] = false;
  socket[0].disconnectFromHost();
  socket[1].disconnectFromHost();
  disconnect(&socket[0], SIGNAL(readyRead()));
  disconnect(&socket[1], SIGNAL(readyRead()));

  connect(&socket[0], SIGNAL(readyRead()), this, SLOT(receive1()));
  connect(&socket[1], SIGNAL(readyRead()), this, SLOT(receive2()));

  // send a dummy message to port to verify if cwdaemon is present
  for (int i = 0; i < NRIG; i++) {
    const char buff[2] = {0x1b, 0x68}; // <ESC> h
    sendBuff = " ";
    socket[i].writeDatagram(
        buff, 2, QHostAddress::LocalHost,
        settings.value(s_cwdaemon_udp[i], s_cwdaemon_udp_def[i]).toInt());
    socket[i].writeDatagram(
        sendBuff.data(), sendBuff.size(), QHostAddress::LocalHost,
        settings.value(s_cwdaemon_udp[i], s_cwdaemon_udp_def[i]).toInt());
    sendBuff.clear();
  }
}

void Cwdaemon::close() {
  open_[0] = false;
  open_[1] = false;
}
