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

#ifndef NETWORK_H
#define NETWORK_H
#include "sdrdatasource.h"
#include <QObject>
#include <QTcpSocket>
#include <QUdpSocket>

class NetworkSDR : public SdrDataSource {
  Q_OBJECT
public:
  explicit NetworkSDR(const QString &settingsFile, QObject *parent = nullptr);
  ~NetworkSDR();
  unsigned int sampleRate() const override;
  bool isSlave() const override { return false; }

public slots:
  void stop() override;
  void initialize() override;
  void setRfFreq(double f) override;
  void setRfFreqChannel(double f, int c) override {
    Q_UNUSED(f)
    Q_UNUSED(c)
  }

private slots:
  void readDatagram();
  void tcpError(QAbstractSocket::SocketError err);
  void readTcp();

protected:
  void send_rx_command(int);
  void get_name();
  void stopNetwork();

  QTcpSocket tsocket;
  QUdpSocket usocket;
  unsigned char *buff;
  unsigned int bpmax;
  unsigned int bptr;
  unsigned int iptr;

private:
  void set_sample_rate(unsigned long sample_rate);
};

#endif
