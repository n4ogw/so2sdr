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
#ifndef CWDIALOG_H
#define CWDIALOG_H

#include <QSettings>
#include "ui_cwdialog.h"
#include "cwmanager.h"
#include "winkey.h"

class QString;

/*!
   Dialog for winkey parameters
 */
class CWDialog : public QDialog, public Ui::CwDialog
{
Q_OBJECT

public:
    CWDialog(QSettings& s,QWidget *parent = nullptr);
    ~CWDialog();
signals:
    void setType(cwtype);
    void startCw();

public slots:
    void setWinkeyVersionLabel(int version);
    void rejectChanges();
    void updateCW();

private:
    QSettings& settings;
};


#endif
