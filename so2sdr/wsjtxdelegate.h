/*! Copyright 2010-2022 R. Torsten Clay N4OGW

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

#ifndef WSJTXDELEGATE_H
#define WSJTXDELEGATE_H
#include <QAbstractItemDelegate>
#include <QStyledItemDelegate>
#include <QModelIndex>
#include <QStyleOptionViewItem>
#include <QObject>
#include <QWidget>
#include "defines.h"

/*!
  subclass of delegate for displaying log in main window
  */
class wsjtxDelegate  : public QStyledItemDelegate
{
Q_OBJECT

public:
    wsjtxDelegate();

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

};


#endif // WSJTXDELEGATE_H

