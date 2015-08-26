# This file is part of so2sdr.
# so2sdr is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
# so2sdr is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# You should have received a copy of the GNU General Public License
# along with so2sdr.  If not, see <http://www.gnu.org/licenses/>.
#

TEMPLATE = app
TARGET = so2sdr-bandmap

QT += network

HEADERS += \ 
    network.h \
    networksetup.h \
    spectrum.h \
    signal.h \
    sdrdialog.h \
    iqbalance.h \
    bandmapentry.h \
    audioreader_portaudio.h \
    afedri.h \
    so2sdr-bandmap.h \
    defines.h \
    utils.h \
    sdrdatasource.h \
    afedrisetup.h \
    soundcardsetup.h \
    sdr-ip.h \
    afedri-cmd.h \
    call.h \
    bandmap-tcp.h \
    bandmapdisplay.h \
    helpdialog.h
FORMS += \
    iqbalance.ui \
    bandmap.ui \
    soundcard.ui \
    sdrdialog.ui \
    afedrisetup.ui \
    networksetup.ui \
    helpdialog.ui
SOURCES += \
    network.cpp \
    networksetup.cpp \
    spectrum.cpp \
    signal.cpp \
    sdrdialog.cpp \
    main.cpp \
    iqbalance.cpp \
    bandmapentry.cpp \
    audioreader_portaudio.cpp \
    afedri.cpp \
    so2sdr-bandmap.cpp \
    utils.cpp \
    sdrdatasource.cpp \
    afedrisetup.cpp \
    soundcardsetup.cpp \
    call.cpp \
    bandmapdisplay.cpp \
    helpdialog.cpp


 RESOURCES +=  so2sdr-bandmap.qrc

    
  unix {
    include (../common.pri)
    CONFIG += link_pkgconfig
    PKGCONFIG += fftw3 portaudio-2.0

    QMAKE_CXXFLAGS += -O2 \
        -DINSTALL_DIR=\\\"$$SO2SDR_INSTALL_DIR\\\"

    install.target = install
    install.commands = install -d $$SO2SDR_INSTALL_DIR/bin; \
        install -o root -m 755 so2sdr-bandmap $$SO2SDR_INSTALL_DIR/bin; \
        install -d $$SO2SDR_INSTALL_DIR/share/so2sdr; \
        install -d $$SO2SDR_INSTALL_DIR/share/so2sdr/so2sdr-bandmap-help; \
        install -o root -m 644 ../share/so2sdr-bandmap-help/* $$SO2SDR_INSTALL_DIR/share/so2sdr/so2sdr-bandmap-help; \
        install -o root -m 644 ../share/icon24x24.png $$SO2SDR_INSTALL_DIR/share/icons/hicolor/24x24/apps/so2sdr.png; \
        install -o root -m 644 ../share/icon48x48.png $$SO2SDR_INSTALL_DIR/share/icons/hicolor/48x48/apps/so2sdr.png
    QMAKE_EXTRA_TARGETS += install
}

#Windows flags for i686-w64-ming32 cross-compile
win32 { 
    # show console; useful for seeing QDebug output while debugging
    #CONFIG += console
    CONFIG += release
    LIBS +=   -lfftw3 -lportaudio -lhid -lsetupapi
    RC_FILE = so2sdr.rc
}

