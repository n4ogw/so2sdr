---
layout: page
title: "Changelog"
category: changes
date: 2015-07-31 08:53:50
order: 1
---
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

