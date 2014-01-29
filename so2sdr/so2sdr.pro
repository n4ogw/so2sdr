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
# change to vcapp to generate MSVC files
# generate VS solutions with the following from the top-level directory
#
# qmake -tp vc -r
#
#TEMPLATE = vcapp
TEMPLATE = app
TARGET = so2sdr

# needed for qttelnet
QT += network sql
DEPENDPATH += ../qextserialport/build \
    ../qttelnet/lib \
    ../portaudio-win32/lib
INCLUDEPATH += ../qttelnet/src \
    ../qextserialport
HEADERS += cwmessagedialog.h \
    serial.h \
    so2sdr.h \
    winkeydialog.h \
    stationdialog.h \
    log.h \
    radiodialog.h \
    cty.h \
    contest.h \
    contest_cq160.h \
    contest_sprint.h \
    sdrdialog.h \
    contestoptdialog.h \
    newcontestdialog.h \
    notedialog.h \
    dupesheet.h \
    bandmap.h \
    winkey.h \
    contest_wpx.h \
    spectrum.h \
    master.h \
    defines.h \
    contest_naqp.h \
    contest_iaru.h \
    telnet.h \
    qso.h \
    contest_cwops.h \
    contest_fd.h \
    contest_cqww.h \
    helpdialog.h \
    signal.h \
    bandmapentry.h \
    cabrillodialog.h \
    iqbalance.h \
    audioreader_portaudio.h \
    contest_sweepstakes.h \
    contest_stew.h \
    utils.h \
    contest_arrl160.h \
    contest_arrl10.h \
    contest_arrldx.h \
    contest_dxped.h \
    logedit.h \
    detailededit.h \
    mytableview.h \
    contest_cqp.h \
    contest_kqp.h \
  #  dvk.h \
    otrsp.h \
    ssbmessagedialog.h \
    settingsdialog.h \
    microham.h \
    history.h
FORMS += cwmessagedialog.ui \
    so2sdr.ui \
    winkeydialog.ui \
    stationdialog.ui \
    radiodialog.ui \
    sdrdialog.ui \
    contestoptdialog.ui \
    newcontestdialog.ui \
    notedialog.ui \
    dupesheet.ui \
    bandmap.ui \
    telnet.ui \
    helpdialog.ui \
    cabrillo.ui \
    iqbalance.ui \
    detailededit.ui \
    ssbmessagedialog.ui \
    settingsdialog.ui
SOURCES += cwmessagedialog.cpp \
    main.cpp \
    serial.cpp \
    so2sdr.cpp \
    winkeydialog.cpp \
    stationdialog.cpp \
    log.cpp \
    radiodialog.cpp \
    cty.cpp \
    contest.cpp \
    so2sdr_keys.cpp \
    contest_cq160.cpp \
    contest_sprint.cpp \
    sdrdialog.cpp \
    contestoptdialog.cpp \
    newcontestdialog.cpp \
    notedialog.cpp \
    dupesheet.cpp \
    so2sdr_dupesheet.cpp \
    bandmap.cpp \
    winkey.cpp \
    contest_wpx.cpp \
    spectrum.cpp \
    master.cpp \
    contest_naqp.cpp \
    contest_iaru.cpp \
    telnet.cpp \
    so2sdr_telnet.cpp \
    so2sdr_bandmap.cpp \
    qso.cpp \
    contest_cwops.cpp \
    contest_fd.cpp \
    contest_cqww.cpp \
    helpdialog.cpp \
    signal.cpp \
    bandmapentry.cpp \
    cabrillodialog.cpp \
    iqbalance.cpp \
    audioreader_portaudio.cpp \
    contest_sweepstakes.cpp \
    contest_stew.cpp \
    contest_arrl160.cpp \
    contest_arrl10.cpp \
    contest_arrldx.cpp \
    utils.cpp \
    contest_dxped.cpp \
    logedit.cpp \
    detailededit.cpp \
    mytableview.cpp \
    contest_cqp.cpp \
    contest_kqp.cpp \
  #  dvk.cpp \
    otrsp.cpp \
    ssbmessagedialog.cpp \
    settingsdialog.cpp \
    microham.cpp \
    history.cpp
