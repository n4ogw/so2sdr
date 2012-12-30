/*! Copyright 2010-2013 R. Torsten Clay N4OGW

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
#include <QDialog>
#include <QMessageBox>
#include <QSettings>
#include <QString>
#include "defines.h"
#include "winkeydialog.h"

/*!
  WinkeyDialog is a dialog for entering winkey parameters

  s is station config file (so2sdr.ini) in QSettings .ini format
 */
WinkeyDialog::WinkeyDialog(QSettings *s,QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    // load from settings file
    settings=s;
    DeviceLineEdit->setText(settings->value(s_winkey_device,s_winkey_device_def).toString());
#ifdef Q_OS_WIN
    DeviceLineEdit->setToolTip("serial port COM1, COM2, ...");
#endif
#ifdef Q_OS_LINUX
    DeviceLineEdit->setToolTip("serial port /dev/ttyS0, /dev/ttyS1, /dev/ttyUSB0, ...");
#endif

    CWCheckBox->setChecked(settings->value(s_winkey_cwon,s_winkey_cwon_def).toBool());
    CTCheckBox->setChecked(settings->value(s_winkey_ctspace,s_winkey_ctspace_def).toBool());
    PaddleSwapCheckBox->setChecked(settings->value(s_winkey_paddle_swap,s_winkey_paddle_swap_def).toBool());
    PaddleSidetoneCheckBox->setChecked(settings->value(s_winkey_sidetonepaddle,s_winkey_sidetonepaddle_def).toBool());
    PaddleModeComboBox->insertItem(0, "Iambic B");
    PaddleModeComboBox->insertItem(1, "Iambic A");
    PaddleModeComboBox->insertItem(2, "Ultimatic");
    PaddleModeComboBox->insertItem(3, "Bug");
    PaddleModeComboBox->setCurrentIndex(settings->value(s_winkey_paddle_mode,s_winkey_paddle_mode_def).toInt());
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
    SidetoneFreqComboBox->setCurrentIndex(settings->value(s_winkey_sidetone,s_winkey_sidetone_def).toInt());
    connect(winkey_buttons, SIGNAL(rejected()), this, SLOT(rejectChanges()));
    connect(winkey_buttons, SIGNAL(accepted()), this, SLOT(updateWinkey()));
}

WinkeyDialog::~WinkeyDialog()
{
}

/*!
   sets text showing Winkey version
 */
void WinkeyDialog::setWinkeyVersionLabel(int version)
{
    QString label = QString::number(version);
    VersionLabel->setText(label);
}

/*!
   update Winkey parameters from form input
 */
void WinkeyDialog::updateWinkey()
{
    settings->setValue(s_winkey_device,DeviceLineEdit->text());
    settings->setValue(s_winkey_sidetonepaddle,PaddleSidetoneCheckBox->isChecked());
    settings->setValue(s_winkey_sidetone,SidetoneFreqComboBox->currentIndex());
    settings->setValue(s_winkey_paddle_swap,PaddleSwapCheckBox->isChecked());
    settings->setValue(s_winkey_ctspace,CTCheckBox->isChecked());
    settings->setValue(s_winkey_paddle_mode,PaddleModeComboBox->currentIndex());
    settings->sync();

    // emit signal to start/restart winkey
    emit(startWinkey());

    setResult(1);
}

/*! called if dialog rejected */
void WinkeyDialog::rejectChanges()
{
    DeviceLineEdit->setText(settings->value(s_winkey_device,s_winkey_device_def).toString());
    PaddleSidetoneCheckBox->setChecked(settings->value(s_winkey_sidetonepaddle,s_winkey_sidetonepaddle_def).toBool());
    SidetoneFreqComboBox->setCurrentIndex(settings->value(s_winkey_sidetone,s_winkey_sidetone_def).toInt());
    PaddleSwapCheckBox->setChecked(settings->value(s_winkey_paddle_swap,s_winkey_paddle_swap_def).toBool());
    CTCheckBox->setChecked(settings->value(s_winkey_ctspace,s_winkey_ctspace_def).toBool());
    PaddleModeComboBox->setCurrentIndex(settings->value(s_winkey_paddle_mode,s_winkey_paddle_mode_def).toInt());
    reject();
}
