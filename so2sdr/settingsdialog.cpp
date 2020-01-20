/*! Copyright 2010-2020 R. Torsten Clay N4OGW

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
#include "settingsdialog.h"

SettingsDialog::SettingsDialog(QSettings &s, uiSize sizes, QWidget *parent)  : QDialog(parent),settings(s)
{
    setupUi(this);

    AutoSendComboBox->setFixedWidth(sizes.width*8);
    AutoSendLineEdit->setFixedWidth(sizes.width*5);
    UDPPortLineEdit->setFixedWidth(sizes.width*5);
    CQRepeatLineEdit->setFixedWidth(sizes.width*5);
    DuelingCQLineEdit->setFixedWidth(sizes.width*5);
    label_5->setFixedWidth(sizes.width*8);
    ctyLineEdit->setFixedWidth(sizes.width*20);
    kbd1LineEdit->setFixedWidth(sizes.width*20);
    kbd2LineEdit->setFixedWidth(sizes.width*20);
    AutoSendComboBox->insertItem(0, "Semi");
    AutoSendComboBox->insertItem(1, "Auto");
    adjustSize();
    setFixedSize(size());
    connect(settings_dialog_buttons, SIGNAL(rejected()), this, SLOT(rejectChanges()));
    connect(settings_dialog_buttons, SIGNAL(accepted()), this, SLOT(updateSettings()));
    loadSettings();
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::loadSettings()
{
    QSYFocusCheckBox->setChecked(settings.value(s_settings_qsyfocus,s_settings_qsyfocus_def).toBool());
    FocusIndicatorsCheckBox->setChecked(settings.value(s_settings_focusindicators,s_settings_focusindicators_def).toBool());
    ExchangeLogCheckBox->setChecked(settings.value(s_settings_exchangelogs,s_settings_exchangelogs_def).toBool());
    AutoSendLineEdit->setText(settings.value(s_settings_autosend,s_settings_autosend_def).toString());
    CQRepeatLineEdit->setText(QString::number(settings.value(s_settings_cqrepeat,s_settings_cqrepeat_def).toInt()/1000.0,'f',1));
    DuelingCQLineEdit->setText(settings.value(s_settings_duelingcqdelay,s_settings_duelingcqdelay_def).toString());
    AutoSendComboBox->setCurrentIndex(settings.value(s_settings_autosend_mode,s_settings_autosend_mode_def).toInt());
    ctyLineEdit->setText(settings.value(s_cty_url,s_cty_url_def).toString());
    ctyLineEdit->setCursorPosition(0);
    UDPCheckBox->setChecked(settings.value(s_wsjtx_enable,s_wsjtx_enable_def).toBool());
    UDPPortLineEdit->setText(settings.value(s_wsjtx_udp,s_wsjtx_udp_def).toString());
    kbd1LineEdit->setText(settings.value(s_twokeyboard_device[0],s_twokeyboard_device_def[0]).toString());
    kbd2LineEdit->setText(settings.value(s_twokeyboard_device[1],s_twokeyboard_device_def[1]).toString());
    kbd1LineEdit->setEnabled(settings.value(s_twokeyboard_enable,s_twokeyboard_enable_def).toBool());
    kbd2LineEdit->setEnabled(settings.value(s_twokeyboard_enable,s_twokeyboard_enable_def).toBool());
    kbdCheckBox->setChecked(settings.value(s_twokeyboard_enable,s_twokeyboard_enable_def).toBool());
    queueCheckBox->setChecked(settings.value(s_queuemessages,s_queuemessages_def).toBool());
}

void SettingsDialog::updateSettings()
{
    settings.setValue(s_settings_qsyfocus,QSYFocusCheckBox->isChecked());
    settings.setValue(s_settings_focusindicators,FocusIndicatorsCheckBox->isChecked());
    settings.setValue(s_settings_exchangelogs,ExchangeLogCheckBox->isChecked());
    settings.setValue(s_settings_autosend,AutoSendLineEdit->text().toInt());
    settings.setValue(s_settings_cqrepeat,(int)(CQRepeatLineEdit->text().toDouble()*1000));
    settings.setValue(s_settings_duelingcqdelay,DuelingCQLineEdit->text().toDouble());
    settings.setValue(s_settings_autosend_mode,AutoSendComboBox->currentIndex());
    settings.setValue(s_cty_url,ctyLineEdit->text().trimmed());
    settings.setValue(s_wsjtx_enable,UDPCheckBox->isChecked());
    settings.setValue(s_wsjtx_udp,UDPPortLineEdit->text().toInt());
    settings.setValue(s_twokeyboard_enable,kbdCheckBox->isChecked());
    settings.setValue(s_twokeyboard_device[0],kbd1LineEdit->text());
    settings.setValue(s_twokeyboard_device[1],kbd2LineEdit->text());
    settings.setValue(s_queuemessages,queueCheckBox->isChecked());
    settings.sync();
    emit(settingsUpdate());
    accept();
}

void SettingsDialog::rejectChanges()
{
    loadSettings();
    reject();
}
