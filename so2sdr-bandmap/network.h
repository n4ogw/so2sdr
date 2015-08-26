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

#ifndef NETWORK_H
#define NETWORK_H
#include <QObject>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QAbstractSocket>
#include <sdrdatasource.h>
#include "sdr-ip.h"

class NetworkSDR : public SdrDataSource
{
    Q_OBJECT
public:
    NetworkSDR(QString settingsFile,QObject *parent = 0);
    ~NetworkSDR();

public slots:
    void stop();
    void initialize();

private slots:
    void readDatagram();
    void tcpError(QAbstractSocket::SocketError err);
    void readTcp();

protected:
    void send_rx_command(int);
    void close_udp();
    void get_name();

    QTcpSocket tsocket;
    QUdpSocket usocket;
    unsigned char      *buff;
    unsigned int       bpmax;
    unsigned int       bptr;
    unsigned int       iptr;
};

#endif
