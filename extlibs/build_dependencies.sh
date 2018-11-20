#! /bin/bash

function print_result {
	if [ $2 == -1 ]; then
		echo -e "building '$1': \033[0;33m NOT SELECTED \033[0m" 
	fi
	if [ $2 == -2 ]; then
		echo -e "building '$1': \033[0;31m ERROR IN CONFIGURE \033[0m" 
	fi
	if [ $2 == -3 ]; then
		echo -e "building '$1': \033[0;31m ERROR IN MAKE STEP \033[0m" 
	fi
	if [ $2 == -4 ]; then
		echo -e "building '$1': \033[0;31m ERROR IN INSTALL STEP \033[0m" 
	fi
	if [ $2 == -5 ]; then
		echo -e "building '$1': \033[0;33m INSTALL SCRIPT DOES NOT SUPPORT THIS ON YOUR PLATFORM \033[0m" 
	fi
	if [ $2 == 0 ]; then
		echo -e "building '$1': \033[1;32m OK \033[0m" 
	fi
}



echo -e \
"========================================================================\n"\
"= QuickFit 3 dependency build script                                   =\n"\
"========================================================================\n\n"\
"This script will try to build all the dependency libraries.\n"\
"needed for QuickFit. All libraries will be built in static linking mode\n"\
"and stay inside their subdirectory of this extlibs directory, so they\n"\
"don't interfere with your system-libraries. The .pri-files in this di-\n"\
"rectory include the libraries in a way that configures these libraries.\n\n"\
"========================================================================\n\n"\


echo "detecting compile system ... "
ISMSYS=`uname -a`
echo $ISMSYS
if echo $ISMSYS | grep -iq msys ; then
	echo "   => this is MSys on Windows"
	ISMSYS="1"
else
	echo "   => this is a default non-MSys system"
	ISMSYS="0"
fi


PICFLAGS=""
if [ $ISMSYS == "1" ] ; then

	PICFLAGS="-fPIC"
    echo -e "   => MSys environment on Windows: $PICFLAGS flag required"
	
	echo -e "   => MSys environment on Windows: mingw32-make.exe is used instead of make\n"
	shopt -s expand_aliases
	#source .alias
	alias make='mingw32-make.exe'
	
	MINGWBINPATH="/c/Qt/5.5/mingw492_32/bin:/c/Qt/Tools/mingw492_32/bin"
	#MINGWBINPATH="" # Add your bin paths here
	
	echo -e "Please make sure your compiler is found (seen in compiler missing errors)\n"\
	"(if encountered, add binary path to qmake and C/C++ compilers to path variable)\n"\
	"QuickFit should then ideally be built with the same compiler version.\n"\
	"A default path added here:"
	echo $MINGWBINPATH
	echo -e "If this is wrong, change in build_dependencies.sh accordingly\n"
	export PATH=$PATH:$MINGWBINPATH
fi


CURRENTDIR=${PWD}
QT_INFO_LIBS=`qmake -query QT_INSTALL_LIBS`
QT_INFO_BIN=`qmake -query QT_INSTALL_BINS`
QT_INFO_PLUGINS=`qmake -query QT_INSTALL_PLUGINS`
QT_INFO_INSTALLDIR=`qmake -query QT_INSTALL_PREFIX`
QT_INFO_VERSION=`qmake -query QT_VERSION`

if [[ -z "$QT_INFO_LIBS" ]] ; then
	echo -e "\nERROR: qmake not found.\n"
	read -p "Cancel (some libs won't build, maybe QF won't build) (y/n)? " -n 1  CANCELLING
	echo -e  "\n"
	if [ $CANCELLING == "y" ] ; then
		exit 1
	fi
else
	echo -e "\n\nBuilding for\n    Qt version ${QT_INFO_VERSION}\n       in ${QT_INFO_INSTALLDIR}\n\n"
fi	



echo -e "First we need to set some basics for the builds:"\


read -p "How many parallel builds do you want to use in make (1/2/3...)? " -n 1  MAKE_PARALLEL_BUILDS
echo -e  "\n"
read -p "Do you want to build keep the build directories (y/n)? " -n 1 KEEP_BUILD_DIR
echo -e  "\n"
read -p "Do you want to use more agressive optimizations for the built libraries (y/n/M)? M: no optimizations, use on MacOSX) " -n 1  MAKE_AGRESSIVEOPTIMIZATIONS
echo -e  "\n"
read -p "Do you want to optimize libraries for your local machine? (y/n)? " -n 1  MAKE_COMPILEFORLOCAL
echo -e  "\n"
read -p "Do you want to use OpenMP? (y/n)? " -n 1  MAKE_USEOPENMP
echo -e  "\n"
if [ $ISMSYS == "0" ] ; then
	read -p "Do you need the -fPIC flags? (y/n)? " -n 1  MAKE_PICFLAGS
else
	MAKE_PICFLAGS="n"
fi
echo -e  "\n"




#sh ../output/get_bit_depth.sh
MORECFLAGS=" -march=native"
MORELDFLAGS=
if [ $MAKE_COMPILEFORLOCAL == "y" ] ; then
	MORECFLAGS=" -march=native"
fi
if [ $MAKE_AGRESSIVEOPTIMIZATIONS == "y" ] ; then
	MORECFLAGS=" $MORECFLAGS -ftree-vectorize -funroll-loops "
	MORELDFLAGS=" $MORELDFLAGS "
fi


if [ $MAKE_AGRESSIVEOPTIMIZATIONS == "M" ] ; then
        MORECFLAGS=    
        MORELDFLAGS= 
fi

if [ $MAKE_USEOPENMP == "y" ] ; then
	MORECFLAGS=" $MORECFLAGS -fopenmp"
	MORELDFLAGS=" $MORELDFLAGS -fopenmp"
fi

if [ $MAKE_PICFLAGS == "y" ] ; then
		PICFLAGS="-fPIC"
fi



libnidaqmxOK=-5
if [ $ISMSYS == "1" ] ; then
	libnidaqmxOK=-1
	read -p "Do you want to build 'libNIDAQmx' (windows only!!!) (y/n)? " -n 1 INSTALL_ANSWER
	echo -e  "\n"
	if [ $INSTALL_ANSWER == "y" ] ; then

		echo -e  "------------------------------------------------------------------------\n"\
		"-- BUILDING: libnidaqmxOK                                             --\n"\
		"------------------------------------------------------------------------\n\n"\

		cd nidaqmx
		./makenidaqmx.sh
		libOK=$?
		cd ${CURRENTDIR}
		
		if [ $libOK -eq 0 ] ; then		
			libnidaqmxOK=0
		else
			libnidaqmxOK=-3
		fi
	fi
fi




