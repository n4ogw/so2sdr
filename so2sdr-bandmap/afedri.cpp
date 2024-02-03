/*! Copyright 2010-2024 R. Torsten Clay N4OGW

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

#include "afedri.h"
#include "afedri-cmd.h"
#include "defines.h"
#include "sdr-ip.h"
#include <QDebug>
#include <QThread>

// wait time after each Afedri command (ms)
#define AFEDRI_WAIT 100

Afedri::Afedri(const QString &settingsFile, QObject *parent)
    : NetworkSDR(settingsFile, parent) {
  clockFreq = 0;
  realSampleRate = 0;
}

/*! initialize and start sdr
 */
void Afedri::initialize() {
  delete[] buff;
  bpmax = sizes.chunk_size / sizes.advance_size;
  buff = new unsigned char[sizes.chunk_size];
  for (unsigned long i = 0; i < sizes.chunk_size; i++) {
    buff[i] = 0;
  }
  realSampleRate = settings
                       ->value(s_sdr_afedri_real_sample_freq,
                               s_sdr_afedri_real_sample_freq_def)
                       .toUInt();
  if (tsocket.isOpen()) {
    tsocket.close();
    if (tsocket.state() != QAbstractSocket::UnconnectedState) {
      tsocket.waitForDisconnected(1000);
    }
  }
  if (settings->value(s_sdr_afedri_bcast, s_sdr_afedri_bcast_def).toInt() ==
      0) {
    // non-broadcast connection
    tsocket.connectToHost(
        settings->value(s_sdr_afedri_tcp_ip, s_sdr_afedri_tcp_ip_def)
            .toString(),
        settings->value(s_sdr_afedri_tcp_port, s_sdr_afedri_tcp_port_def)
            .toInt());
    tsocket.setSocketOption(QAbstractSocket::LowDelayOption, QVariant(1));
    connect(&tsocket, SIGNAL(readyRead()), this, SLOT(readTcp()));
    if (tsocket.waitForConnected()) {
      set_broadcast_flag(false);
      if (!usocket.bind(
              settings->value(s_sdr_afedri_udp_port, s_sdr_afedri_udp_port_def)
                  .toInt(),
              QUdpSocket::ShareAddress)) {
        emit error("Afedri: UDP connection failed");
        running = false;
        return;
      }
      connect(&usocket, SIGNAL(readyRead()), this, SLOT(readDatagram()));
      get_sdr_name();
      get_clock_freq();
      set_sample_rate(
          settings
              ->value(s_sdr_afedri_sample_freq, s_sdr_afedri_sample_freq_def)
              .toInt());
      set_multichannel_mode(
          settings->value(s_sdr_afedri_multi, s_sdr_afedri_multi_def).toInt());
      send_rx_command(RCV_START);
      // set initial frequency to frequency in setup dialog if not zero
      if (settings->value(s_sdr_afedri_freq, s_sdr_afedri_freq_def)
              .toULongLong() != 0) {
        set_freq(settings->value(s_sdr_afedri_freq, s_sdr_afedri_freq_def)
                     .toULongLong(),
                 settings->value(s_sdr_afedri_channel, s_sdr_afedri_channel_def)
                     .toInt());
      }
    }
  } else if (settings->value(s_sdr_afedri_bcast, s_sdr_afedri_bcast_def)
                 .toInt() == 1) {
    // broadcast connection, master
    tsocket.connectToHost(
        settings->value(s_sdr_afedri_tcp_ip, s_sdr_afedri_tcp_ip_def)
            .toString(),
        settings->value(s_sdr_afedri_tcp_port, s_sdr_afedri_tcp_port_def)
            .toInt());
    tsocket.setSocketOption(QAbstractSocket::LowDelayOption, QVariant(1));
    connect(&tsocket, SIGNAL(readyRead()), this, SLOT(readTcp()));
    if (tsocket.waitForConnected()) {
      get_sdr_name();
      get_clock_freq();
      set_sample_rate(
          settings
              ->value(s_sdr_afedri_sample_freq, s_sdr_afedri_sample_freq_def)
              .toInt());
      set_multichannel_mode(
          settings->value(s_sdr_afedri_multi, s_sdr_afedri_multi_def).toInt());
      set_broadcast_flag(true);
      connect(&usocket, SIGNAL(readyRead()), this, SLOT(readDatagram()));

      if (!usocket.bind(
              settings->value(s_sdr_afedri_udp_port, s_sdr_afedri_udp_port_def)
                  .toInt(),
              QUdpSocket::ReuseAddressHint | QUdpSocket::ShareAddress)) {
        emit error("Afedri: UDP connection failed");
        running = false;
        return;
      }

      send_rx_command(RCV_START);
      // set initial frequency to frequency in setup dialog if not zero
      if (settings->value(s_sdr_afedri_freq, s_sdr_afedri_freq_def)
              .toULongLong() != 0) {
        set_freq(settings->value(s_sdr_afedri_freq, s_sdr_afedri_freq_def)
                     .toULongLong(),
                 settings->value(s_sdr_afedri_channel, s_sdr_afedri_channel_def)
                     .toInt());
      }
    } else {
      qDebug("tcp not connected");
    }
  } else {
    // broadcast connection, slave. Only connect to udp
    connect(&usocket, SIGNAL(readyRead()), this, SLOT(readDatagram()));

    if (!usocket.bind(
            settings->value(s_sdr_afedri_udp_port, s_sdr_afedri_udp_port_def)
                .toInt(),
            QUdpSocket::ReuseAddressHint | QUdpSocket::ShareAddress)) {
      emit error("Afedri: UDP connection failed");
      running = false;
      return;
    }

    running = true;
  }

  // if RF mode, set flag so next frequency set command will also tune sdr
  if (settings->value(s_sdr_mode, s_sdr_mode_def).toInt() != IF) {
    emit resetRfFlag();
  }
  running = true;
  get_real_sample_rate();
}

