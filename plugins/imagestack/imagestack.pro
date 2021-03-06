#
#    Copyright (c) 2008-2015 Jan W. Krieger (<jan@jkrieger.de>, <j.krieger@dkfz.de>), German Cancer Research Center (DKFZ) & IWR, University of Heidelberg
#	
#	 
#
#    This file is part of QuickFit 3 (http://www.dkfz.de/Macromol/quickfit).
#
#    This software is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

TEMPLATE = lib
CONFIG += plugin


TARGET = image_stack
DEFINES += TARGETNAME=$$TARGET
DEPENDPATH += ./

include(../../qf3plugins.pri)

DESTDIR = $$QFOUTPUT/plugins

include(../../libquickfitwidgets.pri)

# Input
HEADERS += qfrdrimagestack.h \
           qfrdrimagestack_data.h \
		   qfrdrimagestack_dataeditor.h \
    ../interfaces/qfrdrimagestackinterface.h \
    ../base_classes/qfrdrimagemasktools.h \
    qfrdrimagestack3dviewer.h \
    qfrdrimagestack_parserfunctions.h

SOURCES += qfrdrimagestack.cpp \
           qfrdrimagestack_data.cpp \
		   qfrdrimagestack_dataeditor.cpp \
    ../base_classes/qfrdrimagemasktools.cpp \
    qfrdrimagestack3dviewer.cpp \
    qfrdrimagestack_parserfunctions.cpp

FORMS = \ 
    qfrdrimagestack3dviewer.ui

RESOURCES += qfrdrimagestack.qrc
TRANSLATIONS= ./translations/de.image_stack.ts

INCLUDEPATH += ../../lib/ \
               ../../libqf3widgets/

ASSETS_TESTDATA.files = ./examples/testdata/*.*
ASSETS_TESTDATA.path = $${QFOUTPUT}/examples/$${TARGET}/testdata/
INSTALLS += ASSETS_TESTDATA

include(../../quickfit3_configqtwidgets.pri)



