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
#include "udpreader.h"
#include <QDebug>
#include <QString>
#include "utils.h"

UDPReader::UDPReader(QSettings &cs,QObject *parent=0) : QObject(parent),settings(cs)
{
    qRegisterMetaType<QAbstractSocket::SocketError>("socketerror");
    connect(&usocket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(tcpError(QAbstractSocket::SocketError)));
    isOpen=false;
    parser=new ADIFParse();
}

UDPReader::~UDPReader()
{
    delete parser;
}

void UDPReader::stop()
{
    usocket.close();
    if (usocket.state() != QAbstractSocket::UnconnectedState) {
        usocket.waitForDisconnected(1000);
    }
    isOpen=false;
}

void UDPReader::enable(bool b)
{
    if (b) {
        if (usocket.isOpen()) {
            usocket.close();
            if (usocket.state() != QAbstractSocket::UnconnectedState) {
                usocket.waitForDisconnected(1000);
            }
        }
        if (!usocket.bind(settings.value(s_wsjtx_udp,s_wsjtx_udp_def).toInt(), QUdpSocket::ShareAddress)) {
            emit(error("UDPsocket: UDP connection to wsjtx failed"));
            isOpen=false;
            return;
        }
        connect(&usocket,SIGNAL(readyRead()),this,SLOT(readDatagram()));
        isOpen=true;
            } else {
        usocket.close();
        if (usocket.state() != QAbstractSocket::UnconnectedState) {
            usocket.waitForDisconnected(1000);
        }
        isOpen=false;
    }
}

void UDPReader::readDatagram()
{
    qint64 udp_size;
    char *data;

    udp_size=usocket.pendingDatagramSize();
    data=new char[udp_size];
    if (usocket.readDatagram((char*)data,udp_size)==-1) {
        emit(error("UDPReader: UDP read failed"));
        delete data;
        return;
    }
    Qso qso;
    QByteArray tmp=data;
    parser->parse(tmp,&qso);
    emit(wsjtxQso(&qso));
    delete[] data;
}

void UDPReader::tcpError(QAbstractSocket::SocketError err)
{
    Q_UNUSED(err)
    QString errstring="UDReader: "+usocket.errorString();
    qDebug("%s",errstring.toLatin1().data());
}