libandorOK=-5
if [ $ISMSYS == "1" ] ; then
	libandorOK=-1
	read -p "Do you want to build 'libAndor' (windows only!!!) (y/n)? " -n 1 INSTALL_ANSWER
	echo -e  "\n"
	if [ $INSTALL_ANSWER == "y" ] ; then

		echo -e  "------------------------------------------------------------------------\n"\
		"-- BUILDING: libAndor                                                 --\n"\
		"------------------------------------------------------------------------\n\n"\
		echo -e "first we have to copy the file ATMCD32D.H from the Andor SDK to \n"
		echo -e "andor_win32 and andor_win64. Then ATMCD32D.DLL has to be copied \n"
		echo -e "to andor_win32 and atmcd64d.dll to andor_win64. Finally a script\n"
		echo -e "in each of these diretories will create the 32bit/64bit link libs\n"
		echo -e "which are needed to build the andor camera driver with mingw.\n\n"
	    read -p "please enter the directory of the Andor SDK "  ANDORSDK
	    echo -e  "\n"
	    read -p "Do you want to build 32bit or 64bit (3/6)? " -n 1 SELECTBITS
	    echo -e  "\n"
	    if [ $SELECTBITS == "6" ] ; then
            cd andor_win64
			cp $ANDORSDK/atmcd64d.dll .
			cp $ANDORSDK/ATMCD32D.H .
		else
            cd andor_win32
			cp $ANDORSDK/ATMCD32D.DLL .
			cp $ANDORSDK/ATMCD32D.H .
		fi
		
		./create_testcpp.sh
		libOK=$?
		cd ${CURRENTDIR}
		
		if [ $libOK -eq 0 ] ; then		
			libandorOK=0
		else
			libandorOK=-3
		fi
	fi
fi




qtOK=-5
if [ $ISMSYS == "1" ] ; then
	qtOK=-1
	read -p "Do you want to copy 'Qt DLLs' (y/n)? " -n 1 INSTALL_ANSWER
	echo -e  "\n"
	if [ $INSTALL_ANSWER == "y" ] ; then
		echo -e  "------------------------------------------------------------------------\n"\
		"-- COPYING: Qt DLLs                                                   --\n"\
		"------------------------------------------------------------------------\n\n"
		
		cp /mingw/bin/mingwm10.dll ../output/ &> /dev/null
		cp /mingw/bin/libstdc++*.dll ../output/ &> /dev/null
		cp /mingw/bin/libgcc*.dll ../output/ &> /dev/null
		cp /mingw/bin/pthread*.dll ../output/ &> /dev/null
		cp /mingw/bin/libpthread*.dll ../output/ &> / &> /dev/nullev/null
		cp /mingw/bin/libwinpthread*.dll ../output/
		
		USEDQTMODULES="QtCore4 QtGui4 QtOpenGL4 QtScript4 QtScriptTools4 QtSvg4 QtXml4 QtNetwork4"
		USEDQTMODULES5="Qt5Core Qt5Gui Qt5Network Qt5OpenGL Qt5Script Qt5ScriptTools Qt5PrintSupport Qt5Svg Qt5Xml Qt5Widgets Qt5WinExtras"
		USEDQTPLUGINS= "${QT_INFO_PLUGINS}/*"
		mkdir ../output/qtplugins
		for f in $USEDQTMODULES $USEDQTMODULES5
		do
			cp "${QT_INFO_BIN}/${f}d.dll"  "../output/" &> /dev/null
			cp "${QT_INFO_BIN}/${f}.dll"  "../output/" &> /dev/null
		done
		
		cp -rf "${QT_INFO_BIN}/icu*.dll"  "../output/"
		
		for f in $USEDQTPLUGINS
		do
			cp -rf "${f}"  "../output/qtplugins/" &> /dev/null
			cp -rf "${f}"  "../output/qtplugins/" &> /dev/null
		done
		qtOK=0
	fi
fi




read -p "Should all other libraries be built without asking? (y/n)? " -n 1  BUILD_ALL
echo -e  "\n"




zlibOK=-1
if [ $BUILD_ALL == "y" ] ; then
	INSTALL_ANSWER="y"
else
	read -p "Do you want to build 'zlib' (y/n)? " -n 1 INSTALL_ANSWER
	echo -e  "\n"
fi
if [ $INSTALL_ANSWER == "y" ] ; then
	echo -e  "------------------------------------------------------------------------\n"\
	"-- BUILDING: zlib                                                     --\n"\
	"------------------------------------------------------------------------\n\n"\

	cd zlib
	mkdir build
	tar xvf zlib-1.2.11.tar.gz -C ./build/
	cd build/zlib-1.2.11
 	if [ $ISMSYS == "1" ] ; then
		BINARY_PATH='../../bin'
		INCLUDE_PATH='../../include'
		LIBRARY_PATH='../../lib'

		echo -e 'BINARY_PATH='$BINARY_PATH'\nINCLUDE_PATH='$INCLUDE_PATH'\nLIBRARY_PATH='$LIBRARY_PATH'\nLOC='${PICFLAGS} $MORECFLAGS|cat - ./win32/Makefile.gcc > ./Makefile.gcc
		
		MAKEFILE="Makefile.gcc"
	else
		echo -e "Running on native Linux, not MSYS"
		export LDFLAGS="${MORELDFLAGS} ${PICFLAGS}"
		export CFLAGS="${MORECFLAGS} ${PICFLAGS}"
		export CPPFLAGS="${MORECFLAGS} ${PICFLAGS}"


		./configure --static --prefix=${CURRENTDIR}/zlib
		MAKEFILE="Makefile"
	fi
	libOK=$?
	if [ $libOK -eq 0 ] ; then
		 make -j${MAKE_PARALLEL_BUILDS} -f $MAKEFILE
		
		libOK=$?
		if [ $libOK -eq 0 ] ; then		
			 make -j${MAKE_PARALLEL_BUILDS} install -f $MAKEFILE
			libOK=$?
			if [ $libOK -ne 0 ] ; then		
				libOK=-4
			fi
		else
			libOK=-3
		fi
	else
	    libOK=-2
	fi

	cd ../../
	if [ $KEEP_BUILD_DIR == "n" ] ; then
		rm -rf build
	fi
	cd ${CURRENTDIR}
	
	zlibOK=$libOK


	# zlib_CFLAGS=
	# zlib_LDFLAGS="-lz"
	# zlib_CONFIG=
	# if [ -e ${CURRENTDIR}/zlib/lib/libz.a ] ; then
		# zlib_CFLAGS="-I${CURRENTDIR}/zlib/include"
		# zlib_LDFLAGS="-L${CURRENTDIR}/zlib/lib -lz"
		# zlib_CONFIG="--with-zlib-include-dir=${CURRENTDIR}/zlib/include  --with-zlib-lib-dir=${CURRENTDIR}/zlib/lib"
	# fi
	# echo -e "\n\n\n USING THESE zlib compiler commands:\nCFLAGS="
	# echo $zlib_CFLAGS
	# echo -e "\nLDFLAGS="
	# echo $zlib_LDFLAGS
	# echo -e "\n\n\n"
	
fi
print_result "zlib" $zlibOK



