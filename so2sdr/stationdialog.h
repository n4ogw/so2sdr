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
#ifndef STATIONDIALOG_H
#define STATIONDIALOG_H

#include <QString>
#include <QDialog>
#include <QCloseEvent>
#include <QSettings>
#include "ui_stationdialog.h"
#include "utils.h"
/*!
   Station parameters dialog
 */
class StationDialog : public QDialog, public Ui::StationDialog
{
Q_OBJECT

public:
    StationDialog(QSettings& s,QWidget *parent = 0);
    ~StationDialog();
    friend class So2sdr;

    double lat() const;
    double lon() const;

public slots:
    void updateStation();
    void rejectChanges();

signals:
    void stationUpdate();

private:
    double         Lat;
    double         Lon;
    QSettings&      settings;
};

#endif
