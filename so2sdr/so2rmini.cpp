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
#include "so2rmini.h"
#include "defines.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QSerialPortInfo>
#include <QThread>
#include <QTimer>

SO2RMini::SO2RMini(QSettings &s, QObject *parent)
    : QObject(parent), settings(s) {
  QSerialPortInfo info(
      settings.value(s_mini_device, s_mini_device_def).toString());
  SO2RMiniPort = new QSerialPort(info);
  SO2RMiniOpen = false;
  sending = false;
  stereo = false;
  relayBits = 0;
  buffer.clear();
  txRig = 0;
}

SO2RMini::~SO2RMini() {
  closeSO2RMini();
  delete SO2RMiniPort;
}

/*!
 *  \brief cancel cw immediately and release PTT
 */
void SO2RMini::cancelcw() {
  const char cmd = 0x12;
  buffer.clear();
  if (SO2RMiniPort->isOpen()) {
    SO2RMiniPort->write(&cmd, 1);
    SO2RMiniPort->flush();
  }
}

/*! \brief set cw speed
 *  currently assume both computer and paddle are same speed, but
 *  so2rmini allows them to be set differently
 *
 *  if s=0, sets the last saved speed
 */
void SO2RMini::setSpeed(int s) {
  static int last_s = 0;
  const char cmd = 0x08;

  // save speed in case mini is restarted
  if (s!=0) {
      last_s = s;
  } else if (s==0 && last_s!=0) {
      s = last_s;
  } else {
      // this normally shouldn't happen
      s = 32;
      last_s = 32;
  }
  if (s > 1 && s < 100) {
    char ss = s;
    if (SO2RMiniPort->isOpen()) {
      SO2RMiniPort->write(&cmd, 1);
      SO2RMiniPort->write(&ss, 1);
      SO2RMiniPort->flush();
    }
  }
}

void SO2RMini::loadbuff(QByteArray msg) { buffer.append(msg); }

void SO2RMini::sendcw() {
  if (SO2RMiniPort->isOpen()) {

    buffer = buffer.simplified();
    SO2RMiniPort->write(buffer);
    SO2RMiniPort->flush();
  }
  buffer.clear();
}

/*!
   returns true if SO2RMini has been opened successfully
 */
bool SO2RMini::SO2RMiniIsOpen() const { return (SO2RMiniOpen); }

void SO2RMini::sendCommand(QByteArray command) {
  if (!SO2RMiniOpen)
    return;
  SO2RMiniPort->write(command, command.length());
  SO2RMiniPort->flush();
}

/*! \brief process serial data coming from SO2RMini
 */
void SO2RMini::receive() {
  while (SO2RMiniPort->bytesAvailable()) {
    unsigned char dat;
    int n = SO2RMiniPort->read((char *)&dat, 1);
    if (n != -1) {
      // 0x01 = radio1 sending; 0x02 = radio2 sending
      if (dat == 0x01) {
        txRig = 0;
        emit tx(true, 0);
      } else if (dat == 0x02) {
        txRig = 1;
        emit tx(true, 1);
      } else if (dat == 0x00) {
        sending = false;
        emit tx(false, txRig);
        QTimer::singleShot(queueDelay, this, SIGNAL(finished()));
      }
    }
  }
}

/*!
 * \brief SO2RMini::switchRadios
 * switches both RX and TX focus to other radio
 */
void SO2RMini::switchAudio(int nr) {
  if (!SO2RMiniOpen || nr < 0 || nr > 1)
    return;

  // stereo mode selected by 2nd bit in relayBits
  const char relaycmd = 0x02;

  // if in stereo mode, don't need to switch RX
  if (!stereo) {
    // following keeps mic routing, reset headphone
    relayBits = (relayBits & 0x04);
    if (nr == 0) {
      SO2RMiniPort->write(&relaycmd, 1);
      SO2RMiniPort->write(&relayBits, 1);
    } else {
      relayBits |= 0x01;
      SO2RMiniPort->write(&relaycmd, 1);
      SO2RMiniPort->write(&relayBits, 1);
    }
    SO2RMiniPort->flush();
  }
}

/*! \brief switch cw and mic output
 */
void SO2RMini::switchTransmit(int nr) {
  if (!SO2RMiniOpen || nr < 0 || nr > 1)
    return;
  const char txcw1[2] = {0x0b, 0x01};
  const char txcw2[2] = {0x0b, 0x02};
  const char txmic = 0x02;
  if (nr == 0) {
    SO2RMiniPort->write(txcw1, 2);
    // keep bits 1 and 2, reset bit 3
    relayBits = (relayBits & 0x03);
    SO2RMiniPort->write(&txmic, 1);
    SO2RMiniPort->write(&relayBits, 1);
  } else {
    SO2RMiniPort->write(txcw2, 2);
    // keep bits 1 and 2, set bit 3
    relayBits = (relayBits & 0x03);
    relayBits |= 0x04;
    SO2RMiniPort->write(&txmic, 1);
    SO2RMiniPort->write(&relayBits, 1);
  }
  SO2RMiniPort->flush();
}

