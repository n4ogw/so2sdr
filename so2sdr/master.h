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
#ifndef MASTER_H
#define MASTER_H

#include <QByteArray>
#include <QFile>
#include <QString>

/*!
   Class for supercheck partial lookups (MASTER.DTA)

   based on code from Alex Shovkoplyas VE3NEA
 */
class Master : public QObject
{
Q_OBJECT

public:
    Master();
    ~Master();
    void initialize(QFile &file);
    void search(QByteArray partial, QByteArray &CallList);

signals:
    void masterError(const QString &);

private:
    bool       initialized;
    char       *CallData;
    int        *index;
    int        indexBytes;
    int        indexSize;
    int        nchars;
    QByteArray chars;
    qint64     fileSize;
};

#endif // MASTER_H
