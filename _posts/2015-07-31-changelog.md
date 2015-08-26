---
layout: page
title: "Changelog"
category: changes
date: 2015-07-31 08:53:50
---

## version 2.0.0

* add network interface SDRs and Afedri SDR
* some unfinished features removed for the moment (click filter and
  DVK setup)
* rewrite of SDR bandmap; it is now a separate executable and can
  be used with other programs (see Documentation for use with 
  N1MM).
* known problems:
    + control of Afedri SDR does not completely work for dual-receiver
    models. Under linux, the work-around is to use the sdr_commander 
    program to set the dual-receiver mode and frequency of the second SDR.
    Afedri in single-radio mode seems to work under Windows, but dual
    receiver mode does not.

    + sometimes a radio can get stuck in second-radio-cq mode that ESC
    will not exit. One way to get out is to switch to and from S&P mode.
    + under some Linux distributions  the program may have trouble 
    accessing the sound card if Pulseaudio is in use. A workaround is to
    use the pasuspender utility to stop Pulseaudio while so2sdr is
    running:

            pasuspender -- so2sdr
   
    or

            pasuspender -- so2sdr-bandmap

## version 1.5.2

* fixed several bugs in the alt-D code. Some slight change in how alt-D
  works- now if the radio is changed (alt-R or ctrl-R), and alt-D call
  will be cleared, unless the alt-D qso is already in progress (exchange
  entered).
* fix qso and mult info being shown on wrong radio with alt-D
* Call window has call, ESC pressed; was not clearing the mult and
  worked information, entering S&P mode afterwards would show these for
  the last call even with no entered callsign.
* fixed bandmap TX status bug: changing radios while CW was sending
    would prevent TX icon from turning off. Now turns off TX also whenever
     CW is canceled.


## version 1.5.1

* Some minor bug fixes and code cleanup
* qextserialport code updated
* Build system revised, especially Windows build

