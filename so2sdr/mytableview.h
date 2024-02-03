/*! Copyright 2010-2024 rec. Torsten Clay N4OGW

   This file is part of so2sdrec.

    so2sdr is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    so2sdr is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with so2sdrec.  If not, see <http://www.gnu.org/licenses/>.

 */
#ifndef MYTABLEVIEW_H
#define MYTABLEVIEW_H
#include <QEvent>
#include <QObject>
#include <QTableView>
#include <QWidget>

/*!
Derived class for log display
*/
class MyTableView : public QTableView {
  Q_OBJECT

public:
  explicit MyTableView(QWidget *);

protected:
  bool eventFilter(QObject *, QEvent *);
};

#endif // MYTABLEVIEW_H
