/*! Copyright 2010-2025 R. Torsten Clay N4OGW

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
#include <QDebug>
#include <dirent.h>
#include <fcntl.h>
#include <linux/input.h>
#include <poll.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#
KeyboardHandler::KeyboardHandler(const QString &deviceName, QObject *parent)
    : QObject(parent), device(deviceName), quitFlag(false) {
  fd = 0;
}

void KeyboardHandler::setDevice(const QString &d) { device = d; }

void KeyboardHandler::run() {
  static bool shift = false;
  static bool alt = false;
  static bool ctrl = false;
  struct input_event ev;
  int size = sizeof(struct input_event);
  struct pollfd pfd[1];

  quitFlag = false;
  fd = open(device.toStdString().c_str(), O_RDONLY);
  pfd[0].fd = fd;
  pfd[0].events = POLLIN;

  if (fd == -1) {
    qDebug("KeyboardHandler: error opening %s", device.toStdString().data());
    return;
  }
  if (fd >= 0) {
    ioctl(fd, EVIOCGRAB, 1);
    while (!quitFlag) {
      // timeout is set to 5 ms. Does this need adjusting?
      int ret = poll(pfd, 1, 5);
      if (ret > 0) {
        if (pfd[0].revents == POLLIN) {
          int rd = read(fd, &ev, size);
          if (rd != size)
            break;
          // value 1 catches single keypresses; value 2 catches autorepeat

          if (ev.type == EV_KEY) {
            if ((ev.value == 1) || (ev.value == 2)) {
              switch (ev.code) {
              case KEY_LEFTSHIFT:
              case KEY_RIGHTSHIFT:
                shift = true;
                break;
              case KEY_LEFTALT:
              case KEY_RIGHTALT:
                alt = true;
                break;
              case KEY_LEFTCTRL:
              case KEY_RIGHTCTRL:
                ctrl = true;
                break;
              default:
                emit readKey(ev.code, shift, ctrl, alt);
                break;
              }
              // value = 0 is a key release
            } else if (ev.value == 0) {
              switch (ev.code) {
              case KEY_LEFTSHIFT:
              case KEY_RIGHTSHIFT:
                shift = false;
                break;
              case KEY_LEFTALT:
              case KEY_RIGHTALT:
                alt = false;
                break;
              case KEY_LEFTCTRL:
              case KEY_RIGHTCTRL:
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
}

void KeyboardHandler::quitHandler() {
  ioctl(fd, EVIOCGRAB, 0);
  quitFlag = true;
}
