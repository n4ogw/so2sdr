/*! Copyright 2010-2013 R. Torsten Clay N4OGW

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
#ifndef LINUX_PP_H
#define LINUX_PP_H
#include <sys/io.h>
#include <unistd.h>
#include <stdio.h>
#include "defines.h"
#include <QSettings>
#include <QString>

/*! Parallel port access under Linux

   bit to pin mapping
                   Bit:     7   6   5   4   3   2   1   0

   Base (Data port)     Pin:    9   8   7   6   5   4   3   2

   Base+1 (Status port)     Pin:    ~11     10  12  13  15

   Base+2 (Control port)    Pin:                ~17     16  ~14     ~1

 */
class ParallelPort : public QObject
{
Q_OBJECT

public:
    ParallelPort(QSettings& s);
    ~ParallelPort();
    void switchAudio(int r);
    void toggleStereoPin();   
    void switchTransmit(int r);

public slots:
    void initialize();

signals:
    void parallelPortError(const QString &);

private:
    bool    initialized;
    bool stereoPinStatus;

    int     parallelFD;

    void PinLow(const int p);
    void PinHigh(const int p);
    QSettings& settings;
};
#endif
