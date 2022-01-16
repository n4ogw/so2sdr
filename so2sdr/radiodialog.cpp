/*! Copyright 2010-2022 R. Torsten Clay N4OGW

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
#include "radiodialog.h"
#include "hamlib/rig.h"
#include <QByteArray>
#include <QComboBox>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QString>


RadioDialog::RadioDialog(QSettings &s, RigSerial &cat, uiSize sizes, QWidget *parent) : QDialog(parent),settings(s),catptr(cat)
{
    setupUi(this);
    lineEditIp1->setFixedWidth(qRound(sizes.width*15));
    lineEditPort1->setFixedWidth(qRound(sizes.width*15));
    lineEditIp2->setFixedWidth(qRound(sizes.width*15));
    lineEditPort2->setFixedWidth(qRound(sizes.width*15));
    adjustSize();
    setFixedSize(size());

    hamlibVersionLabel->setText(QString(hamlib_version));
    radioBaudComboBox[0]  = Radio1BaudComboBox;
    radioBaudComboBox[1]  = Radio2BaudComboBox;
    radioMfgComboBox[0]   = Radio1MfgComboBox;
    radioMfgComboBox[1]   = Radio2MfgComboBox;
    radioModelComboBox[0] = Radio1ModelComboBox;
    radioModelComboBox[1] = Radio2ModelComboBox;
    radioPollTimeEdit[0]  = Rig1PollEdit;
    radioPollTimeEdit[1]  = Rig2PollEdit;
    radioDevEdit[0]       = Radio1DeviceLineEdit;
    radioDevEdit[1]       = Radio2DeviceLineEdit;
    radioPttComboBox[0]   = Radio1PttComboBox;
    radioPttComboBox[1]   = Radio2PttComboBox;
    radioIFEdit[0]        = Rig1IFEdit;
    radioIFEdit[1]        = Rig2IFEdit;
    connect(checkBoxRigctld1,SIGNAL(toggled(bool)),this,SLOT(rigctld1Checked(bool)));
    connect(checkBoxRigctld2,SIGNAL(toggled(bool)),this,SLOT(rigctld2Checked(bool)));

    for (int i = 0; i < NRIG; i++) {
        radioDevEdit[i]->setToolTip("serial port /dev/ttyS0, /dev/ttyS1, etc. \nUsb-serial devices /dev/ttyUSB0, etc");
    }
    for (int i = 0; i < catptr.hamlibNMfg(); i++) {
        radioMfgComboBox[0]->insertItem(i, catptr.hamlibMfgName(i));
        radioMfgComboBox[1]->insertItem(i, catptr.hamlibMfgName(i));
    }
    connect(radioMfgComboBox[0], SIGNAL(currentIndexChanged(int)), this, SLOT(populateModels1(int)));
    connect(radioMfgComboBox[1], SIGNAL(currentIndexChanged(int)), this, SLOT(populateModels2(int)));

    radioMfgComboBox[0]->setCurrentIndex(0);
    radioMfgComboBox[1]->setCurrentIndex(0);
    populateModelCombo(0, 0);
    populateModelCombo(1, 0);
    for (int i = 0; i < NRIG; i++) {
        radioBaudComboBox[i]->insertItem(0, "1200");
        radioBaudComboBox[i]->insertItem(0, "4800");
        radioBaudComboBox[i]->insertItem(0, "9600");
        radioBaudComboBox[i]->insertItem(0, "19200");
        radioBaudComboBox[i]->insertItem(0, "38400");
        radioPttComboBox[i]->insertItem(0,"DTR");
        radioPttComboBox[i]->insertItem(0,"RTS");
        radioPttComboBox[i]->insertItem(0,"None");
    }
    connect(radiodialog_buttons, SIGNAL(rejected()), this, SLOT(rejectChanges()));
    connect(radiodialog_buttons, SIGNAL(accepted()), this, SLOT(updateRadio()));
    updateFromSettings();
}

RadioDialog::~RadioDialog()
{
}

void RadioDialog::populateModels1(int indx)
{
    populateModelCombo(0, indx);
}

void RadioDialog::populateModels2(int indx)
{
    populateModelCombo(1, indx);
}


/*! populate the list of possible rig models
 */
void RadioDialog::populateModelCombo(int nrig, int mfg_indx)
{
    radioModelComboBox[nrig]->clear();
    for (int i = 0; i < catptr.hamlibNModels(mfg_indx); i++) {
        radioModelComboBox[nrig]->insertItem(i, catptr.hamlibModelName(mfg_indx, i));
    }
}


/*!
   update all params when dialog accepted
 */
