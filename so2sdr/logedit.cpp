/*! Copyright 2010-2017 R. Torsten Clay N4OGW

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
 *    logDelegate : subclass of QStyledItemDelegate
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
Qt::ItemFlags tableModel::flags ( const QModelIndex & index ) const
{
    Qt::ItemFlags f;
    f=Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    // set flags appropriate for each column
    // this defines which columns are editable
    switch (index.column()) {
    case SQL_COL_NR:
        f=Qt::NoItemFlags;
        break;
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

/*! logDelegate:

   Controls display of sent exchange, received exchange, and callsign in displayed log. Changes color
   of entries that are new multipliers

   needs pointer to contest object to get score/multiplier information
 */
logDelegate::logDelegate(QObject *parent, const Contest *c, bool *e, QList<int> *l) : QStyledItemDelegate(parent)
{
    contest = c;
    logSearchFlag = e;
    searchList = l;
}

/*!
  called when a log item is edited. When this happens, emit a signal.
 */
bool logDelegate::editorEvent(QEvent *e, QAbstractItemModel *m, const QStyleOptionViewItem & option, const QModelIndex & index )
{
    Q_UNUSED(m)
    Q_UNUSED(option)
    Q_UNUSED(index)

    int col=index.column();
    // this is for "inline" editing of a single cell
    if (e->type()==QEvent::MouseButtonDblClick && col!=SQL_COL_VALID) {
        currentlyEditingIndex=index;
        emit(startLogEdit());
        return false;
    }
    // handle clicks in valid column
    if (e->type() == QEvent::MouseButtonRelease  && col==SQL_COL_VALID) {
        // make sure that we have a check state
        QVariant value = index.data(Qt::CheckStateRole);
        if (!value.isValid()) {
            return false;
        }
        bool b=index.data(Qt::CheckStateRole).toBool();
        if (b) {
            m->setData(index,QVariant(Qt::Unchecked),Qt::CheckStateRole);
        } else {
            m->setData(index,QVariant(Qt::Checked),Qt::CheckStateRole);
        }
    }
    // this is needed before control-E is pressed
    if (e->type()==QEvent::MouseButtonPress) {
        currentlyEditingIndex=index;
        return false;
    }
    if (e->type()==QEvent::KeyPress) {
        // return true to prevent default column search when typing letter
        return true;
    }
    return false;
}

/*!
  creates editors for each column.

  certain columns (id #, freq, qso pts) are not editable, return NULL in these cases
  other columns use a QLineEdit restricted to upper case
  */
QWidget* logDelegate::createEditor ( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    // no editor for checkbox
    if (index.column()==SQL_COL_VALID) return 0;

    QLineEdit *le=new LogQLineEdit(parent);
    le->installEventFilter(le);
    if (index.column()==SQL_COL_TIME) {
        // validator for time column
        le->setValidator(new TimeValidator(le));
    } else {
        // edit in upper case
        le->setValidator(new UpperValidator(le));
    }
    return(le);
}

/*!
 Start detailed qso edit if a row has been highlighted
 */
void logDelegate::startDetailedEdit()
{
    if (currentlyEditingIndex.isValid())
        emit(editLogRow(currentlyEditingIndex));
}

/*! paints data from log into exchange columns on screen.

   Columns that are a new multiplier are highlighted in red.
   Use the points from contest->score rather than in the SQL log
 */
void logDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // for qso valid column draw a checkbox
    if (index.column()==SQL_COL_VALID) {
        bool data=index.data(Qt::CheckStateRole).toBool();
        QStyleOptionButton checkboxstyle;
        QRect checkbox_rect = QApplication::style()->subElementRect(QStyle::SE_CheckBoxIndicator, &checkboxstyle);
        checkboxstyle.rect = option.rect;
        checkboxstyle.rect.setLeft(option.rect.x() + option.rect.width()/2 - checkbox_rect.width()/2);
        if (data) {
            checkboxstyle.state = QStyle::State_On|QStyle::State_Enabled;
        } else {
            checkboxstyle.state = QStyle::State_Off|QStyle::State_Enabled;
        }
        QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkboxstyle, painter);
        return;
    }
    // get the real row for this qso. When a log search is performed, index.row() gives the
    // row within the restricted filter, and not the true row. The true row is needed to
    // display newmult and valid status
    int realRow;
    if (*logSearchFlag) {
        realRow=(*searchList).at(index.row());
    } else {
        realRow=index.row();
    }

    QStyleOptionViewItemV4 opt = option;
    initStyleOption(&opt, index);
    QString s         = index.model()->data(index).toString();

    // display frequency in KHz
    // (this is the reason editing the frequency column is a problem)
    if (index.column()==SQL_COL_FREQ) {
        int f_khz=index.model()->data(index).toInt();
        f_khz = qRound(f_khz / 1000.0);
        s = QString::number(f_khz, 10);
    }

    if (index.column() == SQL_COL_MODE) {
        rmode_t m = (rmode_t)index.model()->data(index).toInt();

        switch(m) {
        case RIG_MODE_CW:
            s = "CW";
            break;
        case RIG_MODE_CWR:
            s = "CWR";
            break;
        case RIG_MODE_LSB:
            s = "LSB";
            break;
        case RIG_MODE_USB:
            s = "USB";
            break;
        case RIG_MODE_FM:
            s = "FM";
            break;
        case RIG_MODE_AM:
            s = "AM";
            break;
        default:
            break;
        }
    }

    // 0 = regular text
    // 1 = red (new multiplier)
    // 2 = grey (dupe)
    int    highlight = 0;

    // get qso points from contest object instead of sql database
    if (index.column() == SQL_COL_PTS) {
        s = QString::number(contest->points(realRow));
    }

    // check to see if a column is a new multiplier and needs highlighting
    if ((contest->newMult(realRow, 0)) == index.column()) {
        highlight = 1;
    }
    if ((contest->newMult(realRow, 1)) == index.column()) {
        highlight = 1;
    }

    // dupes are grayed out
    if (contest->dupe(realRow) || !contest->valid(realRow)) highlight = 2;

    // draw correct background
    opt.text = "";
    QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);

    QRect                rect = opt.rect;
    QPalette::ColorGroup cg   = opt.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;
    if (cg == QPalette::Normal && !(opt.state & QStyle::State_Active)) {
        cg = QPalette::Inactive;
    }

    // set pen color
    if (opt.state & QStyle::State_Selected) {
        painter->setPen(opt.palette.color(cg, QPalette::HighlightedText));
    } else {
        switch (highlight) {
        case 1:
            painter->setPen(Qt::red); // new multiplier: red
            break;
        case 2:
            painter->setPen(Qt::lightGray); // dupes and invalids: light grey
            break;
        default:
            painter->setPen(opt.palette.color(cg, QPalette::Text)); // regular text: black
            break;
        }
    }

    // draw text
    painter->drawText(QRect(rect.left(), rect.top(), rect.width(), rect.height()), opt.displayAlignment, s);
}


/*! event filter for log display delegate
 */
bool logDelegate::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type()==QEvent::KeyPress) {
        QKeyEvent* kev = static_cast<QKeyEvent*>(event);
        if (kev->key()==Qt::Key_Escape) {
            return false;
        }
    }
    return QStyledItemDelegate::eventFilter(obj, event);
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
            emit(editingFinished());
            break;
        }
    }
    return QLineEdit::eventFilter(obj,event);
}