unix { 
    include(../common.pri)

    CONFIG += link_pkgconfig
    PKGCONFIG += fftw3 hamlib portaudio-2.0 # sndfile

    QMAKE_CXXFLAGS += -O2 \
        -DINSTALL_DIR=\\\"$$SO2SDR_INSTALL_DIR\\\"
    HEADERS += linux_pp.h \
         glbandmap.h \
        ../qextserialport/qextserialport_global.h \
        ../qextserialport/qextserialport.h \
        ../qextserialport/qextserialenumerator.h \
        ../qttelnet/src/qttelnet.h
    SOURCES += linux_pp.cpp \
        ../qttelnet/src/qttelnet.cpp \
         glbandmap.cpp
    SOURCES += ../qextserialport/qextserialenumerator_unix.cpp \
        ../qextserialport/qextserialport.cpp \
        ../qextserialport/posix_qextserialport.cpp

    QMAKE_LFLAGS += -Wl,--as-needed
    install.target = install
    install.commands = install -d $$SO2SDR_INSTALL_DIR/bin; \
        install -d $$SO2SDR_INSTALL_DIR/share/applications; \
        install -d $$SO2SDR_INSTALL_DIR/share/icons/hicolor/24x24/apps; \
        install -d $$SO2SDR_INSTALL_DIR/share/icons/hicolor/48x48/apps; \
        install -d $$SO2SDR_INSTALL_DIR/share/so2sdr; \
        install -d $$SO2SDR_INSTALL_DIR/share/so2sdr/help; \
        install -o root -m 755 so2sdr $$SO2SDR_INSTALL_DIR/bin; \
        install -o root -m 644 ../so2sdr.desktop $$SO2SDR_INSTALL_DIR/share/applications; \
        install -o root -m 644 ../share/* $$SO2SDR_INSTALL_DIR/share/so2sdr; \
        install -o root -m 644 ../share/help/* $$SO2SDR_INSTALL_DIR/share/so2sdr/help; \
        install -o root -m 644 ../share/icon24x24.png $$SO2SDR_INSTALL_DIR/share/icons/hicolor/24x24/apps/so2sdr.png; \
        install -o root -m 644 ../share/icon48x48.png $$SO2SDR_INSTALL_DIR/share/icons/hicolor/48x48/apps/so2sdr.png
    QMAKE_EXTRA_TARGETS += install
}

win32 { 
#set flags for MSVC compiler/linker options
    QMAKE_CXXFLAGS_RELEASE += "/O2 /Oi /Ot /Oy /GL /Gy /arch:SSE2 /fp:fast"
    QMAKE_LFLAGS_RELEASE += "/ltcg"
    CONFIG += release
    HEADERS += win_pp.h \
              glbandmap_win.h
    SOURCES += win_pp.cpp \
              glbandmap_win.cpp
    DEFINES += WINVER=0x0501 # needed for mingw to pull in appropriate dbt business...probably a better way to do this
    LIBS += -L"R:/so2sdr/work/qextserialport/build" \
        -lqextserialport1
    LIBS += -L"R:/so2sdr/work/qttelnet/lib" \
        -lQtSolutions_Telnet-2.1
    LIBS += -L"../" \
        -llibfftw3-3
 #   LIBS += -L"R:/so2sdr/work/sndfile" \
 #       -llibsndfile-1
    LIBS += -L"R:/so2sdr/work/portaudio-win32-noasio/build/msvc/Win32/Release" \
        -lportaudio_x86
    INCLUDEPATH += R:/so2sdr/work/portaudio-win32-noasio/include
 #   INCLUDEPATH += R:/so2sdr/work/hamlib-win32-1.2.15.3/include
    INCLUDEPATH += R:/so2sdr/work/sndfile
    LIBS += -L"R:/so2sdr/work/hamlib-win32-1.2.15.3/lib/msvc" -llibhamlib-2
    RC_FILE = so2sdr.rc
}