/*!
 * \brief SO2RMini::toggleStereo
 * \param nr the active radio
 * Toggles stereo receive mode. SO2RMini needs parameter nr because it
 * can't remember how to get out of stereo split mode.
 */
void SO2RMini::toggleStereo(int nr) {
  if (!SO2RMiniOpen || nr < 0 || nr > 1)
    return;
  if (stereo) {
    stereo = false;
    switchAudio(nr);
  } else {
    const char cmd = 0x02;
    relayBits = (relayBits & 0x04);
    relayBits |= 0x03;
    SO2RMiniPort->write(&cmd, 1);
    SO2RMiniPort->write(&relayBits, 1);
    SO2RMiniPort->flush();
    stereo = true;
  }
}

bool SO2RMini::stereoActive() const { return stereo; }

/*! \brief open SO2RMini device
 */
void SO2RMini::openSO2RMini() {
  // in case we are re-starting SO2RMini
  bool restart = false;
  if (SO2RMiniPort->isOpen()) {
    disconnect(SO2RMiniPort);
    closeSO2RMini();
    SO2RMiniOpen = false;
    restart = true;
  }

  // if restart, must completely restart serial port, otherwise
  // get error rereading version string. Not sure why
  if (restart) {
      delete SO2RMiniPort;
      QSerialPortInfo info(settings.value(s_mini_device, s_mini_device_def).toString());
      SO2RMiniPort = new QSerialPort(info);
  }

  SO2RMiniPort->setPortName(settings.value(s_mini_device, s_mini_device_def).toString());

  SO2RMiniPort->open(QIODevice::ReadWrite);

  // opening port forces Arduino Nano to reset. Must wait
  // for reset to complete
  QThread::sleep(2);

  SO2RMiniPort->setDataTerminalReady(false);
  SO2RMiniPort->setRequestToSend(false);


  if (!SO2RMiniPort->isOpen()) {
    SO2RMiniOpen = false;
    emit miniError("ERROR: could not open SO2RMini");
    qDebug("ERROR: could not open SO2RMini");
    return;
  }
  SO2RMiniPort->clearError();

  if (!SO2RMiniPort->setBaudRate(QSerialPort::Baud19200))
    qDebug() << SO2RMiniPort->errorString();
  if (!SO2RMiniPort->setFlowControl(QSerialPort::NoFlowControl))
    qDebug() << SO2RMiniPort->errorString();
  if (!SO2RMiniPort->setParity(QSerialPort::NoParity))
    qDebug() << SO2RMiniPort->errorString();
  if (!SO2RMiniPort->setDataBits(QSerialPort::Data8))
    qDebug() << SO2RMiniPort->errorString();
  if (!SO2RMiniPort->setStopBits(QSerialPort::OneStop))
    qDebug() << SO2RMiniPort->errorString();
  if (!SO2RMiniPort->setRequestToSend(false))
    qDebug() << SO2RMiniPort->errorString();
  if (!SO2RMiniPort->setDataTerminalReady(false))
    qDebug() << SO2RMiniPort->errorString();

  // try a few times to get the device version
  for (int i = 0; i < 5; i++) {
    const char cmd = 0x01;
    if (!SO2RMiniPort->write(&cmd, 1)) {
        SO2RMiniPort->clearError();
        continue;
    }
    SO2RMiniPort->flush();
    deviceName.clear();
    while (SO2RMiniPort->waitForReadyRead(500)) {
      deviceName = SO2RMiniPort->readAll().simplified();
    }
    if (SO2RMiniPort->error()) {
        SO2RMiniPort->clearError();
    }
    if (deviceName.left(2) != "TR") deviceName.clear();
    if (deviceName.size() > 0)
      break;
  }

  emit miniName(deviceName);
  connect(SO2RMiniPort, SIGNAL(readyRead()), this, SLOT(receive()));
  SO2RMiniOpen = true;

  // reset speed if restarting
  if (restart) setSpeed(0);

  char cmd1[2];
  // sidetone freq, set in units of 10 Hz
  cmd1[0] = 0x03;
  if (settings.value(s_mini_sidetone,s_mini_sidetone_def).toBool()) {
      cmd1[1] = settings.value(s_mini_sidetone_freq,s_mini_sidetone_freq_def).toInt();
  } else {
      cmd1[1] = 0;
  }
  SO2RMiniPort->write(cmd1, 2);

  // paddle sidetone freq, set in units of 10 Hz
  cmd1[0] = 0x04;
  if (settings.value(s_mini_paddle_sidetone,s_mini_paddle_sidetone_def).toBool()) {
      cmd1[1] = settings.value(s_mini_paddle_sidetone_freq,s_mini_paddle_sidetone_freq_def).toInt();
  } else {
      cmd1[1] = 0;
  }
  SO2RMiniPort->write(cmd1, 2);
}

void SO2RMini::closeSO2RMini() {
  if (SO2RMiniOpen)
    SO2RMiniPort->close();
}
