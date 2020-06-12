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
#ifndef SO2RDIALOG_H
#define SO2RDIALOG_H

#include "ui_so2rdialog.h"
#include "defines.h"
#include "serial.h"
#include <QFile>
#include <QSettings>

class QComboBox;
class QLineEdit;

/*!
   Radio serial communication parameters
 */
class So2rDialog : public QDialog, public Ui::So2rDialog
{
Q_OBJECT

public:
    So2rDialog(QSettings& s,uiSize sizes,QWidget *parent = nullptr);
    ~So2rDialog();

signals:
    void setParallelPort();
    void setOTRSP();
    void setMicroHam();

public slots:
    void rejectChanges();
    void updateSo2r();
    void setOtrspName(QByteArray name,int nr);

private:
    void updateFromSettings();
    QSettings&  settings;
};

#endif
