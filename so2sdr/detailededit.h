/*! Copyright 2010-2023 R. Torsten Clay N4OGW

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

#ifndef DETAILEDEDIT_H
#define DETAILEDEDIT_H

#include "serial.h"
#include "ui_detailededit.h"
#include <QSqlRecord>
#include <QString>

/*!
   Dialog for detailed qso editing
 */
class DetailedEdit : public QDialog, public Ui::DetailedQSOEdit {
  Q_OBJECT

public:
  explicit DetailedEdit(uiSize sizes, QWidget *parent = nullptr);
  void loadRecord(const QSqlRecord &r, int nexchange);
private slots:
  void updateRecord();
signals:
  void editedRecord(QSqlRecord &);

private:
  QSqlRecord rec;
};

#endif
