/*! Copyright 2010-2012 R. Torsten Clay N4OGW

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
#ifndef CWMESSAGEDIALOG_H
#define CWMESSAGEDIALOG_H

#include <QByteArray>
#include <QSettings>
#include <QString>
#include "ui_cwmessagedialog.h"
#include "utils.h"

class QLineEdit;
class So2sdr;

/*!
   Dialog for entering CW messages
 */
class CWMessageDialog : public QDialog, public Ui::CWMessageDialog
{
Q_OBJECT

public:
    CWMessageDialog(QWidget *parent = 0);
    ~CWMessageDialog();
    void initialize(QSettings *s);
    friend class So2sdr;

public slots:
    void updateCWMsg();
    void rejectChanges();

private:
    QByteArray     cqCtrlF[12];
    QByteArray     cqF[12];
    QByteArray     cqShiftF[12];
    QByteArray     excF[12];
    QLineEdit      *ctrlFuncEditPtr[12];
    QLineEdit      *excFuncEditPtr[12];
    QLineEdit      *funcEditPtr[12];
    QLineEdit      *shiftFuncEditPtr[12];
    UpperValidator *upperValidate;
    QSettings      *settings;
};

#endif
