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
#include "so2rdialog.h"
#include "defines.h"
#include <QByteArray>
#include <QComboBox>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QString>

So2rDialog::So2rDialog(QSettings &s, QWidget *parent)
    : QDialog(parent), settings(s) {
  setupUi(this);

  comboBoxOTRSPBaud1->insertItem(0, "1200");
  comboBoxOTRSPBaud1->insertItem(0, "4800");
  comboBoxOTRSPBaud1->insertItem(0, "9600");
  comboBoxOTRSPBaud1->insertItem(0, "19200");
  comboBoxOTRSPBaud1->insertItem(0, "38400");
  comboBoxOTRSPBaud1->setCurrentIndex(2);
  comboBoxOTRSPBaud1->setEnabled(false);
  comboBoxOTRSPParity1->insertItem(0, "N");
  comboBoxOTRSPParity1->setCurrentIndex(0);
  comboBoxOTRSPParity1->setEnabled(false);
  comboBoxOTRSPStop1->insertItem(0, "1");
  comboBoxOTRSPStop1->setCurrentIndex(0);
  comboBoxOTRSPStop1->setEnabled(false);
  lineEditOTRSPPort1->clear();

  comboBoxOTRSPBaud2->insertItem(0, "1200");
  comboBoxOTRSPBaud2->insertItem(0, "4800");
  comboBoxOTRSPBaud2->insertItem(0, "9600");
  comboBoxOTRSPBaud2->insertItem(0, "19200");
  comboBoxOTRSPBaud2->insertItem(0, "38400");
  comboBoxOTRSPBaud2->setCurrentIndex(2);
  comboBoxOTRSPBaud2->setEnabled(false);
  comboBoxOTRSPParity2->insertItem(0, "N");
  comboBoxOTRSPParity2->setCurrentIndex(0);
  comboBoxOTRSPParity2->setEnabled(false);
  comboBoxOTRSPStop2->insertItem(0, "1");
  comboBoxOTRSPStop2->setCurrentIndex(0);
  comboBoxOTRSPStop2->setEnabled(false);
  lineEditOTRSPPort2->clear();

  RadioPinComboBox->insertItem(0, "9");
  RadioPinComboBox->insertItem(0, "8");
  RadioPinComboBox->insertItem(0, "7");
  RadioPinComboBox->insertItem(0, "6");
  RadioPinComboBox->insertItem(0, "5");
  RadioPinComboBox->insertItem(0, "4");
  RadioPinComboBox->insertItem(0, "3");
  RadioPinComboBox->insertItem(0, "2");
  TransmitPinComboBox->insertItem(0, "9");
  TransmitPinComboBox->insertItem(0, "8");
  TransmitPinComboBox->insertItem(0, "7");
  TransmitPinComboBox->insertItem(0, "6");
  TransmitPinComboBox->insertItem(0, "5");
  TransmitPinComboBox->insertItem(0, "4");
  TransmitPinComboBox->insertItem(0, "3");
  TransmitPinComboBox->insertItem(0, "2");
  StereoPinComboBox->insertItem(0, "9");
  StereoPinComboBox->insertItem(0, "8");
  StereoPinComboBox->insertItem(0, "7");
  StereoPinComboBox->insertItem(0, "6");
  StereoPinComboBox->insertItem(0, "5");
  StereoPinComboBox->insertItem(0, "4");
  StereoPinComboBox->insertItem(0, "3");
  StereoPinComboBox->insertItem(0, "2");
  ParallelPortComboBox->insertItem(0, "/dev/parport1");
  ParallelPortComboBox->insertItem(0, "/dev/parport0");
  ParallelPortComboBox->setEditable(
      true); // allow other port numbers to be entered
  ParallelPortComboBox->setCurrentIndex(0);
  ParallelPortComboBox->setToolTip(
      "Parallel port access requires the PPDEV kernel module and being in the "
      "correct group (usually lp).");
  pttDelaySpinbox->setMinimum(0);
  pttDelaySpinbox->setMaximum(100);
  pttTailDelaySpinbox->setMinimum(0);
  pttTailDelaySpinbox->setMaximum(100);
  pttPaddleTailDelaySpinbox->setMinimum(0);
  pttPaddleTailDelaySpinbox->setMaximum(100);
  connect(so2rdialog_buttons, SIGNAL(rejected()), this, SLOT(rejectChanges()));
  connect(so2rdialog_buttons, SIGNAL(accepted()), this, SLOT(updateSo2r()));
  updateFromSettings();
}

So2rDialog::~So2rDialog() {}

/*!
   update all params when dialog accepted
 */
