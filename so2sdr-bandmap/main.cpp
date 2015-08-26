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
#include <QApplication>
#include "so2sdr-bandmap.h"
#include <QCoreApplication>
#include <QStringList>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QStringList args=app.arguments();

    So2sdrBandmap *main = new So2sdrBandmap(args);
    QObject::connect(main->actionQuit, SIGNAL(triggered()), main, SLOT(quit()));

    // calls So2sdrBandmap destructor on app exit
    main->setAttribute(Qt::WA_DeleteOnClose);

    if (main->so2sdrBandmapOk()) {
        return app.exec();
    } else {
        return -1;
    }
}


