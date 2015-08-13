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
#include "radiodialog.h"
#include "hamlib/rig.h"
#include <QByteArray>
#include <QComboBox>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QString>


RadioDialog::RadioDialog(QSettings &s, RigSerial &cat, QWidget *parent) : QDialog(parent),settings(s),catptr(cat)
{
    setupUi(this);
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

    for (int i = 0; i < NRIG; i++) {
#ifdef Q_OS_WIN
        radioDevEdit[i]->setToolTip("serial port COM1, COM2, ...");
#endif
#ifdef Q_OS_LINUX
        radioDevEdit[i]->setToolTip("serial port /dev/ttyS0, /dev/ttyS1, etc. \nUsb-serial devices /dev/ttyUSB0, etc");
#endif
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
        radioPttComboBox[i]->insertItem(0,"None");
        radioPttComboBox[i]->insertItem(0,"RTS");
        radioPttComboBox[i]->insertItem(0,"DTR");
        radioPttComboBox[i]->insertItem(0,"Hamlib");
    }
    comboBoxOTRSPBaud->insertItem(0,"1200");
    comboBoxOTRSPBaud->insertItem(0,"4800");
    comboBoxOTRSPBaud->insertItem(0,"9600");
    comboBoxOTRSPBaud->insertItem(0,"19200");
    comboBoxOTRSPBaud->insertItem(0,"38400");
    comboBoxOTRSPBaud->setCurrentIndex(2);
    comboBoxOTRSPBaud->setEnabled(false);
    comboBoxOTRSPParity->insertItem(0,"N");
    comboBoxOTRSPParity->setCurrentIndex(0);
    comboBoxOTRSPParity->setEnabled(false);
    comboBoxOTRSPStop->insertItem(0,"1");
    comboBoxOTRSPStop->setCurrentIndex(0);
    comboBoxOTRSPStop->setEnabled(false);
    lineEditOTRSPPort->clear();
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
#ifdef Q_OS_LINUX
    ParallelPortComboBox->insertItem(0, "/dev/parport1");
    ParallelPortComboBox->insertItem(0, "/dev/parport0");
#endif
#ifdef Q_OS_WIN
    ParallelPortComboBox->insertItem(0, "0x278");
    ParallelPortComboBox->insertItem(0, "0x378");
#endif
    ParallelPortComboBox->setEditable(true); // allow other port numbers to be entered
    ParallelPortComboBox->setCurrentIndex(0);
#ifdef Q_OS_LINUX
    ParallelPortComboBox->setToolTip("Parallel port access requires the PPDEV kernel module and being in the correct group (usually lp).");
#endif
#ifdef Q_OS_WIN
    ParallelPortComboBox->setToolTip("Parallel port access uses INPOUT32.");
#endif
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
    }
    settings.setValue(s_radios_focus,RadioPinComboBox->currentIndex() + 2);
    settings.setValue(s_radios_txfocus,TransmitPinComboBox->currentIndex() + 2);
    settings.setValue(s_radios_stereo,StereoPinComboBox->currentIndex() + 2);
    settings.setValue(s_radios_focusinvert,RadioInvertCheckBox->isChecked());
    settings.setValue(s_radios_txfocusinvert,TransmitInvertCheckBox->isChecked());
    bool pportUpdate=false;
    if (settings.value(s_radios_pport,defaultParallelPort).toString()!=ParallelPortComboBox->currentText()) {
        settings.setValue(s_radios_pport,ParallelPortComboBox->currentText());
        pportUpdate=true;
    }
    if (settings.value(s_radios_pport_enabled,s_radios_pport_enabled_def).toBool()!=checkBoxParallelPort->isChecked()) {
        settings.setValue(s_radios_pport_enabled,checkBoxParallelPort->isChecked());
        pportUpdate=true;
    }
    bool otrspUpdate=false;
    if (settings.value(s_otrsp_enabled,s_otrsp_enabled_def).toBool()!=checkBoxOTRSP->isChecked()) {
        settings.setValue(s_otrsp_enabled,checkBoxOTRSP->isChecked());
        if (settings.value(s_otrsp_enabled,s_otrsp_enabled_def).toBool()) {
            otrspUpdate=true;
        }
    }
    if (settings.value(s_otrsp_device,s_otrsp_device_def).toString()!=lineEditOTRSPPort->text()) {
        settings.setValue(s_otrsp_device,lineEditOTRSPPort->text());
        otrspUpdate=true;
    }
    bool microHamUpdate=false;
    if (settings.value(s_microham_enabled,s_microham_enabled_def).toBool()!=checkBoxMicroHam->isChecked()) {
        settings.setValue(s_microham_enabled,checkBoxMicroHam->isChecked());
        if (settings.value(s_microham_enabled,s_microham_enabled_def).toBool()) {
            microHamUpdate=true;
        }
    }
    if (settings.value(s_microham_device,s_microham_device_def).toString()!=lineEditMicroHamPort->text()) {
        settings.setValue(s_microham_device,lineEditMicroHamPort->text());
        microHamUpdate=true;
    }
    settings.sync();
    // in case parallel port is changed
    if (pportUpdate) {
        emit(setParallelPort());
    }
    // restart otrsp if needed
    if (otrspUpdate) {
        emit(setOTRSP());
    }
    if (microHamUpdate) {
        emit(setMicroHam());
    }
    // this restarts the radio comms
    emit(startRadios());
    accept();
}

void RadioDialog::updateFromSettings()
{
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
    }
    RadioPinComboBox->setCurrentIndex(settings.value(s_radios_focus,defaultParallelPortAudioPin).toInt() - 2);
    TransmitPinComboBox->setCurrentIndex(settings.value(s_radios_txfocus,defaultParallelPortTxPin).toInt() - 2);
    StereoPinComboBox->setCurrentIndex(settings.value(s_radios_stereo,defaultParallelPortStereoPin).toInt() - 2);
    RadioInvertCheckBox->setChecked(settings.value(s_radios_focusinvert,false).toBool());
    TransmitInvertCheckBox->setChecked(settings.value(s_radios_txfocusinvert,false).toBool());
    bool found=false;
    for (int i=0;i<ParallelPortComboBox->count();i++) {
        if (ParallelPortComboBox->itemText(i)==settings.value(s_radios_pport,defaultParallelPort).toString()) {
            ParallelPortComboBox->setCurrentIndex(i);
            found=true;
        }
    }
    if (!found) {
        ParallelPortComboBox->setCurrentIndex(0);
    }
    checkBoxParallelPort->setChecked(settings.value(s_radios_pport_enabled,s_radios_pport_enabled_def).toBool());
    checkBoxOTRSP->setChecked(settings.value(s_otrsp_enabled,s_otrsp_enabled_def).toBool());
    lineEditOTRSPPort->setText(settings.value(s_otrsp_device,s_otrsp_device_def).toString());
    checkBoxMicroHam->setChecked(settings.value(s_microham_enabled,s_microham_enabled_def).toBool());
    lineEditMicroHamPort->setText(settings.value(s_microham_device,s_microham_device_def).toString());
}

/*! called if dialog rejected */
void RadioDialog::rejectChanges()
{
    updateFromSettings();
    reject();
}
