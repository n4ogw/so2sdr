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
#include <QSettings>
#include "defines.h"
#include "winkey.h"

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>

/*!
   WinkeyDevice : serial port of device
 */
Winkey::Winkey(QSettings &s, QObject *parent) : QObject(parent), settings(s)
{
    QSerialPortInfo info(settings.value(s_winkey_device,s_winkey_device_def).toString());
    winkeyPort = new QSerialPort(info);

    txPtr=0;
    ignoreEcho=false;
    echoMode=true;
    winkeyVersion  = 0;
    nchar          = 0;
    sendBuff.clear();
    sent.clear();
    sending        = false;
    winkeyOpen     = false;
    winkeySpeedPot = 0;
    rigNum         = 0;
    txRig          = 0;
    winkeySendingCmd=false;
}

Winkey::~Winkey()
{
    closeWinkey();
    delete winkeyPort;
}

/*!
   returns true if winkey has been opened successfully
 */
bool Winkey::winkeyIsOpen() const
{
    return(winkeyOpen);
}

/*!
   returns true if winkey is sending cw
 */
bool Winkey::isSending() const
{
    return(sending);
}

/*!
   Slot triggered when data is available at port
 */
void Winkey::receive()
{
    unsigned char wkbyte;

    while (winkeyPort->bytesAvailable()) {
        int n = winkeyPort->read((char *) &wkbyte, 1);
        if (n > 0) {
            if ((wkbyte & 0xc0) == 0xc0) {
                // Status byte: currently sending CW?
                if (wkbyte & 4) {
                    if (winkeySendingCmd==true) continue; // ignore duplicate tx messages
                    sending = true;
                    winkeySendingCmd=true;
                    txRig=rigNum;
                    emit(winkeyTx(true, rigNum));
                } else {
                    sending = false;
                    winkeySendingCmd=false;
                    // signal to rx is sent for radio txRig, not rigNum. This is because
                    // the radio may have been switched since transmit started, need rx signal
                    // for the sending radio
                    emit(winkeyTx(false, txRig));
                }
                // Pushbutton status only sent to host in WK2 mode by admin command, (0x00, 11)
            } else if ((wkbyte & 0xc0) == 0x80) {
                // speed pot setting in 6 lowest bits
                winkeySpeedPot = wkbyte & 0x3f;
            } else    {
                // echo byte
                // in case CW was canceled, ignore any echo still associated with previous
                // message
                if (ignoreEcho) {
                    ignoreEcho=false;
                    return;
                }
                if (echoMode) processEcho(wkbyte);
            }
        }
    }
}

/*! process echo bytes
  */
void Winkey::processEcho(unsigned char byte)
{
    // find first occurence of this character starting at txPtr; this
    // will skip some special characters which are not echoed (half space, etc)
    int i=sent.indexOf(byte,txPtr);
    if (i!=-1) {
        txPtr=i+1;
        QString s=QString::number(rigNum+1)+":"+sent;
        s.truncate(i+3);
        emit(textSent(s,1700));
    }
}

void Winkey::setEchoMode(bool b)
{
    echoMode=b;
}


/*!
   Slot triggered when data is available at port, used during winkey
   initialization
 */
void Winkey::receiveInit()
{
    unsigned char wkbyte;

    int n = winkeyPort->read((char *) &wkbyte, 1);
    if (n > 0) {
        if (wkbyte==0x55) {
            // this was the echo test

        } else {
            // otherwise assume this is version number; call open part 2
            winkeyVersion = wkbyte;
	    emit(version(winkeyVersion));
	    disconnect(winkeyPort,SIGNAL(readyRead()),this,SLOT(receiveInit()));
	    openWinkey2();
        }
    }
}

/*!
   load a message into buffer
 */
void Winkey::loadbuff(QByteArray msg)
{
    if (winkeyOpen) {
        sendBuff.append(msg);
        sent=QString::fromLatin1(sendBuff);

        sent.remove('|'); // remove half spaces
        sent.remove(QChar(0x1e)); // this was added in So2sdr::expandMacro to cancel buffered speed change
        nchar = sent.length();
        txPtr=0;
    } else {
        // if winkey not open, just echo back the sent text as if it had been sent
        QString tmp=QString::number(rigNum+1)+":"+QString::fromLatin1(msg);
        tmp.remove('|');
        tmp.remove(QChar(0x1e));
        emit(textSent(tmp,3600));
    }
}

/*!
   Slot to start sending cw
 */
void Winkey::sendcw()
{
    if (winkeyPort->isOpen()) {
        winkeyPort->write(sendBuff.data(), sendBuff.length());
    } else {
        winkeyOpen = false;
    }
    sendBuff.clear();
}

/*!
   cancel winkey sending (command=10)
 */
void Winkey::cancelcw()
{
    const unsigned char cancel = 0x0a;

    if (winkeyPort->isOpen()) {
        winkeyPort->write((char *) &cancel, 1);
    } else {
        winkeyOpen = false;
    }
    sending=false;
    sent.clear();
    if (txPtr) ignoreEcho=true;
    txPtr=0;
    sendBuff.resize(0);
    nchar = 0;
    emit(cwCanceled());
}


/*!
   set speed directly in WPM
 */
void Winkey::setSpeed(int speed)
{
    if (!winkeyPort->isOpen() || speed < 5 || speed > 99) {
        return;
    }
    unsigned char buff[2];
    buff[0] = 0x02;
    buff[1] = (unsigned char) speed;
    winkeyPort->write((char *) buff, 2);
}

/*!
   set output port on Winkey
   sets PTT, sidetone on for either
 */
