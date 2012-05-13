PROJECT                 = qextserialport
TEMPLATE                = lib
VERSION                 = 1.2.0
#DESTDIR                 = build
DESTDIR = $$PWD/build
target.path = DESTDIR
#CONFIG                 += qt debug_and_release build_all
CONFIG += release
#warn_on debug_and_release
CONFIG                  += dll
DEFINES                 += QEXTSERIALPORT_LIB QT_NO_DEBUG_OUTPUT
#CONFIG                 += staticlib

# event driven device enumeration on windows requires the gui module
!win32:QT               -= gui

OBJECTS_DIR             = tmp
MOC_DIR                 = tmp
DEPENDDIR               = .
INCLUDEDIR              = .
HEADERS                 = qextserialport.h \
                          qextserialenumerator.h \
                          qextserialport_global.h
SOURCES                 = qextserialport.cpp

unix:SOURCES           += posix_qextserialport.cpp
unix:!macx:SOURCES     += qextserialenumerator_unix.cpp
macx {
  SOURCES          += qextserialenumerator_osx.cpp
  LIBS             += -framework IOKit -framework CoreFoundation
}

win32 {
  SOURCES          += qextserialenumerator_win.cpp win_qextserialport.cpp
  DEFINES          += WINVER=0x0501 # needed for mingw to pull in appropriate dbt business...probably a better way to do this
  LIBS             += -lsetupapi
}

CONFIG(debug, debug|release) {
    TARGET = qextserialportd
} else {
    TARGET = qextserialport
}
