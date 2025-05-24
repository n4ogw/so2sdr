# So2sdr

A ham radio contest logging program incorporating software-defined
radio (SDR) spectral displays. In runs on Linux. Version
2 has two separate executables: 1) so2sdr, the logging program
and 2) so2sdr-bandmap, the SDR spectral display. So2sdr-bandmap
can be used separately from so2sdr.

Copyright 2010-2025 R. Torsten Clay N4OGW

# News
* 05/24/2025 version 2.8.0; project has moved to Qt version 6. This required many changes. The grab keyboard function changed: now the keyboard device in settings must be given for keyboard grabbing.
* 08/12/2022 version 2.5.15; improvements to scripts called from message macros
* 01/10/2022 version 2.5.14; add CWDaemon support
* 09/14/2019 new feature in version 2.5.0: two keyboard support (experimental)
* 06/08/2018 support ARRL June VHF contest
* 04/05/2018 support Mississippi qso party
* 02/18/2018 lots of code updates; Qt4 support dropped- you must now use Qt5
* 12/02/2016 support for voice keyer using sound card


------------------------


## Install instructions

You will need the following development libraries installed: Qt (version 6), FFTW, Hamlib, and PortAudio. Other various development packages include g++, Git, and pkg-config. 

1. Clone the git repository to your local machine:
```
    git clone git://github.com/n4ogw/so2sdr.git
```
or download one of the source code distributions and extract it:
````
    tar xzvof so2sdr-2.8.0.tgz
````

2. By default, so2sdr will be installed in /usr/local/bin, and associated
data files will be placed in /usr/local/share/so2sdr. If you want to
change the location of the program, edit SO2SDR_INSTALL_DIR in common.pri

3. qmake 

4. make

(make -jx  will use x cores and go faster)


N.B. Subdirectory Makefiles will be created from the top level Makefile.

5. (as superuser) make install

6. Test and contribute!


-----------------------


Additional help is available in the file share/so2sdrhelp.html or via the
Help menu in the program.


Torsten Clay
so2sdr@gmail.com
