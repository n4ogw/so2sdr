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
#include "cwmanager.h"

CWManager::CWManager(QSettings &s, QObject *parent)
    : QObject(parent), settings(s) {
  mode = (cwtype)settings.value(s_cw_device, s_cw_device_def).toInt();
  winkey = new Winkey(s);
  cwdaemon = new Cwdaemon(s);
  connect(winkey, SIGNAL(version(int)), this, SIGNAL(winkeyVersion(int)));
  connect(winkey, SIGNAL(winkeyError(const QString &)), this,
          SIGNAL(CWError(const QString &)));
  connect(cwdaemon, SIGNAL(error(const QString &)), this,
          SIGNAL(CWError(const QString &)));
  connect(winkey, SIGNAL(finished()), this, SIGNAL(finished()));
  connect(cwdaemon, SIGNAL(finished()), this, SIGNAL(finished()));
  connect(winkey, SIGNAL(winkeyTx(bool, int)), this, SIGNAL(tx(bool, int)));
  connect(cwdaemon, SIGNAL(tx(bool, int)), this, SIGNAL(tx(bool, int)));
  connect(winkey, SIGNAL(textSent(const QString &, int)), this,
          SIGNAL(textSent(const QString &, int)));
  connect(cwdaemon, SIGNAL(textSent(const QString &, int)), this,
          SIGNAL(textSent(const QString &, int)));
  miniSending = false;
}

CWManager::~CWManager() {
  delete winkey;
  delete cwdaemon;
}

void CWManager::cancelcw() {
  switch (mode) {
  case modeNone:
    break;
  case modeWinkey:
    winkey->cancelcw();
    break;
  case modeCwdaemon:
    cwdaemon->cancelcw();
    break;
  case modeSo2rMini:
    emit so2rMiniCancelCW();
    break;
  }
}

bool CWManager::isSending() const {
  switch (mode) {
  case modeNone:
    return false;
    break;
  case modeWinkey:
    return winkey->isSending();
    break;
  case modeCwdaemon:
    return cwdaemon->isSending();
    break;
  case modeSo2rMini:
    return miniSending;
    break;
  }
  return false;
}

void CWManager::loadbuff(QByteArray msg) {
  switch (mode) {
  case modeNone:
    break;
  case modeWinkey:
    winkey->loadbuff(msg);
    break;
  case modeCwdaemon:
    cwdaemon->loadbuff(msg);
    break;
  case modeSo2rMini:
    emit(so2rMiniLoadbuff(msg));
    break;
  }
}

void CWManager::sendcw() {
  switch (mode) {
  case modeNone:
    break;
  case modeWinkey:
    winkey->sendcw();
    break;
  case modeCwdaemon:
    cwdaemon->sendcw();
    break;
  case modeSo2rMini:
    emit so2rMiniSendCW();
    break;
  }
}

void CWManager::setSpeed(int speed) {
  switch (mode) {
  case modeNone:
    break;
  case modeWinkey:
    winkey->setSpeed(speed);
    break;
  case modeCwdaemon:
    cwdaemon->setSpeed(speed);
    break;
  case modeSo2rMini:
    emit so2rMiniSpeed(speed);
    break;
  }
}

void CWManager::setType(cwtype t) {
  mode = t;
  open();
}

void CWManager::open() {
  switch (mode) {
  case modeNone:
    break;
  case modeWinkey:
    winkey->openWinkey();
    break;
  case modeCwdaemon:
    cwdaemon->open();
    break;
  case modeSo2rMini:
    // this is opened in so2r dialog
    break;
  }
}

void CWManager::setEchoMode(bool b) {
  switch (mode) {
  case modeNone:
  case modeCwdaemon:
  case modeSo2rMini:
    break;
  case modeWinkey:
    winkey->setEchoMode(b);
    break;
  }
}

void CWManager::switchTransmit(int nrig) {
  switch (mode) {
  case modeNone:
    break;
  case modeWinkey:
    winkey->switchTransmit(nrig);
    break;
  case modeCwdaemon:
    cwdaemon->switchTransmit(nrig);
    break;
  case modeSo2rMini:
    // transmit switching for mini is in so2r.h
    break;
  }
}

QString CWManager::textStatus() const {
  switch (mode) {
  case modeNone:
    return QString("");
    break;
  case modeWinkey:
    if (winkey->winkeyIsOpen())
      return (QString("WK:ON"));
    else
      return (QString("<font color=#FF0000>WK:OFF </font>"));
    break;
  case modeCwdaemon:
    if (cwdaemon->isOpen(0) || cwdaemon->isOpen(1))
      return (QString("CW:ON"));
    else
      return (QString("<font color=#FF0000>CW:OFF </font>"));
    break;
  case modeSo2rMini:
    break;
  }
  return QString("");
}
