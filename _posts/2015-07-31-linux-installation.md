---
layout: page
title: "Linux"
category: install
date: 2016-01-07 08:55:00
order: 1
---

You will need the following development libraries installed: Qt4,
FFTW, Hamlib, and PortAudio. Other various development packages
include g++  and pkg-config.


### From released packages

Download one of the source code packages:

* [so2sdr-2.0.5.tgz](../downloads/so2sdr-2.0.5.tgz)


1. Unpack the archive:

        tar xzvof sosdr-2.0.5.tgz

2. Build it


        cd so2sdr
        qmake
        make

3. The default install location is in /usr/local. You can change this in common.pri
Then, as root:

        make install


### From git source


1. Clone the git repository to your local machine:
    
        git clone git://github.com/n4ogw/so2sdr.git

2. By default, so2sdr will be installed in /usr/local/bin, and associated
  data files will be placed in /usr/local/share/so2sdr. If you want to
  change the location of the program, edit SO2SDR_INSTALL_DIR in common.pri

3. In the directory so2sdr, 

    ```
    qmake
    make
    ```

    ``make -j 2``  will use 2 cores and go faster.
    Subdirectory Makefiles are created from the top level Makefile.

5. (as superuser) 
         
        make install
 
6. Test and contribute!


