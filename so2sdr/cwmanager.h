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
#ifndef CWMANAGER_H
#define CWMANAGER_H

#include "cwdaemon.h"
#include "defines.h"
#include "winkey.h"
#include <QByteArray>
#include <QObject>
#include <QSettings>
#include <QString>

typedef enum cwtype { modeNone = 0, modeWinkey = 1, modeCwdaemon = 2 } cwtype;

class CWManager : public QObject {
  Q_OBJECT
public:
  explicit CWManager(QSettings &s, QObject *parent = nullptr);
  ~CWManager();
  void loadbuff(QByteArray msg);
  bool isSending() const;
  void sendcw();
  void switchTransmit(int nrig);
  void setEchoMode(bool b);
  void setSpeed(int speed);
  QString textStatus() const;

signals:
  void cwCanceled();
  void finished();
  void textSent(const QString &t, int);
  void winkeyVersion(int ver);
  void tx(bool, int);
  void CWError(const QString &);

public slots:
  void cancelcw();
  void open();
  void setType(cwtype t);

private:
  cwtype mode;
  QSettings &settings;
  Winkey *winkey;
  Cwdaemon *cwdaemon;
};

#endif // CWMANAGER_H
