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

#include <portaudio.h>
#include <QList>
#include <QSettings>
#include <QString>
#include "defines.h"
#include "ui_sdrdialog.h"

class QComboBox;
class QCheckBox;

/*!
   Visible bandmap parameters
 */
class SDRDialog : public QDialog, public Ui::SDRDialog
{
Q_OBJECT

public:
    SDRDialog(QSettings& s,QWidget *parent = 0);
    ~SDRDialog();
    PaStreamParameters &format(int nrig);

signals:
    void updateCQLimits();
    void updateDVK();

public slots:
    void updateSDR();
    void rejectChanges();

private slots:
    void launchUpdateDeviceList0(int);
    void launchUpdateDeviceList1(int);
    void launchUpdateDeviceList2(int);

private:
    int                nAPI;
    int                *nApiDevices;
    PaStreamParameters Format[NRIG];
    QCheckBox          *Checkbox[NRIG+1];
    QComboBox          *APICombo[NRIG+1]; // extra for DVK
    QComboBox          *BitsCombo[NRIG];
    QComboBox          *DeviceCombo[NRIG+1]; // extra for DVK
    QIcon              iconNOK;
    QIcon              iconOK;
    QLineEdit          *OffsetLineEditPtr[NRIG];
    QList<bool>        *deviceOK;
    QList<QString>     audioDevices;
    QList<QString>     *nApiDeviceNames;
    QSettings&           settings;
    void updateDeviceList(int, int);
    void updateFromSettings();
};
#endif
