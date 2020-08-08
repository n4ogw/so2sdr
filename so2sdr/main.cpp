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
#include <QApplication>
#include "so2sdr.h"
#include "menustyle.h"
#include <QCoreApplication>
#include <QStringList>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QStringList args=app.arguments();

    // set style that prevents menubar from grabbing focus when Alt pressed
    app.setStyle(new MenuStyle());

    So2sdr *main = new So2sdr(args);
    QObject::connect(main->actionQuit, SIGNAL(triggered()), &app, SLOT(quit()));

    // calls So2sdr destructor on app exit
    main->setAttribute(Qt::WA_DeleteOnClose);

    if (main->so2sdrOk()) {
        return app.exec();
    } else {
        return -1;
    }
}
