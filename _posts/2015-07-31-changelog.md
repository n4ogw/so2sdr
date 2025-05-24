---
layout: page
title: "Changelog"
category: changes
date: 2015-07-31 08:53:50
order: 1
---

## version 2.8.0 (05/10/2025)

* updated code to use Qt version 6. This required many small changes. One important change
was made to the way keyboard grabbing works. In Qt6 the method previously used in Qt5 to grab the
keyboard no longer works. Now for keyboard grabbing to work, the /dev/input device for the
keyboard must be provided in the General Settings dialog.

* fix DX cluster telnet list not being saved

## version 2.7.4 (02/01/2025)

* fix display above callsign entry windows

## version 2.7.3 (11/24/2024)

* fix column names in contests with serial numbers
* correct mode in adif output for ssb
* make qso/mult displays line up correctly
* fix cabrillo for overlays in CQWW
* exchange setup box was cut off in ARRL 10

## version 2.7.2 (09/08/2024)

* Show CW text sent by So2rMini on status bar

## version 2.7.1 (06/10/2024)

* Fix bug in wsjtx message window. Did not work correctly unless computer timezone
set to UTC

## version 2.7.0 (01/28/2024)

* so2sdr-bandmap: add setting to control behavior when dragging the frequency
scale. There are now two possible behaviors: (a) the bandmap is 
recentered when the mouse is released, and (b) the bandmap is recentered
when the radio is tune (the frequency sent to so2sdr-bandmap changes).

* add settings to set the font and font size in so2sdr and so2sdr-bandmap.

* add information in help on setup and display resolution.

## version 2.6.10 (12/06/2023)

* add Labrador as mult in ARRL 10m contest

## version 2.6.9 (10/03/2023)

* add option to control sidetone frequency on so2rMini

## version 2.6.8 (10/01/2023)

* Support for So2rMini with N6TR firmware. This is available from
https://github.com/trlinux/trlinux (see src directory).

## version 2.6.7 (03/12/2023)

* Fix bug with deleting calls with right mouse click. Wrong call
was being sent.

## version 2.6.6 (01/20/2023)

* Afedri: wait for acknowledgement after each tcp command. This should
help prevent commands being sent too fast to the sdr.

## version 2.6.5 (01/17/2023)

* Add indicator showing number of tcp clients connected (two maximum)
* New method of tuning SDR in RF/RFauto modes for Afedri dual receiver
sdr. One bandmap is set up as master and controls the sdr parameters.
The other bandmap is set up as slave. The slave
now connects to the tcp server of the master bandmap and passes frequency
changes to the master bandmap.
* Do not retune SDR in RFauto mode when dragging frequency scale with mouse
* Fix bug where bandmap would only start on second click

## version 2.6.4 (01/10/2023)

* Add option to stop bandmap during transmit
* Improve recentering of display in RFauto mode
* Fix dual channel Afedri in RF/RFauto modes

## version 2.6.3 (01/01/2023)

* fix signal highlighting in RF/autocenter mode

## version 2.6.2 (12/25/2022)

* add options for IF versus RF panadapters
* reformat .h and .cpp files with clang-format

## version 2.6.1

* various bug fixes. Fix ARRL 10m contest indication of which band the radio is on

## version 2.6.0 (12/03/2022)

* initial support for RTL-sdr sdr's. librtl-sdr is now needed to compile so2sdr-bandmap.
To be used for HF, the device (and librtl-sdr) needs to support "direct sampling." Not
all rtl-sdr devices are capable of this. Two modes are available: direct (x1) and
x16 oversampled. The x16 improves the SNR slightly.

## version 2.5.17 (11/17/2022)

* remove dependency on qtx11extras
* so2sdr-bandmap bug fix: when running without standalone (without so2sdr attached), swap iq and offset were ignored because band wasn't being set.

## version 2.5.16 (11/13/2022)

* fix bug preventing rigctld from working

## version 2.5.15 (08/12/2022)

* improvements to script capability in message macros (see Help)

## version 2.5.14 (01/10/2022)

* add support for CWDdaemon

## version 2.5.13 (10/30/2021)

* fix voice recording/playback

## version 2.5.12 (08/06/2021)

* updates for depreciated Qt features
* changes to code that reads radio IF offset; now should read from any
radio that hamlib can read IF offset

## version 2.5.10 (01/04/2021)

* Fix bug with saving bandmap settings

## version 2.5.9 (11/23/2020)

* Fix bug in call/exchange entry in Sweepstakes; if call edited after exchange accepted, qso would not log
* Code changes for depreciated Qt features
* macro SCRIPT now reads script from user directory (/home/username/.so2sdr/scripts)

