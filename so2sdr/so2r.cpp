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
#include "so2r.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

/*! Control the serial port RTS/DTR line.
 *
 * rtsdtr=0 : none; 1 : RTS; 2 : DTR
 *
 * @todo eventually this will support other methods of ptt control:
 *    hamlib (bad latency either with cat command or RTS/DTR)
 *    otrsp
 */
void So2r::setPtt(int nr, int state) {
  if (settings.value(s_radios_ptt_type[nr], s_radios_ptt_type_def).toInt() == 0)
    return;
  int fd = open(settings.value(s_radios_port[nr], defaultSerialPort[nr])
                    .toByteArray()
                    .data(),
                O_RDWR | O_NOCTTY | O_NONBLOCK);
  int flag;
  if (settings.value(s_radios_ptt_type[nr], s_radios_ptt_type_def).toInt() ==
      1) {
    flag = TIOCM_RTS;
  } else {
    flag = TIOCM_DTR;
  }
  if (state == 1) {
    ioctl(fd, TIOCMBIS, &flag);
  } else {
    ioctl(fd, TIOCMBIC, &flag);
  }
  close(fd);
}

So2r::So2r(QSettings &s, const uiSize &sz, QObject *parent,
           QWidget *widgetParent)
    : QObject(parent), settings(s), sizes(sz) {
  microham = new MicroHam(settings, this);
  otrsp[0] = new OTRSP(settings, 0, this);
  otrsp[1] = new OTRSP(settings, 1, this);
  pport = new ParallelPort(settings);
  mini = new SO2RMini(settings);
  so2rDialog = new So2rDialog(settings, sizes, widgetParent);
  so2rDialog->hide();
  connect(so2rDialog, SIGNAL(accepted()), this, SIGNAL(So2rDialogAccepted()));
  connect(so2rDialog, SIGNAL(rejected()), this, SIGNAL(So2rDialogRejected()));
  connect(so2rDialog, SIGNAL(setParallelPort()), pport, SLOT(initialize()));
  connect(so2rDialog, SIGNAL(setOTRSP()), otrsp[0], SLOT(openOTRSP()));
  connect(so2rDialog, SIGNAL(setOTRSP()), otrsp[1], SLOT(openOTRSP()));
  connect(so2rDialog, SIGNAL(setMini()), mini, SLOT(openSO2RMini()));
  connect(otrsp[0], SIGNAL(otrspError(const QString &)), this,
          SIGNAL(error(const QString &)));
  connect(otrsp[1], SIGNAL(otrspError(const QString &)), this,
          SIGNAL(error(const QString &)));
  connect(microham, SIGNAL(microhamError(const QString &)), this,
          SIGNAL(error(const QString &)));
  connect(pport, SIGNAL(parallelPortError(const QString &)), this,
          SIGNAL(error(const QString &)));
  connect(otrsp[0], SIGNAL(otrspNameSet(QByteArray, int)), so2rDialog,
          SLOT(setOtrspName(QByteArray, int)));
  connect(otrsp[1], SIGNAL(otrspNameSet(QByteArray, int)), so2rDialog,
          SLOT(setOtrspName(QByteArray, int)));
  connect(mini, SIGNAL(miniError(const QString &)), this,
          SIGNAL(error(const QString &)));
  connect(mini, SIGNAL(miniName(QByteArray)), so2rDialog,
          SLOT(setMiniName(QByteArray)));
  connect(mini, SIGNAL(finished()), this, SIGNAL(So2rMiniFinished()));
  connect(mini, SIGNAL(tx(bool, int)), this, SIGNAL(So2rMiniTx(bool, int)));
  connect(so2rDialog, SIGNAL(setMicroHam()), microham, SLOT(openMicroHam()));
  connect(mini, SIGNAL(textSent(const QString &, int)), this,
          SIGNAL(textSent(const QString &, int)));
  if (settings.value(s_radios_pport_enabled, s_radios_pport_enabled_def)
          .toBool()) {
    pport->initialize();
  }
  if (settings.value(s_otrsp_enabled[0], s_otrsp_enabled_def).toBool()) {
    otrsp[0]->openOTRSP();
  }
  if (settings.value(s_otrsp_enabled[1], s_otrsp_enabled_def).toBool()) {
    otrsp[1]->openOTRSP();
  }
  if (settings.value(s_microham_enabled, s_microham_enabled_def).toBool()) {
    microham->openMicroHam();
  }
  if (settings.value(s_mini_enabled, s_mini_enabled_def).toBool()) {
    mini->openSO2RMini();
  }
  txRadio = 0;
  if (settings.value(s_settings_focusindicators, s_settings_focusindicators_def)
          .toBool()) {
    emit setRX1(redLED);
    emit setRX2(clearLED);
    emit setTX1(redLED);
    emit setTX2(clearLED);
  }
}

