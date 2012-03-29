TEMPLATE = lib
CONFIG += plugin

TARGET = importers_basicimages
DEPENDPATH += ./

include(../plugins.pri)
include(../../extlibs/zlib.pri)
include(../../extlibs/tiff.pri)
DESTDIR = $$QFOUTPUT/plugins

# Input
HEADERS += qfbasicimageimporters.h \
           qfimagereaderlibtiff.h \
    ../../../../../LIB/trunk/libtiff_tools.h \
    qfimagereaderradhard.h \
    ../../../../../LIB/trunk/crc_ccitt.h \
    ../../../../../LIB/trunk/crc.h \
    ../../../../../LIB/trunk/frame.h

SOURCES += qfbasicimageimporters.cpp\
           qfimagereaderlibtiff.cpp \
    ../../../../../LIB/trunk/libtiff_tools.cpp \
    qfimagereaderradhard.cpp

FORMS =

RESOURCES +=

TRANSLATIONS= ./translations/de.importers_basicimages.ts


INCLUDEPATH += ../../lib/


QT += core
CONFIG += exceptions rtti stl