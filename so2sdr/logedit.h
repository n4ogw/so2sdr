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
#ifndef LOGEDIT_H
#define LOGEDIT_H

#include "contest.h"
#include "defines.h"
#include "utils.h"
#include <QApplication>
#include <QEvent>
#include <QLineEdit>
#include <QObject>
#include <QPainter>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QStyle>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QWidget>

/*!
  subclass of QSqlTableModel needed to specify flags separately for
  each column, and specify checkbox for qso valid column
  */
class tableModel : public QSqlTableModel {
  Q_OBJECT

public:
  tableModel(QObject *parent = nullptr, QSqlDatabase db = QSqlDatabase());

protected:
  virtual Qt::ItemFlags flags(const QModelIndex &index) const;
  virtual QVariant data(const QModelIndex &index, int role) const;
  virtual bool setData(const QModelIndex &index, const QVariant &value,
                       int role);
};

/*!
 derived line editor class for editing log cells
 */
class LogQLineEdit : public QLineEdit {
  Q_OBJECT

public:
  explicit LogQLineEdit(QWidget *w);

private slots:
  void fixSelect();

protected:
  bool eventFilter(QObject *, QEvent *);

private:
  QString undoText;
};

#endif // LOGEDIT_H
