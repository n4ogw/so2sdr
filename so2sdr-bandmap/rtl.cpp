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

#include "rtl.h"
#include <QDebug>

RtlSDR::RtlSDR(QString settingsFile, QObject *parent)
    : SdrDataSource(settingsFile, parent) {
  bptr = 0;
  ptr = nullptr;
  buff = nullptr;
  rawBuff = nullptr;
  dev = nullptr;
  iptr = 0;
  stopFlag = false;
  bpmax = 0;
}

/*
 * initialize rtl-sdr
 */
void RtlSDR::initialize() {
  bpmax = sizes.chunk_size / sizes.advance_size;
  if (buff) {
    delete[] buff;
  }
  buff = new unsigned char[sizes.chunk_size];
  for (unsigned long i = 0; i < sizes.chunk_size; i++) {
    buff[i] = 0;
  }
  if (rawBuff) {
      delete[] rawBuff;
  }
  rawBuff = new unsigned char[sizes.advance_size * 8];
  for (unsigned long i = 0; i < sizes.advance_size * 8; i++) {
    rawBuff[i] = 0;
  }
  ptr = buff;
  iptr = 0;
  bptr = 0;

  rtlsdr_open(
      &dev,
      settings->value(s_sdr_rtl_dev_index, s_sdr_rtl_dev_index_def).toInt());
  if (dev == nullptr) {
    qDebug("RTL-SDR: device open failed");
  }
  if (settings->value(s_sdr_rtl_direct, s_sdr_rtl_direct_def).toBool()) {
    rtlsdr_set_direct_sampling(dev, 2);
    qDebug("RTL-SDR: direct sampling enabled");
  }
  if (settings->value(s_sdr_rtl_sample_freq, s_sdr_rtl_sample_freq_def)
          .toInt() == 262144) {
    rtlsdr_set_sample_rate(dev, 262144);
    qDebug("RTL-SDR: set sample rate 262144");
  } else if (settings->value(s_sdr_rtl_sample_freq, s_sdr_rtl_sample_freq_def)
                 .toInt() == 100000) {
    rtlsdr_set_sample_rate(dev, 1600000);
    qDebug("RTL-SDR: set sample rate 1600000");
  } else {
    rtlsdr_set_sample_rate(dev, 2048000);
    qDebug("RTL-SDR: set sample rate 2048000");
  }
  if (rfFreq == 0) {
    // use IF freq if initial RF freq not set
    rtlsdr_set_center_freq(
        dev,
        settings->value(s_sdr_rtl_if_freq, s_sdr_sample_freq_def).toUInt());
  } else {
    rtlsdr_set_center_freq(dev, rfFreq);
  }
  if (settings->value(s_sdr_rtl_tuner_gain, s_sdr_rtl_tuner_gain_def).toInt() !=
      0) {
    rtlsdr_set_tuner_gain_mode(dev, 1);
    if (!rtlsdr_set_tuner_gain(
            dev, settings->value(s_sdr_rtl_tuner_gain, s_sdr_rtl_tuner_gain_def)
                     .toInt())) {
      qDebug("RTL-SDR: set tuner gain to %f dB",
             settings->value(s_sdr_rtl_tuner_gain, s_sdr_rtl_tuner_gain_def)
                     .toInt() /
                 10.0);
    } else {
      qDebug("RTL-SDR: gain setting failed");
    }
  } else {
    rtlsdr_set_tuner_gain_mode(dev, 0);
    qDebug("RTL-SDR: tuner gain automatic");
  }
  rtlsdr_reset_buffer(dev);
  running = true;
  stopFlag = false;

  if (settings->value(s_sdr_rtl_sample_freq, s_sdr_rtl_sample_freq_def)
          .toInt() == 262144) {
    stream();
  } else {
    streamx16();
  }
}

/*
 * stream data from rtl sdr
 *
 * continues until another thread sets stopFlag = true
 *
 */
void RtlSDR::stream() {
  do {
    int n_read;

    if (stopFlag) {
        running = false;
        emit stopped();
        break;
    }

    // should check to make sure n_read == sizes.advance_size
    rtlsdr_read_sync(dev, ptr, sizes.advance_size, &n_read);
    ptr += sizes.advance_size;
    iptr += sizes.advance_size;
    if (iptr == sizes.chunk_size) {
      iptr = 0;
      ptr = buff;
    }
    emit ready(buff, bptr);
    bptr++;
    bptr = bptr % bpmax;
  } while (1);

  running = false;

  emit stopped();
}

/*
 * stream data from rtl sdr using x16 oversampling
 *  data is output as a 16-bit I/Q stream
 * continues until another thread sets stopFlag = true
 */
void RtlSDR::streamx16() {
  do {
    int n_read;

    if (stopFlag) {
        running = false;
        emit stopped();
        break;
    }

    // should check to make sure n_read == sizes.advance_size
    rtlsdr_read_sync(dev, rawBuff, sizes.advance_size * 8, &n_read);

    // perform averaging and copy data into buff
    for (long unsigned int i = 0; i < sizes.advance_size * 8; i += 32) {
      uint16_t tmpr = rawBuff[i];
      uint16_t tmpi = rawBuff[i + 1];
      for (int j = 2; j < 32; j += 2) {
        tmpr += rawBuff[i + j];
        tmpi += rawBuff[i + j + 1];
      }
      // shift right to divide
      tmpr = tmpr >> 2;
      tmpi = tmpi >> 2;

      // output 16 bit I/Q stream, LSB first
      *ptr = (unsigned char)(tmpr & 0xff);
      ptr++;
      *ptr = (unsigned char)(tmpr >> 8);
      ptr++;
      *ptr = (unsigned char)(tmpi & 0xff);
      ptr++;
      *ptr = (unsigned char)(tmpi >> 8);
      ptr++;
      iptr += 4;
    }

    if (iptr == sizes.chunk_size) {
      iptr = 0;
      ptr = buff;
    }
    emit ready(buff, bptr);
    bptr++;
    bptr = bptr % bpmax;
  } while (1);
  running = false;
  emit stopped();
}

RtlSDR::~RtlSDR() {
  if (buff) {
    delete[] buff;
  }
  if (rawBuff) {
      delete[] rawBuff;
  }
}

void RtlSDR::stop() {
  stopFlag = true;
}

/* set center frequency
 *
 * If device is running, this must be called by a different thread than the
 * thread reading data from the RTL
 */
void RtlSDR::setRfFreq(double f) {
  rfFreq = f;
  unsigned int uif = f;
  rtlsdr_set_center_freq(dev, uif);
}

unsigned int RtlSDR::sampleRate() const
{
    return settings->value(s_sdr_rtl_sample_freq, s_sdr_rtl_sample_freq_def).toUInt();
}
