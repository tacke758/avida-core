#!/bin/bash

# Installs Python from the specified archive.

usage()
{
	echo "Use: $0 -u <repository_url> [-r <revision_number>] -p <svn_password> -b <build_path> -s <source_subdir> -q <qt_dir> -n <boost_version_number_string> [-v Y]" >&2
	echo "  -u <repository_url> -- full url to repository." >&2
	echo "  -r <revision_number> -- ." >&2
	echo "  -p <svn_password> -- password for svn user." >&2
	echo "  -s <source_dir> -- full path to directory where source code should be stored." >&2
	echo "  -b <build_dir> -- full path to directory where build should take place." >&2
	echo "  -q <qt_dir> -- full path to qt." >&2
	echo "  -n <boost_version_number_string>" >&2
	echo "  -v Y -- be verbose." >&2
	exit 1
}

SCRIPT_DIR=`dirname "$0"`

# Parse command-line arguments
for arg
do
	case "$arg" in
		-u )	REPOSITORY_URL=$2; shift 2;;
		-p )	SVN_PASSWORD=$2; shift 2;; 
		-r )	REVISION=$2; shift 2;;
		-s )	SOURCE_DIR=$2; shift 2;;
		-b )	BUILD_DIR=$2; shift 2;;
		-q )	QTDIR=$2; shift 2;;
		-n )	BOOST_VERSION=$2; shift 2;;
		-v )	DEBUG=$2; shift 2;;
	esac
done

if [ "${DEBUG}" == "Y" ]; then echo "I'm going to be verbose, to make debugging easier..."; set -x; fi
if [ $# != 0 ]; then usage; fi
if [ "$REPOSITORY_URL" == "" ]; then usage; fi
if [ "$SOURCE_DIR" == "" ]; then usage; fi
if [ "$BUILD_DIR" == "" ]; then usage; fi
if [ "$QTDIR" == "" ]; then usage; fi
if [ "$BOOST_VERSION" == "" ]; then usage; fi

`"$SCRIPT_DIR/qt-environment.bash" -q "$QTDIR" -v "$DEBUG"`
PYTHON_PREFIX=`/usr/local/bin/python -c "import distutils.sysconfig; print distutils.sysconfig.PREFIX"`
PYTHON_FRAMEWORK_PREFIX=`/usr/local/bin/python -c "import distutils.sysconfig; print distutils.sysconfig.get_config_var('PYTHONFRAMEWORKPREFIX')"`
PYUIC_EXE="$PYTHON_PREFIX/bin/pyuic"
BOOST_INCLUDE_PATH="/usr/local/include/boost-$BOOST_VERSION"
BOOST_LIBRARY="boost_python-$BOOST_VERSION"

mkdir -p "$SOURCE_DIR"
pushd "$SOURCE_DIR"

# Hide svn password...
if [ "${DEBUG}" == "Y" ]; then set +x; fi
if [ "$REVISION" == "" ]
then
	/usr/local/bin/svn checkout --username avidaedward --password "$SVN_PASSWORD" "$REPOSITORY_URL" Avida-ED
else
	/usr/local/bin/svn checkout -r "$REVISION" --username avidaedward --password "$SVN_PASSWORD" "$REPOSITORY_URL" Avida-ED
fi
if [ "${DEBUG}" == "Y" ]; then set -x; fi

if [ $? != 0 ]
then
	echo "... Failed."
	echo "Couldn't retrieve Avida-ED source code from repository URL \"$REPOSITORY_URL\"."
	exit 1
fi
popd

mkdir -p "$BUILD_DIR/Avida-ED"
pushd "$BUILD_DIR/Avida-ED"
rm -fr "$BUILD_DIR/Avida-ED/lib/AvidaCore"* "$BUILD_DIR/Avida-ED/bin/Avida-ED.app" "$BUILD_DIR/Avida-ED/source/python/build"
/usr/local/bin/cmake \
 -DAVD_GUI_NCURSES:BOOL=ON \
 -DAVD_GUI_PYQT:BOOL=ON \
 -DAVD_PRIMITIVE:BOOL=ON \
 -DAVD_PY_BINDINGS:BOOL=ON \
 -DAVD_PY_MONOLITHIC:BOOL=ON \
 -DAVD_PY_MODULE:BOOL=OFF \
 -DAVD_UNIT_TESTS:BOOL=OFF \
 -DBOOST_INCLUDE_PATH:PATH="$BOOST_INCLUDE_PATH" \
 -DBOOST_LIBRARY:FILEPATH="$BOOST_LIBRARY" \
 -DBOOST_VERSION:STRING="$BOOST_VERSION" \
 -DCMAKE_BACKWARDS_COMPATIBILITY:STRING=2.0 \
 -DCMAKE_BUILD_TYPE:STRING=Release \
 -DPYTHON_FRAMEWORK_PATH:PATH="$PYTHON_FRAMEWORK_PREFIX" \
 -DPYUIC:FILEPATH="$PYUIC_EXE" \
 "$SOURCE_DIR/Avida-ED" 

make -k

