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
#ifndef DUPESHEET_H
#define DUPESHEET_H
#include "ui_dupesheet.h"

/*!
   Class for visible dupesheet
 */
class DupeSheet : public QDialog, public Ui::Dupesheet
{
Q_OBJECT

public:
    explicit DupeSheet(QWidget *parent = 0);
    ~DupeSheet();

protected:
    void keyPressEvent(QKeyEvent *event);
};
#endif