lzmaOK=-1
#read -p "Do you want to build 'lzma' (y/n)? " -n 1 INSTALL_ANSWER
#echo -e  "\n"
#if [ $INSTALL_ANSWER == "y" ] ; then
#	echo -e  "------------------------------------------------------------------------\n"\
#	"-- BUILDING: lzma                                                     --\n"\
#	"------------------------------------------------------------------------\n\n"\
#
#	cd lzma
#	mkdir build
#	tar xvf lzma-4.32.7.tar.gz -C ./build/
#	cd build/lzma-4.32.7
#	export LDFLAGS="${LDFLAGS} ${PICFLAGS} "
#        export CFLAGS="${CFLAGS} ${PICFLAGS} "
#        export CPPFLAGS="${CPPFLAGS} ${PICFLAGS}"
#        ./configure --enable-static --disable-shared --prefix=${CURRENTDIR}/lzma  CFLAGS="${PICFLAGS} ${MORECFLAGS}" CPPFLAGS="${PICFLAGS} ${MORECFLAGS}"	LDFLAGS="${PICFLAGS}"
#	libOK=$?
#	if [ $libOK -eq 0 ] ; then
#		 make -j${MAKE_PARALLEL_BUILDS} 
#		
#		libOK=$?
#		if [ $libOK -eq 0 ] ; then		
#			 make -j${MAKE_PARALLEL_BUILDS} install 
#			libOK=$?
#			if [ $libOK -ne 0 ] ; then		
#				libOK=-4
#			fi
#		else
#			libOK=-3
#		fi
#	else
#	    libOK=-2
#	fi
#
#	cd ../../
#	if [ $KEEP_BUILD_DIR == "n" ] ; then
#		rm -rf build
#	fi
#	cd ${CURRENTDIR}
#	
#	lzmaOK=$libOK
#
#fi

# lzma_CFLAGS=
# #lzma_LDFLAGS="-llzma"
# lzma_LDFLAGS=
# if [ -e ${CURRENTDIR}/lzma/lib/liblzma.a ] ; then
	# lzma_CFLAGS="-I${CURRENTDIR}/lzma/include"
	# lzma_LDFLAGS="-L${CURRENTDIR}/lzma/lib -llzma"
# fi
# echo -e "\n\n\n USING THESE lzma compiler commands:\nCFLAGS="
# echo $lzma_CFLAGS
# echo -e "\nLDFLAGS="
# echo $lzma_LDFLAGS
# echo -e "\n\n\n"
print_result "lzma (old age)" $lzmaOK



 lmfitOK=-1
# if [ $BUILD_ALL == "y" ] ; then
	# INSTALL_ANSWER="y"
# else
	# read -p "Do you want to build 'lmfit' (y/n)? " -n 1 INSTALL_ANSWER
	# echo -e  "\n"
# fi
# if [ $INSTALL_ANSWER == "y" ] ; then
	# echo -e  "\n------------------------------------------------------------------------\n"\
	# "-- BUILDING: lmfit                                                    --\n"\
	# "------------------------------------------------------------------------\n\n"\

	# cd lmfit
	# mkdir build
	# tar xvf lmfit-3.2.tar -C ./build/
	# cd build/lmfit-3.2
	# export LDFLAGS="${LDFLAGS} ${PICFLAGS} -lm"
	# export CFLAGS="${CFLAGS} ${PICFLAGS} "
	# export CPPFLAGS="${CPPFLAGS} ${PICFLAGS}"
	# ./configure --enable-static --disable-shared --prefix=${CURRENTDIR}/lmfit  CFLAGS="${PICFLAGS} ${MORECFLAGS}" CPPFLAGS="${PICFLAGS} ${MORECFLAGS}"	LDFLAGS="${PICFLAGS} ${MORELDFLAGS} -lm" 
	# libOK=$?
	# if [ $libOK -eq 0 ] ; then
		#  make -j${MAKE_PARALLEL_BUILDS}
		
		# libOK=$?
		# if [ $libOK -eq 0 ] ; then		
		    # mkdir ${CURRENTDIR}/lmfit/include
		    # cp ./lib/*.h ${CURRENTDIR}/lmfit/include
			# make -j${MAKE_PARALLEL_BUILDS} install
			# libOK=$?
			# if [ $libOK -ne 0 ] ; then		
				# libOK=-4
			# fi
		# else
			# libOK=-3
		# fi
	# else
	    # libOK=-2
	# fi
	

	# cd ../../
	# if [ $KEEP_BUILD_DIR == "n" ] ; then
		# rm -rf build
	# fi
	# cd ${CURRENTDIR}
	
	# lmfitOK=$libOK

# fi
print_result "lmfit(old age)" $lmfitOK



 lmfit5OK=-1
# if [ $BUILD_ALL == "y" ] ; then
	# INSTALL_ANSWER="y"
# else
	# read -p "Do you want to build 'lmfit v5' (y/n)? " -n 1 INSTALL_ANSWER
	# echo -e  "\n"
# fi
# if [ $INSTALL_ANSWER == "y" ] ; then
	# echo -e  "\n------------------------------------------------------------------------\n"\
	# "-- BUILDING: lmfit v5                                                 --\n"\
	# "------------------------------------------------------------------------\n\n"\

	# cd lmfit5
	# mkdir build
	# tar xvf lmfit-5.1.tar -C ./build/
	# cd build/lmfit-5.1
        # export LDFLAGS="${LDFLAGS} "
        # export CFLAGS="${CFLAGS} "
        # export CPPFLAGS="${CPPFLAGS} "
        # ./configure --enable-static --disable-shared --prefix=${CURRENTDIR}/lmfit6  CFLAGS="-Wno-error ${PICFLAGS} ${MORECFLAGS}" CPPFLAGS="-Wno-error ${PICFLAGS} ${MORECFLAGS}"	LDFLAGS="${PICFLAGS} ${MORELDFLAGS}"
	# libOK=$?
	# if [ $libOK -eq 0 ] ; then
		# make -j${MAKE_PARALLEL_BUILDS}
		
		# libOK=$?
		# if [ $libOK -eq 0 ] ; then		
		    # mkdir ${CURRENTDIR}/lmfit5/include
		    # cp ./lib/*.h ${CURRENTDIR}/lmfit5/include
			# make -j${MAKE_PARALLEL_BUILDS} install
			# libOK=$?
			# if [ $libOK -ne 0 ] ; then		
				# libOK=-4
			# fi
		# else
			# libOK=-3
		# fi
	# else
	    # libOK=-2
	# fi
	

	# cd ../../
	# if [ $KEEP_BUILD_DIR == "n" ] ; then
		# rm -rf build
	# fi
	# cd ${CURRENTDIR}
	
	# lmfit5OK=$libOK

# fi
print_result "lmfit v5(old age)" $lmfit5OK



lmfit6OK=-1
if [ $BUILD_ALL == "y" ] ; then
	INSTALL_ANSWER="y"
else
	read -p "Do you want to build 'lmfit v6.2' (y/n)? " -n 1 INSTALL_ANSWER
	echo -e  "\n"
