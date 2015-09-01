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
#include <QComboBox>
#include <QDebug>
#include <QSettings>
#include <QString>
#include "sdrdialog.h"
#include "utils.h"

/*!
 * \brief SDRDialog::SDRDialog
 *   Dialog to configure SDR hardware
 * \param s
 *   Global QSettings object
 * \param parent
 */
SDRDialog::SDRDialog(QSettings &s,QWidget *parent) : QDialog(parent),settings(s)
{
    setupUi(this);
    n1mmUdpLineEdit->setEnabled(false);
    comboBoxSdrType->addItem("Soundcard");
    comboBoxSdrType->addItem("SDR-IP SDR");
    comboBoxSdrType->addItem("Afedri Net SDR");
    connect(configureButton,SIGNAL(clicked()),this,SLOT(launchConfigure()));
    soundcard=new SoundCardSetup(settings,this);
    connect(soundcard,SIGNAL(PortAudioError(QString)),this,SIGNAL(setupErrors(QString)));
    soundcard->hide();
    afedri=new AfedriSetup(settings,this);
    connect(afedri,SIGNAL(afedriError(QString)),this,SIGNAL(setupErrors(QString)));
    afedri->hide();
    network=new NetworkSetup(settings,this);
    connect(network,SIGNAL(networkError(QString)),this,SIGNAL(setupErrors(QString)));
    network->hide();
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(updateSDR()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(rejectChanges()));
    updateFromSettings();
}

/*!
 * \brief SDRDialog::launchConfigure
 *   Launch the appropriate configure dialog when the "configure" button is clicked
 */
void SDRDialog::launchConfigure()
{
    switch (comboBoxSdrType->currentIndex()) {
    case 0:
        soundcard->show();
        break;
    case 1:
        network->show();
        break;
    case 2:
        afedri->show();
        break;
    }
}

/*!
 * \brief SDRDialog::updateFromSettings
 *   Update dialog widgets from settings object
 */
void SDRDialog::updateFromSettings()
{
    n1mmUdpCheckBox->setChecked(settings.value(s_sdr_n1mm,s_sdr_n1mm_def).toBool());
    n1mmUdpLineEdit->setText(settings.value(s_sdr_n1mm_port,s_sdr_n1mm_port_def).toString());
    tcpPortLineEdit->setText(settings.value(s_sdr_bandmap_tcp_port,s_sdr_bandmap_tcp_port_def).toString());
    udpPortLineEdit->setText(settings.value(s_sdr_bandmap_udp_port,s_sdr_bandmap_udp_port_def).toString());
    comboBoxSdrType->setCurrentIndex(settings.value(s_sdr_type,s_sdr_type).toInt());
    lineEditIntegTime->setText(settings.value(s_sdr_cqtime,s_sdr_cqtime_def).toString());
    comboBoxIDNumber->setCurrentIndex(settings.value(s_sdr_nrig,s_sdr_nrig_def).toInt());
    reverseScrollCheckBox->setChecked(settings.value(s_sdr_reverse_scroll,s_sdr_reverse_scroll_def).toBool());
    switch ((SdrType)settings.value(s_sdr_type,s_sdr_type_def).toInt()) {
    case soundcard_t:
        settings.setValue(s_sdr_offset,settings.value(s_sdr_offset_soundcard,s_sdr_offset_soundcard_def).toInt());
        settings.setValue(s_sdr_swapiq,settings.value(s_sdr_swap_soundcard,s_sdr_swap_soundcard_def).toBool());
        break;
    case afedri_t:
        settings.setValue(s_sdr_offset,settings.value(s_sdr_offset_afedri,s_sdr_offset_afedri_def).toInt());
        settings.setValue(s_sdr_swapiq,settings.value(s_sdr_swap_afedri,s_sdr_swap_afedri_def).toBool());
        break;
    case network_t:
        settings.setValue(s_sdr_offset,settings.value(s_sdr_offset_network,s_sdr_offset_network_def).toInt());
        settings.setValue(s_sdr_swapiq,settings.value(s_sdr_swap_network,s_sdr_swap_network_def).toBool());
        break;
    }
}

SDRDialog::~SDRDialog()
{
    delete afedri;
    delete soundcard;
    delete network;
}

/*!
 * \brief SDRDialog::updateSDR
 *  Update settings object from dialog widgets
 */
void SDRDialog::updateSDR()
{
    settings.setValue(s_sdr_n1mm,n1mmUdpCheckBox->isChecked());
    settings.setValue(s_sdr_n1mm_port,n1mmUdpLineEdit->text());
    settings.setValue(s_sdr_bandmap_tcp_port,tcpPortLineEdit->text().toInt());
    settings.setValue(s_sdr_bandmap_udp_port,udpPortLineEdit->text().toInt());
    SdrType old=(SdrType)settings.value(s_sdr_type,s_sdr_type_def).toInt();
    settings.setValue(s_sdr_type,comboBoxSdrType->currentIndex());
    settings.setValue(s_sdr_nrig,comboBoxIDNumber->currentIndex());
    settings.setValue(s_sdr_reverse_scroll,reverseScrollCheckBox->isChecked());
    switch ((SdrType)settings.value(s_sdr_type,s_sdr_type_def).toInt()) {
    case soundcard_t:
        settings.setValue(s_sdr_offset,settings.value(s_sdr_offset_soundcard,s_sdr_offset_soundcard_def).toInt());
        settings.setValue(s_sdr_swapiq,settings.value(s_sdr_swap_soundcard,s_sdr_swap_soundcard_def).toBool());
        break;
    case afedri_t:
        settings.setValue(s_sdr_offset,settings.value(s_sdr_offset_afedri,s_sdr_offset_afedri_def).toInt());
        settings.setValue(s_sdr_swapiq,settings.value(s_sdr_swap_afedri,s_sdr_swap_afedri_def).toBool());
        break;
    case network_t:
        settings.setValue(s_sdr_offset,settings.value(s_sdr_offset_network,s_sdr_offset_network_def).toInt());
        settings.setValue(s_sdr_swapiq,settings.value(s_sdr_swap_network,s_sdr_swap_network_def).toBool());
        break;
    }
    emit(update());

    // restart sdr if type changed
    if (old!=(SdrType)settings.value(s_sdr_type,s_sdr_type_def).toInt()) {
        emit(restartSdr());
    }
}

/*!
 * \brief SDRDialog::rejectChanges
 *   called when "cancel" is clicked. Resets widgets to values in settings.
 */
void SDRDialog::rejectChanges()
{
    updateFromSettings();
    reject();
}

