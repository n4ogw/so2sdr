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
#include "rtlsetup.h"

/*!
 * \brief RtlSetup::RtlSetup
 *     Dialog to configure RTL SDR (in network mode via rtl_tcp)
 * \param s
 *     global settings object
 * \param parent
 */
RtlSetup::RtlSetup(QSettings &s, uiSize sizes, QWidget *parent)
    : QDialog(parent), settings(s) {
  setupUi(this);
  lineEditOffset->setFixedWidth(qRound(sizes.width * 15));
  tunerGainLineEdit->setFixedWidth(qRound(sizes.width * 15));
  deviceIndexLineEdit->setFixedWidth(qRound(sizes.width * 15));
  IFFreqLineEdit->setFixedWidth(qRound(sizes.width * 15));
  speedComboBox->setFixedWidth(qRound(sizes.width * 15));
  sampleRateComboBox->setFixedWidth(qRound(sizes.width * 15));
  bandOffsetPushButton->setFixedWidth(qRound(sizes.width * 15));
  sampleRateComboBox->addItem("262144");
  sampleRateComboBox->addItem("100000 (x16 avg)");
  sampleRateComboBox->addItem("128000 (x16 avg)");

  adjustSize();
  setFixedSize(size());

  offsetSetup = new BandOffsetSetup(s, network_t, sizes, this);
  connect(bandOffsetPushButton, SIGNAL(clicked(bool)), offsetSetup,
          SLOT(exec()));
  updateFromSettings();
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(updateRtl()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(rejectChanges()));
}

RtlSetup::~RtlSetup() { delete offsetSetup; }

double RtlSetup::offset(int band) const {
  if (band == BAND_NONE)
    return settings.value(s_sdr_offset_rtl, s_sdr_offset_rtl_def).toDouble();

  if (offsetSetup->hasOffset(band)) {
    return offsetSetup->offset(band);
  } else {
    return settings.value(s_sdr_offset_rtl, s_sdr_offset_rtl_def).toDouble();
  }
}

bool RtlSetup::invert(int band) const {
  if (band == BAND_NONE)
    return settings.value(s_sdr_swap_rtl, s_sdr_swap_rtl_def).toBool();

  if (offsetSetup->hasOffset(band)) {
    return offsetSetup->invert(band);
  } else {
    return settings.value(s_sdr_swap_rtl, s_sdr_swap_rtl_def).toBool();
  }
}

/*!
 * \brief RtlSetup::updateFromSettings
   update widgets from settings object
*/
void RtlSetup::updateFromSettings() {
  switch (settings.value(s_sdr_rtl_speed, s_sdr_rtl_speed_def).toInt()) {
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
  lineEditOffset->setText(
      settings.value(s_sdr_offset_rtl, s_sdr_offset_rtl_def).toString());
  checkBoxSwap->setChecked(
      settings.value(s_sdr_swap_rtl, s_sdr_swap_rtl_def).toBool());
  deviceIndexLineEdit->setText(
      settings.value(s_sdr_rtl_dev_index, s_sdr_rtl_dev_index_def).toString());
  tunerGainLineEdit->setText(
      settings.value(s_sdr_rtl_tuner_gain, s_sdr_rtl_tuner_gain_def)
          .toString());
  IFFreqLineEdit->setText(
      settings.value(s_sdr_rtl_if_freq, s_sdr_rtl_if_freq_def).toString());
  directCheckBox->setChecked(
      settings.value(s_sdr_rtl_direct, s_sdr_rtl_direct_def).toBool());
  if (settings.value(s_sdr_rtl_sample_freq, s_sdr_rtl_sample_freq_def)
          .toInt() == 262144) {
    sampleRateComboBox->setCurrentIndex(0);
    settings.setValue(s_sdr_rtl_bits, 3);
  } else if (settings.value(s_sdr_rtl_sample_freq, s_sdr_rtl_sample_freq_def)
                 .toInt() == 100000) {
    sampleRateComboBox->setCurrentIndex(1);
    settings.setValue(s_sdr_rtl_bits, 0);
  } else {
    sampleRateComboBox->setCurrentIndex(2);
    settings.setValue(s_sdr_rtl_bits, 0);
  }
}

/*!
 * \brief RtlSetup::updateRtl
 *  update settings from widgets
 */
void RtlSetup::updateRtl() {
  switch (speedComboBox->currentIndex()) {
  case 0:
    settings.setValue(s_sdr_rtl_speed, 1);
    break;
  case 1:
    settings.setValue(s_sdr_rtl_speed, 2);
    break;
  case 2:
    settings.setValue(s_sdr_rtl_speed, 4);
    break;
  }
  settings.setValue(s_sdr_swap_rtl, checkBoxSwap->isChecked());
  settings.setValue(s_sdr_offset_rtl, lineEditOffset->text().toInt());
  settings.setValue(s_sdr_rtl_dev_index, deviceIndexLineEdit->text().toInt());
  settings.setValue(s_sdr_rtl_if_freq, IFFreqLineEdit->text().toInt());
  settings.setValue(s_sdr_rtl_tuner_gain, tunerGainLineEdit->text().toInt());
  settings.setValue(s_sdr_rtl_direct, directCheckBox->isChecked());
  switch (sampleRateComboBox->currentIndex()) {
  case 0:
    settings.setValue(s_sdr_rtl_sample_freq, 262144);
    settings.setValue(s_sdr_rtl_bits, 3);
    break;
  case 1:
    settings.setValue(s_sdr_rtl_sample_freq, 100000);
    settings.setValue(s_sdr_rtl_bits, 0);
    break;
  case 2:
    settings.setValue(s_sdr_rtl_sample_freq, 128000);
    settings.setValue(s_sdr_rtl_bits, 0);
    break;
  default:
    settings.setValue(s_sdr_rtl_sample_freq, 262144);
    settings.setValue(s_sdr_rtl_bits, 3);
  }
}

/*!
 * \brief RtlSetup::rejectChanges
  called when cancel is clicked; reset widgets to values from settings
*/
void RtlSetup::rejectChanges() {
  updateFromSettings();
  reject();
}
