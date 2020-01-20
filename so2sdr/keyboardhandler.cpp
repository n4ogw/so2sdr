/*! Copyright 2010-2020 R. Torsten Clay N4OGW

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
#include "keyboardhandler.h"
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <termios.h>
#include <signal.h>
#include <poll.h>

#include <QDebug>
#
KeyboardHandler::KeyboardHandler(QString deviceName,QObject *parent) : QObject(parent)
{
    device=deviceName;
    quitFlag=false;
    fd=0;
}

void KeyboardHandler::setDevice(QString d)
{
    device=d;
}

void KeyboardHandler::run()
{
    struct input_event ev;
    int size = sizeof(struct input_event);
    int rd,ret;
    bool shift = false;
    bool alt = false;
    bool ctrl = false;
    struct pollfd pfd[1];

    quitFlag=false;
    fd = open(device.toStdString().c_str(), O_RDONLY);
    pfd[0].fd=fd;
    pfd[0].events=POLLIN;

    if (fd == -1) {
        qDebug("KeyboardHandler: error opening %s",device.toStdString().data());
        return;
    } else {
        //qDebug("KeyboardHandler: opening %s",device.toStdString().data());
    }

    if (fd >= 0) {
        ioctl(fd, EVIOCGRAB, 1);
        while (!quitFlag) {
            // timeout is set to 5 ms. Does this need adjusting?
            ret = poll(pfd, 1, 5);
            if (ret>0) {
                if (pfd[0].revents == POLLIN) {
                    rd = read(fd, &ev, size);
                    if (rd!=size) break;
                    // value 1 catches single keypresses; value 2 catches autorepeat
                    if (ev.type == EV_KEY && ((ev.value == 1) || ev.value==2)) {
                        switch (ev.code) {
                        case KEY_LEFTSHIFT:case KEY_RIGHTSHIFT:
                            shift = true;
                            break;
                        case KEY_LEFTALT:case KEY_RIGHTALT:
                            alt=true;
                            break;
                        case KEY_LEFTCTRL:case KEY_RIGHTCTRL:
                            ctrl = true;
                            break;
                        default:
                            emit readKey(ev.code,shift,ctrl,alt);
                            break;
                        }
                    } else if (ev.type == EV_KEY && ev.value == 0) {
                        switch (ev.code) {
                        case KEY_LEFTSHIFT:case KEY_RIGHTSHIFT:
                            shift = false;
                            break;
                        case KEY_LEFTALT:case KEY_RIGHTALT:
                            alt=false;
                            break;
                        case KEY_LEFTCTRL:case KEY_RIGHTCTRL:
                            ctrl = false;
                            break;
                        default:
                            break;
                        }
                    }
                }
            }
        }

    }
}

void KeyboardHandler::quitHandler()
{
    ioctl(fd, EVIOCGRAB, 0);
    quitFlag=true;
}
