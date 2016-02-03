TEMPLATE = subdirs

#Qt 5 includes qtserialport; for Qt 4 must build it from source

lessThan(QT_MAJOR_VERSION, 5): SUBDIRS = qtserialport so2sdr so2sdr-bandmap
greaterThan(QT_MAJOR_VERSION, 4): SUBDIRS = so2sdr so2sdr-bandmap
