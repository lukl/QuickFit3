TEMPLATE = lib
CONFIG += plugin

TARGET = cam_andor
DEPENDPATH += ./

include(../plugins.pri)

DESTDIR = $$QFOUTPUT/plugins


DEFINES += TARGETNAME=$$TARGET

win32 {
  LIBS+= -L../../extlibs/andor_win32
  INCLUDEPATH += ../../extlibs/andor_win32
}

LIBS += -landor -L$$QFOUTPUT -lquickfit3lib -lquickfit3widgets

# Input
HEADERS += cam_andor.h \
           camandoracquisitionthread.h \
           ../interfaces/qfextensioncamera.h \
           ../../../../../LIB/trunk/highrestimer.h \
           andorglobalcamerasettingswidget.h \
           andorsettingsdialog.h

SOURCES += cam_andor.cpp \
           camandoracquisitionthread.cpp \
           ../../../../../LIB/trunk/highrestimer.cpp \
           andorglobalcamerasettingswidget.cpp \
           andorsettingsdialog.cpp

FORMS = \
    andorglobalcamerasettingswidget.ui \
    andorsettingsdialog.ui

RESOURCES += cam_andor.qrc

TRANSLATIONS= ./translations/de.cam_andor.ts

INCLUDEPATH += ../../lib/ \
               ../../libqf3widgets/ \
               ../../../../../LIB/trunk/ \
               ../../../../../LIB/trunk/qt/ \
               ../../../../../../LIB/trunk/qt/





QT += gui xml svg opengl
CONFIG += exceptions rtti stl
