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
#include "cwdialog.h"
#include "defines.h"
#include <QDialog>
#include <QMessageBox>
#include <QSettings>
#include <QString>

/*!
  CWDialog is a dialog for entering winkey and cwdaemon parameters

  s is station config file (so2sdr.ini) in QSettings .ini format
 */
CWDialog::CWDialog(QSettings &s, QWidget *parent)
    : QDialog(parent), settings(s) {
  setupUi(this);
  adjustSize();
  setFixedSize(size());

  // load from settings file
  DeviceLineEdit->setText(
      settings.value(s_winkey_device, s_winkey_device_def).toString());
  DeviceLineEdit->setToolTip(
      "serial port /dev/ttyS0, /dev/ttyS1, /dev/ttyUSB0, ...");
  CTCheckBox->setChecked(
      settings.value(s_winkey_ctspace, s_winkey_ctspace_def).toBool());
  PaddleSwapCheckBox->setChecked(
      settings.value(s_winkey_paddle_swap, s_winkey_paddle_swap_def).toBool());
  PaddleSidetoneCheckBox->setChecked(
      settings.value(s_winkey_sidetonepaddle, s_winkey_sidetonepaddle_def)
          .toBool());
  PaddleModeComboBox->setCurrentIndex(
      settings.value(s_winkey_paddle_mode, s_winkey_paddle_mode_def).toInt());
  SidetoneFreqComboBox->insertItem(0, "4000 Hz");
  SidetoneFreqComboBox->insertItem(1, "2000 Hz");
  SidetoneFreqComboBox->insertItem(2, "1333 Hz");
  SidetoneFreqComboBox->insertItem(3, "1000 Hz");
  SidetoneFreqComboBox->insertItem(4, "800 Hz");
  SidetoneFreqComboBox->insertItem(5, "666 Hz");
  SidetoneFreqComboBox->insertItem(6, "571 Hz");
  SidetoneFreqComboBox->insertItem(7, "500 Hz");
  SidetoneFreqComboBox->insertItem(8, "444 Hz");
  SidetoneFreqComboBox->insertItem(9, "400 Hz");
  SidetoneFreqComboBox->setCurrentIndex(
      settings.value(s_winkey_sidetone, s_winkey_sidetone_def).toInt());
  UDP1LineEdit->setText(
      settings.value(s_cwdaemon_udp[0], s_cwdaemon_udp_def[0]).toString());
  UDP1LineEdit->setToolTip("UDP port number for rig 1");
  UDP2LineEdit->setText(
      settings.value(s_cwdaemon_udp[1], s_cwdaemon_udp_def[1]).toString());
  UDP2LineEdit->setToolTip("UDP port number for rig 2");

  comboBox->setCurrentIndex(
      settings.value(s_cw_device, s_cw_device_def).toInt());

  connect(cwdialog_buttons, SIGNAL(rejected()), this, SLOT(rejectChanges()));
  connect(cwdialog_buttons, SIGNAL(accepted()), this, SLOT(updateCW()));
  setWinkeyVersionLabel(0);
}

CWDialog::~CWDialog() {}

/*!
   sets text showing Winkey version
 */
void CWDialog::setWinkeyVersionLabel(int version) {
  QString label = QString::number(version);
  VersionLabel->setText(label);
}

/*!
   update CW device parameters from form input
 */
void CWDialog::updateCW() {
  settings.setValue(s_winkey_device, DeviceLineEdit->text());
  settings.setValue(s_winkey_sidetonepaddle,
                    PaddleSidetoneCheckBox->isChecked());
  settings.setValue(s_winkey_sidetone, SidetoneFreqComboBox->currentIndex());
  settings.setValue(s_winkey_paddle_swap, PaddleSwapCheckBox->isChecked());
  settings.setValue(s_winkey_ctspace, CTCheckBox->isChecked());
  settings.setValue(s_winkey_paddle_mode, PaddleModeComboBox->currentIndex());
  settings.setValue(s_cw_device, comboBox->currentIndex());
  settings.setValue(s_cwdaemon_udp[0], UDP1LineEdit->text().toInt());
  settings.setValue(s_cwdaemon_udp[1], UDP2LineEdit->text().toInt());
  settings.sync();

  emit setType((cwtype)comboBox->currentIndex());
  emit startCw();
  setResult(1);
}

/*! called if dialog rejected */
void CWDialog::rejectChanges() {
  DeviceLineEdit->setText(
      settings.value(s_winkey_device, s_winkey_device_def).toString());
  PaddleSidetoneCheckBox->setChecked(
      settings.value(s_winkey_sidetonepaddle, s_winkey_sidetonepaddle_def)
          .toBool());
  SidetoneFreqComboBox->setCurrentIndex(
      settings.value(s_winkey_sidetone, s_winkey_sidetone_def).toInt());
  PaddleSwapCheckBox->setChecked(
      settings.value(s_winkey_paddle_swap, s_winkey_paddle_swap_def).toBool());
  CTCheckBox->setChecked(
      settings.value(s_winkey_ctspace, s_winkey_ctspace_def).toBool());
  PaddleModeComboBox->setCurrentIndex(
      settings.value(s_winkey_paddle_mode, s_winkey_paddle_mode_def).toInt());
  UDP1LineEdit->setText(
      settings.value(s_cwdaemon_udp[0], s_cwdaemon_udp_def[0]).toString());
  UDP2LineEdit->setText(
      settings.value(s_cwdaemon_udp[1], s_cwdaemon_udp_def[1]).toString());

  comboBox->setCurrentIndex(
      settings.value(s_cw_device, s_cw_device_def).toInt());

  reject();
}