/*!
 * \brief Afedri::set_freq
 *  Set frequency
 * \param frequency  Frequency (Hz)
 * \param channel (0,1,2,3)
 */
void Afedri::set_freq(unsigned long frequency, int channel) {

  if (tsocket.state() != QAbstractSocket::ConnectedState ||
      tsocket.state() == QAbstractSocket::ClosingState) {
    return;
  }

  // slave cannot set frequency
  if (isSlave())
    return;

  // channels are 0, 1, 2, 3 in rest of code; Afedri expects 0, 2, 3, 4
  if (channel > 0)
    channel++;

  // version 1
  unsigned short control_code = CI_FREQUENCY;
  const int size = 10;
  char block[size];

  block[0] = size;
  block[1] = (SET_CONTROL_ITEM << 5);
  block[2] = control_code & 0xFF;
  block[3] = (control_code >> 8) & 0xFF;
  block[4] = channel;
  for (int i = 0; i < 4; i++) {
    block[5 + i] = (frequency >> (i * 8)) & 0xFF;
  }
  block[9] = 0;

  // version 2 using Afedri HID command set
  // does not seem to work
  /*
  const int size = 9;
  char block[size];
  block[0] = size;
  block[1] = TCP_HID_PACKET << 5;
  block[2] = HID_FREQUENCY_REPORT;
  for (int i = 0; i < 4; i++) {
    block[3 + i] = (frequency >> (i * 8)) & 0xFF;
  }
  block[7] = channel;
  block[8] = 0;
*/

  if (tsocket.write(block, size) == -1) {
    emit error("Afedri: TCP write set_frequency failed");
  }
  tsocket.flush();
  tsocket.waitForReadyRead(1000);
  QThread::msleep(AFEDRI_WAIT);
}

/*! sets single, dual, qual channel modes
 * channel=0 : single
 * channel=1 : dual
 * channel=2 : quad
 */
