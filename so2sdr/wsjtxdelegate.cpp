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
#include "wsjtxdelegate.h"
#include "udpreader.h"
#include <QBrush>
#include <QEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include <QPainter>
#include <QPalette>
#include <QRect>
#include <QStyle>

/*! wsjtxDelegate:

   Controls display of information in wsjtx call list
 */
wsjtxDelegate::wsjtxDelegate() : QStyledItemDelegate() {}

/*! paints data into columns of wsjtx call list
 */
void wsjtxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const {
  bool dupe = index.model()
                  ->data(index.model()->index(index.row(), WSJTX_SQL_COL_DUPE))
                  .toBool();
  bool mult = index.model()
                  ->data(index.model()->index(index.row(), WSJTX_SQL_COL_MULT))
                  .toBool();

  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);
  QString s = index.model()->data(index).toString();

  // draw correct background
  opt.text = "";
  QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
  style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);

  QRect rect = opt.rect;
  QPalette::ColorGroup cg =
      opt.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;
  if (cg == QPalette::Normal && !(opt.state & QStyle::State_Active)) {
    cg = QPalette::Inactive;
  }

  // set pen color
  if (opt.state & QStyle::State_Selected) {
    painter->setPen(opt.palette.color(cg, QPalette::HighlightedText));
  } else {
    if (dupe) {
      QBrush brush;
      brush.setColor(DUPE_COLOR);
      brush.setStyle(Qt::SolidPattern);
      painter->fillRect(opt.rect, brush);
    } else if (mult) {
      QBrush brush;
      brush.setColor(Qt::cyan);
      brush.setStyle(Qt::SolidPattern);
      painter->fillRect(opt.rect, brush);
    }
  }
  painter->drawText(QRect(rect.left(), rect.top(), rect.width(), rect.height()),
                    opt.displayAlignment, s);
}
