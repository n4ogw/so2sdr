# So2sdr

A ham radio contest logging program incorporating software-defined
radio (SDR) spectral displays. Runs under Linux and Windows. Version
2 has two separate executables: 1) so2sdr, the logging program
and 2) so2sdr-bandmap, the SDR spectral display. So2sdr-bandmap
can be used separately from so2sdr.

Copyright 2010-2016 R. Torsten Clay N4OGW

# News

* 10/04/2016 Qt5 update: the main issue with Qt 5 (not creating the log Sqlite
file correctly) hopefully has been fixed.

* 02/07/2016 Qt4 versus Qt5: currently the code compiles with either Qt4 or Qt5.
The version of Qt is selected by using the appropriate version of qmake.
However, there are a number of issues with Qt5 that make the program unusable.
These are a combination of Qt bugs and undocumented differences between Qt4 and Qt5.
For example, UDP socket support was broken in Qt 5.4.x but fixed in Qt 5.5.x.
There also appear to be differences in the SQL database classes in Qt5 versus Qt4.
It's going to take me a while to figure them out- for now I recommend sticking with
Qt 4.8.x

------------------------


## Install instructions (Linux)

You will need the following development libraries installed: Qt4 or Qt5, FFTW, Hamlib, and PortAudio. Other various development packages include g++, Git, and pkg-config. 

1. Clone the git repository to your local machine:
```
    git clone git://github.com/n4ogw/so2sdr.git
```
or download one of the source code distributions and extract it:
````
    tar xzvof so2sdr-2.0.6.tgz
````

2. By default, so2sdr will be installed in /usr/local/bin, and associated
data files will be placed in /usr/local/share/so2sdr. If you want to
change the location of the program, edit SO2SDR_INSTALL_DIR in common.pri

3. qmake 

4. make

(make -j 2  will use 2 cores and go faster)


N.B. Subdirectory Makefiles will be created from the top level Makefile.

5. (as superuser) make install

6. Test and contribute!


-----------------------


## Building for Windows

See the file windows/so2sdr-windows-compile.md for information on how
to cross-compile so2sdr for Windows using the [MXE](http://mxe.cc) environment.

------------------------


Additional help is available in the file share/so2sdrhelp.html or via the
Help menu in the program.


Torsten Clay
so2sdr@gmail.com