void Afedri::set_multichannel_mode(int channel) {

  if (tsocket.state() != QAbstractSocket::ConnectedState ||
      tsocket.state() == QAbstractSocket::ClosingState)
    return;

  if (isSlave())
    return;

  const int size = 9;
  char block[size];

  block[0] = 9;
  block[1] = TCP_HID_PACKET << 5; // 0xE0 TCP header
  block[2] = HID_GENERIC_REPORT;
  block[3] = HID_GENERIC_SET_MULTICHANNEL_COMMAND;
  switch (channel) {
  case 0: // single
    block[4] = 0;
    break;
  case 1: // dual
    block[4] = 2;
    break;
  case 2: // quad
    block[4] = 5;
    break;
  }
  block[5] = 0;
  block[6] = 0;
  block[7] = 0;
  block[8] = 0;
  if (tsocket.write(block, size) == -1) {
    emit error("Afedri: TCP write error, set_multichannel_mode");
  }
  tsocket.flush();
  tsocket.waitForReadyRead(1000);
  QThread::msleep(AFEDRI_WAIT);
}

/*!
 * \brief Afedri::set_sample_rate
 *    Set the Afedri sampling rate
 * \param rate  sample rate in Hz
 *
 */
void Afedri::set_sample_rate(unsigned long sample_rate) {
  unsigned short control_code = CI_DDC_SAMPLE_RATE;
  const int size = 9;
  char block[size];

  if (tsocket.state() != QAbstractSocket::ConnectedState ||
      tsocket.state() == QAbstractSocket::ClosingState)
    return;

  if (isSlave())
    return;

  block[0] = size;
  block[1] = (SET_CONTROL_ITEM << 5);
  block[2] = control_code & 0xFF;
  block[3] = (control_code >> 8) & 0xFF;
  block[4] = 0;
  for (int i = 0; i < 4; i++) {
    block[5 + i] = (sample_rate >> (i * 8)) & 0xFF;
  }
  if (tsocket.write(block, size) == -1) {
    emit error("Afedri: TCP write error, set_sample_rate");
  }
  tsocket.flush();
  tsocket.waitForReadyRead(1000);
  QThread::msleep(AFEDRI_WAIT);
}

/*!
 * \brief Afedri::set_broadcast_flag
 * \param b true=broadcast on false=off
 *
 * Sets Afedri net UDP broadcast option
 *
 */
void Afedri::set_broadcast_flag(bool b) {
  if (tsocket.state() != QAbstractSocket::ConnectedState ||
      tsocket.state() == QAbstractSocket::ClosingState)
    return;

  if (isSlave())
    return;

  const int size = 9;
  char block[size];

  block[0] = 9;
  block[1] = TCP_HID_PACKET << 5;
  block[2] = HID_GENERIC_REPORT;
  block[3] = HID_GENERIC_BROADCAST_COMMAND;
  if (b) {
    block[4] = 1;
  } else {
    block[4] = 0;
  }
  block[5] = 0;
  block[6] = 0;
  block[7] = 0;
  block[8] = 0;
  if (tsocket.write(block, size) == -1) {
    emit error("Afedri: TCP write error, set_broadcast_flag");
  }
  tsocket.flush();
  tsocket.waitForReadyRead(1000);
  QThread::msleep(AFEDRI_WAIT);
}

Afedri::~Afedri() {}

/* read data from tcp socket
 */
