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
#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QString>
#include <QDialog>
#include <QCloseEvent>
#include <QSettings>
#include "ui_settingsdialog.h"
#include "defines.h"
#include "utils.h"

class SettingsDialog : public QDialog, public Ui::SettingsDialog
{
Q_OBJECT

public:
    SettingsDialog(QSettings &s, uiSize sizes,QWidget *parent = nullptr);
    ~SettingsDialog();
    friend class So2sdr;

public slots:
    void updateSettings();
    void rejectChanges();

signals:
    void settingsUpdate();

private:
    QSettings&      settings;
    void loadSettings();
};

#endif // SETTINGSDIALOG_H
