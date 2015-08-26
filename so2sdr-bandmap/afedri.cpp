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

#include "afedri.h"
#include "afedri-cmd.h"
#include "utils.h"
#include <QDebug>
#include <QHostAddress>
#ifdef Q_OS_LINUX
#include <unistd.h>
#include <sys/socket.h>
#endif

Afedri::Afedri(QString settingsFile, QObject *parent) : NetworkSDR(settingsFile,parent)
{

}

/*! initialize and start sdr
 */
void Afedri::initialize()
{
    bpmax           = sizes.chunk_size / sizes.advance_size;
    if (buff) {
        delete [] buff;
    }
    buff = new unsigned char[sizes.chunk_size];
    for (unsigned long i = 0; i < sizes.chunk_size; i++) {
        buff[i] = 0;
    }
    if (settings->value(s_sdr_afedri_bcast,s_sdr_afedri_bcast_def).toInt()==0)
    {
        // non-broadcast connection
        tsocket.connectToHost(settings->value(s_sdr_afedri_tcp_ip,s_sdr_afedri_tcp_ip_def).toString(),
                              settings->value(s_sdr_afedri_tcp_port,s_sdr_afedri_tcp_port_def).toInt());
        tsocket.setSocketOption(QAbstractSocket::KeepAliveOption,QVariant(1));
        connect(&tsocket,SIGNAL(readyRead()),this,SLOT(readTcp()));
        if (tsocket.waitForConnected()) {
            set_broadcast_flag(false);
            if (!usocket.bind(settings->value(s_sdr_afedri_udp_port,s_sdr_afedri_udp_port_def).toInt(),
                              QUdpSocket::ShareAddress)) {
                emit(error("Afedri: UDP connection failed"));
                mutex.lock();
                initialized=false;
                running=false;
                mutex.unlock();
                return;
            }
            connect(&usocket, SIGNAL(readyRead()),this, SLOT(readDatagram()));
            set_sample_rate(settings->value(s_sdr_afedri_sample_freq,s_sdr_afedri_sample_freq_def).toInt());
            set_multichannel_mode(settings->value(s_sdr_afedri_multi,s_sdr_afedri_multi_def).toInt());
            send_rx_command(RCV_START);
            set_freq(settings->value(s_sdr_afedri_freq1,s_sdr_afedri_freq1_def).toUInt(),1);
            set_freq(settings->value(s_sdr_afedri_freq2,s_sdr_afedri_freq2_def).toUInt(),2);
            mutex.lock();
            running=true;
            initialized=true;
            mutex.unlock();
        }
    } else if (settings->value(s_sdr_afedri_bcast,s_sdr_afedri_bcast_def).toInt()==1) {
        // broadcast connection, master
        tsocket.connectToHost(settings->value(s_sdr_afedri_tcp_ip,s_sdr_afedri_tcp_ip_def).toString(),
                              settings->value(s_sdr_afedri_tcp_port,s_sdr_afedri_tcp_port_def).toInt());
        tsocket.setSocketOption(QAbstractSocket::KeepAliveOption,QVariant(1));
        connect(&tsocket,SIGNAL(readyRead()),this,SLOT(readTcp()));
        if (tsocket.waitForConnected()) {
            set_sample_rate(settings->value(s_sdr_afedri_sample_freq,s_sdr_afedri_sample_freq_def).toInt());
            set_multichannel_mode(settings->value(s_sdr_afedri_multi,s_sdr_afedri_multi_def).toInt());
            set_broadcast_flag(true);
            connect(&usocket, SIGNAL(readyRead()),this, SLOT(readDatagram()));
#ifdef Q_OS_LINUX
            /* Qt bug:
             * on linux, multiple connections to same UDP port fail.
             *
             * workaround: create a socket using OS calls and set SO_REUSEADDR.
             *
             * see https://bugreports.qt.io/browse/QTBUG-33419
             */
            int socket_descriptor;
            if ((socket_descriptor = ::socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
                qDebug() << "Afedri: error creating datagram socket";
                return;
            }
            usocket.setSocketDescriptor(socket_descriptor,QAbstractSocket::UnconnectedState);
            int reuse = 1;
            if (::setsockopt(usocket.socketDescriptor(), SOL_SOCKET, SO_REUSEADDR,
                    &reuse, sizeof(reuse)) < 0) {
                qDebug() << "Afedri: error setting SO_REUSEADDR";
                return;
            }

            if (!usocket.bind(settings->value(s_sdr_afedri_udp_port,s_sdr_afedri_udp_port_def).toInt())) {
                emit(error("Afedri: UDP connection failed"));
                mutex.lock();
                initialized=false;
                running=false;
                mutex.unlock();
                return;
            }
#endif
#ifdef Q_OS_WIN
            if (!usocket.bind(settings->value(s_sdr_afedri_udp_port,s_sdr_afedri_udp_port_def).toInt()),
                    QUdpSocket::ReuseAddressHint) {
                emit(error("Afedri: UDP connection failed"));
                mutex.lock();
                initialized=false;
                running=false;
                mutex.unlock();
                return;
            }
#endif
            send_rx_command(RCV_START);

            delay(200);  // this might not be needed

            set_freq(settings->value(s_sdr_afedri_freq1,s_sdr_afedri_freq1_def).toULongLong(),1);
            set_freq(settings->value(s_sdr_afedri_freq2,s_sdr_afedri_freq2_def).toULongLong(),2);
            mutex.lock();
            running=true;
            initialized=true;
            mutex.unlock();
        }
    } else {
        // broadcast connection, slave
        connect(&usocket, SIGNAL(readyRead()),this, SLOT(readDatagram()));

#ifdef Q_OS_LINUX
        /* Qt bug:
         * on linux, multiple connections to same UDP port fail.
         *
         * workaround: create a socket using OS calls and set SO_REUSEADDR.
         *
         * see https://bugreports.qt.io/browse/QTBUG-33419
         */
        int socket_descriptor;
        if ((socket_descriptor = ::socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
            qDebug() << "Afedri: error creating datagram socket";
            return;
        }
        usocket.setSocketDescriptor(socket_descriptor,QAbstractSocket::UnconnectedState);
        int reuse = 1;
        if (::setsockopt(usocket.socketDescriptor(), SOL_SOCKET, SO_REUSEADDR,
                         &reuse, sizeof(reuse)) < 0) {
            qDebug() << "Afedri: error setting SO_REUSEADDR";
            return;
        }

        if (!usocket.bind(settings->value(s_sdr_afedri_udp_port,s_sdr_afedri_udp_port_def).toInt())) {
            emit(error("Afedri: UDP connection failed"));
            mutex.lock();
            initialized=false;
            running=false;
            mutex.unlock();
            return;
        }
#endif
#ifdef Q_OS_WIN
        if (!usocket.bind(settings->value(s_sdr_afedri_udp_port,s_sdr_afedri_udp_port_def).toInt()),
                   QUdpSocket::ReuseAddressHint) {
            emit(error("Afedri: UDP connection failed"));
            mutex.lock();
            initialized=false;
            running=false;
            mutex.unlock();
            return;
        }
#endif
        mutex.lock();
        running=true;
        initialized=true;
        mutex.unlock();
    }
}


/*!
 * \brief Afedri::set_freq
 *  Set frequency
 * \param frequency  Frequency (Hz)
 * \param channel (1,2,3,4)
 */
void Afedri::set_freq(unsigned long frequency, int channel)
{
    // version 1
    unsigned short control_code = CI_FREQUENCY;
    const int size = 10;
    char block[size];

    block[0] = size;
    block[1] = (SET_CONTROL_ITEM << 5);
    block[2] = control_code & 0xFF;
    block[3] = (control_code >> 8) & 0xFF;
    block[4] = channel;
    for (int i = 0; i < 4; i++)
    {
        block[5+i] = (frequency >> (i*8)) & 0xFF;
    }
    block[9] = 0;


    // version 2
    /*
    const int size = 9;
    char block[size];
    block[0]=size;
    block[1]=TCP_HID_PACKET << 5;
    block[2]=HID_FREQUENCY_REPORT;
    for (int i = 0; i < 4; i++)
    {
        block[3+i] = (frequency >> (i*8)) & 0xFF;
    }
    block[7]=channel;
    block[8]=0;
    */
    if (tsocket.write(block,size)==-1) {
        emit(error("Afedri: TCP write set_frequency failed"));
    }
    tsocket.flush();
}

/*! sets single, dual, qual channel modes
 * channel=0 : single
 * channel=1 : dual
 * channel=2 : quad
 */
void Afedri::set_multichannel_mode(int channel)
{
    const int size=9;
    char block[size];

    block[0]=9;
    block[1]=TCP_HID_PACKET << 5;
    block[2]=HID_GENERIC_REPORT;
    block[3]=HID_GENERIC_SET_MULTICHANNEL_COMMAND;
    switch (channel) {
    case 0: //single
        block[4]=0;
        break;
    case 1: //dual
        block[4]=2;
        break;
    case 2: //quad
        block[4]=5;
        break;
    }
    block[5]=0;
    block[6]=0;
    block[7]=0;
    block[8]=0;
    if (tsocket.write(block,size)==-1) {
        emit(error("Afedri: TCP write error, set_multichannel_mode"));
    }
    tsocket.flush();
}

/*!
 * \brief Afedri::set_sample_rate
 *    Set the Afedri sampling rate
 * \param rate  sample rate in Hz
 */
void Afedri::set_sample_rate(unsigned long sample_rate)
{
    unsigned short control_code = CI_DDC_SAMPLE_RATE;
    const int size = 9;
    char block[size];

    if (tsocket.state()!=QAbstractSocket::ConnectedState) return;
    block[0] = size;
    block[1] = (SET_CONTROL_ITEM << 5);
    block[2] = control_code & 0xFF;
    block[3] = (control_code >> 8) & 0xFF;
    block[4] = 0;
    for (int i = 0; i <4; i++)
    {
        block[5+i] = (sample_rate >> (i*8)) & 0xFF;
    }
    if (tsocket.write(block,size)==-1) {
        emit(error("Afedri: TCP write error, set_sample_rate"));
    }
    tsocket.flush();
}

/*!
 * \brief Afedri::set_broadcast_flag
 * \param b true=broadcast on false=off
 *
 * Sets Afedri net UDP broadcast option
 *
 */
void Afedri::set_broadcast_flag(bool b)
{
    const int size=9;
    char block[size];

    block[0]=9;
    block[1]=TCP_HID_PACKET << 5;
    block[2]=HID_GENERIC_REPORT;
    block[3]=HID_GENERIC_BROADCAST_COMMAND;
    if (b) {
        block[4]=1;
    } else {
        block[4]=0;
    }
    block[5]=0;
    block[6]=0;
    block[7]=0;
    block[8]=0;
    if (tsocket.write(block,size)==-1) {
        emit(error("Afedri: TCP write error, set_broadcast_flag"));
    }
    tsocket.flush();
}


Afedri::~Afedri()
{
}

void Afedri::readTcp()
{
    QByteArray dat=tsocket.readAll();
    //qDebug("Afedri: tcp rx <%s>",dat.data());
}

void Afedri::readDatagram()
{
    if (usocket.pendingDatagramSize()==-1) return;

    // actual number of IQ samples per UDP read for different channel modes
    const int read_size1=1024;
    const int read_size2=512;
    const int read_size4=256;

    // in broadcast mode, have 16 bytes extra in header
    qint64 udp_size=1028;
    if (settings->value(s_sdr_afedri_bcast,s_sdr_afedri_bcast_def).toInt()>0) {
        udp_size+=16;
    }

    char data[MAX_UDP_SIZE];

    if (usocket.readDatagram((char*)data,udp_size)==-1) {
        emit(error("Afedri: UDP read failed"));
        return;
    }

    if (settings->value(s_sdr_afedri_multi,s_sdr_afedri_multi_def).toInt()==0) {
        // single receiver
        for (int i = 0; i < read_size1; i++) {
            buff[bptr * sizes.advance_size + iptr + i] = data[i+4];
        }
        iptr+=read_size1;
    } else if (settings->value(s_sdr_afedri_multi,s_sdr_afedri_multi_def).toInt()==1) {
        // dual receiver
        switch (settings->value(s_sdr_afedri_channel,s_sdr_afedri_channel_def).toInt()) {
        case 0:
            // dual channel, channel 1
            // with 16 bit data, this is (16 bit I1),(16 bit Q1),32 bits skipped, etc
            for (int i = 0, j = 0; i < 1024; i+=8,j+=4) {
                buff[bptr * sizes.advance_size + iptr + j] = data[i+20];
                buff[bptr * sizes.advance_size + iptr + j+1] = data[i+21];
                buff[bptr * sizes.advance_size + iptr + j+2] = data[i+22];
                buff[bptr * sizes.advance_size + iptr + j+3] = data[i+23];
            }
            break;
        case 1:
            // dual channel, channel 2
            // with 16 bit data, this is 32 bits skipped, (16 bit I2),(16 bit Q2),etc
            for (int i = 0, j = 0; i < 1024; i+=8,j+=4) {
                buff[bptr * sizes.advance_size + iptr + j] = data[i+24];
                buff[bptr * sizes.advance_size + iptr + j+1] = data[i+25];
                buff[bptr * sizes.advance_size + iptr + j+2] = data[i+26];
                buff[bptr * sizes.advance_size + iptr + j+3] = data[i+27];
            }
            break;
        default:
            break;
        }
        iptr+=read_size2;
    } else {
        // quad receiver
        switch (settings->value(s_sdr_afedri_channel,s_sdr_afedri_channel_def).toInt()) {
        case 0:
            // quad channel, channel 1
            // with 16 bit data, this is (16 bit I1),(16 bit Q1),96 bits skipped, etc
            for (int i = 0, j = 0; i < 1024; i+=16,j+=4) {
                buff[bptr * sizes.advance_size + iptr + j] = data[i+20];
                buff[bptr * sizes.advance_size + iptr + j+1] = data[i+21];
                buff[bptr * sizes.advance_size + iptr + j+2] = data[i+22];
                buff[bptr * sizes.advance_size + iptr + j+3] = data[i+23];
            }
            break;
        case 1:
            // quad channel, channel 2
            // with 16 bit data, this is 32 bits skipped, (16 bit I2),(16 bit Q2),64 bits skipped,...
            for (int i = 0, j = 0; i < 1024; i+=16,j+=4) {
                buff[bptr * sizes.advance_size + iptr + j] = data[i+24];
                buff[bptr * sizes.advance_size + iptr + j+1] = data[i+25];
                buff[bptr * sizes.advance_size + iptr + j+2] = data[i+26];
                buff[bptr * sizes.advance_size + iptr + j+3] = data[i+27];
            }
            break;
        case 2:
            // quad channel, channel 3
            // with 16 bit data, this is 64 bits skipped, (16 bit I3),(16 bit Q3),32 bits skipped,...
            for (int i = 0, j = 0; i < 1024; i+=16,j+=4) {
                buff[bptr * sizes.advance_size + iptr + j] = data[i+28];
                buff[bptr * sizes.advance_size + iptr + j+1] = data[i+29];
                buff[bptr * sizes.advance_size + iptr + j+2] = data[i+30];
                buff[bptr * sizes.advance_size + iptr + j+3] = data[i+31];
            }
            break;
        case 3:
            // quad channel, channel 4
            // with 16 bit data, this is 96 bits skipped, (16 bit I4),(16 bit Q4),...
            for (int i = 0, j = 0; i < 1024; i+=16,j+=4) {
                buff[bptr * sizes.advance_size + iptr + j] = data[i+32];
                buff[bptr * sizes.advance_size + iptr + j+1] = data[i+33];
                buff[bptr * sizes.advance_size + iptr + j+2] = data[i+34];
                buff[bptr * sizes.advance_size + iptr + j+3] = data[i+35];
            }
            break;
        default:
            break;
        }
        iptr+=read_size4;
    }

    // see if one spectrum scan advance is completed
    if (iptr==sizes.advance_size) {
        iptr=0;
        bptr++;
        bptr = bptr % bpmax;
        emit(ready(buff, bptr));
    }
}

void Afedri::stop()
{
    if (settings->value(s_sdr_afedri_bcast,s_sdr_afedri_bcast_def).toInt()!=2)
    {
        send_rx_command(RCV_STOP);
        tsocket.flush();
        tsocket.close();
        if (tsocket.state() != QAbstractSocket::UnconnectedState) {
            tsocket.waitForDisconnected(1000);
        }
    }
    close_udp();
    usocket.close();
    if (usocket.state() != QAbstractSocket::UnconnectedState) {
        usocket.waitForDisconnected(1000);
    }

    mutex.lock();
    running=false;
    mutex.unlock();
    emit(stopped());
}

