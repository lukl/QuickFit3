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
TARGET = meas_b040resheater
DEFINES += TARGETNAME=$$TARGET
DEPENDPATH += ./


include(../../qf3plugins.pri)


DESTDIR = $$QFOUTPUT/plugins


include(../../libquickfitwidgets.pri)

# Input
HEADERS += meas_b040resheater.h \
    ../base_classes/qf3simpleb040serialprotocolhandler.h \
    ../interfaces/qfextensionmeasurementdevice.h

SOURCES += meas_b040resheater.cpp \
    ../base_classes/qf3simpleb040serialprotocolhandler.cpp

FORMS =

RESOURCES += meas_b040resheater.qrc

TRANSLATIONS= ./translations/de.meas_b040resheater.ts

INCLUDEPATH += ../../lib/ \
               ../../libqf3widgets/

ASSETS_FW_FILES.files = ./assets/electronics/software_resheater/*.*
ASSETS_FW_FILES.path = $${QFOUTPUT}/examples/$${TARGET}/electronics/software_resheater/

ASSETS_FW2_FILES.files = ./assets/electronics/software_resheater/c_testread/*.*
ASSETS_FW2_FILES.path = $${QFOUTPUT}/examples/$${TARGET}/electronics/software_resheater/c_testread/

ASSETS_FW3_FILES.files = ./assets/electronics/software_resheater/Matlab/*.*
ASSETS_FW3_FILES.path = $${QFOUTPUT}/examples/$${TARGET}/electronics/software_resheater/Matlab/

INSTALLS += ASSETS_FW_FILES ASSETS_FW3_FILES ASSETS_FW2_FILES

include(../../quickfit3_configqtwidgets.pri)






