/*! Copyright 2010-2015 R. Torsten Clay N4OGW

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
AfedriSetup::AfedriSetup(QSettings &s,QWidget *parent) : QDialog(parent),settings(s)
{
    setupUi(this);
    updateFromSettings();
    connect(buttonBox,SIGNAL(accepted()),this,SLOT(updateAfedri()));
    connect(buttonBox,SIGNAL(rejected()),this,SLOT(rejectChanges()));
    connect(broadcastOffButton,SIGNAL(clicked()),this,SLOT(enableControls()));
    connect(broadcastMasterButton,SIGNAL(clicked()),this,SLOT(enableControls()));
    connect(broadcastSlaveButton,SIGNAL(clicked()),this,SLOT(enableControls()));
}

/*! enable/disable controls depending on master/slave settings
 *  some controls disabled if in slave mode
 **/
void AfedriSetup::enableControls()
{
    int b=broadcastButtonGroup->checkedId();
    switch (b) {
    case -2:case -4:  // master, off
        tcpipaddressLineEdit->setEnabled(true);
        tcpportLineEdit->setEnabled(true);
        lineEditFreq1->setEnabled(true);
        lineEditFreq2->setEnabled(true);
        lineEditFreq3->setEnabled(true);
        lineEditFreq4->setEnabled(true);
        break;
    case -3:   // slave
        tcpipaddressLineEdit->setEnabled(false);
        tcpportLineEdit->setEnabled(false);
        lineEditFreq1->setEnabled(false);
        lineEditFreq2->setEnabled(false);
        lineEditFreq3->setEnabled(false);
        lineEditFreq4->setEnabled(false);
        break;
    }

}

/*!
 * \brief AfedriSetup::updateFromSettings
   update widgets from settings object
*/
void AfedriSetup::updateFromSettings()
{
    switch (settings.value(s_sdr_afedri_speed,s_sdr_afedri_speed_def).toInt()) {
    case 1:
        fftComboBox->setCurrentIndex(0);
        break;
    case 2:
        fftComboBox->setCurrentIndex(1);
        break;
    case 4:
        fftComboBox->setCurrentIndex(2);
        break;
    default:
        fftComboBox->setCurrentIndex(0);

    }
    sampleFreqLineEdit->setText(settings.value(s_sdr_afedri_sample_freq,s_sdr_afedri_sample_freq_def).toString());
    multiChannelComboBox->setCurrentIndex(settings.value(s_sdr_afedri_multi,s_sdr_afedri_multi_def).toInt());
    udpportLineEdit->setText(settings.value(s_sdr_afedri_udp_port,s_sdr_afedri_udp_port_def).toString());
    tcpportLineEdit->setText(settings.value(s_sdr_afedri_tcp_port,s_sdr_afedri_tcp_port_def).toString());
    channelComboBox->setCurrentIndex(settings.value(s_sdr_afedri_channel,s_sdr_afedri_channel_def).toInt());
    switch (settings.value(s_sdr_afedri_bcast,s_sdr_afedri_bcast_def).toInt()) {
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
    enableControls();
    tcpipaddressLineEdit->setText(settings.value(s_sdr_afedri_tcp_ip,s_sdr_afedri_tcp_ip_def).toString());
    lineEditFreq1->setText(settings.value(s_sdr_afedri_freq1,s_sdr_afedri_freq1_def).toString());
    lineEditFreq2->setText(settings.value(s_sdr_afedri_freq2,s_sdr_afedri_freq2_def).toString());
    lineEditFreq3->setText(settings.value(s_sdr_afedri_freq3,s_sdr_afedri_freq3_def).toString());
    lineEditFreq4->setText(settings.value(s_sdr_afedri_freq4,s_sdr_afedri_freq4_def).toString());
    lineEditOffset->setText(settings.value(s_sdr_offset_afedri,s_sdr_offset_afedri_def).toString());
    checkBoxSwap->setChecked(settings.value(s_sdr_swap_afedri,s_sdr_swap_afedri_def).toBool());
}

/*!
 * \brief AfedriSetup::updateAfedri
 *  update settings from widgets
 */
void AfedriSetup::updateAfedri()
{
    switch (fftComboBox->currentIndex()) {
    case 0:
        settings.setValue(s_sdr_afedri_speed,1);
        break;
    case 1:
        settings.setValue(s_sdr_afedri_speed,2);
        break;
    case 2:
        settings.setValue(s_sdr_afedri_speed,4);
        break;
    }
    settings.setValue(s_sdr_afedri_sample_freq,sampleFreqLineEdit->text().toInt());
    settings.setValue(s_sdr_afedri_multi,multiChannelComboBox->currentIndex());
    settings.setValue(s_sdr_afedri_udp_port,udpportLineEdit->text().toInt());
    settings.setValue(s_sdr_afedri_channel,channelComboBox->currentIndex());
    settings.setValue(s_sdr_afedri_tcp_port,tcpportLineEdit->text().toInt());
    if (broadcastOffButton->isChecked()) settings.setValue(s_sdr_afedri_bcast,0);
    if (broadcastMasterButton->isChecked()) settings.setValue(s_sdr_afedri_bcast,1);
    if (broadcastSlaveButton->isChecked()) settings.setValue(s_sdr_afedri_bcast,2);
    settings.setValue(s_sdr_afedri_tcp_ip,tcpipaddressLineEdit->text());
    settings.setValue(s_sdr_afedri_freq1,lineEditFreq1->text().toInt());
    settings.setValue(s_sdr_afedri_freq2,lineEditFreq2->text().toInt());
    settings.setValue(s_sdr_afedri_freq3,lineEditFreq3->text().toInt());
    settings.setValue(s_sdr_afedri_freq4,lineEditFreq4->text().toInt());
    settings.setValue(s_sdr_swap_afedri,checkBoxSwap->isChecked());
    settings.setValue(s_sdr_offset_afedri,lineEditOffset->text().toInt());
}

/*!
 * \brief AfedriSetup::rejectChanges
  called when cancel is clicked; reset widgets to values from settings
*/
void AfedriSetup::rejectChanges()
{
    updateFromSettings();
    reject();
}
