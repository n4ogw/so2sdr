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
#include "defines.h"
#include "so2sdr-bandmap.h"
#include <QApplication>
#include <QCoreApplication>
#include <QStringList>
#include <QCommandLineParser>

int main(int argc, char *argv[]) {
    // check for -s option. This option must be done before QApplication is created
    bool scale=false;
    for (int i=0;i<argc;i++) {
        if (strcmp(argv[i],"-s")==0) {
            scale = true;
        }
    }
    if (!scale) {
        qunsetenv("QT_SCALE_FACTOR");
    }
    QApplication app(argc, argv);
    QApplication::setApplicationName("so2sdr-bandmap");
    QApplication::setApplicationVersion(Version);

    QCommandLineParser parser;
    parser.setApplicationDescription("so2sdr-bandmap: a SDR panadapter program, https://github.com/n4ogw/so2sdr");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("configfile","(optional) configuration file with full path");

    QCommandLineOption rescaleOption("s", "allow ui rescaling with QT_SCALE_FACTOR");
    parser.addOption(rescaleOption);
    parser.process(app);
    const QStringList args = parser.positionalArguments();

    So2sdrBandmap *main = new So2sdrBandmap(args);

    // calls So2sdrBandmap destructor on app exit
    main->setAttribute(Qt::WA_DeleteOnClose);

    if (main->so2sdrBandmapOk()) {
        return app.exec();
    } else {
        return -1;
    }
}
