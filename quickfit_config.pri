#
#    Copyright (c) 2008-2015 Jan W. Krieger (<jan@jkrieger.de>, <j.krieger@dkfz.de>), German Cancer Research Center (DKFZ) & IWR, University of Heidelberg
#
#	last modification: $LastChangedDate: 2014-06-20 17:06:59 +0200 (Fr, 20 Jun 2014) $  (revision $Rev: 3266 $)
#
#    This file is part of QuickFit 3 (http://www.dkfz.de/Macromol/quickfit).
#
#    This software is free software: you can redistribute it and/or modify
#    it under the terms of the GNU Lesser General Public License (LGPL) as published by
#    the Free Software Foundation, either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU Lesser General Public License (LGPL) for more details.
#
#    You should have received a copy of the GNU Lesser General Public License (LGPL)
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.



## This file presets some basic configuration options. If you want to change one of them,
# create a file quickfit.inc in the base directory and change the options there!
#

# location of the global configuration directory on unix systems (QuickFit will add 'quickfit3/' to this string)
GLOBALCONFIGDIR = /usr/share/

# indicate whether the library lapack is present, if so, you can define the link options in LAPACK_LINK (e.g. "-lblas -llapack")
# and the include directories in LAPACK_INCLUDE
HAS_LAPACK = false
LAPACK_LINK =
LAPACK_INCLUDE =

# indicate whether the library blas is present, if so, you can define the link options in BLAS_LINK (e.g. "-lblas -lf2c -lm")
# and the include directories in BLAS_INCLUDE
HAS_BLAS = false
BLAS_LIB =
BLAS_INCLUDE =

OUTPUTDIR_NAME = $$PWD/output

CXXTUNE= -mtune=generic
exists(quickfit.inc):include(quickfit.inc)

!win32 {
    !macx:CXXTUNE= -march=native
    DEFINES += \'GLOBALCONFIGDIR=\"$$GLOBALCONFIGDIR\"\'
}
QMAKE_CXXFLAGS += -Wno-unused-variable -Wno-unused-but-set-variable -Werror=return-type $${CXXTUNE}
!macx:QMAKE_CXXFLAGS += -malign-double
!debug:QMAKE_CXXFLAGS += -msse -msse2 -mmmx -m3dnow -mfpmath=sse -ftree-vectorize -funroll-loops

#-ftree-vectorizer-verbose=5

contains( HAS_LAPACK, true ) {
    DEFINES += QF_HAS_LAPACK
}

contains( HAS_CBLAS, true ) {
    DEFINES += QF_HAS_CBLAS
}

contains( HAS_LMFIT3, true ) {
    DEFINES += QF_HAS_LMFIT3
}

contains( HAS_LMFIT5, true ) {
    DEFINES += QF_HAS_LMFIT5
}

contains( HAS_CAIRO, true ) {
    DEFINES += QF_HAS_CAIRO
}

contains( HAS_CIMG, true ) {
    DEFINES += QF_HAS_CIMG
}

contains( HAS_EIGEN3, true ) {
    DEFINES += QF_HAS_EIGEN3
}

contains( HAS_GSL, true ) {
    DEFINES += QF_HAS_GSL
}

contains( HAS_JPEG, true ) {
    DEFINES += QF_HAS_JPEG
}

contains( HAS_LEVMAR, true ) {
    DEFINES += QF_HAS_LEVMAR
}

contains( HAS_MINIZIP, true ) {
    DEFINES += QF_HAS_MINIZIP
}

contains( HAS_NLOPT, true ) {
    DEFINES += QF_HAS_NLOPT
}

contains( HAS_OOL, true ) {
    DEFINES += QF_HAS_OOL
}

contains( HAS_PIXMAN, true ) {
    DEFINES += QF_HAS_PIXMAN
}

contains( HAS_PNG, true ) {
    DEFINES += QF_HAS_PNG
}

contains( HAS_TIFF, true ) {
    DEFINES += QF_HAS_TIFF
}

contains( HAS_ZLIB, true ) {
    DEFINES += QF_HAS_ZLIB
}

contains( HAS_USB0, true ) {
    DEFINES += QF_HAS_USB0
}

contains( HAS_USB1, true ) {
    DEFINES += QF_HAS_USB1
}

!contains(QF3CONFIG, noeigen3lib) {
    include(extlibs/eigen.pri)
    DEFINES += STATISTICS_TOOLS_MAY_USE_EIGEN3
}











DEFINES += STATISTICS_TOOLS_USE_QFTOOLS_H
