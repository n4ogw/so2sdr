---
layout: default
title: "Introduction"
---

## So2sdr
------------

A ham radio contest log program that features a software-defined radio (SDR) 
bandscope.  The main ideas behind the program are described in an article
in May/June 2013 National Contest Journal [NCJ](http://ncjweb.com).
The program is  developed in Linux.

### News

* 05/24/2025 so2sdr now requires Qt version 6
  
* 09/14/2019 new feature version 2.5.0: two keyboard support (experimental)

* 06/08/2018 support ARRL June VHF contest

* 04/05/2018 support Mississippi qso party

* 02/18/2018 lots of code updates; Qt4 support dropped- you must now use Qt5

* 10/21/2017 update SCP and CTY files; winkey bug fix

* 05/31/2017 2.1.4 fix bugs for WPX contest

* 12/02/2016 2.1.0 new support for voice keyer using the sound card

### Features

* Two-radio (SO2R) support. Headphone and radio switching via parallel port 
or [OTRSP](http://www.k1xm.org/OTRSP/) (Open two-radio switching protocol)
USB devices. Support for using two keyboards.

* Uses [hamlib](http://sourceforge.net/projects/hamlib/)  to control radios via serial port.

* Supports soundcard based I/Q SDRs and some other SDRs.
This include the [Softrock](http://fivedash.com/) series,
[LP-PAN](http://www.telepostinc.com/LP-PAN.html), 
the [Afedri](http://www.afedri-sdr.com/) SDR, including the dual-receiver Afedri
AFE822x SDR-Net, and RTL-sdr SDRs.

* Uses [Winkey](http://k1el.tripod.com/WhatisWK.html) or [N6TR's So2rmini](https://github.com/n4ogw/so2rmini-n6tr) for CW generation.

* Interfaces with WSJT-X UDP server.

### Limitations

* So2sdr is primarily designed for unassisted operating (it
can connect to telnet dxclusters however).

* There is currently no support for digital modes, with the exception
of WSJTX modes in VHF contests.


* The number of different contests supported is small so far.

### Screenshots

#### Main Window 

* Sosdr tries to present all basic information you need
in one window so the user doesn't have to search multiple
open windows: two radio interfaces, multiplier information, previous
call information, etc.

![2015 NAQP CW](images/main_window.png "Main window")

#### SDR bandmap

* A unique feature of So2sdr is color highlighting of actual signals
on the bandmap. This can be used to rapidly identify new stations to
work without tuning through the entire band. 
Below dupes are highlighted in pink and unknown signals in white.
White signals are much more likely to be stations you haven't
worked before. Clicking on the bandmap tunes the radio.

* Note that despite some similarity to the display of CW Skimmer,
So2sdr does not decode CW. That is up to the operator.

* So2sdr can also search the bandmap for the frequencies with no
signals. This is a fast way to find a run frequency on a crowded
band.

![bandmap](images/bandmap.png "Bandmap")

