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
#ifndef CABRILLODIALOG_H
#define CABRILLODIALOG_H
#include "ui_cabrillo.h"
#include "defines.h"
#include <QSettings>

class QComboBox;
class QFile;
class QLabel;
class QLineEdit;
class So2sdr;

/*!
   Dialog for entering cabrillo data
 */
class CabrilloDialog : public QDialog, public Ui::CabrilloDialog
{
Q_OBJECT

public:
    explicit CabrilloDialog(QWidget *parent = 0);
    void initialize(QSettings *s1,QSettings *s2);
    void updateExch();
    void writeHeader(QFile *cbrFile,int score);
    friend class So2sdr;

private:
    QLineEdit *sent[MAX_EXCH_FIELDS];
    QLabel *cabLabel[MAX_CAB_FIELDS];
    QComboBox *cabCombo[MAX_CAB_FIELDS];
    QSettings  *stnSettings;
    QSettings *settings;
};

#endif
