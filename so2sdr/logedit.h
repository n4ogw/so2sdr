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
#ifndef LOGEDIT_H
#define LOGEDIT_H
#include <QApplication>
#include <QEvent>
#include <QLineEdit>
#include <QObject>
#include <QPainter>
#include <QWidget>
#include <QStyle>
#include <QStyledItemDelegate>
#include <QSqlTableModel>
#include <QTableView>
#include "contest.h"
#include "defines.h"
#include "log.h"
#include "utils.h"

/*!
  subclass of delegate for displaying log in main window
  */
class logDelegate : public QStyledItemDelegate
{
Q_OBJECT

public:
    logDelegate(QObject *parent, const Contest *c, bool *e, QList<int> *l);
signals:
    void startLogEdit();
    void editLogRow(QModelIndex);
protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual QWidget *	createEditor ( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
    bool editorEvent(QEvent *e, QAbstractItemModel *m, const QStyleOptionViewItem & option, const QModelIndex & index );
    bool eventFilter(QObject *obj, QEvent *event);
public slots:
    void startDetailedEdit();
private:
    const Contest *contest;
    bool *logSearchFlag;
    QList<int> *searchList;
    QModelIndex currentlyEditingIndex;
};

/*!
  subclass of QSqlTableModel needed to specify flags separately for
  each column, and specify checkbox for qso valid column
  */
class tableModel : public QSqlTableModel
{
    Q_OBJECT

public:
    tableModel(QObject * parent = 0, QSqlDatabase db = QSqlDatabase());

protected:
    virtual Qt::ItemFlags flags ( const QModelIndex & index ) const;
    virtual QVariant data( const QModelIndex& index, int role ) const;
    virtual bool setData( const QModelIndex& index, const QVariant&value, int role );
};


/*!
 derived line editor class for editing log cells
 */
class LogQLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    LogQLineEdit(QWidget *w);

private slots:
    void fixSelect();
protected:
    bool eventFilter(QObject *, QEvent *);
private:
    QString undoText;
};



#endif // LOGEDIT_H
