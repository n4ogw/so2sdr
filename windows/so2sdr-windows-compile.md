## Cross-building so2sdr for Windows under Linux

This uses the cross-build tool mxe  [MXE](http://mxe.cc). The default
compiler target for MXE is 32-bit, static linkage. It may be possible
to build shared and 64-bit versions, but I have not tested this.

1. install mxe

```
     git clone https://github.com/mxe/mxe.git
```
2. Install needed packages for mxe. In mxe directory (in my case /usr/local/src/mxe)

```
     make qt
     make portaudio
     make fftw
     make libltdl
     make nsis
```
3. Set the path to pick up the MXE tools:

```
     export PATH=/usr/local/src/mxe/usr/bin:$PATH
```
4. build hamlib

note: the hamlib version from sourceforge is old and does not build correctly with mxe!
in mxe directory:

```
     git clone https://github.com/N0NB/hamlib.git
     cd hamlib
     sh ./autogen.sh
     ./configure --host=i686-w64-mingw32.static --enable-static --disable-shared --prefix=/usr/local/src/mxe/usr/i686-w64-mingw32.static
     make
     make install
```	 

5. some small fixes are needed:

+ in mxe/usr/i686-w64-mingw32.static/lib
```
    ln -s libws2_32.a libWs2_32.a
```

+ qextserialport uses a private Qt header (bad practice!). It is missing
from the installed MXE Qt files and has to be added.
```
    cd /usr/local/src/mxe/usr/i686-w64-mingw32.static/qt/include/QtCore
    mkdir private
    cp /usr/include/qt4/QtCore/private/qwineventnotifier_p.h private/.
````

+ the current Portaudio does not work correctly with WDMKS when compiled
with Mingw; when run an error box will pop up. It seems ok to just dismiss
this box and not use the WDMKS driver. Alternatively, remove 'wdmks' in the
MXW config script for Portaudio in src/portaudio.mk in the MXE install
directory.


6. build so2sdr
```
    git clone https://github.com/n4ogw/so2sdr.git
    cd so2sdr/so2sdr
```

Note: there are some separate .ui files for Windows. These make
the fonts more readable:

```
    cp so2sdr.ui.win so2sdr.ui
    cp dupesheet.ui.win dupesheet.ui
    i686-w64-mingw32.static-qmake-qt4
    make
```

Afterwards so2sdr.exe should be in /release

7. Compress the exe file. With static linkage the exe file is (24 MB). upx can be used to reduce its size to about 30% of original (UPX version 3.91). If this step is skipped, NSIS also applies some compression to the executable.
```
    upx --best release/so2sdr.exe
```

8. Make Windows installer package:
```
    cd ..
    i686-w64-mingw32.static-makensis -X"SetCompressor /FINAL lzma" so2sdr-2.0.0
```
This is a very basic NSIS installer. To uninstall, run the "uninstall" in the directory so2sdr is installed into. Nothing in the registry is modified. There is also a separate NSIS script for making an install package with just the so2sdr-bandmap program.

## Issues with Windows build

The mingw compiler has some issues building for Windows. While the program is linked as "static", it still depends on several MS Windows DLL's, including msvcrt.dll. This DLL changes with the version of Windows. In some cases the program may not run on older versions of Windows (XP) because it is missing some dependency in msvcrt.dll.
