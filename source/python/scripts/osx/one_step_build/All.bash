#!/bin/bash

usage()
{
	echo "Use: $0 [-a] [-b] [-v] [-p <svn_password>] [-r <revision_number] -t <path_to_archives_dir>" >&2
	echo "  -a -- build Avida-ED." >&2
	echo "  -b -- build the build-environment required to build Avida-ED." >&2
	echo "  -v -- be verbose." >&2
	echo "  -p <svn_password> -- password for svn user." >&2
	echo "  -r <revision_number> -- ." >&2
	echo "  -t <path_to_archives_dir> -- full path to directory containing archived third-party sources." >&2
	exit 1
}

# Parse command-line arguments
for arg
do
	case "$arg" in
		-a )	SHOULD_BUILD_AVIDA_ED=Y; shift 1;;
		-b )	SHOULD_BUILD_THIRD_PARTY=Y; shift 1;;
		-v )	DEBUG=Y; shift 1;; 
		-p )	SVN_PASSWORD=$2; shift 2;; 
		-r )	REVISION=$2; shift 2;;
		-t )	ARCHIVES_DIR=$2; shift 2;;
	esac
done

if [ "${DEBUG}" == "Y" ]
then
	echo "I'm going to be verbose, to make debugging easier..."
	set -x
else
	DEBUG=N
fi
if [ $# != 0 ]; then usage; fi
if [ "$SHOULD_BUILD_THIRD_PARTY" == "Y" ]
then
	if [ "$ARCHIVES_DIR" == "" ]; then usage; fi
fi
if [ "$SHOULD_BUILD_AVIDA_ED" == "Y" ]
then
	if [ "$SVN_PASSWORD" == "" ]; then usage; fi
fi

export GCCXML_COMPILER=g++-3.3
export MACOS_DEPLOYMENT_TARGET=10.4
export PATH=/usr/local/bin:$PATH

SCRIPT_DIR=`dirname "$0"`
AVIDA_SRC_DIR="$PWD"
AVIDA_BLD_DIR="$PWD/bld"
AVIDA_REPOSITORY="svn://myxo.css.msu.edu/avida2/branches/developers/avida-edward"
BOOST_VERSION_STRING=1_33

if [ "$SHOULD_BUILD_THIRD_PARTY" == "Y" ]
then
	"$SCRIPT_DIR"/Subversion.bash -a "$ARCHIVES_DIR"/subversion-client-1.2.3.dmg -p "*SubversionClient*.pkg" -v "$DEBUG"

	"$SCRIPT_DIR"/CMake.bash -a "$ARCHIVES_DIR"/cmake-2.0.5-osx.dmg -p "*CMakeInstaller*.pkg" -v "$DEBUG"

	#"$SCRIPT_DIR"/Xcode.bash -a "$ARCHIVES_DIR"/xcode_2.2_8m654_018213974.dmg -p "*XcodeTools*.mpkg" -v "$DEBUG"
	"$SCRIPT_DIR"/Xcode.bash -a "$ARCHIVES_DIR"/xcode_2.2.1_8g1165_018213632.dmg -p "*XcodeTools*.mpkg" -v "$DEBUG"

	"$SCRIPT_DIR"/ReadLine.bash -a "$ARCHIVES_DIR"/readline-5.0-4-osx4.tar.gz -b "$ARCHIVES_DIR" -s readline-5.0 -v "$DEBUG"

	"$SCRIPT_DIR"/Python.bash -a "$ARCHIVES_DIR"/Python-2.4.2.tar.bz2 -b "$ARCHIVES_DIR" -s Python-2.4.2 -v "$DEBUG"

	"$SCRIPT_DIR"/Qt.bash -a "$ARCHIVES_DIR"/qt-mac-free-3.3.5.tar.bz2 -b "$ARCHIVES_DIR" -s qt-mac-free-3.3.5 -q "$ARCHIVES_DIR"/qt-mac-free-3.3.5 -v "$DEBUG"

	"$SCRIPT_DIR"/SIP.bash -a "$ARCHIVES_DIR"/sip-4.3.2.tar.gz -b "$ARCHIVES_DIR" -s sip-4.3.2 -q "$ARCHIVES_DIR"/qt-mac-free-3.3.5 -v "$DEBUG"

	"$SCRIPT_DIR"/PyQt.bash -a "$ARCHIVES_DIR"/PyQt-mac-gpl-3.15.1.tar.gz -b "$ARCHIVES_DIR" -s PyQt-mac-gpl-3.15.1 -q "$ARCHIVES_DIR"/qt-mac-free-3.3.5 -v "$DEBUG"

	"$SCRIPT_DIR"/Numeric.bash -a "$ARCHIVES_DIR"/Numeric-23.6-osx3.tar.gz -b "$ARCHIVES_DIR" -s Numeric-23.6-osx3 -v "$DEBUG"

	"$SCRIPT_DIR"/PyQwt.bash -a "$ARCHIVES_DIR"/PyQwt-4.2.tar.gz -b "$ARCHIVES_DIR" -s PyQwt-4.2 -q "$ARCHIVES_DIR"/qt-mac-free-3.3.5 -v "$DEBUG"

	"$SCRIPT_DIR"/ElementTree.bash -a "$ARCHIVES_DIR"/elementtree-1.2.6-20050316.tar.gz -b "$ARCHIVES_DIR" -s elementtree-1.2.6-20050316 -v "$DEBUG"

	"$SCRIPT_DIR"/GCC-XML.bash -a "$ARCHIVES_DIR"/gccxml-0.6.0.tar.gz -b "$ARCHIVES_DIR" -s gccxml-0.6.0 -v "$DEBUG"

	"$SCRIPT_DIR"/Boost.bash -a "$ARCHIVES_DIR"/boost_1_33_0-osx4.tar.bz2 -b "$ARCHIVES_DIR" -s boost_1_33_0 -v "$DEBUG"

	"$SCRIPT_DIR"/Py2App.bash -a "$ARCHIVES_DIR"/py2app-0.1.7.tar.gz -b "$ARCHIVES_DIR" -s py2app-0.1.7 -v "$DEBUG"

	"$SCRIPT_DIR"/IPython.bash -a "$ARCHIVES_DIR"/ipython-0.6.15.tar.gz -b "$ARCHIVES_DIR" -s ipython-0.6.15 -v "$DEBUG"
fi

if [ "$SHOULD_BUILD_AVIDA_ED" == "Y" ]
then
	if [ "${DEBUG}" == "Y" ]; then set +x; fi
	if [ "$REVISION" == "" ]
	then
		"$SCRIPT_DIR"/Avida-ED.bash -u "$AVIDA_REPOSITORY" -p "$SVN_PASSWORD" -s "$AVIDA_SRC_DIR" -b "$AVIDA_BLD_DIR" -q "$ARCHIVES_DIR"/qt-mac-free-3.3.5 -n "$BOOST_VERSION_STRING" -v "$DEBUG"
	else
		"$SCRIPT_DIR"/Avida-ED.bash -u "$AVIDA_REPOSITORY" -r "$REVISION" -p "$SVN_PASSWORD" -s "$AVIDA_SRC_DIR" -b "$AVIDA_BLD_DIR" -q "$ARCHIVES_DIR"/qt-mac-free-3.3.5 -n "$BOOST_VERSION_STRING" -v "$DEBUG"
	fi
	if [ "${DEBUG}" == "Y" ]; then set -x; fi
fi