void So2rDialog::updateSo2r() {
  settings.setValue(s_radios_focus, RadioPinComboBox->currentIndex() + 2);
  settings.setValue(s_radios_txfocus, TransmitPinComboBox->currentIndex() + 2);
  settings.setValue(s_radios_stereo, StereoPinComboBox->currentIndex() + 2);
  settings.setValue(s_radios_focusinvert, RadioInvertCheckBox->isChecked());
  settings.setValue(s_radios_txfocusinvert,
                    TransmitInvertCheckBox->isChecked());
  bool pportUpdate = false;
  if (settings.value(s_radios_pport, defaultParallelPort).toString() !=
      ParallelPortComboBox->currentText()) {
    settings.setValue(s_radios_pport, ParallelPortComboBox->currentText());
    pportUpdate = true;
  }
  if (settings.value(s_radios_pport_enabled, s_radios_pport_enabled_def)
          .toBool() != checkBoxParallelPort->isChecked()) {
    settings.setValue(s_radios_pport_enabled,
                      checkBoxParallelPort->isChecked());
    pportUpdate = true;
  }
  bool otrspUpdate = false;
  if (settings.value(s_otrsp_enabled[0], s_otrsp_enabled_def).toBool() !=
      checkBoxOTRSP1->isChecked()) {
    settings.setValue(s_otrsp_enabled[0], checkBoxOTRSP1->isChecked());
    if (settings.value(s_otrsp_enabled[0], s_otrsp_enabled_def).toBool()) {
      otrspUpdate = true;
    }
  }
  if (settings.value(s_otrsp_enabled[1], s_otrsp_enabled_def).toBool() !=
      checkBoxOTRSP2->isChecked()) {
    settings.setValue(s_otrsp_enabled[1], checkBoxOTRSP2->isChecked());
    if (settings.value(s_otrsp_enabled[1], s_otrsp_enabled_def).toBool()) {
      otrspUpdate = true;
    }
  }
  if (settings.value(s_otrsp_device[0], s_otrsp_device_def).toString() !=
      lineEditOTRSPPort1->text()) {
    settings.setValue(s_otrsp_device[0], lineEditOTRSPPort1->text());
    otrspUpdate = true;
  }
  if (settings.value(s_otrsp_device[1], s_otrsp_device_def).toString() !=
      lineEditOTRSPPort2->text()) {
    settings.setValue(s_otrsp_device[1], lineEditOTRSPPort2->text());
    otrspUpdate = true;
  }
  settings.setValue(s_otrsp_focus[0], checkBoxOTRSPEvent1->isChecked());
  settings.setValue(s_otrsp_focus[1], checkBoxOTRSPEvent2->isChecked());

  bool microHamUpdate = false;
  if (settings.value(s_microham_enabled, s_microham_enabled_def).toBool() !=
      checkBoxMicroHam->isChecked()) {
    settings.setValue(s_microham_enabled, checkBoxMicroHam->isChecked());
    if (settings.value(s_microham_enabled, s_microham_enabled_def).toBool()) {
      microHamUpdate = true;
    }
  }
  if (settings.value(s_microham_device, s_microham_device_def).toString() !=
      lineEditMicroHamPort->text()) {
    settings.setValue(s_microham_device, lineEditMicroHamPort->text());
    microHamUpdate = true;
  }
  bool miniUpdate = false;
  if (settings.value(s_mini_enabled, s_mini_enabled_def).toBool() !=
      checkBoxMini->isChecked()) {
    settings.setValue(s_mini_enabled, checkBoxMini->isChecked());
    if (settings.value(s_mini_enabled, s_mini_enabled_def).toBool()) {
      miniUpdate = true;
    }
  }
  if (settings.value(s_mini_device, s_mini_device_def).toString() !=
      lineEditMiniPort->text()) {
    settings.setValue(s_mini_device, lineEditMiniPort->text());
    miniUpdate = true;
  }
  if (settings.value(s_mini_sidetone_freq, s_mini_sidetone_freq_def).toInt() !=
      spinBoxSidetone->value() / 10) {
    settings.setValue(s_mini_sidetone_freq, spinBoxSidetone->value() / 10);
    miniUpdate = true;
  }
  if (settings
          .value(s_mini_paddle_sidetone_freq, s_mini_paddle_sidetone_freq_def)
          .toInt() != spinBoxPaddleSidetone->value() / 10) {
    settings.setValue(s_mini_paddle_sidetone_freq,
                      spinBoxPaddleSidetone->value() / 10);
    miniUpdate = true;
  }
  if (settings.value(s_mini_ptt_delay, s_mini_ptt_delay_def).toInt() !=
      pttDelaySpinbox->value()) {
    settings.setValue(s_mini_ptt_delay, pttDelaySpinbox->value());
    miniUpdate = true;
  }
  if (settings.value(s_mini_ptt_tail_delay, s_mini_ptt_tail_delay_def)
          .toInt() != pttTailDelaySpinbox->value()) {
    settings.setValue(s_mini_ptt_tail_delay, pttTailDelaySpinbox->value());
    miniUpdate = true;
  }
  if (settings
          .value(s_mini_ptt_paddle_tail_delay, s_mini_ptt_paddle_tail_delay_def)
          .toInt() != pttPaddleTailDelaySpinbox->value()) {
    settings.setValue(s_mini_ptt_paddle_tail_delay,
                      pttPaddleTailDelaySpinbox->value());
    miniUpdate = true;
  }
  if (settings.value(s_mini_sidetone, s_mini_sidetone_def).toBool() !=
      checkBoxSidetone->isChecked()) {
    settings.setValue(s_mini_sidetone, checkBoxSidetone->isChecked());
    miniUpdate = true;
  }
  if (settings.value(s_mini_paddle_sidetone, s_mini_paddle_sidetone_def)
          .toBool() != checkBoxPaddleSidetone->isChecked()) {
    settings.setValue(s_mini_paddle_sidetone,
                      checkBoxPaddleSidetone->isChecked());
    miniUpdate = true;
  }
  settings.sync();
  // in case parallel port is changed
  if (pportUpdate) {
    emit setParallelPort();
  }
  // restart otrsp if needed
  if (otrspUpdate) {
    emit setOTRSP();
  }
  if (microHamUpdate) {
    emit setMicroHam();
  }
  if (miniUpdate) {
    emit setMini();
  }
  accept();
}

