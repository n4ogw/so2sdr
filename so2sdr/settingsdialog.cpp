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
#include "defines.h"
#include "settingsdialog.h"

SettingsDialog::SettingsDialog(QSettings &s, QWidget *parent)  : QDialog(parent),settings(s)
{
    setupUi(this);

    // load settings
    QSYFocusCheckBox->setChecked(settings.value(s_settings_qsyfocus,false).toBool());
    FocusIndicatorsCheckBox->setChecked(settings.value(s_settings_focusindicators,false).toBool());
    ExchangeLogCheckBox->setChecked(settings.value(s_settings_exchangelogs,false).toBool());
    CQRepeatLineEdit->setText(settings.value(s_settings_cqrepeat,s_settings_cqrepeat_def).toString());
    AutoSendLineEdit->setText(settings.value(s_settings_autosend,s_settings_autosend_def).toString());
    DuelingCQLineEdit->setText(settings.value(s_settings_duelingcqdelay,s_settings_duelingcqdelay_def).toString());
    AutoSendComboBox->insertItem(0, "Semi");
    AutoSendComboBox->insertItem(1, "Auto");
    AutoSendComboBox->setCurrentIndex(settings.value(s_settings_autosend_mode,s_settings_autosend_mode_def).toInt());
    connect(settings_dialog_buttons, SIGNAL(rejected()), this, SLOT(rejectChanges()));
    connect(settings_dialog_buttons, SIGNAL(accepted()), this, SLOT(updateSettings()));
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::updateSettings()
{
    settings.setValue(s_settings_qsyfocus,QSYFocusCheckBox->isChecked());
    settings.setValue(s_settings_focusindicators,FocusIndicatorsCheckBox->isChecked());
    settings.setValue(s_settings_exchangelogs,ExchangeLogCheckBox->isChecked());
    settings.setValue(s_settings_autosend,AutoSendLineEdit->text().toInt());
    settings.setValue(s_settings_cqrepeat,CQRepeatLineEdit->text().toDouble());
    settings.setValue(s_settings_duelingcqdelay,DuelingCQLineEdit->text().toDouble());
    settings.setValue(s_settings_autosend_mode,AutoSendComboBox->currentIndex());
    settings.sync();
    emit(settingsUpdate());
    accept();
}

void SettingsDialog::rejectChanges()
{
    QSYFocusCheckBox->setChecked(settings.value(s_settings_qsyfocus,s_settings_qsyfocus_def).toBool());
    FocusIndicatorsCheckBox->setChecked(settings.value(s_settings_focusindicators,s_settings_focusindicators_def).toBool());
    ExchangeLogCheckBox->setChecked(settings.value(s_settings_exchangelogs,s_settings_exchangelogs_def).toBool());
    AutoSendLineEdit->setText(settings.value(s_settings_autosend,s_settings_autosend_def).toString());
    CQRepeatLineEdit->setText(settings.value(s_settings_cqrepeat,s_settings_cqrepeat_def).toString());
    DuelingCQLineEdit->setText(settings.value(s_settings_duelingcqdelay,s_settings_duelingcqdelay_def).toString());
    AutoSendComboBox->setCurrentIndex(settings.value(s_settings_autosend_mode,s_settings_autosend_mode_def).toInt());
    reject();
}
