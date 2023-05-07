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
#ifndef SO2RMINI_H
#define SO2RMINI_H
#include <QByteArray>
#include <QSerialPort>
#include <QSettings>

/*
 * Support for So2rMini device (with N6TR firmware)
 *
 */
class SO2RMini : public QObject {
  Q_OBJECT

public:
  SO2RMini(QSettings &s, QObject *parent = nullptr);
  ~SO2RMini();
  void cancelcw();
  void loadbuff(QByteArray msg);
  void sendcw();
  bool SO2RMiniIsOpen() const;
  void switchAudio(int nr);
  void toggleStereo(int nr);
  void setSpeed(int s);
  void switchTransmit(int nr);
  bool stereoActive() const;
  void sendCommand(QByteArray command);

signals:
  void finished();
  void miniError(const QString &);
  void miniName(const QByteArray &);
  void tx(bool, int);

public slots:
  void openSO2RMini();

private slots:
  void receive();

private:
  char relayBits;
  bool SO2RMiniOpen;
  bool sending;
  bool stereo;
  int txRig;
  QSerialPort *SO2RMiniPort;
  QSettings &settings;
  QByteArray buffer;
  QByteArray deviceName;
  void closeSO2RMini();
};

#endif // SO2RMINI_H
