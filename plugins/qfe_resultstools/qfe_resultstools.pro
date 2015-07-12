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

TARGET = qfe_resultstools
DEFINES += TARGETNAME=$$TARGET
DEPENDPATH += ./


include(../plugins.pri)


include(../../libquickfitwidgets.pri)

DESTDIR = $${QFOUTPUT}/plugins/

# Input
HEADERS += qferesultstools.h \
    qfertresultcalculator.h \
    qfertregexpdialog.h

SOURCES += qferesultstools.cpp \
    qfertresultcalculator.cpp \
    qfertregexpdialog.cpp

FORMS = \
    qfertresultcalculator.ui \
    qfertregexpdialog.ui

RESOURCES += qferesultstools.qrc \
    ../../lib/lib.qrc

TRANSLATIONS= ./translations/de.qfe_resultstools.ts

INCLUDEPATH += ../../lib/ \
               ../../libqf3widgets/

include(../../quickfit3_configqtwidgets.pri)



