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
TARGET = so2sdr

QT += network sql widgets serialport x11extras


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
    winkey.h \
    contest_wpx.h \
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
    otrsp.h \
    ssbmessagedialog.h \
    settingsdialog.h \
    microham.h \
    history.h \
    bandmapinterface.h \
    contest_paqp.h \
    logdelegate.h \
    so2r.h \
    so2rdialog.h \
    contest_msqp.h \
    filedownloader.h \
    contest_junevhf.h \
    udpreader.h \
    multdisplay.h \
    centergriddialog.h \
    adifparse.h \
    keyboardhandler.h \
    wsjtxcalldialog.h \
    wsjtxdelegate.h \
    wsjtxmessage.h

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
    telnet.ui \
    helpdialog.ui \
    cabrillo.ui \
    detailededit.ui \
    ssbmessagedialog.ui \
    settingsdialog.ui \
    so2rdialog.ui \
    centergrid.ui \
    wsjtxcalldialog.ui
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
    winkey.cpp \
    contest_wpx.cpp \
    master.cpp \
    contest_naqp.cpp \
    contest_iaru.cpp \
    telnet.cpp \
    so2sdr_telnet.cpp \
    qso.cpp \
    contest_cwops.cpp \
    contest_fd.cpp \
    contest_cqww.cpp \
    helpdialog.cpp \
    signal.cpp \
    bandmapentry.cpp \
    cabrillodialog.cpp \
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
    otrsp.cpp \
    ssbmessagedialog.cpp \
    settingsdialog.cpp \
    microham.cpp \
    history.cpp \
    bandmapinterface.cpp \
    contest_paqp.cpp \
    logdelegate.cpp \
    so2r.cpp \
    so2rdialog.cpp \
    contest_msqp.cpp \
    filedownloader.cpp \
    contest_junevhf.cpp \
    udpreader.cpp \
    multdisplay.cpp \
    centergriddialog.cpp \
    adifparse.cpp \
    keyboardhandler.cpp \
    wsjtxcalldialog.cpp \
    wsjtxdelegate.cpp \
    wsjtxmessage.cpp

 unix { 
    include (../common.pri)
    include (../qttelnet/src/qttelnet.pri)

    CONFIG += link_pkgconfig
    PKGCONFIG += hamlib
    QMAKE_CXXFLAGS += -O2 -Wall -DINSTALL_DIR=\\\"$$SO2SDR_INSTALL_DIR\\\"
    HEADERS += linux_pp.h
    SOURCES += linux_pp.cpp 

    install.target = install
    install.commands = install -d $$SO2SDR_INSTALL_DIR/bin; \
        install -d $$SO2SDR_INSTALL_DIR/lib; \
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
        install -o root -m 644 ../share/icon48x48.png $$SO2SDR_INSTALL_DIR/share/icons/hicolor/48x48/apps/so2sdr.png;

    QMAKE_EXTRA_TARGETS += install
}
