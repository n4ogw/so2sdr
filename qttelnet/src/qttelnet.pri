include(../common.pri)
INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

qttelnet-uselib:!qttelnet-buildlib {
    LIBS += -L$$QTTELNET_LIBDIR -l$$QTTELNET_LIBNAME
} else {
    SOURCES += $$PWD/qttelnet.cpp
    HEADERS += $$PWD/qttelnet.h
    win32:LIBS += -lWs2_32
}
QT += network

win32 {
    contains(TEMPLATE, lib):contains(CONFIG, shared):DEFINES += QT_QTTELNET_EXPORT
    else:qttelnet-uselib:DEFINES += QT_QTTELNET_IMPORT
}