## version 2.5.8 (11/06/2020)

* Add PE as mult in ARRL Sweepstakes and other ARRL contests

## version 2.5.7 (08/29/2020)

* Add WW Digi contest

## version 2.5.6 (08/08/2020)

* Make cursor visible in two keyboard mode
* Correct Sprint behavior in two keyboard mode
* Fix behavior of 2KBD macro. Required adding a slight delay when this macro is triggered
* add status bar message when starting/stopping two keyboard mode
* Starting two keyboard mode now disables Grab mode- the two keyboard handler automatically grabs the keyboard

## version 2.5.5 (07/31/2020)

* change how entry of zones is handled in RST+zone contests. Now can update zone by simply typing a space and the new zone. In some cases (SSB mode) this will no longer recognize a non-standard RST/RS. Non-standard signal reports can be entered in this case after logging the qso using the edit function.

* fix bug with bandmap dupe marking ("*" call and -/= keys). All dupe marks were being  deleted at once.

## version 2.5.4 (06/20/2020)

* fix bugs with WSJTX interface; add dislay of sequence information. Double clicking on a call now sets the correct sequence
* fix dupe status of rovers in June VHF contest logged from WSJTX

## version 2.5.3 (06/11/2020)

* change in log file format. Logs from older version cannot be opened, must be imported from Cabrillo
* add new WSJTX window showing decoded calls and dupe/mult status. This is only fully functional for ARRL June VHF contest
* many code cleanups and small bug fixes


## version 2.5.4 (06/20/2020)

* fix bugs with WSJTX interface; add dislay of sequence information. Double clicking on a call now sets the correct sequence
* fix dupe status of rovers in June VHF contest logged from WSJTX

## version 2.5.3 (06/11/2020)

* change in log file format. Logs from older version cannot be opened, must be imported from Cabrillo
* add new WSJTX window showing decoded calls and dupe/mult status. This is only fully functional for ARRL June VHF contest
* many code cleanups and small bug fixes

## version 2.5.2 (04/28/2020)

* fix dupe checking in multi-mode contests
* fix cabrillo import
* allow changing mode when editing qso

## version 2.5.1 (11/23/2019)

* minor fixes to UI and fonts

## version 2.5.0 (09/14/2019)

* add two keyboard support
* update help file

## version 2.4.8 (06/05/2019)

* fix bugs with echo of winkey sending
* fix some typos in help file

## version 2.4.7 (04/07/2019)

* fix bug in multimode contests. Second mode qsos were showing as dupes
* fix multiplier bug in CQP

## version 2.4.6 (11/10/2018)

* add 630M and 2200M bands
* fix crashes when not on a contest band
* fix bug with window layout changing when call entered
* fix bug could not delete call/freq moved
* fix bug calls on bandmap not removed if band had changed

## version 2.4.5 (08/10/2018)

* improve appeareance on high-DPI displays
* fix bug with dupesheet in Sprint

## version 2.4.4 (07/11/2018)

* update IARU HQ mults
* fix display of frequency when editing qso with ctrl-e

## version 2.4.3 (07/03/2018)

bug fixes:

* fix core dump if no network connection for CTY download
* fix x2 zoom in bandmap
* make bandmap offset/swap IQ consistent between main setting and per-band
* prevent some writes to network SDR when port closed
* fix qsy by clicking on bandmap on 144 MHz (was only qsying to nearest KHz)
* correct freq format in cabrillo output for VHF contests; only give band
* clicking on entry window changes RX and TX. This fixes bug where entering a frequency immediately after program start also changed the radio
* Fix error in contest.cpp where uninitialized score object might be accessed.
* Fix error in mobile dupe checking.

## version 2.4.2 (06/09/2018)

* frequency not sent correctly to bandmap; caused bandmap freq to only update every KHz

## version 2.4.1 (06/09/2018)

* bug fixes: program was crashing when tuned out of band
* fix freq labels in bandmap
* start up with serial port RTS/DTR PTT signal off

## version 2.4.0 (06/08/2018)

* support for the ARRL June VHF contest
* support for reading ADIF qso information via UDP from WSJT-X
* allow per-band setting of IF offset and inversion (same as swap IQ) in so2sdr-bandmap
* To support higher frequency bands, the  internal format for frequency stored in log was changed.

## version 2.3.5 (04/14/2018)

* bug fix: store auto-CQ time as integer to prevent roundoff issues
* new feature: automatically download CTY files. 
* wl_cty.dat is now read from ~/.so2sdr

## version 2.3.4 (04/14/2018)

* bug fix: update score after onscreen log edit
* update to phone macros in all cfg files

