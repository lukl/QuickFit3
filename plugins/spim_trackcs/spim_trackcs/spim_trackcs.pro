#-------------------------------------------------
#
# Project created by QtCreator 2016-10-12T15:32:05
#
#-------------------------------------------------

QT       += core gui

TARGET = spim_trackcs
TEMPLATE = lib
CONFIG += plugin

DESTDIR = $$[QT_INSTALL_PLUGINS]/generic

SOURCES += qftrackcoverslipwidget.cpp

HEADERS += qftrackcoverslipwidget.h
DISTFILES += spim_trackcs.json

unix {
    target.path = /usr/lib
    INSTALLS += target
}
