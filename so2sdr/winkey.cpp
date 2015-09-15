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
#include <QSettings>
#include "defines.h"
#include "winkey.h"

#ifdef Q_OS_LINUX
#include <unistd.h>
#endif

#ifdef Q_OS_WIN
#include <windows.h>
#endif

/*!
   WinkeyDevice : serial port of device
 */
Winkey::Winkey(QSettings &s, QObject *parent) : QObject(parent), settings(s)
{
    winkeyPort     = new QextSerialPort(settings.value(s_winkey_device,s_winkey_device_def).toString(), QextSerialPort::EventDriven);
    winkeyVersion  = 0;
    nchar          = 0;
    sendBuff       = "";
    sending        = false;
    winkeyOpen     = false;
    winkeySpeedPot = 0;
    rigNum         = 0;
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

    int           n = winkeyPort->read((char *) &wkbyte, 1);
    if (n > 0) {
        if ((wkbyte & 0xc0) == 0xc0) {
            // Status byte: currently sending CW?
            if (wkbyte & 4) {
                sending = true;
                emit(winkeyTx(true, rigNum));
            } else {
                sending = false;
                emit(winkeyTx(false, rigNum));
            }
        // Pushbutton status only sent to host in WK2 mode by admin command, (0x00, 11)
        } else if ((wkbyte & 0xc0) == 0x80) {
            // speed pot setting in 6 lowest bits
            winkeySpeedPot = wkbyte & 0x3f;
        } else    {
            // This would be an echo byte
        }
    }
}

/*!
   load a message into buffer
 */
void Winkey::loadbuff(QByteArray msg)
{
    sendBuff.append(msg);
    nchar = sendBuff.length();
}

/*!
   Slot to start sending cw
 */
void Winkey::sendcw()
{
    if (winkeyPort->isOpen()) {
        winkeyPort->write(sendBuff.data(), nchar);
    } else {
        winkeyOpen = false;
    }
    nchar = 0;
    sendBuff.resize(0);
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
    emit(winkeyTx(false, rigNum));
    sendBuff.resize(0);
    nchar = 0;
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
    if (!winkeyPort->isOpen()) return;
    rigNum = nrig;
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

    winkeyPort->setBaudRate(BAUD1200);
    winkeyPort->setFlowControl(FLOW_OFF);
    winkeyPort->setParity(PAR_NONE);
    winkeyPort->setDataBits(DATA_8);
    winkeyPort->setStopBits(STOP_2);

    winkeyPort->setTimeout(250);
    winkeyPort->open(QIODevice::ReadWrite | QIODevice::Unbuffered);
    winkeyPort->setRts(0);
    winkeyPort->setDtr(1);
    winkeyPort->flush();

    // have to wait a while at various places
#ifdef Q_OS_LINUX
    usleep(100000);
#endif
#ifdef Q_OS_WIN
    Sleep(100);
#endif

    if (!winkeyPort->isOpen()) {
        winkeyOpen = false;
        return;
    }

    // Send three null commands to resync host to WK2
    unsigned char buff[64];
    buff[0] = 0x13;
    winkeyPort->write((char *) buff, 1);
    winkeyPort->write((char *) buff, 1);
    winkeyPort->write((char *) buff, 1);
#ifdef Q_OS_LINUX
    usleep(100000);
#endif
#ifdef Q_OS_WIN
    Sleep(100);
#endif

    // read any echo from winkey
    int n = winkeyPort->bytesAvailable();
    if (n > 64) n = 64;
    winkeyPort->read((char *) buff, n);

    // Echo Test to see if WK is really there
    // note: serial port events will not be emitted yet,
    // since event loop has not started (exec).
    buff[0] = 0x00;     // WK admin command, next byte sets admin function
    buff[1] = 4;        // Echo function, echoes next received character to host
    buff[2] = 0x55;     // Send 'U' to WK
    winkeyPort->write((char *) buff, 3);
#ifdef Q_OS_LINUX
    usleep(100000);
#endif
#ifdef Q_OS_WIN
    Sleep(100);
#endif

    n = winkeyPort->bytesAvailable();
    if (n > 64) n = 64;
    winkeyPort->read((char *) buff, n);

    // Was the 'U' received?
    if (buff[0] == 0x55) {
        buff[0] = 0x00;     // WK admin command
        buff[1] = 2;        // Host open, WK will now receive commands and Morse characters
        winkeyPort->write((char *) buff, 2);
#ifdef Q_OS_LINUX
        usleep(100000);
#endif
#ifdef Q_OS_WIN
        Sleep(100);
#endif


        n = winkeyPort->bytesAvailable();
        if (n > 64) n = 64;

        // read Winkey firmware version
        buff[0] = 0;
        winkeyPort->read((char *) buff, n);
        winkeyVersion = buff[0];
        emit(version(winkeyVersion));
        if (winkeyVersion == 0) {
            // winkey open failed
            closeWinkey();
            winkeyOpen = false;
            return;
        }

        // now reduce timeout
        winkeyPort->setTimeout(100);

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
#ifdef Q_OS_LINUX
        usleep(100000);
#endif
#ifdef Q_OS_WIN
        Sleep(100);
#endif

        // set other winkey features
        buff[0] = 0x0e;     // Set WK options command, next byte sets WK options
        buff[1] = 0;
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
#ifdef Q_OS_LINUX
        usleep(10000);
#endif
#ifdef Q_OS_WIN
        Sleep(10);
#endif

        // Pot min/max
        // must set this up or paddle speed screwed up.
        // winkey bug/undocumented feature?
        buff[0] = 0x05;     // Setup speed pot command, next three bytes setup the speed pot
        buff[1] = 10;       // min wpm
        buff[2] = 80;       // wpm range (min wpm + wpm range = wpm max)
        buff[3] = 0;        // Used only on WK1 keyers (does 0 cause a problem on WK1?)
        winkeyPort->write((char *) buff, 4);
#ifdef Q_OS_LINUX
        usleep(100000);
#endif
#ifdef Q_OS_WIN
        Sleep(100);
#endif

        winkeyOpen = true;
    }
}

void Winkey::closeWinkey()
{
    unsigned char buff[2];
    buff[0] = 0x00;     // Admin command, next byte is function
    buff[1] = 0x03;     // Host close
    winkeyPort->write((char *) &buff, 2);
    winkeyPort->close();
}


