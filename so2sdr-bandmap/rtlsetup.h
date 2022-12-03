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
#ifndef RTLSETUP_H
#define RTLSETUP_H

#include "defines.h"
#include <QDialog>
#include <QSettings>
#include "bandoffsetsetup.h"
#include "ui_rtlsetup.h"


class RtlSetup : public QDialog, public Ui::rtlSetup
{
    Q_OBJECT
public:
    explicit RtlSetup(QSettings &s, uiSize sizes, QWidget *parent = nullptr);
    ~RtlSetup();
    double offset(int band) const;
    bool invert(int band) const;

signals:
    void rtlError(const QString &);

private slots:
    void updateRtl();
    void rejectChanges();

private:
    QSettings &settings;
    void updateFromSettings();
    BandOffsetSetup *offsetSetup;
};

#endif // RTLSETUP_H