So2r::~So2r() {
  delete microham;
  delete otrsp[0];
  delete otrsp[1];
  delete pport;
  delete mini;
  delete so2rDialog;
}

bool So2r::isVisible() {
  if (so2rDialog) {
    return so2rDialog->isVisible();
  } else {
    return false;
  }
}
int So2r::transmitRadio() const { return txRadio; }

void So2r::toggleStereo(int activeRadio) {
  if (settings.value(s_radios_pport_enabled, s_radios_pport_enabled_def)
          .toBool()) {
    pport->toggleStereoPin();
  }
  for (int i = 0; i < NRIG; i++) {
    if (settings.value(s_otrsp_enabled[i], s_otrsp_enabled_def).toBool() &&
        settings.value(s_otrsp_focus[i], s_otrsp_focus_def).toBool()) {
      otrsp[i]->toggleStereo(activeRadio);
    }
  }
  if (settings.value(s_microham_enabled, s_microham_enabled_def).toBool()) {
    microham->toggleStereo(activeRadio);
  }
  if (settings.value(s_mini_enabled, s_mini_enabled_def).toBool()) {
    mini->toggleStereo(activeRadio);
  }
}

bool So2r::stereoActive() const {
  return (pport->stereoActive() || otrsp[0]->stereoActive() ||
          otrsp[1]->stereoActive() || microham->stereoActive() ||
          mini->stereoActive());
}

void So2r::switchAudio(int r) {
  if (r < 0 || r >= NRIG)
    return;

  if (settings.value(s_radios_pport_enabled, s_radios_pport_enabled_def)
          .toBool()) {
    pport->switchAudio(r);
  }
  for (int i = 0; i < NRIG; i++) {
    if (settings.value(s_otrsp_enabled[i], s_otrsp_enabled_def).toBool() &&
        settings.value(s_otrsp_focus[i], s_otrsp_focus_def).toBool()) {
      otrsp[i]->switchAudio(r);
    }
  }
  if (settings.value(s_microham_enabled, s_microham_enabled_def).toBool()) {
    microham->switchAudio(r);
  }
  if (settings.value(s_mini_enabled, s_mini_enabled_def).toBool()) {
    mini->switchAudio(r);
  }
}

void So2r::switchTransmit(int r) {
  if (r < 0 || r >= NRIG)
    return;

  if (settings.value(s_radios_pport_enabled, s_radios_pport_enabled_def)
          .toBool()) {
    pport->switchTransmit(r);
  }
  for (int i = 0; i < NRIG; i++) {
    if (settings.value(s_otrsp_enabled[i], s_otrsp_enabled_def).toBool() &&
        settings.value(s_otrsp_focus[i], s_otrsp_focus_def).toBool()) {
      otrsp[i]->switchTransmit(r);
    }
  }
  if (settings.value(s_microham_enabled, s_microham_enabled_def).toBool()) {
    microham->switchTransmit(r);
  }
  if (settings.value(s_mini_enabled, s_mini_enabled_def).toBool()) {
    mini->switchTransmit(r);
  }
  txRadio = r;
}

void So2r::showDialog() { so2rDialog->show(); }

void So2r::sendOtrspCommand(QByteArray c, int nr) {
  if (settings.value(s_otrsp_enabled[nr], s_otrsp_enabled_def).toBool()) {
    otrsp[nr]->sendCommand(c);
  }
}

void So2r::sendMicrohamCommand(QByteArray c) {
  if (settings.value(s_microham_enabled, s_microham_enabled_def).toBool()) {
    microham->sendCommand(c);
  }
}

void So2r::sendMiniCommand(QByteArray c) {
  if (settings.value(s_mini_enabled, s_mini_enabled_def).toBool()) {
    mini->sendCommand(c);
  }
}

void So2r::updateIndicators(int activeRadio) {
  if (settings.value(s_settings_focusindicators, s_settings_focusindicators_def)
          .toBool()) {
    if (stereoActive()) {
      emit setRX1(greenLED);
      emit setRX2(greenLED);
    } else {
      if (activeRadio == 1) {
        emit setRX1(clearLED);
        emit setRX2(greenLED);
      } else {
        emit setRX1(greenLED);
        emit setRX2(clearLED);
      }
    }
    if (transmitRadio()) {
      emit setTX1(clearLED);
      emit setTX2(redLED);
    } else {
      emit setTX1(redLED);
      emit setTX2(clearLED);
    }
  } else {
    emit setRX1(clearLED);
    emit setRX2(clearLED);
    emit setTX1(clearLED);
    emit setTX2(clearLED);
  }
}
