INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS                += $$PWD/qextserialbase.h \
		          $$PWD/qextserialport.h 


SOURCES                += $$PWD/qextserialbase.cpp \
		          $$PWD/qextserialport.cpp 
			  
unix:HEADERS           += $$PWD/posix_qextserialport.h		  
unix:SOURCES           += $$PWD/posix_qextserialport.cpp
unix:DEFINES           += _TTY_POSIX_
unix:VERSION           = 1.2.0

win32:HEADERS          += $$PWD/win_qextserialport.h \
                          $$PWD/qextserialenumerator.h 
win32:SOURCES          += $$PWD/win_qextserialport.cpp \
		          $$PWD/qextserialenumerator.cpp 
win32:DEFINES          += _TTY_WIN_


win32:LIBS             += -lsetupapi 

