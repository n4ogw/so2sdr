TEMPLATE = lib
CONFIG += qt \
    dll \
    qttelnet-buildlib
mac:CONFIG += absolute_library_soname
win32|mac:!wince*:!win32-msvc:!macx-xcode:CONFIG += debug_and_release \
    build_all
include(../src/qttelnet.pri)
TARGET = $$QTTELNET_LIBNAME
DESTDIR = $$QTTELNET_LIBDIR
win32 { 
    DLLDESTDIR = $$[QT_INSTALL_BINS]
    QMAKE_DISTCLEAN += $$[QT_INSTALL_BINS]\\$${QTTELNET_LIBNAME}.dll
}
target.path = $$DESTDIR
INSTALLS += target
SOURCES += 
HEADERS += 
