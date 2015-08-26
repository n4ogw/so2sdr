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
#include <QDir>
#include <QApplication>
#include <QTime>
#include "defines.h"
#include "utils.h"

/*! returns directory where program data is stored
*/
QString dataDirectory()
{
#ifdef Q_OS_WIN
    // in Windows keep data files in same directory as executable
	return qApp->applicationDirPath();
#endif

#ifdef Q_OS_LINUX
	// INSTALL_DIR is usually /usr/local
        return QString(INSTALL_DIR)+QString("/share/so2sdr/");
#endif
}

/*! returns directory where user data (station config, hamlib cache,...) are stored
	*/
QString userDirectory()
{
#ifdef Q_OS_WIN
	return QDir::homePath() + "/so2sdr";
#endif

#ifdef Q_OS_LINUX
	return QDir::homePath() + "/.so2sdr";
#endif
}

/* see http://stackoverflow.com/questions/3752742/how-do-i-create-a-pause-wait-function-using-qt */
void delay( int millisecondsToWait )
{
    QTime dieTime = QTime::currentTime().addMSecs( millisecondsToWait );
    while( QTime::currentTime() < dieTime )
    {
        QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );
    }
}
