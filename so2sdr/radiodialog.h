/*! Copyright 2010-2025 R. Torsten Clay N4OGW

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
#ifndef RADIODIALOG_H
#define RADIODIALOG_H

#include "defines.h"
#include "serial.h"
#include "ui_radiodialog.h"
#include <QFile>
#include <QSettings>

class QComboBox;
class QLineEdit;

/*!
   Radio serial communication parameters
 */
class RadioDialog : public QDialog, public Ui::RadioDialog {
  Q_OBJECT

public:
  RadioDialog(QSettings &s, RigSerial &cat, uiSize sizes,
              QWidget *parent = nullptr);
  ~RadioDialog();

signals:
  void startRadios();

public slots:
  void rejectChanges();
  void updateRadio();

private slots:
  void populateModels1(int);
  void populateModels2(int);
  void rigctld1Checked(bool);
  void rigctld2Checked(bool);

private:
  void populateModelCombo(int, int);
  void updateFromSettings();

  QComboBox *radioBaudComboBox[NRIG];
  QComboBox *radioMfgComboBox[NRIG];
  QComboBox *radioModelComboBox[NRIG];
  QComboBox *radioPttComboBox[NRIG];
  QLineEdit *radioDevEdit[NRIG];
  QLineEdit *radioPollTimeEdit[NRIG];
  QLineEdit *radioIFEdit[NRIG];
  QSettings &settings;
  RigSerial &catptr;
};

#endif
