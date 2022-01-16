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
#ifndef LINEEDIT_H
#define LINEEDIT_H
#include "defines.h"
#include <QEvent>
#include <QLineEdit>
#include <QObject>
#include <QWidget>

/* modified QLinedEdit for the callsign and exchange entry windows.
 * modifications allow controlling focus while in two keyboard mode
 */

class LineEdit : public QLineEdit
{
public:
    LineEdit(QWidget *parent);
    void setMyFocus(bool b);

protected:
    bool eventFilter(QObject*, QEvent* e);

private:
    bool myFocus;
};

#endif // LINEEDIT_H
