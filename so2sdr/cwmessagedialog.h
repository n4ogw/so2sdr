/*! Copyright 2010-2021 R. Torsten Clay N4OGW

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
#include "defines.h"
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
    CWMessageDialog(ModeTypes modetype,uiSize sizes,QWidget *parent = nullptr);
    ~CWMessageDialog();
    void initialize(QSettings *s);
    friend class So2sdr;

public slots:
    void updateCWMsg();
    void rejectChanges();

private:
    int            m;
    QByteArray     cqCtrlF[N_FUNC];
    QByteArray     cqF[N_FUNC];
    QByteArray     cqShiftF[N_FUNC];
    QByteArray     excF[N_FUNC];
    QLineEdit      *ctrlFuncEditPtr[N_FUNC];
    QLineEdit      *excFuncEditPtr[N_FUNC];
    QLineEdit      *funcEditPtr[N_FUNC];
    QLineEdit      *shiftFuncEditPtr[N_FUNC];
    UpperValidator *upperValidate;
    QSettings      *settings;
};

#endif
