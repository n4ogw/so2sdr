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
#include "afedrisetup.h"

/*!
 * \brief AfedriSetup::AfedriSetup
 *     Dialog to configure Afedri SDR (in network mode)
 * \param s
 *     global settings object
 * \param parent
 */
AfedriSetup::AfedriSetup(QSettings &s, uiSize sizes, QWidget *parent)
    : QDialog(parent), settings(s) {
  setupUi(this);
  tcpipaddressLineEdit->setFixedWidth(sizes.uiWidth * 15);
  tcpportLineEdit->setFixedWidth(sizes.uiWidth * 15);
  udpportLineEdit->setFixedWidth(sizes.uiWidth * 15);
  IFFreqLineEdit->setFixedWidth(sizes.uiWidth * 15);
  sampleFreqLineEdit->setFixedWidth(sizes.uiWidth * 15);
  lineEditOffset->setFixedWidth(sizes.uiWidth * 15);
  adjustSize();
  setFixedSize(size());

  updateFromSettings();
  offsetSetup = new BandOffsetSetup(s, afedri_t, sizes, this);
  connect(bandOffsetPushButton, SIGNAL(clicked(bool)), offsetSetup,
          SLOT(exec()));
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(updateAfedri()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(rejectChanges()));
}

/*!
 * \brief AfedriSetup::updateFromSettings
   update widgets from settings object
*/
void AfedriSetup::updateFromSettings() {
  switch (settings.value(s_sdr_afedri_speed, s_sdr_afedri_speed_def).toInt()) {
  case 1:
    speedComboBox->setCurrentIndex(0);
    break;
  case 2:
    speedComboBox->setCurrentIndex(1);
    break;
  case 4:
    speedComboBox->setCurrentIndex(2);
    break;
  default:
    speedComboBox->setCurrentIndex(0);
  }
  sampleFreqLineEdit->setText(
      settings.value(s_sdr_afedri_sample_freq, s_sdr_afedri_sample_freq_def)
          .toString());
  multiChannelComboBox->setCurrentIndex(
      settings.value(s_sdr_afedri_multi, s_sdr_afedri_multi_def).toInt());
  udpportLineEdit->setText(
      settings.value(s_sdr_afedri_udp_port, s_sdr_afedri_udp_port_def)
          .toString());
  tcpportLineEdit->setText(
      settings.value(s_sdr_afedri_tcp_port, s_sdr_afedri_tcp_port_def)
          .toString());
  channelComboBox->setCurrentIndex(
      settings.value(s_sdr_afedri_channel, s_sdr_afedri_channel_def).toInt());
  switch (settings.value(s_sdr_afedri_bcast, s_sdr_afedri_bcast_def).toInt()) {
  case 0:
    broadcastOffButton->setChecked(true);
    break;
  case 1:
    broadcastMasterButton->setChecked(true);
    break;
  case 2:
    broadcastSlaveButton->setChecked(true);
    break;
  }
  tcpipaddressLineEdit->setText(
      settings.value(s_sdr_afedri_tcp_ip, s_sdr_afedri_tcp_ip_def).toString());
  IFFreqLineEdit->setText(
      settings.value(s_sdr_afedri_freq, s_sdr_afedri_freq_def).toString());
  lineEditOffset->setText(
      settings.value(s_sdr_offset_afedri, s_sdr_offset_afedri_def).toString());
  checkBoxSwap->setChecked(
      settings.value(s_sdr_swap_afedri, s_sdr_swap_afedri_def).toBool());
}

double AfedriSetup::offset(int band) const {
  if (band == BAND_NONE)
    return settings.value(s_sdr_offset_afedri, s_sdr_offset_afedri_def)
        .toDouble();

  if (offsetSetup->hasOffset(band)) {
    return offsetSetup->offset(band);
  } else {
    return settings.value(s_sdr_offset_afedri, s_sdr_offset_afedri_def)
        .toDouble();
  }
}

bool AfedriSetup::invert(int band) const {
  if (band == BAND_NONE)
    return settings.value(s_sdr_swap_afedri, s_sdr_swap_afedri_def).toBool();

  if (offsetSetup->hasOffset(band)) {
    return offsetSetup->invert(band);
  } else {
    return settings.value(s_sdr_swap_afedri, s_sdr_swap_afedri_def).toBool();
  }
}

/*!
 * \brief AfedriSetup::updateAfedri
 *  update settings from widgets
 */
void AfedriSetup::updateAfedri() {
  switch (speedComboBox->currentIndex()) {
  case 0:
    settings.setValue(s_sdr_afedri_speed, 1);
    break;
  case 1:
    settings.setValue(s_sdr_afedri_speed, 2);
    break;
  case 2:
    settings.setValue(s_sdr_afedri_speed, 4);
    break;
  }
  settings.setValue(s_sdr_afedri_sample_freq,
                    sampleFreqLineEdit->text().toInt());
  settings.setValue(s_sdr_afedri_multi, multiChannelComboBox->currentIndex());
  settings.setValue(s_sdr_afedri_udp_port, udpportLineEdit->text().toInt());
  settings.setValue(s_sdr_afedri_channel, channelComboBox->currentIndex());
  settings.setValue(s_sdr_afedri_tcp_port, tcpportLineEdit->text().toInt());
  if (broadcastOffButton->isChecked())
    settings.setValue(s_sdr_afedri_bcast, 0);
  if (broadcastMasterButton->isChecked())
    settings.setValue(s_sdr_afedri_bcast, 1);
  if (broadcastSlaveButton->isChecked())
    settings.setValue(s_sdr_afedri_bcast, 2);
  settings.setValue(s_sdr_afedri_tcp_ip, tcpipaddressLineEdit->text());
  settings.setValue(s_sdr_afedri_freq, IFFreqLineEdit->text().toInt());
  settings.setValue(s_sdr_swap_afedri, checkBoxSwap->isChecked());
  settings.setValue(s_sdr_offset_afedri, lineEditOffset->text().toInt());
}

/*!
 * \brief AfedriSetup::rejectChanges
  called when cancel is clicked; reset widgets to values from settings
*/
void AfedriSetup::rejectChanges() {
  updateFromSettings();
  reject();
}
