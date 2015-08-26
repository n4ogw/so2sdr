## Communicating with so2sdr-bandmap
---------------------------------------------------

So2sdr-bandmap uses TCP/UDP as the most portable and flexible option.
So2sdr-bandmap accepts commands via TCP using a very simple
protocol. Commands are sent to so2sdr-bandmap as a string of bytes
(little-endian for multiple byte data):

byte   : command  
byte   : length in bytes of following packet (n in this example)
            minimum length is 0  
byte 1  : byte n: data (optional), format depends on command


## General commands
--------------------------

+ Set Center Frequency: 'f' 0x66 dec 102
  followed by ascii characters giving frequency in Hz

+ Exit program: 'q' 0x71 dec 113
  No data follows (length=0). Using this command before terminating
  the  bandmap process allows the bandmap to shut down properly and
   update its config files.

+ Set TX state: 't' 0x74 dec 116
  No data follows (length=0). If transmit state is set, the bandmap will stop   peak detecting signals and display the red "TX" icon.

+ Set RX state: 'r' 0x72 dec 114 Cancels transmit state (see 0x74)

+ Find open frequency: 'g' 0x67 dec 103 Finds an open frequency on the bandmap. This will only function after the center frequency has been set. The found frequency will be returned via UDP (see below).

+ Set freq finder lower limit: 'l' 0x6C dec 108
    Set lower limit frequency for "Find open freq"
    followed by ascii characters giving frequency in Hz

+ Set freq finder upper limit: 'u' 0x75 dec 117
    Set upper limit frequency for "Find open freq"
    followed by ascii characters giving frequency in Hz

+ Set bandmap offset: 'o' 0x6f dec 111
   Set offset in Hz between frequency displayed and actual center freq.
   Followed by ascii characters giving offset in Hz.

+ Set bandmap invert: 'i'  0x69 dec 105
   If turned on, spectrum will be inverted. Useful for CW-reverse mode
   and some radios that have inverted IF on certain bands.
   Followed by single byte- if 0x00, sets invert off, any other byte
   sets invert on.

+ Qsy to next higher signal:  'U' 0x55 dec 85
   Length zero. Qsy's to next higher marked signal (black dot) on band. The new frequency is returned to the controlling program via UDP.

+ Qsy to next lower signal:  'D' 0x44 dec 68
   Length zero. Qsy's to next lower marked signal (black dot) on band. The new frequency  is returned to the controlling program via UDP.


## Call marking
------------------

So2sdr-bandmap can display callsigns and optionally highlight the
actual CW signal of a station with a specific color. The program
stores a list of callsigns and frequencies that are currently being
displayed.  TCP commands are used to manage this list. The program is
not designed to intelligently manage this list (for example finding
dupes or removing calls after a set time), this is left up to the
managing TCP connection.

1. Add callsign: 'a' 0x61 dec 97
 followed by
   * command string length byte
   * command string:
```
    callsign,frequency,R1G1B1R2G2B2flag
```
   + callsign = ASCII callsign data
   + frequency = frequency in Hz
   + R1 = byte giving red color value (0:255) for call
   + G1 = byte giving green color value (0:255) for call
   + B1 = byte giving blue color value (0:255) for call
   + R2 = byte giving red color byte for signal: 0 or 1
   + G2 = byte giving green color byte for signal: 0 or 1
   + B2 = byte giving blue color byte for signal: 0 or 1
   + flag = 0x00 means no highlight, any other value means highlight

 example: puts N4OGW on bandmap at 14.035100, callsign in magenta
and signal higlighted in magenta:
 
```
    0x61 0x16 N4OGW,14035100,0xff 0x00 0xff 0x01 0x00 0x01 1
```

2. Delete callsign: 'd'  0x64 dec 100
  followed by
    - length byte
    - callsign

3. Clear all calls: 'x' 0x78 dec 120
   command length 0


## UDP broadcasts
-----------------

So2sdr-bandmap sends out UDP packets in response to several types
of events:

1. clicking on the bandmap or a marked signal. The UDP packet contains
the new frequency:
```
    <?xml version="1.0" encoding="UTF-8"?>
    <So2sdr>
        <bandmap RadioNr="1" freq="14037726"/>
    </So2sdr>
```

RadioNr is the bandmap ID number. A similar packet is broadcast in
response to a request to find an open frequency. Note that the frequency
is given in Hz.


2. deleting a call with the mouse. The right-click menu gives the option
to delete a call. The bandmap itself does not delete the call from its
list, this should be done by the controlling program in response to
the UDP packet.
```
    <?xml version="1.0" encoding="UTF-8"?>
    <So2sdr>
      <bandmap RadioNr="1" freq="14022977" call="N4OGW" operation="delete"/>
    </So2sdr>
```