void Afedri::readTcp() {
  QByteArray dat = tsocket.readAll();

  // process data packets. First byte is length of following packet
  // each tcp read may contain several Afedri command responses

  // Afedri SDR speaks two protocols: i) SDR-IP ii) Afedri HID over TCP
  // rsponse packet for ii) has extra 2-byte header of 0x09 0xe0
  int ptr = 0;
  while (ptr < dat.size()) {
    int sz = (unsigned char)dat.data()[ptr];
    if (dat.data()[ptr + 1] == 0) {
      // SDR-IP response

      switch ((unsigned char)dat.data()[ptr + 2]) {
      case 0xb0:
        // Afedri clock frequency
        qDebug("Afedri tcp ack: get clock frequency");
        clockFreq = dat.data()[ptr + 5] + (dat.data()[ptr + 6] << 8) +
                    (dat.data()[ptr + 7] << 16) + (dat.data()[ptr + 8] << 24);
        get_real_sample_rate();
        break;
      case CI_DDC_SAMPLE_RATE:
        qDebug("Afedri tcp ack: set sample rate");
        break;
      case CI_FREQUENCY:
        qDebug("Afedri tcp ack: set freq for chan = %d f = %u",
               dat.data()[ptr + 4],
               (unsigned char)dat.data()[ptr + 5] +
                   ((unsigned char)dat.data()[ptr + 6] << 8) +
                   ((unsigned char)dat.data()[ptr + 7] << 16) +
                   ((unsigned char)dat.data()[ptr + 8] << 24));
        break;
      case CI_RECEIVER_STATE:
        qDebug("Afedri tcp ack: set receiver state %d", dat.data()[ptr + 5]);
        break;
      default:
        qDebug("Afedri tcp ack:  sdr-ip format cmd=%x",
               (unsigned char)dat.data()[ptr + 2]);
      }
    } else if (((unsigned char)dat.data()[ptr] == 0x09) &&
               ((unsigned char)dat.data()[ptr + 1] == 0xe0)) {
      // Afedri HID over TCP response
      switch ((unsigned char)dat.data()[ptr + 3]) {
      case HID_GENERIC_GET_AFEDRI_ID_COMMAND: {
        int id = dat.data()[ptr + 4];
        if (id >= afedriNames->size()) {
          id = afedriNames->size() - 1;
        }
        name = afedriNames[id];
        break;
      }
      case HID_GENERIC_GET_SERIAL_NUMBER_COMMAND:

        break;
      case HID_GENERIC_ACK:
        qDebug("Afedri tcp ack: general hid ack");
        break;
      default:
        qDebug("Afedri tcp ack: hid format cmd=%x",
               (unsigned char)dat.data()[ptr + 3]);
      }
    }
    ptr += sz;
  }
}

/*
 * process udp data from sdr
 */
