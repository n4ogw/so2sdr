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

#include <QLabel>
#include <QSettings>
#include "defines.h"
#include "ui_sdrdialog.h"

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

signals:
    void updateCQLimits();

public slots:
    void updateSDR();
    void rejectChanges();

private slots:
    void findExeFile1();
    void findExeFile2();
    void findConfig1();
    void findConfig2();

private:
    void fileGetter(QString msg,QString path,QString files,QString key,QLabel *label);
    QString shortName(QString s);
    void updateFromSettings();
    QLabel             *pathLabel[NRIG];
    QLineEdit          *ipPtr[NRIG];
    QLineEdit          *portPtr[NRIG];
    QLabel             *configLabel[NRIG];
    QSettings&         settings;
};
#endif