## version 2.3.3 (04/07/2018)

* bug fix MS qso party cfg file
* remove check on changed freq when qsying bandmap; fails in some cases

## version 2.3.2 (04/05/2018)

* support Mississippi qso party

## version 2.3.1 (02/18/2018)

* fix bug: peak detection not turned of during transmit

## version 2.3.0 (02/18/2018)

* many internal changes and reorganization of code
* better support for multi-mode contests
* improve bandmap peak detection: when radio is tuned, momentarily turn off peak detection

## version 2.2.0 (11/19/2017)

* rewrite and simplification of the audio play and record features. See the section
in the help file on voice messages.

## version 2.1.5 (10/21/2017)

* update SCP and CTY files
* fix winkey bug- changing parameters now works without restart

## version 2.1.4 (05/31/2017)

* Bug fixes for WPX contest
* Update help file
* Other bug fixes: fix fonts in some dialogs, make multipliers window scrollable

## version 2.1.3 (01/10/2017)

* Update multipliers for NAQP and Sprint contests

## version 2.1.2 (12/30/2016)

* Several fixes/enhancements from NO3M:

* Several AutoCQ bugs, esp in conjunction w/ Alt-D;
AutoCQ calls sendFunc() instead of enter();
Fix autoCQ related switchTransmit calls in expandMacro function

* Alt+Space to start Alt-D QSO at any time, including during active CQ
QSO; space still works the same when focused in CQ call field.

* Alt+ESC to wipe focused field without killing CW

* Sometimes call or exchange field data would be selected when
re-focused.  Added deselect() to several places

* CW speed under certain radio switching conditions (ie. cntrl-R) would
not reflect current active radio speed.  Added conditional statements at
the top of expandMacro to set tmp_wpm variable.

* Prevent literal spacebar processing when using Alt-space keypress to
start Alt-D QSO during CQ QSO

* Silently switch radios with Alt-D esc in case alt-esc (silent) keypress
is used

## version 2.1.1 (12/05/2016)

* wav directory sometimes created in wrong place

## version 2.1.0 (12/02/2016)

* Add support for voice recording. See new section in help.

* So2sdr-bandmap: add option to also ignore spotted calls when searching for open cq frequencies.

* NO3M: activeRadio is temporarily set to autoCQRadio in enter when
AutoCQ is active and unpaused to prevent exchange being sent on CQ
radio if a call is present in AltD radio field.

* NO3M: AltD QSO is logged without interrupting CW on CQ radio.


## version 2.0.10 (10/26/2016)

* Another fix for winkey status getting stuck in TX

## version 2.0.9 (10/22/2016)

* Add support for radio control via hamlib rigctld

* Bug fix for auto cq and alt-D qsos

* shorten some delays in winkey initialization

## version 2.0.8 (10/06/2016)

* Bug fix for Qt5- SQL changes broke dupe checking

* Activating AutoCQ or dueling CQ will not interrupt current CW/msg
    transmission.  Dueling will start on focused radio if no CW, otherwise
    focuses other radio ready for toggling sequence. (NO3M)

## version 2.0.7 (10/04/2016)

* fixes to Winkey initialization (NO3M)

* fixes for Qt 5 sqlite

* Initialize activeTxRadio to -1 so switchTransmit is forced to
update SO2R device on startup (NO3M)

* Set QSO invalid if clearing Exchange line with ESC; wiping prefill
would allow QSO to be logged with no text in Exchange line (NO3M)

* When dupe allowed (no checking or WORK DUPES), focus exchange field
after sending CQ exchange (NO3M)

* Revert change made in 2013; block Alt-D during Sprint (NO3M)

* Remove {CALL_ENTERED} from Dupe MSG when using Autosend to avoid
sending call twice (NO3M)

* AutoCQ disables Dueling CQ and Toggle ESM (NO3M)

* Dueling CQ or Toggle ESM disables AutoCQ (NO3M)

* TAB (S&P) on AutoCQRadio or SWAP radios (macro) kills AutoCQ,
Dueling CQ, Toggle ESM (NO3M)

* Send F1 (long CQ) immediately when AutoCQ or Dueling CQ first
enabled unless QSO in progress, then start in SLEEP mode (NO3M)

* Disable AutoCQ and Dueling CQ in sprint mode (NO3M)


* AutoCQ:
Several bugs fixes, including with Alt-D interaction and timing/debounce
Pressing F1/F2 while un-paused sets long/short CQ silently (same as before)
Pressing F3-F12, ESC, or entering text in callsign field pauses
F1/F2, ENTER, or logging QSO un-pauses (NO3M)

