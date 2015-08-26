# So2sdr

A ham radio contest logging program incorporating software-defined
radio (SDR) spectral displays. Runs under Linux and Windows. Version
2 has two separate executables: 1) so2sdr, the logging program
and 2) so2sdr-bandmap, the SDR spectral display. So2sdr-bandmap
can be used separately from so2sdr.

Copyright 2010-2015 R. Torsten Clay N4OGW

------------------------


## Install instructions (Linux)

You will need the following development libraries installed: Qt4, FFTW, Hamlib, and PortAudio. Other various development packages include g++, Git, and pkg-config. 

1. Clone the git repository to your local machine:
```
    git clone git://github.com/n4ogw/so2sdr.git
```
or, from your account on Git Hub create your own fork of Tor's repository and
clone a read/write repository to your local machine.

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

Nate Bargmann
n0nb@n0nb.us
