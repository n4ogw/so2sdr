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
#ifndef SDRDATASOURCE_H
#define SDRDATASOURCE_H

#include <QMutex>
#include <QObject>
#include <QString>
#include <QSettings>
#include "defines.h"

class SdrDataSource : public QObject
{
    Q_OBJECT
public:
    explicit SdrDataSource(QString settingsFile,QObject *parent = 0);
    ~SdrDataSource();
    void setSampleSizes(sampleSizes s);
    bool isRunning();

signals:
    void error(const QString &);
    void ready(unsigned char *, unsigned char);
    void stopped();

public slots:
    virtual void stop() = 0;
    virtual void initialize() = 0;

protected:
    QMutex      mutex;
    sampleSizes sizes;
    QSettings   *settings;
    bool        running;
    bool        initialized;
};

#endif // SDRDATASOURCE_H
