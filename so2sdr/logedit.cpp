/*! Copyright 2010-2020 R. Torsten Clay N4OGW

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
#include <QKeyEvent>
#include <QSqlRecord>
#include "logedit.h"

/*!
 *Contains several classes used to display/edit log data:
 *    tableModel : subclass of QSqlTableModel
 *    LogQLineEdit : subclass of QLineEdit
 */

/*!
  subclass of QSqlTableModel for log data
  */
tableModel::tableModel(QObject * parent, QSqlDatabase db) : QSqlTableModel(parent,db)
{
    setEditStrategy(QSqlTableModel::OnFieldChange);
}

/*!
  returns appropriate flags for each log column
  */
Qt::ItemFlags tableModel::flags( const QModelIndex & index ) const
{
    Qt::ItemFlags f;
    f=Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    // set flags appropriate for each column
    // this defines which columns are editable
    switch (index.column()) {
    case SQL_COL_NR:
    case SQL_COL_BAND:
    case SQL_COL_DATE:
    case SQL_COL_CALL:
    case SQL_COL_FREQ:
    case SQL_COL_TIME:
    case SQL_COL_SNT1:
    case SQL_COL_SNT2:
    case SQL_COL_SNT3:
    case SQL_COL_SNT4:
    case SQL_COL_RCV1:
    case SQL_COL_RCV2:
    case SQL_COL_RCV3:
    case SQL_COL_RCV4:
    case SQL_COL_MODE:
    case SQL_COL_MODE_TYPE:
    case SQL_COL_ADIF_MODE:
        f=f | Qt::ItemIsEditable;
        break;
    case SQL_COL_VALID:
        f=f | Qt::ItemIsEditable;
        f=f | Qt::ItemIsUserCheckable;
        break;
    }
    return f;
}

/*!
  only SQL_COL_VALID needs a special value here: return a Qt::CheckState
  */
QVariant tableModel::data( const QModelIndex& index, int role ) const
{
    if (index.column()==SQL_COL_VALID) {
        if (role==Qt::CheckStateRole) {
            bool b=record(index.row()).value(SQL_COL_VALID).toBool();
            if (b) {
                return Qt::Checked;
            } else {
                return Qt::Unchecked;
            }
        } else if (role==Qt::DisplayRole) {
            return QVariant();
        }
    }
    return QSqlTableModel::data(index,role);
}

/*!
  only SQL_COL_VALID is a special case: translate CheckState into boolean
  */
bool tableModel::setData( const QModelIndex& index, const QVariant&value, int role )
{
    if (index.column()==SQL_COL_VALID && role == Qt::CheckStateRole ) {
        QVariant newValue;
        if (value.toBool()) {
            newValue=QVariant(true);
        } else {
            newValue=QVariant(false);
        }
        return QSqlTableModel::setData(index,newValue,Qt::EditRole);
    }
    return QSqlTableModel::setData(index,value,role);
}

/*!
 derived line editor class for editing log cells.
   -Escape key handling is different from standard QLineEdit
   -selection (highlighting) of text is removed
 */
LogQLineEdit::LogQLineEdit(QWidget *w) : QLineEdit(w)
{
    connect(this,SIGNAL(selectionChanged()),this,SLOT(fixSelect()));
    undoText.clear();
}

/*!
 ugly hack to get QLineEdit with no selection, save original text in lineedit
 */
void LogQLineEdit::fixSelect()
{
    deselect();
    // this catches the original text set in the editor
    if (undoText.isEmpty()) undoText=text();
}

/*!
 catch ESC key in log editor. If ESC pressed, restore original contents of
 editor AND finish editing. Using standard QLineEdit requires a second ESC
 press to exit editor.
*/
bool LogQLineEdit::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type()==QEvent::KeyPress) {
        QKeyEvent* kev = static_cast<QKeyEvent*>(event);
        switch (kev->key()) {
        case Qt::Key_Escape:
            if (!undoText.isEmpty()) {
                setText(undoText);
            }
            break;
        }
    }
    return QLineEdit::eventFilter(obj,event);
}

