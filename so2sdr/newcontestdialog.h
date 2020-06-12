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
#ifndef NEWCONTESTDIALOG_H
#define NEWCONTESTDIALOG_H

#include <QByteArray>
#include <QList>
#include <QString>
#include "ui_newcontestdialog.h"

/*!
   New contest selection dialog
 */
class NewDialog : public QDialog, public Ui::NewContestDialog
{
Q_OBJECT

public:
    NewDialog(QWidget *parent = nullptr);
    bool readContestList(QString fileName);
    QString selectedContest();
signals:
    void newContestError(const QString &);

private:
    QList<QByteArray>    configFiles;
};

#endif
