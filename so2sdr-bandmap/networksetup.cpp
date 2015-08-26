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
#include "networksetup.h"

/*!
 * \brief NetworkSetup::NetworkSetup
 *     Dialog to configure Network SDR (in network mode)
 * \param s
 *     global settings object
 * \param parent
 */
NetworkSetup::NetworkSetup(QSettings &s,QWidget *parent) : QDialog(parent),settings(s)
{
    setupUi(this);
    updateFromSettings();
    connect(buttonBox,SIGNAL(accepted()),this,SLOT(updateNetwork()));
    connect(buttonBox,SIGNAL(rejected()),this,SLOT(rejectChanges()));
}

/*!
 * \brief NetworkSetup::updateFromSettings
   update widgets from settings object
*/
void NetworkSetup::updateFromSettings()
{
    switch (settings.value(s_sdr_net_speed,s_sdr_net_speed_def).toInt()) {
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
    tcpipaddressLineEdit->setText(settings.value(s_sdr_net_tcp_ip,s_sdr_net_tcp_ip_def).toString());
    tcpportLineEdit->setText(settings.value(s_sdr_net_tcp_port,s_sdr_net_tcp_port_def).toString());
    udpportLineEdit->setText(settings.value(s_sdr_net_udp_port,s_sdr_net_udp_port_def).toString());
    lineEditOffset->setText(settings.value(s_sdr_offset_network,s_sdr_offset_network_def).toString());
    checkBoxSwap->setChecked(settings.value(s_sdr_swap_network,s_sdr_swap_network_def).toBool());
}

/*!
 * \brief NetworkSetup::updateNetwork
 *  update settings from widgets
 */
void NetworkSetup::updateNetwork()
{
    switch (fftComboBox->currentIndex()) {
    case 0:
        settings.setValue(s_sdr_net_speed,1);
        break;
    case 1:
        settings.setValue(s_sdr_net_speed,2);
        break;
    case 2:
        settings.setValue(s_sdr_net_speed,4);
        break;
    }
    settings.setValue(s_sdr_net_tcp_port,tcpportLineEdit->text().toInt());
    settings.setValue(s_sdr_net_udp_port,udpportLineEdit->text().toInt());
    settings.setValue(s_sdr_net_tcp_ip,tcpipaddressLineEdit->text());
    settings.setValue(s_sdr_swap_network,checkBoxSwap->isChecked());
    settings.setValue(s_sdr_offset_network,lineEditOffset->text().toInt());
}

/*!
 * \brief NetworkSetup::rejectChanges
  called when cancel is clicked; reset widgets to values from settings
*/
void NetworkSetup::rejectChanges()
{
    updateFromSettings();
    reject();
}