* Dueling CQ: Re-implemented with better control and sleep (pause)
mode CQ starts on focused radio Pressing F1/F2 while un-paused sets
long/short CQ silently Pressing F3-F12, ESC, or ALT-R / CTRL-R, or
entering text in either call field pauses F1/F2, ENTER, or logging QSO
un-pauses if both call fields empty When paused (SLEEP), goes into
Toggle ESM, notes below (NO3M)

* Toggle ESM / Dueling CQ (SLEEP): Numerous bug fixes and cleaner
implementation, much better and more flexible than previously
Re-implemented; toggleEnter() function removed.  Uses regular ESM
enter() function now Focused radio behavior same as normal non-toggle
operation.  ESM processed by enter(). ESM, Fx, "\" keypresses start CW
immediately on focused radio and shift focus to opposite radio Works
with CTRL-ENTER and SHIFT-ENTER. Backslash "\" doesn't toggle ready
for next quick station ALT-R or CTRL-R allowed to manually toggle
focus, next CW message (ESM, Fx, etc) goes out on focused radio and
toggling resumes Can use empty Fx message to toggle w/o sending CW to
maintain QSO flow control / timing or use ALT/CTRL-R as noted above (NO3M)

## version 2.0.6 (02/02/2016)

* internal changes to dupesheet

* sunrise/sunset was displayed incorrectly for some calls in wl_cty.dat

* fix build for Qt 4 and Qt 5. To rebuild and switch Qt versions, you
must run "make distclean". The version of qmake used selects the version
of Qt.

* Another change to try to keep the TX status of the bandmap in sync

* Bug fix: under certain conditions, when logging a qso with ctrl-Enter, the
  sent qso number was logged as zero.

## version 2.0.5

* Bug fix: telnet spots were not working

* Add support for PA QSO Party. Some issues however:

        1. the one DX mult for PA stations is not counted
        2. county line operations have to be worked as two separate
		qsos, which will however double-count qso points
		3. portable (/P) and rover (/R) calls are not able to
		change counties. Only mobile /M can change counties.

* Bug fix: geometry of bandmap windows was not being saved when
closing them from within so2sdr.

* Update default config file for CA QSO Party (out of state). Was
missing state in exchange.

## version 2.0.4

* Bug fix: when cancelling CW (for example when switch radios), the
"TX" indicator on the bandmap should be turned off.

* Bug fix for autosend: if CNTRL-R is pressed before the callsign is
 done being sent in Autosend mode, the CW pauses for as long as focus
 is on the other radio, and then starts sending the callsign from the
 begnning if CNTRL-R back to the original radio. (NO3M)

* If already in a S/P QSO, call and exch fields populated, keyboard focus on
 exchange field, but QSY before logging.  QSO is wiped but Sprint space no
 longer works, required hitting ESC to restore. Added extra cleanups to 'qso'
 object, et.al., and set focus to call field when QSYing. (NO3M)

* When QSYing active radio before a QSO was logged, and active radio was
 also the active TX radio (last to transmit),  activeR2CQ was set to false
 but inactive radio callsign field remained "CQCQCQ"/colorized.  Fixed
 activeR2CQ check during QSY to always match inactive radio as QSYing radio
 before clearing R2CQ status. R2CQ radio should always be inactive since
 making that radio active requires [CNTRL/ALT]-R or mouse click focus, which
 all clear R2CQ status. (NO3M)

* add option to scroll to the right in so2sdr-bandmap (NO3M)

* check for a standard contest config file in the user directory
(i.e. ~/.sosdr) when starting a new contest. If this file exists,
it will be used instead of the program default from share/so2sdr (NO3M)

## version 2.0.3

* remove line leftover from testing preventing callsign clearing on radio 2

## version 2.0.2

* fix some bugs related to setting the config file for so2sdr-bandmap,
and choosing the executable for the bandmap in so2sdr.
* hopefully fix some bugs related to S&P mode


## version 2.0.1

* update help files

## version 2.0.0

* add network interface SDRs and Afedri SDR
* some unfinished features removed for the moment (click filter and
  DVK setup)
* rewrite of SDR bandmap; it is now a separate executable and can
  be used with other programs (see Documentation for use with 
  N1MM).
* known problems:
    + control of Afedri SDRs is somewhat buggy for dual-receiver
    models (when using both receivers).

    + under some Linux distributions  so2sdr-bandmap may have trouble 
    accessing the sound card if Pulseaudio is in use. A workaround is to
    use the pasuspender utility to stop Pulseaudio while so2sdr is
    running. If starting the bandmap from so2sdr, do this

            pasuspender -- so2sdr
   
        or if running so2sdr-bandmap separately,

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

