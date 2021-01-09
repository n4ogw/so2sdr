/*! Copyright 2010-2021 R. Torsten Clay N4OGW

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
#ifndef CENTERGRIDDIALOG_H
#define CENTERGRIDDIALOG_H

#include <QByteArray>
#include <QDialog>
#include <QWidget>
#include "ui_centergrid.h"

class CenterGridDialog : public QDialog, public Ui::CenterGridForm
{
    Q_OBJECT
public:
    explicit CenterGridDialog(QWidget *parent = nullptr);
    void setText(QByteArray s);
signals:
    void grid(QByteArray);

private slots:
    void emitgrid();
};

#endif // CENTERGRIDDIALOG_H
