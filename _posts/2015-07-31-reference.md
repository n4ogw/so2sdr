---
layout: page
title: "Reference"
category: doc
date: 2015-07-31 22:15:54
order: 2
---


* [Operating notes](#notes)
* [Key reference](#keyref)
* [List of macros](#macros)

---
<a name="keyref"></a>
### Key reference


* Radio QSY: enter a number corresponding to a frequency in
KHz in the callsign window. If the number is followed by a semicolon
(like 14005;), the frequency change will apply to the inactive radio.
* ctrl+Enter : logs qso with no dupe checking or exchange validation (be
careful). The qso may not be scored correctly.
* shift+Enter : logs qso without sending CW
* alt+Enter : start toggle mode. In this mode, each enter press toggles
back and forth between the two radios. Use this to call alternating cq's on
both radios.
* Backslash : log qso and send Quick QSL message
* PdDn : decrease CW speed
* PgUp : increase CW speed 
* ctrl+PdDn : decrease CW speed on inactive radio
* ctrl+PgUp : increase CW speed  on inactive radio
* alt+PgDn : in Auto CQ mode, decrease delay
* alt+PgUp : in Auto CQ mode, increase delay
* up/down arrow : switch between call and exchange windows
* ctrl+up : In S&P mode, tune to next higher detected signal on bandmap. 
In CQ mode  tune to the next higher signal on the INACTIVE radio.
* ctrl+down : In S&P mode, tune to next lower detected signal on bandmap. 
In CQ mode  tune to the next lower signal on the INACTIVE radio.
* ctrl+- : (Ctrl-Minus) Activates Dueling-CQ mode.
* alt+- : (Alt-Minus) Activates auto-send mode.
* Tab : enter S&P mode
* Esc : exit S&P mode; clear logging fields; reset Alt-D status
* minus (-) : mark frequency on active radio as a dupe, or clear
the current mark
* equals (=) : mark frequency on inactive radio as a dupe, or clear
the current mark
* left quote (`) : toggle audio stereo mode (split/non-split)
* alt+C : bring up Config menu
* alt+D : dupe check on 2nd radio
* ctrl+E : when QSO row is selected with the mouse, brings up
QSO edit dialog.
* alt+F : bring up File menu
* ctrl+F : search log for callsign fragment. ESC clears search results.
* alt+H : bring up Help menu
* alt+M : switch mult display mode
* ctrl+N : make a note
* alt+Q : start auto (repeating) CQ mode. While in Auto CQ, Alt-PageUp
and Alt-PageDn adjust the delay time between CQ's. Auto CQ sends F1 by default;
pressing F1 or F2 will change the message that is repeating. ESC or alt+Q
exits Auto CQ mode.
* alt+R : Switch radios
* ctrl+R : switch radios without killing cw.
* alt+S : Set CW speed. Followed by two numeric digits.
* ctrl+alt+S : take screenshot of main and bandmap windows.
Screenshot files are placed in the same directory as the log file.
* alt+W : bring up Windows menu
* ctrl+Z (in call/exchange entry field) : undo

---
<a name="macros"></a>
### CW/SSB Message macros



Two separate sets of macros are stored by the program, one for CW
and one for voice. In voice modes external scripts (by default
pulseaudio and gstreamer) are used to play and record messages.


* {CALL} :     callsign
* {#} :        qso number
* {UP} :       increase speed by 5 WPM
* {DN} :       decrease speed by 5 WPM
* {CANCEL} :   cancel any previous speed change
* {R2} :       send on other radio
* {R2CQ} :     send on other radio, marked as CQ. If a call is entered, program will switch to other radio to answer CQ. ESC or a F1 CQ will clear this state.
* {STATE} :    state
* {SECTION} :  ARRL section
* {NAME} :     name
* {CQZ} :      CQ zone
* {ITUZ} :     ITU zone
* {GRID} :     grid
* {CALL_ENTERED} :  contents of call entry window
* {TOGGLESTEREOPIN} :  toggle parallel port pin for audio control. This macro should be used alone and will not work with other CW macros
* {CQMODE} :   switch to CQ mode
* {SPMODE} :   switch to SP mode
* {SWAP_RADIOS} :  swap frequencies between radios
  <li> {SWITCH_RADIOS} : same as alt-R
* {REPEAT_LAST} :  repeats previously sent message
* {REPEAT_NR} :  if the call entry line is not empty, send current qso #. If call entry line is empty, sends number sent for last logged qso.
*   {CLEAR_RIT} : clear the RIT
* {RIG_FREQ} : send frequency of radio rounded to nearest KHz
* {RIG2_FREQ} : send frequency of 2nd radio rounded to nearest KHz
* {BEST_CQ} qsy current radio to "best" CQ freq
* {BEST_CQ_R2} qsy 2nd radio to "best" CQ freq
* | : insert 1/2 dit extra space
* {MCP}{/MCP} : send Microham Control Protocol commands
* {OTRSP}{/OTRSP} : send OTRSP Control Protocol commands
* {CAT}{/CAT} : send raw string to radio. This can be used
to trigger a variety of radio functions. Numbers placed inside of
< and > will be interpreted as hexadecimal bytes. Example: {CAT}SWT25;{/CAT}
will switch the RX antenna on the Elecraft K3.
* {CATR2}/{/CATR2} : same, except send to inactive radio
* {CAT1}/{/CAT1} : same, except send to radio 1
* {CAT2}/{/CAT2} : same, except send to radio 2
* {CALL_OK} : This will reset the check if the original call has been
corrected. Used when repeating the exchange to a station, it will prevent
the "Call Updated QSL" message from being sent when not needed.
* {SCRIPT}{/SCRIPT} : run a script in the /share/so2sdr/scripts directory
* {SCRIPTNR}{/SCRIPTNR} : run a script in the /share/so2sdr/scripts directory, where '#' is replaced with the active radio number (0 or 1)
* {PTTON} {PTTOFF} : turn active radio PTT on/off
* {PTTON1} {PTTOFF1} : turn radio 1 PTT on/off
* {PTTON2} {PTTOFF2} : turn radio 2 PTT on/off
* {PTTONR2} {PTTOFFR2} : turn inactive radio PTT on/off
* {PLAY} : play a voice message. Followed by a string which is the filename
that will be played. {PLAY}call will play the file call.wav
* {RECORD} : record a voice message. Followed by a string which is the filename
that will be recorded. {RECORD}call will record call.wav
* {2KBD} : toggle two keyboard mode

---

<a name="notes"></a>
### Operating notes


#### Running stations


SO2SDR uses the "Enter sends message" approach used by many other logging programs:

1. Enter callsign, press enter. The CQ exchange is sent
2. Enter exchange, press enter. The QSL message is sent
3. repeat

The program tries to be intelligent in interpreting the entered
exchange. There needs to be a space between exchange elements in
most cases (123a is acceptable in Sweepstakes for the number/prec).
In most cases, you do not need to backspace to correct
an exchange mistake- the program will take the last valid exchange
on the line.

That the exchange must be "validated", or enter will not log
the qso. In case there is a problem, it is possible to "force log"
the qso- press ctrl-Enter instead of just enter. Be careful with
these cases, the number and position of exchange elements has to
match exactly in this case. These qso's may also be scored
incorrectly

Shift-Enter instead of Enter will also log the qso without sending any CW.

The Backslash key ("\") will log the qso and send the "Quick QSL"
message instead of the usual QSL message.

#### S&P

TAB enters S&P mode.

1. type callsign. When enter is pressed, your call is sent
2. enter exchange. When enter is pressed, you exchange is sent
3. press enter again to log the qso. I am planning on making
this extra enter optional.

#### Using the bandmap

In the bandmap, the radio frequency is in the center at the red
line. A right mouse click will bring up several options:


* Zoom X1, Zoom X2: set scale of bandmap.
* Delete call: use to remove a call from the bandmap. Note that
the "-" and "=" keys can be used to remove a call.
* IQ Balance: open the IQ balance dialog. The bandmap uses strong
signals to correct gain and phase errors in the SDR hardware. 
This correction will be saved when quitting the program.
If the "IQ" box is checked, the correction is applied; if the "IQ Data"
box is checked, new signals are used to improve the correction.

Options at the bottom of the bandmap window:

* Mark signals: this uses a peak detection algorithm to
try to determine where signals are on the band. This does not decode
any CW. Each detected peak is marked with a small black dot; clicking
on it will tune the radio to that signal. Ctrl-up and Ctrl-down
arrows can be used to tune the radio to the next signal up or down the 
band. In S&P mode, they will tune the active radio; otherwise they apply to
the inactive radio.
* Signal level slider: adjusts sensitivity of peak detection 
algorithm.
* IQ: correct I-Q balance errors
* IQ Data: collect data from received signals to improve I-Q balance

Left click: this will tune the radio to this frequency.

Left Click+drag in frequency scale: moves center of the display

When in S&P mode, pressing Space after typing a callsign will add
it to the bandmap. If the call is a dupe, it will be highlighted
in color on the bandmap. Using this trick, new signals become obvious
on the bandmap. It is of course possible that a station can be
replaced by another on exactly the same frequency. However, in practice
this does not happen very often--it is a much more probable that
unhighlighted signals are unworked stations. The Spot Timeout
setting is also critical here.


#### Editing log information

Previous qso's can be edited in the log window by clicking on 
a field, editing the information, and pressing enter. Pressing escape
instead cancels the changes. The following fields
are editable: time, callsign, sent exchange, and received exchange. The
qso points (if displayed for that contest) will be recalculated 
automatically.

You can also bring up a more complete edit window by clicking on
a row in the logbook and pressing control-e.

To search for a call (or partial call) in the log, enter a call
fragment in the callsign window and press ctrl-F. ESC clears the 
search results.

Marking qso's as invalid: there is no way to delete qso's from the
log. However, for each qso there is a checkbox. If this is unchecked, the
qso is marked as invalid and 
completely removed from dupe checking and scoring, and will not
appear in the final Cabrillo output.

#### SO2R

For working stations on a second band while CQing on a different
band SO2SDR uses a system similar to TRLOG:


1. Tune in a station on the second radio. By clicking on a
signal or using ctrl-up/down this is very easy to do.
2. Press alt-D ; enter the callsign- it will show in the
2nd radio callsign window. The color is changed to indicate
this will be a second radio qso.
3. When ready to call the station, hit Space. Now work
the station as usual. You may want to define function
keys to send messages on the other radio during the 2nd radio
qso.
4. The program will return to the original radio when the
2nd radio qso is logged.

Note that if you tune by a station you are sure you have worked,
you can use the "-" or "=" keys to mark that frequency as a dupe
without having to enter the whole callsign. This can save a lot
of typing and allow one to check a 2nd band for new stations
very quickly.

An alternate way to work SO2R with the program is using two
keyboards. These need to be set up in Config->General Settings.

#### Sprint/Sprint+SO2R

SO2SDR emulates the behavior of DOS TRLOG for the Sprint contest. This
behavior is enabled when the Contest Options->Sprint Mode is
checked. <i>Note that in sprint mode, alt-D is disabled. The method of
SO2R used for the sprint does not use alt-D.</i>

The Sprint is operated "backwards". This means that the <i>active</i>
radio is used to tune for stations (S&P), while the inactive radio is
used for calling CQ. I recommend reading the TRLOG description of
Sprint operating by N6TR, SO2SDR operates in the same manner.  The
default CW macros are set up in the way described below if North
American Sprint or NCCC Sprint is selected as a contest.

To do S&P, leave the active radio in <i>CQ mode</i>, not S&P mode. It
is helpful to open one bandmap, which will automatically follow the
active band. When you find a station you want to
call, type the call and simply hit Space.  The program will send your
call and switch to exchange mode. Copy the exchange and hit enter to
send your exchange. After the qso is logged, you will be back in CQ
mode, ready to send CQ when you press enter.

During the Sprint, the second radio is typically used to call CQ while you
are receiving the exchange on the active radio. This is set up by default 
in macros F7 and F8. For example, F7 should be defined something like:

````
{R2CQ}TEST {CALL}
````

Suppose 80M is the active radio (frequency in bold) and you call CQ on 40M.
The color of the second radio entry box changes color to remind you that a
2nd-radio CQ is in effect. If someone answers the CQ, enter their call in
<i>the 80M entry box</i>. Do not change radios first: when the 2nd
radio CQ is active, the program will take the call from the "wrong"
radio, transfer it to the new radio, and switch radios. Now you
continue as before.  During this qso you can again press F7 to call CQ
on 80M. If you have the bandmaps running, using the {BEST_CQ} or
{BEST_CQ_R2} macro is very helpful to quickly find an open frequency
to call CQ on in this situation.

