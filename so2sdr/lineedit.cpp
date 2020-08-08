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
#include "lineedit.h"
#include <QCoreApplication>

LineEdit::LineEdit(QWidget *parent=nullptr) :QLineEdit(parent)
{
    myFocus=false;
    installEventFilter(this);
}

void LineEdit::setMyFocus(bool b)
{
    myFocus=b;
    if (b) {
        QCoreApplication::postEvent(this,new QEvent(QEvent::FocusIn));
    } else {
        QCoreApplication::postEvent(this,new QEvent(QEvent::FocusOut));
    }
}

bool LineEdit::eventFilter(QObject *o, QEvent *e)
{
    if (myFocus && e->type() == QEvent::FocusOut) {
        return true;
    }
    else return QLineEdit::eventFilter(o,e);
}
