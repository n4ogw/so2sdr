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
#ifndef AFEDRI_H
#define AFEDRI_H

#include "network.h"
#include <QAbstractSocket>
#include <QObject>
#include <QString>

#define TARGET_NAME "AFEDRI SDR Network"
#define SERIAL_NUMBER "AN000102"
#define IF_VERSION 0x101
#define BOOT_CODE_VER 0x000
#define APP_FW_VER 0x112
#define HW_VER 0x000
#define PRODUCT_ID 0x03524453L

#define MAX_UDP_SIZE 1044

const QString afedriNames[4] = {
    QStringLiteral("AFEDRI SDR-Net"), QStringLiteral("SDR-IP"),
    QStringLiteral("AFE822x SDR-Net"), QStringLiteral("Unknown")};

class Afedri : public NetworkSDR {
  Q_OBJECT
public:
  explicit Afedri(const QString &settingsFile, QObject *parent = nullptr);
  ~Afedri();
  unsigned int sampleRate() const override;
  bool isSlave() const override;

public slots:
  void stop() override;
  void initialize() override;
  void setRfFreq(double f) override;
  void setRfFreqChannel(double f, int c) override;

private slots:
  void readDatagram();
  void readTcp();

private:
  void get_clock_freq();
  void get_real_sample_rate();
  void get_sdr_name();
  void set_broadcast_flag(bool);
  void set_freq(unsigned long frequency, int channel);
  void set_multichannel_mode(int channel);
  void set_sample_rate(unsigned long sample_rate);
  void stopAfedri();

  unsigned int clockFreq;
  unsigned int realSampleRate;
  QString name;
};

#endif // AFEDRI_H
