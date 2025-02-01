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

#include "network.h"
#include "sdr-ip.h"
#include <QDebug>
#include <QHostAddress>

NetworkSDR::NetworkSDR(const QString &settingsFile, QObject *parent)
    : SdrDataSource(settingsFile, parent) {
  bptr = 0;
  buff = nullptr;
  iptr = 0;
  tsocket.setParent(this);
  usocket.setParent(this);
}

void NetworkSDR::initialize() {
  stopFlag = false;
  bpmax = sizes.chunk_size / sizes.advance_size;
  if (buff) {
    delete[] buff;
  }
  buff = new unsigned char[sizes.chunk_size];
  for (unsigned long i = 0; i < sizes.chunk_size; i++) {
    buff[i] = 0;
  }
  qRegisterMetaType<QAbstractSocket::SocketError>("socketerror");
  connect(&tsocket, SIGNAL(error(QAbstractSocket::SocketError)), this,
          SLOT(tcpError(QAbstractSocket::SocketError)));

  // setup tcp socket
  tsocket.connectToHost(
      settings->value(s_sdr_tcp_ip, s_sdr_tcp_ip_def).toString(),
      settings->value(s_sdr_tcp_port, s_sdr_tcp_port_def).toInt());
  tsocket.setSocketOption(QAbstractSocket::KeepAliveOption, QVariant(1));
  if (tsocket.waitForConnected()) {
    if (!usocket.bind(
            settings->value(s_sdr_udp_port, s_sdr_udp_port_def).toInt(),
            QUdpSocket::ShareAddress)) {
      emit error("NetworkSDR: UDP connection failed");
      running = false;
      return;
    }
    connect(&usocket, SIGNAL(readyRead()), this, SLOT(readDatagram()));
    set_sample_rate(
        settings->value(s_sdr_net_sample_freq, s_sdr_net_sample_freq_def)
            .toInt());
    if (rfFreq == 0) {
      // use IF freq if initial RF freq not set
      setRfFreq(
          settings->value(s_sdr_net_if_freq, s_sdr_net_if_freq_def).toDouble());
    } else {
      setRfFreq(rfFreq);
      // force retuning SDR
      emit resetRfFlag();
    }
    send_rx_command(RCV_START);
    running = true;
  }
}

/*!
 * \brief NetworkSDR::tcpError
 *  connects to tcpsocket error() signal
 */
void NetworkSDR::tcpError(QAbstractSocket::SocketError err) {
  Q_UNUSED(err)
  QString errstring = "NetworkSDR: " + tsocket.errorString();
  emit error(errstring);
}

/*!
 * \brief NetworkSDR::send_rx_command
 *   Send command to NetworkSDR NET SDR via TCP
 * \param cmd  command number
 *   RCV_STOP  1
 *   RCV_START 2
 */
void NetworkSDR::send_rx_command(int cmd) {
  unsigned short control_code = CI_RECEIVER_STATE;
  const int size = 8;
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
  block[4] = 0x80;
  block[5] = cmd;
  block[6] = 0;
  block[7] = 0;
  if (tsocket.write(block, size) == -1) {
    emit error("NetworkSDR: TCP write error send_rx_command");
  }
}

/*! get name of SDR device */
void NetworkSDR::get_name() {
  if (tsocket.state() != QAbstractSocket::ConnectedState ||
      tsocket.state() == QAbstractSocket::ClosingState)
    return;

  if (isSlave())
    return;

  char block[4];
  block[0] = 0x04;
  block[1] = 0x20;
  block[2] = 0x01;
  block[3] = 0x00;
  if (tsocket.write(block, 4) == -1) {
    emit error("NetworkSDR: TCP write error get_name");
  }
}

NetworkSDR::~NetworkSDR() {
  if (buff) {
    delete[] buff;
  }
}

void NetworkSDR::readDatagram() {
  const qint64 udp_size = 1028;
  static unsigned char data[udp_size];

  if (stopFlag == true) {
    stopNetwork();
    return;
  }

  if (usocket.readDatagram((char *)data, udp_size) == -1) {
    emit error("NetworkSDR: UDP read failed");
    return;
  }

  // copy into circular buffer; skip the first 4 bytes in the packet
  int read_size = udp_size - 4;
  for (int i = 0; i < udp_size - 4; i++) {
    buff[bptr * sizes.advance_size + iptr + i] = data[i + 4];
  }

  // see if one spectrum scan advance is completed
  iptr += read_size;
  if (iptr == sizes.advance_size) {
    iptr = 0;
    bptr++;
    bptr = bptr % bpmax;
    emit ready(buff, bptr);
  }
}

void NetworkSDR::stop() { stopFlag = true; }

void NetworkSDR::stopNetwork() {
  send_rx_command(RCV_STOP);
  tsocket.flush();

  usocket.close();
  if (usocket.state() != QAbstractSocket::UnconnectedState) {
    usocket.waitForDisconnected(1000);
  }
  tsocket.close();
  if (tsocket.state() != QAbstractSocket::UnconnectedState) {
    tsocket.waitForDisconnected(1000);
  }
  running = false;
  stopFlag = false;
  emit stopped();
}

void NetworkSDR::readTcp() {
  QByteArray data;
  // continue on as long as data is available
  while (tsocket.bytesAvailable()) {
    data = tsocket.read(128);
    // qDebug("<%s>", data.data());
  }
}

/*!
 * \brief NetworkSDR::set_sample_rate
 *    Set the sampling rate
 * \param rate  sample rate in Hz
 *
 */
void NetworkSDR::set_sample_rate(unsigned long sample_rate) {
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
    emit error("NetworkSDR: TCP write error, set_sample_rate");
  }
  tsocket.flush();
}

/* set center frequency
 *
 */
void NetworkSDR::setRfFreq(double f) {
  if (tsocket.state() != QAbstractSocket::ConnectedState ||
      tsocket.state() == QAbstractSocket::ClosingState)
    return;

  if (isSlave())
    return;

  unsigned short control_code = CI_FREQUENCY;
  const int size = 10;
  char block[size];

  block[0] = size;
  block[1] = (SET_CONTROL_ITEM << 5);
  block[2] = control_code & 0xFF;
  block[3] = (control_code >> 8) & 0xFF;
  block[4] = 0;
  unsigned int frequency = f;
  for (int i = 0; i < 4; i++) {
    block[5 + i] = (frequency >> (i * 8)) & 0xFF;
  }
  block[9] = 0;

  if (tsocket.write(block, size) == -1) {
    emit error("NetworkSDR: TCP write set_frequency failed");
  }
  tsocket.flush();
}

unsigned int NetworkSDR::sampleRate() const {
  return settings->value(s_sdr_net_sample_freq, s_sdr_net_sample_freq_def)
      .toUInt();
}
