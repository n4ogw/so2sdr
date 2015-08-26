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
#ifndef SDRDIALOG_H
#define SDRDIALOG_H

#include <QIcon>
#include <QList>
#include <QSettings>
#include <QString>
#include "defines.h"
#include "soundcardsetup.h"
#include "afedrisetup.h"
#include "networksetup.h"
#include "ui_sdrdialog.h"

class SDRDialog : public QDialog, public Ui::SDRDialog
{
Q_OBJECT

public:
    SDRDialog(QSettings &s,QWidget *parent = 0);
    ~SDRDialog();
    SoundCardSetup     *soundcard;

signals:
    void setupErrors(const QString &);
    void update();
    void restartSdr();

public slots:
    void updateSDR();
    void rejectChanges();

private slots:
    void launchConfigure();

private:
    QSettings          &settings;
    AfedriSetup        *afedri;
    NetworkSetup       *network;
    void updateFromSettings();
};
#endif
