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

#ifndef LOGDELEGATE_H
#define LOGDELEGATE_H
#include "contest.h"
#include <QAbstractItemDelegate>
#include <QEvent>
#include <QList>
#include <QModelIndex>
#include <QObject>
#include <QSqlRecord>
#include <QStyleOptionViewItem>
#include <QStyledItemDelegate>
#include <QWidget>

/*!
  subclass of delegate for displaying log in main window
  */
class logDelegate : public QStyledItemDelegate {
  Q_OBJECT

public:
  logDelegate(QObject *parent, Contest &c, bool *e, QList<int> *l);
  bool currentlyEditing;
  mutable QWidget *currentEditor;

signals:
  void startLogEdit();
  void logUpdate(QModelIndex);
  void editLogRow(QModelIndex);
  void editLogRowDetail(QModelIndex);
  void setOrigRecord(QModelIndex);
  void update();

protected:
  void paint(QPainter *painter, const QStyleOptionViewItem &option,
             const QModelIndex &index) const;
  virtual QWidget *createEditor(QWidget *parent,
                                const QStyleOptionViewItem &option,
                                const QModelIndex &index) const;
  bool editorEvent(QEvent *e, QAbstractItemModel *m,
                   const QStyleOptionViewItem &option,
                   const QModelIndex &index);
  bool eventFilter(QObject *obj, QEvent *event);

public slots:
  void startDetailedEdit();

private slots:
  void closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint =
                                        QAbstractItemDelegate::NoHint);

private:
  Contest &contest;
  bool *logSearchFlag;
  QList<int> *searchList;
  QModelIndex currentlyEditingIndex;
};

#endif // LOGDELEGATE_H