void RadioDialog::updateRadio()
{
    for (int i=0;i<NRIG;i++) {
        int b;
        switch (radioBaudComboBox[i]->currentIndex()) {
        case 0: b = 38400; break;
        case 1: b = 19200; break;
        case 2: b = 9600; break;
        case 3: b = 4800; break;
        case 4: b = 1200; break;
        default: b=4800;break;
        }
        settings.setValue(s_radios_baud[i],b);
        settings.setValue(s_radios_poll[i],radioPollTimeEdit[i]->text());
        settings.setValue(s_radios_port[i],radioDevEdit[i]->text());
        settings.setValue(s_radios_rig[i],catptr.hamlibModelIndex(radioMfgComboBox[i]->currentIndex(),
                                                                   radioModelComboBox[i]->currentIndex()));
        settings.setValue(s_radios_ptt_type[i],radioPttComboBox[i]->currentIndex());
        settings.setValue(s_radios_if[i],radioIFEdit[i]->text().toDouble());
    }
    settings.setValue(s_radios_rigctld_enable[0],checkBoxRigctld1->isChecked());
    settings.setValue(s_radios_rigctld_enable[1],checkBoxRigctld2->isChecked());
    settings.setValue(s_radios_rigctld_ip[0],lineEditIp1->text());
    settings.setValue(s_radios_rigctld_ip[1],lineEditIp2->text());
    settings.setValue(s_radios_rigctld_port[0],lineEditPort1->text());
    settings.setValue(s_radios_rigctld_port[1],lineEditPort2->text());
    // this restarts the radio comms
    emit(startRadios());
    accept();
}

void RadioDialog::updateFromSettings()
{
    checkBoxRigctld1->setChecked(settings.value(s_radios_rigctld_enable[0],s_radios_rigctld_enable_def[0]).toBool());
    checkBoxRigctld2->setChecked(settings.value(s_radios_rigctld_enable[1],s_radios_rigctld_enable_def[1]).toBool());
    rigctld1Checked(settings.value(s_radios_rigctld_enable[0],s_radios_rigctld_enable_def[0]).toBool());
    rigctld2Checked(settings.value(s_radios_rigctld_enable[1],s_radios_rigctld_enable_def[1]).toBool());
    lineEditIp1->setText(settings.value(s_radios_rigctld_ip[0],s_radios_rigctld_ip_def[0]).toString());
    lineEditIp2->setText(settings.value(s_radios_rigctld_ip[1],s_radios_rigctld_ip_def[1]).toString());
    lineEditPort1->setText(settings.value(s_radios_rigctld_port[0],s_radios_rigctld_port_def[0]).toString());
    lineEditPort2->setText(settings.value(s_radios_rigctld_port[1],s_radios_rigctld_port_def[1]).toString());

    for (int i=0;i<NRIG;i++) {
        radioDevEdit[i]->setText(settings.value(s_radios_port[i],defaultSerialPort[i]).toString());
        radioPollTimeEdit[i]->setText(settings.value(s_radios_poll[i],s_radios_poll_def[i]).toString());
        int indx1;
        int indx2;
        catptr.hamlibModelLookup(settings.value(s_radios_rig[i],s_radios_rig_def[i]).toInt(), indx1, indx2);
        radioMfgComboBox[i]->setCurrentIndex(indx1);
        radioModelComboBox[i]->setCurrentIndex(indx2);

        radioDevEdit[i]->setText(settings.value(s_radios_port[i],s_radios_port_def[i]).toString());
        radioPollTimeEdit[i]->setText(settings.value(s_radios_poll[i],s_radios_poll_def[i]).toString());
        radioIFEdit[i]->setText(settings.value(s_radios_if[i],s_radios_if_def[i]).toString());
        switch (settings.value(s_radios_baud[i],4800).toInt()) {
        case 1200:
            radioBaudComboBox[i]->setCurrentIndex(4);
            break;
        case 4800:
            radioBaudComboBox[i]->setCurrentIndex(3);
            break;
        case 9600:
            radioBaudComboBox[i]->setCurrentIndex(2);
            break;
        case 19200:
            radioBaudComboBox[i]->setCurrentIndex(1);
            break;
        case 38400:
            radioBaudComboBox[i]->setCurrentIndex(0);
            break;
        }
        radioPttComboBox[i]->setCurrentIndex(settings.value(s_radios_ptt_type[i],s_radios_ptt_type_def).toInt());
    }
}

/*! called if dialog rejected */
void RadioDialog::rejectChanges()
{
    updateFromSettings();
    reject();
}

void RadioDialog::rigctld1Checked(bool b)
{
    Radio1MfgComboBox->setEnabled(!b);
    Radio1BaudComboBox->setEnabled(!b);
    Radio1DeviceLineEdit->setEnabled(!b);
    Radio1ModelComboBox->setEnabled(!b);
    Radio1PttComboBox->setEnabled(!b);
    lineEditIp1->setEnabled(b);
    lineEditPort1->setEnabled(b);
}

void RadioDialog::rigctld2Checked(bool b)
{
    Radio2MfgComboBox->setEnabled(!b);
    Radio2BaudComboBox->setEnabled(!b);
    Radio2DeviceLineEdit->setEnabled(!b);
    Radio2ModelComboBox->setEnabled(!b);
    Radio2PttComboBox->setEnabled(!b);
    lineEditIp2->setEnabled(b);
    lineEditPort2->setEnabled(b);
}

