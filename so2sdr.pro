#Building under Windows: (need Visual Studio or Visual Studio Express Edition)
#
# 1. change to "app" to "vcapp" in so2sdr/so2sdr.pro to generate MSVC files
# 2. check directories and libraries for portaudio, fftw, and hamlib in the 
#    win32 section of so2sdr/so2sdr.pro
# 3. generate VS Solution with the following from the top-level directory
#     qmake -tp vc -r
#

TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS = qttelnet/buildlib \
	qextserialport \
          so2sdr
