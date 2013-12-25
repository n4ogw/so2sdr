/*! Copyright 2010-2014 R. Torsten Clay N4OGW

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
#include <QFileDialog>
#include "newcontestdialog.h"

NewDialog::NewDialog(QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    newContestIndx = 0;
}

/*!
   Return index number of selected contest
 */
int NewDialog::newIndx()
{
    newContestIndx = NewContestComboBox->currentIndex();
    return(newContestIndx);
}

/*!
   Add a contest to list

   These are read from config file contest_list.dat by the main program
 */
void NewDialog::addContest(QByteArray name)
{
    NewContestComboBox->insertItem(newContestIndx++, name);
}