void Winkey::switchTransmit(int nrig)
{
    rigNum = nrig;
    if (!winkeyPort->isOpen()) return;
    unsigned char buff[2];
    buff[0] = 0x09;
    switch (nrig) {
    case 0:
        buff[1] = 0x04 + 0x01 + 0x02;
        break;
    case 1:
        buff[1] = 0x08 + 0x01 + 0x02;
        break;
    default:
        return;
    }
    winkeyPort->write((char *) buff, 2);
}

/*!
   start Winkey and get version number
 */
void Winkey::openWinkey()
{
    // in case we are re-starting winkey
    if (winkeyPort->isOpen()) {
        closeWinkey();
        winkeyOpen = false;
    }

    winkeyPort->setPortName(settings.value(s_winkey_device,s_winkey_device_def).toString());

    winkeyPort->setBaudRate(QSerialPort::Baud1200);
    winkeyPort->setDataBits(QSerialPort::Data8);
    winkeyPort->setStopBits(QSerialPort::TwoStop);
    winkeyPort->setParity(QSerialPort::NoParity);
    winkeyPort->setFlowControl(QSerialPort::NoFlowControl);
    winkeyPort->open(QIODevice::ReadWrite);
    if (!winkeyPort->isOpen()) {
        winkeyOpen = false;
        emit(winkeyError("ERROR: could not open WinKey device"));
        return;
    }
    connect(winkeyPort,SIGNAL(readyRead()),this,SLOT(receiveInit()));
    winkeyPort->setRequestToSend(false);
    winkeyPort->setDataTerminalReady(true);

    // Send three null commands to resync host to WK2
    unsigned char buff[64];
    buff[0] = 0x13;
    winkeyPort->write((char *) buff, 1);
    winkeyPort->write((char *) buff, 1);
    winkeyPort->write((char *) buff, 1);

    winkeyPort->waitForBytesWritten(100);;
    winkeyPort->waitForReadyRead(100);

    // Echo Test to see if WK is really there
    buff[0] = 0x00;     // WK admin command, next byte sets admin function
    buff[1] = 4;        // Echo function, echoes next received character to host
    buff[2] = 0x55;     // Send 'U' to WK
    winkeyPort->write((char *) buff, 3);
    winkeyPort->waitForBytesWritten(100);
    winkeyPort->waitForReadyRead(100);

    // command to get Winkey version
    buff[0] = 0x00;     // WK admin command
    buff[1] = 2;        // Host open, WK will now receive commands and Morse characters
    winkeyPort->write((char *) buff, 2);
    winkeyPort->waitForBytesWritten(100);
    winkeyPort->waitForReadyRead(100);
}

/*!
 second part of winkey initialization
*/
void Winkey::openWinkey2()
{
    char buff[4];
    connect(winkeyPort, SIGNAL(readyRead()), this, SLOT(receive()));
    //  set some saved user settings
    // set sidetone config
    buff[0] = 0x01;     // Sidetone control command, next byte sets sidetone parameters
    buff[1] = 0;

    // Paddle sidetone only?  Set bit 7 (msb) of buff[1]
    if (settings.value(s_winkey_sidetonepaddle,s_winkey_sidetonepaddle_def).toBool()) {
        buff[1] += 128;
    }
    // Set sidetone frequency (chosen in GUI)
    buff[1] += settings.value(s_winkey_sidetone,s_winkey_sidetone_def).toInt();
    winkeyPort->write((char *) buff, 2);
    winkeyPort->waitForBytesWritten(100);

    // set other winkey features
    buff[0] = 0x0e;     // Set WK options command, next byte sets WK options
    buff[1] = 4;        // enables serial echoback

    // CT spacing?  Set bit 0 (lsb) of buff[1]
    if (settings.value(s_winkey_ctspace,s_winkey_ctspace_def).toBool()) {
        buff[1] += 1;
    }

    // Paddle swap?  Set bit 3 of buff[1]
    if (settings.value(s_winkey_paddle_swap,s_winkey_paddle_swap_def).toBool()) {
        buff[1] += 8;
    }
    // Paddle mode, set bits 5,4 to bit mask, 00 = iambic B, 01 = iambic A,
    // 10 = ultimatic, 11 = bug
    buff[1] += (settings.value(s_winkey_paddle_mode,s_winkey_paddle_mode_def).toInt()) << 4;

    winkeyPort->write((char *) buff, 2);
    winkeyPort->waitForBytesWritten(100);

    // Pot min/max
    // must set this up or paddle speed screwed up.
    // winkey bug/undocumented feature?
    buff[0] = 0x05;     // Setup speed pot command, next three bytes setup the speed pot
    buff[1] = 10;       // min wpm
    buff[2] = 80;       // wpm range (min wpm + wpm range = wpm max)
    buff[3] = 0;        // Used only on WK1 keyers (does 0 cause a problem on WK1?)
    winkeyPort->write((char *) buff, 4);
    winkeyPort->waitForBytesWritten(100);
    winkeyOpen = true;
}

void Winkey::closeWinkey()
{
    if (!winkeyOpen || !winkeyPort->isOpen()) return;

    unsigned char buff[2];
    buff[0] = 0x00;     // Admin command, next byte is function
    buff[1] = 0x03;     // Host close
    winkeyPort->write((char *) &buff, 2);
    if (winkeyPort->isOpen()) {
        winkeyPort->flush();
    }
    winkeyPort->close();
    disconnect(winkeyPort,SIGNAL(readyRead()),0,0);
    winkeyOpen=false;
}