fi
if [ $INSTALL_ANSWER == "y" ] ; then
	echo -e  "\n------------------------------------------------------------------------\n"\
	"-- BUILDING: lmfit v6.2                                                 --\n"\
	"------------------------------------------------------------------------\n\n"\

	cd lmfit6
	mkdir build
	tar xvf lmfit-6.2.tgz -C ./build/
	cd build/lmfit-6.2
        export LDFLAGS="${LDFLAGS} "
        export CFLAGS="${CFLAGS} "
        export CPPFLAGS="${CPPFLAGS} "
        ./configure --enable-static --disable-shared --prefix=${CURRENTDIR}/lmfit6  CFLAGS="-Wno-error ${PICFLAGS} ${MORECFLAGS}" CPPFLAGS="-Wno-error ${PICFLAGS} ${MORECFLAGS}"	LDFLAGS="${PICFLAGS} ${MORELDFLAGS}"
	libOK=$?
	if [ $libOK -eq 0 ] ; then
		make -j${MAKE_PARALLEL_BUILDS}
		
		libOK=$?
		if [ $libOK -eq 0 ] ; then		
		    mkdir ${CURRENTDIR}/lmfit6/include
		    cp ./lib/*.h ${CURRENTDIR}/lmfit6/include
			make -j${MAKE_PARALLEL_BUILDS} install
			libOK=$?
			if [ $libOK -ne 0 ] ; then		
				libOK=-4
			fi
		else
			libOK=-3
		fi
	else
	    libOK=-2
	fi
	

	cd ../../
	if [ $KEEP_BUILD_DIR == "n" ] ; then
		rm -rf build
	fi
	cd ${CURRENTDIR}
	
	lmfit6OK=$libOK

fi
print_result "lmfit v6" $lmfit6OK



levmarOK=-1
if [ $BUILD_ALL == "y" ] ; then
	INSTALL_ANSWER="y"
else
	read -p "Do you want to build 'levmar' (y/n)? " -n 1 INSTALL_ANSWER
	echo -e  "\n"
fi

if [ $INSTALL_ANSWER == "y" ] ; then
	echo -e  "\n------------------------------------------------------------------------\n"\
	"-- BUILDING: levmar                                                    --\n"\
	"------------------------------------------------------------------------\n\n"\

	
	cd levmar
	mkdir build
	tar xvf levmar-2.6.tar.gz -C ./build/
	cd build/levmar-2.6

	
	
	SKIP="0"
	HAVE_LAPACK="n"
	#read -p "Do you have 'lapack' on your system (y/n)? " -n 1 HAVE_LAPACK
	#echo -e  "\n"
	if [ $HAVE_LAPACK == "y" ] ; then
	
		echo -e  "Possibilities to link agains LAPACK/BLAS:\n"\
		"  1) simple LAPACK+BLAS (-llapack -lblas)\n"\
		"  2) simple LAPACK+CBLAS (-llapack -lcblas)\n"\
		"  3) F2C'd LAPACK (-llapack -lblas -lf2c)\n"\
		"  4) ATLAS LAPACK (-L/usr/local/atlas/lib -llapack -lcblas -lf77blas -latlas -lf2c)\n"\
		"\n  5) build without lapack\n"\
		"\n  6) skip levmar\n"

		read -p "Please enter your option: " -n 1 i

		#  #define HAVE_LAPACK is already set in the library header, so we don't need to change it there
		# we only need to alter the Makefile!
		#case $i in
		#1) sed 's/LAPACKLIBS=/LAPACKLIBS=-llapack -lblas#/g' Makefile > Makefile.temp | mv Makefile.temp Makefile ;;
		#2) sed 's/LAPACKLIBS=/LAPACKLIBS=-llapack -lcblas#/g' Makefile > Makefile.temp | mv Makefile.temp Makefile ;;
		#3) sed 's/LAPACKLIBS=/LAPACKLIBS=-llapack -lblas -lf2c#/g' Makefile > Makefile.temp | mv Makefile.temp Makefile ;;
		#4) sed 's/LAPACKLIBS=/LAPACKLIBS=-L\/usr\/local\/atlas\/lib -llapack -lcblas -lf77blas -latlas -lf2c#/g' Makefile > Makefile.temp | mv Makefile.temp Makefile ;;
		#5) HAVE_LAPACK="n" ;;
		#6) SKIP="1" ;;
		#esac
		case $i in
		1) echo "LAPACKLIBS=-llapack -lblas" > Makefile; cat ../../buildscript_tools/Makefile >> Makefile; ;;
		2) echo "LAPACKLIBS=-llapack -lcblas" > Makefile; cat ../../buildscript_tools/Makefile >> Makefile; ;;
		3) echo "LAPACKLIBS=-llapack -lblas -lf2c" > Makefile; cat ../../buildscript_tools/Makefile >> Makefile; ;;
		4) echo "LAPACKLIBS=-L\/usr\/local\/atlas\/lib -llapack -lcblas -lf77blas -latlas -lf2c" > Makefile; cat ../../buildscript_tools/Makefile >> Makefile; ;;
		5) HAVE_LAPACK="n" ;;
		6) SKIP="1" ;;
		esac
	fi	
	if [ $HAVE_LAPACK == "n" ] ; then
		# and alter the Makefile
		cp ../../buildscript_tools/levmar.h .
		echo "LAPACKLIBS=" > Makefile; cat ../../buildscript_tools/Makefile >> Makefile; 
	else
		cp ../../buildscript_tools/levmar.h ./levmar2.h
		sed 's/\/\/\#define HAVE\_LAPACK/\#define HAVE\_LAPACK/g' levmar2.h >> levmar.h
		rm ./levmar2.h
		
	fi 
	echo "OPTIMIZATIONFLAGS=${MORECFLAGS}"|cat - Makefile > Makefile.tmp && mv Makefile.tmp Makefile
	
	if [ $ISMSYS == "1" ] ; then
		echo "CXX=gcc"|cat - Makefile > Makefile.tmp && mv Makefile.tmp Makefile # Dirty workaround to prevent cxx compilation of c code happening in mingw32-make.exe
		echo -e "Dirty workaround on Windows, shoving in gcc as CXX compiler\n"
	fi

	if [ $SKIP == "0" ] ; then
		make -j${MAKE_PARALLEL_BUILDS} 
		
		libOK=$?
		if [ $libOK -eq 0 ] ; then		

			mkdir ../../lib
			mkdir ../../include
			mkdir ../../include/levmar
			cp *.a ../../lib/
			cp levmar.h ../../include/
			cp levmar.h ../../include/levmar/
			libOK=$?
			if [ $libOK -ne 0 ] ; then		
				libOK=-4
			fi
		else
			libOK=-3
		fi
		levmarOK=$libOK
	fi
	cd ../../
	if [ $KEEP_BUILD_DIR == "n" ] ; then
		rm -rf build
	fi
	cd ${CURRENTDIR}

fi
print_result "levmar" $levmarOK



libpngOK=-1
if [ $BUILD_ALL == "y" ] ; then
	INSTALL_ANSWER="y"
else
	read -p "Do you want to build 'libpng' (y/n)? " -n 1 INSTALL_ANSWER
	echo -e  "\n"
fi
if [ $INSTALL_ANSWER == "y" ] ; then
	echo -e  "\n------------------------------------------------------------------------\n"\
	"-- BUILDING: libpng                                                    --\n"\
	"------------------------------------------------------------------------\n\n"\

	cd libpng
	mkdir build
	tar xvf libpng-1.6.35.tar.gz -C ./build/
	cd build/libpng-1.6.35
	ZLIBINC=../../../zlib/lib/include/
	ZLIBLIB=../../../zlib/lib/
	if [ -e ../../../zlib/lib/libz.a ] ; then
		echo -e "\nzlib build found in extlibs, using copy in /extlibs/zlib \n"
		./configure --enable-static --disable-shared --with-zlib-prefix=${CURRENTDIR}/zlib/ --prefix=${CURRENTDIR}/libpng LDFLAGS="${PICFLAGS} ${MORELDFLAGS}" CFLAGS="${PICFLAGS} ${MORECFLAGS}" CXXFLAGS="${PICFLAGS} ${MORECFLAGS}"
	else
		ZLIBINC=.
		ZLIBLIB=.
		./configure --enable-static --disable-shared --prefix=${CURRENTDIR}/libpng CFLAGS="${PICFLAGS} ${MORECFLAGS}" CPPFLAGS="${PICFLAGS} ${MORECFLAGS}"     LDFLAGS="${PICFLAGS} ${MORELDFLAGS}"
	fi
	libOK=$?
	if [ $libOK -eq 0 ] ; then
		make -j${MAKE_PARALLEL_BUILDS}
		
		libOK=$?
		if [ $libOK -eq 0 ] ; then		
			make -j${MAKE_PARALLEL_BUILDS} install
			libOK=$?
			if [ $libOK -ne 0 ] ; then		
				libOK=-4
			fi
		else
			libOK=-3
		fi
	else
		echo "CONFIGURE-script failes. We will try it with a simple Makefile now!"
		

		echo -e 'ZLIBINC='$ZLIBINC'\nZLIBLIB='$ZLIBLIB | cat - ../../makefile.gcc > ./makefile.gcc
		
		make -j${MAKE_PARALLEL_BUILDS} -f makefile.gcc
		
		libOK=$?
		if [ $libOK -ne 0 ] ; then		
			libOK=-4
		else
			mkdir ../../include
			mkdir ../../include/libpng16
			mkdir ../../lib
			
			cp -f *.a ../../lib &> /dev/null
			cp -f png*.h ../../include &> /dev/null
			cp -f png*.h ../../include/libpng16 &> /dev/null
		fi
	fi
	

	cd ../../
	if [ $KEEP_BUILD_DIR == "n" ] ; then
		rm -rf build
	fi
	cd ${CURRENTDIR}
	
	libpngOK=$libOK

fi
print_result "libpng" $libpngOK



libJPEGOK=-1
if [ $BUILD_ALL == "y" ] ; then
	INSTALL_ANSWER="y"
else
	read -p "Do you want to build 'libJPEG' (y/n)? " -n 1 INSTALL_ANSWER
	echo -e  "\n"
fi
if [ $INSTALL_ANSWER == "y" ] ; then
	echo -e  "\n------------------------------------------------------------------------\n"\
	"-- BUILDING: libJPEG                                                  --\n"\
	"------------------------------------------------------------------------\n\n"\

	cd libjpeg
	mkdir build
	tar xvf jpegsrc.v9a.tar.gz -C ./build/
	cd build/jpeg-9a
	if [ -e ../../../zlib/lib/libz.a ] ; then
		./configure --enable-static --disable-shared --disable-dependency-tracking --prefix=${CURRENTDIR}/libjpeg LDFLAGS="${PICFLAGS} ${MORELDFLAGS} -L${CURRENTDIR}/zlib/lib" CFLAGS=" ${PICFLAGS} ${MORECFLAGS} -I${CURRENTDIR}/zlib/include" CXXFLAGS="${PICFLAGS} ${MORECFLAGS} -I${CURRENTDIR}/zlib/include/"
	else
		./configure --enable-static --disable-shared --disable-dependency-tracking --prefix=${CURRENTDIR}/libjpeg CFLAGS="${PICFLAGS} ${MORECFLAGS}" CPPFLAGS="${PICFLAGS} ${MORECFLAGS}"     LDFLAGS="${PICFLAGS} ${MORELDFLAGS}"
	fi
	libOK=$?
	if [ $libOK -eq 0 ] ; then
		make -j${MAKE_PARALLEL_BUILDS}
		
		libOK=$?
		if [ $libOK -eq 0 ] ; then		
			make -j${MAKE_PARALLEL_BUILDS} install
			libOK=$?
			if [ $libOK -ne 0 ] ; then		
				libOK=-4
			fi
		else
			libOK=-3
		fi
	else
	    libOK=-2
	fi
	

	cd ../../
	if [ $KEEP_BUILD_DIR == "n" ] ; then
		rm -rf build
	fi
	cd ${CURRENTDIR}
	
	libJPEGOK=$libOK

fi

LIBJPEG_CONFIGFLAGS=
if [ -e ${CURRENTDIR}/libjpeg/lib/libjpeg.a ] ; then
	LIBJPEG_CONFIGFLAGS="--with-jpeg-include-dir=${CURRENTDIR}/libjpeg/include --with-jpeg-lib-dir=${CURRENTDIR}/libjpeg/lib"
fi
print_result "libjpeg" $libJPEGOK



libtiffOK=-1
if [ $BUILD_ALL == "y" ] ; then
	INSTALL_ANSWER="y"
else
	read -p "Do you want to build 'libtiff' (y/n)? " -n 1 INSTALL_ANSWER
	echo -e  "\n"
fi
if [ $INSTALL_ANSWER == "y" ] ; then
	echo -e  "\n------------------------------------------------------------------------\n"\
	"-- BUILDING: libtiff                                                   --\n"\
	"------------------------------------------------------------------------\n\n"\

	cd libtiff
	mkdir build
	tar xvf tiff-4.0.10.tar.gz -C ./build/
	cd build/tiff-4.0.10
	if [ -e ${CURRENTDIR}/libjpeg/lib/libjpeg.a ] ; then
		./configure --enable-static --disable-shared  --enable-jpeg --enable-old-jpeg --disable-jbig ${zlib_CONFIG}  ${LIBJPEG_CONFIGFLAGS} --prefix=${CURRENTDIR}/libtiff   CFLAGS="${PICFLAGS} ${MORECFLAGS}" CPPFLAGS="${PICFLAGS} ${MORECFLAGS}"     LDFLAGS="${PICFLAGS} ${MORELDFLAGS}"
	else
		./configure --enable-static --disable-shared  --disable-jbig ${zlib_CONFIG}  ${LIBJPEG_CONFIGFLAGS} --prefix=${CURRENTDIR}/libtiff   CFLAGS="${PICFLAGS} ${MORECFLAGS}" CPPFLAGS="${PICFLAGS} ${MORECFLAGS}"     LDFLAGS="${PICFLAGS} ${MORELDFLAGS}"
	fi
	
	libOK=$?
	if [ $libOK -eq 0 ] ; then
		make 
		
		libOK=$?
		if [ $libOK -eq 0 ] ; then		
			make  install
			libOK=$?
			if [ $libOK -ne 0 ] ; then		
				libOK=-4
			fi
		else
			libOK=-3
		fi
	else
	    libOK=-2
	fi
	

	cd ../../
	if [ $KEEP_BUILD_DIR == "n" ] ; then
		rm -rf build
	fi
	cd ${CURRENTDIR}
	
	libtiffOK=$libOK

fi
print_result "libtiff" $libtiffOK



libgslOK=-1
if [ $BUILD_ALL == "y" ] ; then
	INSTALL_ANSWER="y"
else
	read -p "Do you want to build 'gsl' (y/n)? " -n 1 INSTALL_ANSWER
	echo -e  "\n"
fi
if [ $INSTALL_ANSWER == "y" ] ; then
	echo -e  "\n------------------------------------------------------------------------\n"\
	"-- BUILDING: gsl                                                       --\n"\
	"------------------------------------------------------------------------\n\n"\

	cd gsl
	mkdir build
	tar xvf gsl-1.16.tar.gz -C ./build/
	cd build/gsl-1.16
	./configure --enable-static --disable-shared --prefix=${CURRENTDIR}/gsl   CFLAGS="${PICFLAGS} ${MORECFLAGS}" CPPFLAGS="${PICFLAGS} ${MORECFLAGS}"  LDFLAGS="${MORELDFLAGS}"
	libOK=$?
	if [ $libOK -eq 0 ] ; then
		make -j${MAKE_PARALLEL_BUILDS}
		
		libOK=$?
		if [ $libOK -eq 0 ] ; then		
			make -j${MAKE_PARALLEL_BUILDS} install
			libOK=$?
			if [ $libOK -ne 0 ] ; then		
				libOK=-4
			fi
		else
			libOK=-3
		fi
	else
	    libOK=-2
	fi
	

	cd ../../
	if [ $KEEP_BUILD_DIR == "n" ] ; then
		rm -rf build
	fi
	cd ${CURRENTDIR}
	
	libgslOK=$libOK

fi

gsl_CFLAGS=
gsl_LDDIRS=
if [ -e ${CURRENTDIR}/gsl/lib/libgsl.a ] ; then
	gsl_CFLAGS="-I${CURRENTDIR}/gsl/include"
	gsl_LDDIRS="-L${CURRENTDIR}/gsl/lib"
fi
print_result "gsl" $libgslOK


libusbOK=-1
if [ $BUILD_ALL == "y" ] ; then
	INSTALL_ANSWER="y"
else
	read -p "Do you want to build 'libusb' (y/n)? " -n 1 INSTALL_ANSWER
	echo -e  "\n"
fi
if [ $INSTALL_ANSWER == "y" ] ; then

	echo -e  "------------------------------------------------------------------------\n"\
	"-- BUILDING: libusb                                                   --\n"\
	"------------------------------------------------------------------------\n\n"\

	cd libusb
	mkdir build
	mkdir lib
	mkdir bin
	mkdir include
	tar xvf ./libusbx-1.0.16.tar.gz -C ./build/
	cd build/libusbx-1.0.16/
	./configure --prefix=${CURRENTDIR}/libusb  CFLAGS="${MORECFLAGS}" CPPFLAGS="${MORECFLAGS}"
	libOK=$?
	if [ $libOK -eq 0 ] ; then
		make -j${MAKE_PARALLEL_BUILDS}
		
		libOK=$?
		if [ $libOK -eq 0 ] ; then		
			make -j${MAKE_PARALLEL_BUILDS} install
			libOK=$?
			if [ $libOK -ne 0 ] ; then		
				libOK=-4
			fi
		else
			libOK=-3
		fi
	else
		libOK=-2
	fi
	cd ../../
	tar xvf ./libusb-compat-0.1.5.tar.gz -C ./build/
	cp fix/libusb-compat-0.1.5/* build/libusb-compat-0.1.5/libusb/
	cd build/libusb-compat-0.1.5/
	./configure --prefix=${CURRENTDIR}/libusb  CFLAGS=" ${MORECFLAGS}" CPPFLAGS="${MORECFLAGS}"  LIBUSB_1_0_CFLAGS="-I${CURRENTDIR}/libusb/include/libusb-1.0" LIBUSB_1_0_LIBS="-L${CURRENTDIR}/libusb/lib -lusb-1.0"
	libOK=$?
	if [ $libOK -eq 0 ] ; then
		make -j${MAKE_PARALLEL_BUILDS}
		
		libOK=$?
		if [ $libOK -eq 0 ] ; then		
			make -j${MAKE_PARALLEL_BUILDS} install
			libOK=$?
			if [ $libOK -ne 0 ] ; then		
				libOK=-4
			fi
		else
			libOK=-3
		fi
	else
		libOK=-2
	fi
	cd ../../
	
	
	if [ $KEEP_BUILD_DIR == "n" ] ; then
		rm -rf build
	fi
	cd ${CURRENTDIR}
	cp libusb/bin/* ../output
	
	libusbOK=$libOK
fi
print_result "libusb" $libusbOK



eigenOK=-1
if [ $BUILD_ALL == "y" ] ; then
	INSTALL_ANSWER="y"
else
	read -p "Do you want to build 'eigen' (y/n)? " -n 1 INSTALL_ANSWER
	echo -e  "\n"
fi
if [ $INSTALL_ANSWER == "y" ] ; then
	echo -e  "------------------------------------------------------------------------\n"\
	"-- BUILDING: eigen                                                    --\n"\
	"------------------------------------------------------------------------\n\n"\

	cd eigen
	mkdir build
	mkdir doc
	mkdir include
	mkdir include/Eigen
	tar xvf ./eigen-3.2.5.tar.gz -C ./build/
	cd build/eigen-3.2.5/
	cp -r ./Eigen/* ../../include/Eigen
	libOK=$?
	cp -r ./doc/* ../../doc
	if [ $libOK -ne 0 ] ; then		
		libOK=-4
	else
		libOK=0
	fi
	cd ../../
	if [ $KEEP_BUILD_DIR == "n" ] ; then
		rm -rf build
	fi
	cd ${CURRENTDIR}
	
	eigenOK=$libOK

fi
print_result "eigen" $eigenOK



libnloptOK=-1
if [ $BUILD_ALL == "y" ] ; then
	INSTALL_ANSWER="y"
else
	read -p "Do you want to build 'NLopt' (y/n)? " -n 1 INSTALL_ANSWER
	echo -e  "\n"	
fi
if [ $INSTALL_ANSWER == "y" ] ; then
	echo -e  "\n------------------------------------------------------------------------\n"\
	"-- BUILDING: nlopt                                                     --\n"\
	"------------------------------------------------------------------------\n\n"\

	cd nlopt
	mkdir build
	tar xvf nlopt-2.4.2.tar.gz -C ./build/
	cd build/nlopt-2.4.2
	./configure --enable-static --disable-shared --prefix=${CURRENTDIR}/nlopt  --with-cxx --with-mthreads --without-guile --without-python --without-octave --without-matlab CFLAGS="${PICFLAGS} ${MORECFLAGS}" CPPFLAGS="${PICFLAGS} ${MORECFLAGS}" LDFLAGS="${MORELDFLAGS}"
	libOK=$?
	if [ $libOK -eq 0 ] ; then
		make -j${MAKE_PARALLEL_BUILDS}
		
		libOK=$?
		if [ $libOK -eq 0 ] ; then		
			make -j${MAKE_PARALLEL_BUILDS} install
			libOK=$?
			if [ $libOK -ne 0 ] ; then		
				libOK=-4
			fi
		else
			libOK=-3
		fi
	else
	    libOK=-2
	fi
	

	cd ../../
	if [ $KEEP_BUILD_DIR == "n" ] ; then
		rm -rf build
	fi
	cd ${CURRENTDIR}
# 	
# 	if [ $libOK != 0 ] ; then
# 	  echo -e  "\n------------------------------------------------------------------------\n"\
# 	  "-- BUILDING: UNOFFICIAL nlopt                                         --\n"\
# 	  "------------------------------------------------------------------------\n\n"
# 	  
# 	  pwd
# 	  cd nlopt
# 	  rm -rf build
# 	  mkdir -p build
# 	  pwd
# 	  ls -l
# 	  tar xvf nlopt-master.tar.gz -C ./build/
# 	  cd build/nlopt-master
# 	  sh autogen.sh
# 	  ./configure --enable-static --disable-shared --prefix=${CURRENTDIR}/nlopt  --with-cxx --with-mthreads --without-guile --without-python --without-octave --without-matlab CFLAGS="${PICFLAGS} ${MORECFLAGS}" CPPFLAGS="${PICFLAGS} ${MORECFLAGS}" LDFLAGS="${MORELDFLAGS}"
# 	  libOK=$?
# 	  if [ $libOK -eq 0 ] ; then
# 		  make -j${MAKE_PARALLEL_BUILDS}
# 		  
# 		  libOK=$?
# 		  if [ $libOK -eq 0 ] ; then		
# 			  make -j${MAKE_PARALLEL_BUILDS} install
# 			  libOK=$?
# 			  if [ $libOK -ne 0 ] ; then		
# 				  libOK=-4
# 			  fi
# 		  else
# 			  libOK=-3
# 		  fi
# 	  else
# 	      libOK=-2
# 	  fi
# 	  
# 
# 	  cd ../../
# 	  if [ $KEEP_BUILD_DIR == "n" ] ; then
# 		  rm -rf build
# 	  fi
# 	  cd ${CURRENTDIR}
# 	fi
	
	
	libnloptOK=$libOK

fi
print_result "nlopt" $libnloptOK



libOOLOK=-1
if [ $BUILD_ALL == "y" ] ; then
	INSTALL_ANSWER="y"
else
	read -p "Do you want to build 'OOL' (y/n)? " -n 1 INSTALL_ANSWER
	echo -e  "\n"
fi
if [ $INSTALL_ANSWER == "y" ] ; then
	echo -e  "\n------------------------------------------------------------------------\n"\
	"-- BUILDING: OOL                                                       --\n"\
	"------------------------------------------------------------------------\n\n"\

	cd ool
	mkdir build
	tar xvf ool-0.2.0.tar.gz -C ./build/
	cd build/ool-0.2.0
	./configure --enable-static --disable-shared --prefix=${CURRENTDIR}/ool  CFLAGS="${PICFLAGS} ${MORECFLAGS} ${gsl_CFLAGS}" CPPFLAGS="${PICFLAGS} ${MORECFLAGS} ${gsl_LDDIRS}" LDFLAGS="${MORELDFLAGS}"
	libOK=$?
	if [ $libOK -eq 0 ] ; then
		make -j${MAKE_PARALLEL_BUILDS}
		
		libOK=$?
		if [ $libOK -eq 0 ] ; then		
			make -j${MAKE_PARALLEL_BUILDS} install
			libOK=$?
			if [ $libOK -ne 0 ] ; then		
				libOK=-4
			fi
		else
			libOK=-3
		fi
	else
	    libOK=-2
	fi
	

	cd ../../
	if [ $KEEP_BUILD_DIR == "n" ] ; then
		rm -rf build
	fi
	cd ${CURRENTDIR}
	
	libOOLOK=$libOK

fi
print_result "OOL" $libOOLOK



cimgOK=-1
if [ $BUILD_ALL == "y" ] ; then
	INSTALL_ANSWER="y"
else
	read -p "Do you want to build 'cimg' (y/n)? " -n 1 INSTALL_ANSWER
	echo -e  "\n"
fi
if [ $INSTALL_ANSWER == "y" ] ; then
	echo -e  "------------------------------------------------------------------------\n"\
	"-- BUILDING: cimg                                                     --\n"\
	"------------------------------------------------------------------------\n\n"\

	cd cimg
	mkdir build
	tar xvf ./CImg-1.6.5.tar.gz -C ./build/
	cd build/CImg-1.6.5/
	cp -r * ../../
	libOK=$?
	if [ $libOK -ne 0 ] ; then		
		libOK=-4
	else
		libOK=0
	fi
	cd ../../
	if [ $KEEP_BUILD_DIR == "n" ] ; then
		rm -rf build
	fi
	cd ${CURRENTDIR}
	
	cimgOK=$libOK

fi
print_result "cimg" $cimgOK



libpixmanOK=-1
if [ $BUILD_ALL == "y" ] ; then
	INSTALL_ANSWER="y"
else
	read -p "Do you want to build 'pixman' (y/n)? " -n 1 INSTALL_ANSWER
	echo -e  "\n"
fi
if [ $INSTALL_ANSWER == "y" ] ; then
	echo -e  "\n------------------------------------------------------------------------\n"\
	"-- BUILDING: pixman                                                   --\n"\
	"------------------------------------------------------------------------\n\n"\

	cd pixman
	mkdir build
	tar xvf pixman-0.32.8.tar.gz -C ./build/
	cd build/pixman-0.32.8
	./configure --enable-static --disable-shared --enable-libpng --disable-gtk  --disable-dependency-tracking --prefix=${CURRENTDIR}/pixman   CFLAGS="${PICFLAGS} ${MORECFLAGS} ${zlib_CFLAGS}" CPPFLAGS="${PICFLAGS} ${MORECFLAGS} ${zlib_CFLAGS}" LDFLAGS=" ${zlib_LDFLAGS} ${MORELDFLAGS}" PNG_LIBS="-L${CURRENTDIR}/libpng -L${CURRENTDIR}/libpng/lib -lpng ${zlib_LDFLAGS}" PNG_CFLAGS=" -I${CURRENTDIR}/libpng -I${CURRENTDIR}/libpng/include"
	libOK=$?
	if [ $libOK -eq 0 ] ; then
		make -j${MAKE_PARALLEL_BUILDS}
		
		libOK=$?
		if [ $libOK -eq 0 ] ; then		
			make -j${MAKE_PARALLEL_BUILDS} install
			libOK=$?
			if [ $libOK -ne 0 ] ; then		
				libOK=-4
			fi
		else
			libOK=-3
		fi
	else
	    libOK=-2
	fi
	

	cd ../../
	if [ $KEEP_BUILD_DIR == "n" ] ; then
		rm -rf build
	fi
	cd ${CURRENTDIR}
	
	libpixmanOK=$libOK

fi
print_result "pixman" $libpixmanOK



libcairoOK=-1
if [ $BUILD_ALL == "y" ] ; then
	INSTALL_ANSWER="y"
else
	read -p "Do you want to build 'cairo' (y/n)? " -n 1 INSTALL_ANSWER
	echo -e  "\n"
fi
if [ $INSTALL_ANSWER == "y" ] ; then
	echo -e  "\n------------------------------------------------------------------------\n"\
	"-- BUILDING: cairo                                                   --\n"\
	"------------------------------------------------------------------------\n\n"\

	cd cairo
	mkdir build
	
    # faked pkg-config ... hack by http://www.kyngchaos.com/macosx/build/cairo
	echo "if [ \"\$1\" = \"--atleast-pkgconfig-version\" ] ; then
	echo 1
	fi
	if [ \"\$1\" = \"--exists\" ] ; then
	echo 0
	return 1
	fi" > ./build/pkgconfigfake
	chmod +x ./build/pkgconfigfake
	
	tar xvf cairo-1.14.2.tar.gz -C ./build/
	cd build/cairo-1.14.2
	if [ "$ISMSYS" == "1" ] ; then
	    cp ../../Makefile.in_hack ./Makefile.in
		./configure --verbose --enable-static --disable-shared  --disable-ft --disable-dependency-tracking --enable-gobject=no --enable-trace=no --enable-xcb-shm=no --enable-xlib=no --enable-xlib-xrender=no --enable-xcb=no --enable-egl=no --enable-glx=no --enable-wgl=no --enable-ft=no  --enable-fc=no --disable-xlib --without-x --disable-quartz-font --disable-quartz --disable-valgrind --prefix=${CURRENTDIR}/cairo   CFLAGS="${PICFLAGS} ${MORECFLAGS} ${zlib_CFLAGS} -I${CURRENTDIR}/pixman/include/ -I${CURRENTDIR}/pixman/include/pixman -I${CURRENTDIR}/pixman/include/pixman-1 " CPPFLAGS="${PICFLAGS} ${MORECFLAGS} ${zlib_CFLAGS} -I${CURRENTDIR}/pixman/include/ -I${CURRENTDIR}/pixman/include/pixman -I${CURRENTDIR}pixman/include/pixman-1 " LDFLAGS=" ${zlib_LDFLAGS} -L${CURRENTDIR}/pixman/lib -lpixman-1 " png_REQUIRES=libpng png_LIBS="-lpng -L${CURRENTDIR}/libpng -L${CURRENTDIR}/libpng/lib " png_CFLAGS=" -I${CURRENTDIR}/libpng/include " pixman_CFLAGS=" -I${CURRENTDIR}pixman/include/pixman-1" pixman_LIBS=" -L${CURRENTDIR}/pixman/lib -lpixman-1" PKG_CONFIG="${CURRENTDIR}/cairo/build/pkgconfigfake" ax_cv_c_float_words_bigendian=no
	else 
        ./configure --verbose --enable-static --disable-shared --disable-dependency-tracking  --prefix=${CURRENTDIR}/cairo   CFLAGS="${PICFLAGS} ${MORECFLAGS} ${zlib_CFLAGS} -I${CURRENTDIR}/pixman/include/ -I${CURRENTDIR}/pixman/include/pixman -I${CURRENTDIR}/pixman/include/pixman-1 " CPPFLAGS="${PICFLAGS} ${MORECFLAGS} ${zlib_CFLAGS} -I${CURRENTDIR}/pixman/include/ -I${CURRENTDIR}/pixman/include/pixman -I${CURRENTDIR}pixman/include/pixman-1 " LDFLAGS=" ${zlib_LDFLAGS} -L${CURRENTDIR}/pixman/lib -lpixman-1 " png_REQUIRES=libpng png_LIBS="-lpng -L${CURRENTDIR}/libpng -L${CURRENTDIR}/libpng/lib " png_CFLAGS=" -I${CURRENTDIR}/libpng/include " pixman_CFLAGS=" -I${CURRENTDIR}pixman/include/pixman-1" pixman_LIBS=" -L${CURRENTDIR}/pixman/lib -lpixman-1"	ax_cv_c_float_words_bigendian=no
	fi
	libOK=$?
	if [ $libOK -eq 0 ] ; then
		make 
		
		libOK=$?
		if [ $libOK -eq 0 ] ; then		
			make install
			libOK=$?
			if [ $libOK -ne 0 ] ; then		
				libOK=-4
			fi
		else
			libOK=-3
		fi
	else
	    libOK=-2
	fi
	

	cd ../../
	if [ $KEEP_BUILD_DIR == "n" ] ; then
		rm -rf build
	fi
	cd ${CURRENTDIR}
	
	libcairoOK=$libOK

fi
print_result "cairo" $libcairoOK



echo -e  "\n------------------------------------------------------------------------\n"\
"-- BUILD RESULTS                                                      --\n"\
"------------------------------------------------------------------------\n\n"\

echo -e  "-- Windows only libraries                                             --\n"

print_result "libNIDAQmx" $libnidaqmxOK
print_result "libAndor" $libandorOK

echo -e  "\n-- Qt DLLS                                                            --\n"

print_result "\nQt DLLs copy" $qtOK

echo -e  "\n-- General Libraries                                                  --\n"

print_result "zlib" $zlibOK
print_result "lzma (old age)" $lzmaOK
print_result "lmfit(old age)" $lmfitOK
print_result "lmfit v5(old age)" $lmfit5OK
print_result "lmfit v6" $lmfit6OK
print_result "levmar" $levmarOK
print_result "libpng" $libpngOK
print_result "libjpeg" $libJPEGOK
print_result "libtiff" $libtiffOK
print_result "gsl" $libgslOK
print_result "libusb" $libusbOK
print_result "eigen" $eigenOK
print_result "nlopt" $libnloptOK
print_result "OOL" $libOOLOK
print_result "cimg" $cimgOK
print_result "pixman" $libpixmanOK
print_result "cairo" $libcairoOK


