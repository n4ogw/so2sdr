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

#ifndef RTL_H
#define RTL_H
#include "sdrdatasource.h"
#include <rtl-sdr.h>

class RtlSDR : public SdrDataSource {
  Q_OBJECT

public:
  RtlSDR(QString settingsFile, QObject *parent = nullptr);
  ~RtlSDR();
  unsigned int sampleRate() const;

public slots:
  void stop();
  void initialize();
  void setRfFreq(double f);

private:
  void stream();
  void streamx16();

  rtlsdr_dev_t *dev;
  unsigned char *buff;
  unsigned char *rawBuff;
  unsigned char *ptr;
  unsigned int bpmax;
  unsigned int bptr;
  unsigned int iptr;
};

#endif
