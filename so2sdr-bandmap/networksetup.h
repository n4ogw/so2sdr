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
#ifndef NETWORKSETUP_H
#define NETWORKSETUP_H

#include "defines.h"
#include <QDialog>
#include <QSettings>
#include "ui_networksetup.h"


class NetworkSetup : public QDialog, public Ui::networkSetup
{
    Q_OBJECT
public:
    explicit NetworkSetup(QSettings &s,QWidget *parent = 0);

signals:
    void networkError(const QString &);

private slots:
    void updateNetwork();
    void rejectChanges();

private:
    QSettings &settings;
    void updateFromSettings();
};

#endif // NETWORKSETUP_H
