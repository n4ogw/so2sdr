/*! Copyright 2010-2018 R. Torsten Clay N4OGW

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
#ifndef UDPREADER_H
#define UDPREADER_H

#include <QObject>
#include <QSettings>
#include <QString>
#include <QUdpSocket>
#include "adifparse.h"
#include "defines.h"
#include "qso.h"


// interface for external UDP log broadcasts (WSJTX)
class UDPReader : public QObject
{
    Q_OBJECT
public:
    explicit UDPReader(QSettings& cs,QObject *parent);
    ~UDPReader();

signals:
    void error(const QString &);
    void wsjtxQso(Qso *);

public slots:
    void enable(bool);
    void stop();

private slots:
    void readDatagram();
    void tcpError(QAbstractSocket::SocketError err);

private:
    ADIFParse *parser;
    QUdpSocket usocket;
    bool isOpen;
    QSettings&   settings;
};

#endif // UDPREADER_H
