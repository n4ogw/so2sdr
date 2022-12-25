/*! Copyright 2010-2023 R. Torsten Clay N4OGW

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
#include "utils.h"
#include <QApplication>
#include <QDir>
#include <QTime>

/*! returns directory where program data is stored
 */
QString dataDirectory() {
  // INSTALL_DIR is usually /usr/local
  return QString(INSTALL_DIR) + QString("/share/so2sdr/");
}

/*! returns directory where user data (station config, hamlib cache,...) are
 * stored
 */
QString userDirectory() { return QDir::homePath() + "/.so2sdr"; }

/* see
 * http://stackoverflow.com/questions/3752742/how-do-i-create-a-pause-wait-function-using-qt
 */
void delay(int millisecondsToWait) {
  QTime dieTime = QTime::currentTime().addMSecs(millisecondsToWait);
  while (QTime::currentTime() < dieTime) {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
  }
}