void Afedri::readDatagram() {
  if (stopFlag) {
    stopAfedri();
    return;
  }
  if (usocket.state() != QAbstractSocket::BoundState) {
    return;
  }
  if (usocket.pendingDatagramSize() == -1) {
    return;
  }
  // actual number of IQ samples per UDP read for different channel modes
  const int read_size1 = 1024;
  const int read_size2 = 512;
  const int read_size4 = 256;

  // in broadcast mode, have 16 bytes extra in header
  qint64 udp_size = 1028;
  if (settings->value(s_sdr_afedri_bcast, s_sdr_afedri_bcast_def).toInt() > 0) {
    udp_size += 16;
  }
  char data[MAX_UDP_SIZE];
  if (usocket.readDatagram((char *)data, udp_size) == -1) {
    emit error("Afedri: UDP read failed");
    return;
  }
  if (settings->value(s_sdr_afedri_multi, s_sdr_afedri_multi_def).toInt() ==
      0) {
    // single receiver
    for (int i = 0; i < read_size1; i++) {
      buff[bptr * sizes.advance_size + iptr + i] = data[i + 4];
    }
    iptr += read_size1;
  } else if (settings->value(s_sdr_afedri_multi, s_sdr_afedri_multi_def)
                 .toInt() == 1) {
    // dual receiver
    switch (settings->value(s_sdr_afedri_channel, s_sdr_afedri_channel_def)
                .toInt()) {
    case 0:
      // dual channel, channel 1
      // with 16 bit data, this is (16 bit I1),(16 bit Q1),32 bits skipped,
      // etc
      for (int i = 0, j = 0; i < 1024; i += 8, j += 4) {
        buff[bptr * sizes.advance_size + iptr + j] = data[i + 20];
        buff[bptr * sizes.advance_size + iptr + j + 1] = data[i + 21];
        buff[bptr * sizes.advance_size + iptr + j + 2] = data[i + 22];
        buff[bptr * sizes.advance_size + iptr + j + 3] = data[i + 23];
      }
      break;
    case 1:
      // dual channel, channel 2
      // with 16 bit data, this is 32 bits skipped, (16 bit I2),(16 bit
      // Q2),etc
      for (int i = 0, j = 0; i < 1024; i += 8, j += 4) {
        buff[bptr * sizes.advance_size + iptr + j] = data[i + 24];
        buff[bptr * sizes.advance_size + iptr + j + 1] = data[i + 25];
        buff[bptr * sizes.advance_size + iptr + j + 2] = data[i + 26];
        buff[bptr * sizes.advance_size + iptr + j + 3] = data[i + 27];
      }
      break;
    default:
      break;
    }
    iptr += read_size2;
  } else {
    // quad receiver
    switch (settings->value(s_sdr_afedri_channel, s_sdr_afedri_channel_def)
                .toInt()) {
    case 0:
      // quad channel, channel 1
      // with 16 bit data, this is (16 bit I1),(16 bit Q1),96 bits skipped,
      // etc
      for (int i = 0, j = 0; i < 1024; i += 16, j += 4) {
        buff[bptr * sizes.advance_size + iptr + j] = data[i + 20];
        buff[bptr * sizes.advance_size + iptr + j + 1] = data[i + 21];
        buff[bptr * sizes.advance_size + iptr + j + 2] = data[i + 22];
        buff[bptr * sizes.advance_size + iptr + j + 3] = data[i + 23];
      }
      break;
    case 1:
      // quad channel, channel 2
      // with 16 bit data, this is 32 bits skipped, (16 bit I2),(16 bit
      // Q2),64 bits skipped,...
      for (int i = 0, j = 0; i < 1024; i += 16, j += 4) {
        buff[bptr * sizes.advance_size + iptr + j] = data[i + 24];
        buff[bptr * sizes.advance_size + iptr + j + 1] = data[i + 25];
        buff[bptr * sizes.advance_size + iptr + j + 2] = data[i + 26];
        buff[bptr * sizes.advance_size + iptr + j + 3] = data[i + 27];
      }
      break;
    case 2:
      // quad channel, channel 3
      // with 16 bit data, this is 64 bits skipped, (16 bit I3),(16 bit
      // Q3),32 bits skipped,...
      for (int i = 0, j = 0; i < 1024; i += 16, j += 4) {
        buff[bptr * sizes.advance_size + iptr + j] = data[i + 28];
        buff[bptr * sizes.advance_size + iptr + j + 1] = data[i + 29];
        buff[bptr * sizes.advance_size + iptr + j + 2] = data[i + 30];
        buff[bptr * sizes.advance_size + iptr + j + 3] = data[i + 31];
      }
      break;
    case 3:
      // quad channel, channel 4
      // with 16 bit data, this is 96 bits skipped, (16 bit I4),(16 bit
      // Q4),...
      for (int i = 0, j = 0; i < 1024; i += 16, j += 4) {
        buff[bptr * sizes.advance_size + iptr + j] = data[i + 32];
        buff[bptr * sizes.advance_size + iptr + j + 1] = data[i + 33];
        buff[bptr * sizes.advance_size + iptr + j + 2] = data[i + 34];
        buff[bptr * sizes.advance_size + iptr + j + 3] = data[i + 35];
      }
      break;
    default:
      break;
    }
    iptr += read_size4;
  }

  // see if one spectrum scan advance is completed
  if (iptr == sizes.advance_size) {
    iptr = 0;
    bptr++;
    bptr = bptr % bpmax;
    emit ready(buff, bptr);
  }
}

/*
 * Sends cmd via TCP to stop stream; closes sockets
 */
void Afedri::stopAfedri() {
  if (settings->value(s_sdr_afedri_bcast, s_sdr_afedri_bcast_def).toInt() !=
      2) {
    send_rx_command(RCV_STOP);
    if (tsocket.state() == QAbstractSocket::ConnectedState &&
        tsocket.state() != QAbstractSocket::ClosingState) {
      tsocket.flush();
    }
    tsocket.close();
    if (tsocket.state() != QAbstractSocket::UnconnectedState) {
      tsocket.waitForDisconnected(1000);
    }
  }
  usocket.close();
  if (usocket.state() != QAbstractSocket::UnconnectedState) {
    usocket.waitForDisconnected(1000);
  }

  running = false;
  stopFlag = false;
  emit stopped();
}