void So2rDialog::updateFromSettings() {
  RadioPinComboBox->setCurrentIndex(
      settings.value(s_radios_focus, defaultParallelPortAudioPin).toInt() - 2);
  TransmitPinComboBox->setCurrentIndex(
      settings.value(s_radios_txfocus, defaultParallelPortTxPin).toInt() - 2);
  StereoPinComboBox->setCurrentIndex(
      settings.value(s_radios_stereo, defaultParallelPortStereoPin).toInt() -
      2);
  RadioInvertCheckBox->setChecked(
      settings.value(s_radios_focusinvert, false).toBool());
  TransmitInvertCheckBox->setChecked(
      settings.value(s_radios_txfocusinvert, false).toBool());
  bool found = false;
  for (int i = 0; i < ParallelPortComboBox->count(); i++) {
    if (ParallelPortComboBox->itemText(i) ==
        settings.value(s_radios_pport, defaultParallelPort).toString()) {
      ParallelPortComboBox->setCurrentIndex(i);
      found = true;
    }
  }
  if (!found) {
    ParallelPortComboBox->setCurrentIndex(0);
  }
  checkBoxParallelPort->setChecked(
      settings.value(s_radios_pport_enabled, s_radios_pport_enabled_def)
          .toBool());
  checkBoxOTRSP1->setChecked(
      settings.value(s_otrsp_enabled[0], s_otrsp_enabled_def).toBool());
  lineEditOTRSPPort1->setText(
      settings.value(s_otrsp_device[0], s_otrsp_device_def).toString());
  checkBoxOTRSP2->setChecked(
      settings.value(s_otrsp_enabled[1], s_otrsp_enabled_def).toBool());
  lineEditOTRSPPort2->setText(
      settings.value(s_otrsp_device[1], s_otrsp_device_def).toString());
  checkBoxOTRSPEvent1->setChecked(
      settings.value(s_otrsp_focus[0], s_otrsp_focus_def).toBool());
  checkBoxOTRSPEvent2->setChecked(
      settings.value(s_otrsp_focus[1], s_otrsp_focus_def).toBool());
  checkBoxMicroHam->setChecked(
      settings.value(s_microham_enabled, s_microham_enabled_def).toBool());
  lineEditMicroHamPort->setText(
      settings.value(s_microham_device, s_microham_device_def).toString());
  checkBoxMini->setChecked(
      settings.value(s_mini_enabled, s_mini_enabled_def).toBool());
  lineEditMiniPort->setText(
      settings.value(s_mini_device, s_mini_device_def).toString());
  checkBoxSidetone->setChecked(
      settings.value(s_mini_sidetone, s_mini_sidetone_def).toBool());
  checkBoxPaddleSidetone->setChecked(
      settings.value(s_mini_paddle_sidetone, s_mini_paddle_sidetone_def)
          .toBool());
  spinBoxSidetone->setValue(
      settings.value(s_mini_sidetone_freq, s_mini_sidetone_freq_def).toInt() *
      10);
  spinBoxPaddleSidetone->setValue(
      settings
          .value(s_mini_paddle_sidetone_freq, s_mini_paddle_sidetone_freq_def)
          .toInt() *
      10);
  pttDelaySpinbox->setValue(
      settings.value(s_mini_ptt_delay, s_mini_ptt_delay_def).toInt());
  pttPaddleTailDelaySpinbox->setValue(
      settings
          .value(s_mini_ptt_paddle_tail_delay, s_mini_ptt_paddle_tail_delay_def)
          .toInt());
  pttTailDelaySpinbox->setValue(
      settings.value(s_mini_ptt_tail_delay, s_mini_ptt_tail_delay_def).toInt());
}

/*! called if dialog rejected */
void So2rDialog::rejectChanges() {
  updateFromSettings();
  reject();
}

/* slot to set displayed otrsp device name */
void So2rDialog::setOtrspName(QByteArray name, int nr) {
  if (nr == 0) {
    labelOTRSPName1->setText(name);
  } else if (nr == 1) {
    labelOTRSPName2->setText(name);
  }
}

/* set name or version displayed for SO2R Mini */
void So2rDialog::setMiniName(QByteArray name) {
  labelMiniVersion->setText(name);
}
