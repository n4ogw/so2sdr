/*! Copyright 2010-2019 R. Torsten Clay N4OGW

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

#ifndef HISTORY_H
#define HISTORY_H

#include <QByteArray>
#include <QObject>
#include <QSqlDatabase>
#include <QSettings>
#include <QSqlRecord>
#include "qso.h"

/*!
 exchange history database
 */
class History : public QObject
{
    Q_OBJECT
public:
    explicit History(QSettings& csettings,QObject *parent = 0);
    ~History();
    void fillExchange(Qso *qso,QByteArray part);
    void startHistory();
    void stopHistory();
    bool isOpen();

signals:
    void message(const QString &,int);

public slots:
    void addQso(const Qso *qso);

private:
    QSqlDatabase         history;
    QSettings&            csettings;

};

#endif // HISTORY_H