/*
 * set flag to stop sdr
 */
void Afedri::stop() { stopFlag = true; }

/* set center frequency
 */
void Afedri::setRfFreq(double f) {
  // slave should not set frequency
  if (settings->value(s_sdr_afedri_bcast, s_sdr_afedri_bcast_def).toInt() > 1)
    return;

  unsigned int uif = f;
  rfFreq = f;
  set_freq(
      uif,
      settings->value(s_sdr_afedri_channel, s_sdr_afedri_channel_def).toInt());
}

/* set center frequency for channel c (multichannel afedri)
 */
void Afedri::setRfFreqChannel(double f, int c) {

  // slave should not set frequency
  if (isSlave())
    return;
  unsigned int uif = f;
  // only update frequency if c matches channel set for this bandmap
  if (c ==
      settings->value(s_sdr_afedri_channel, s_sdr_afedri_channel_def).toInt()) {
    rfFreq = f;
  }
  set_freq(uif, c);
}

/*!
 * \brief Afedri::get_clock_freq
 *    Set the Afedri clock frequency
 *
 */
void Afedri::get_clock_freq() {
  const int size = 4;
  char block[size];

  if (tsocket.state() != QAbstractSocket::ConnectedState ||
      tsocket.state() == QAbstractSocket::ClosingState)
    return;

  if (isSlave())
    return;

  block[0] = size;
  block[1] = 0x20;
  block[2] = 0xb0;
  block[3] = 0;
  if (tsocket.write(block, size) == -1) {
    emit error("Afedri: TCP write error, get_clock_freq");
  }
  tsocket.flush();
  tsocket.waitForReadyRead(1000);
  QThread::msleep(AFEDRI_WAIT);
}

/*!
 * \brief Afedri::get_sdr_name
 *    Set the Afedri device name
 *
 */
void Afedri::get_sdr_name() {
  unsigned char block[9];

  if (tsocket.state() != QAbstractSocket::ConnectedState ||
      tsocket.state() == QAbstractSocket::ClosingState)
    return;

  if (isSlave())
    return;

  block[0] = 0x09;
  block[1] = 0xe0;
  block[2] = 0x02;
  block[3] = HID_GENERIC_GET_AFEDRI_ID_COMMAND;
  block[4] = 0;
  block[5] = 0;
  block[6] = 0;
  block[7] = 0;
  block[8] = 0;

  if (tsocket.write((char *)block, 9) == -1) {
    emit error("Afedri: TCP write error, get_sdr_name");
  }
  tsocket.flush();
  tsocket.waitForReadyRead(1000);
  QThread::msleep(AFEDRI_WAIT);
}

/* compute actual sampling rate based on requested rate
 * and SDR clock frequency.
 */
void Afedri::get_real_sample_rate() {
  if (clockFreq == 0) {
    realSampleRate = 0;
    return;
  }
  unsigned long temp =
      clockFreq /
      settings->value(s_sdr_afedri_sample_freq, s_sdr_afedri_sample_freq_def)
          .toUInt();
  temp = temp / 4;
  if (clockFreq >
      settings->value(s_sdr_afedri_sample_freq, s_sdr_afedri_sample_freq_def)
              .toUInt() *
          (4 * temp + 2)) {
    temp++;
  }
  realSampleRate = clockFreq / (4 * temp);
  settings->setValue(s_sdr_afedri_real_sample_freq, realSampleRate);
  emit clockFreqSignal(clockFreq);
  emit realSampleRateSignal(realSampleRate);
}

unsigned int Afedri::sampleRate() const {
  if (realSampleRate == 0) {
    return settings
        ->value(s_sdr_afedri_sample_freq, s_sdr_afedri_sample_freq_def)
        .toUInt();
  } else {
    return realSampleRate;
  }
}

bool Afedri::isSlave() const {
  if (settings->value(s_sdr_afedri_bcast, s_sdr_afedri_bcast_def).toInt() ==
      2) {
    return true;
  } else {
    return false;
  }
}
