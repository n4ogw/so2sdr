/*! Copyright 2010-2024 R. Torsten Clay N4OGW

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
#ifndef CONTESTOPTDIALOG_H
#define CONTESTOPTDIALOG_H

#include "defines.h"
#include "ui_contestoptdialog.h"
#include <QSettings>
#include <QString>

/*!
   Dialog for setting misc program options
 */
class ContestOptionsDialog : public QDialog, public Ui::ContestOptDialog {
  Q_OBJECT

public:
  friend class So2sdr;
  explicit ContestOptionsDialog(uiSize sizes, QWidget *parent = nullptr);
  ~ContestOptionsDialog();
  void initialize(QSettings *s);

public slots:
  void updateOptions();
  void rejectChanges();
signals:
  void rescore();
  void multiModeChanged();
  void updateOffTime();

private:
  void setOptions();
  QIntValidator *offValidator;
  QSettings *settings;
  QLineEdit *sent[MAX_EXCH_FIELDS];
  QLabel *sentName[MAX_EXCH_FIELDS];
};

#endif
