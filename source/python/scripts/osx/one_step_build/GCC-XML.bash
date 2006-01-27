#!/bin/bash

# Installs GCC-XML from the specified archive.

usage()
{
	echo "Use: $0 -a <archive_path> -b <build_path> -s <source_subdir> [-v Y]" >&2
	echo "  -a <archive_path> -- full path to archive." >&2
	echo "  -b <build_path> -- full path to directory where build should take place." >&2
	echo "  -s <source_subdir> -- name of subdirectory containing source code." >&2
	echo "  -v Y -- be verbose." >&2
	exit 1
}

SCRIPT_DIR=`dirname "$0"`

# Parse command-line arguments
for arg
do
	case "$arg" in
		-a )	ARCHIVE_PATH=$2; shift 2;;
		-b )	BUILD_PATH=$2; shift 2;;
		-s )	SOURCE_SUBDIR=$2; shift 2;;
		-v )	DEBUG=$2; shift 2;;
	esac
done

if [ "${DEBUG}" == "Y" ]; then echo "I'm going to be verbose, to make debugging easier..."; set -x; fi
if [ $# != 0 ]; then usage; fi
if [ "$ARCHIVE_PATH" == "" ]; then usage; fi
if [ "$BUILD_PATH" == "" ]; then usage; fi
if [ "$SOURCE_SUBDIR" == "" ]; then usage; fi

SOURCE_PREFIX=`dirname "$ARCHIVE_PATH"`
SOURCE_PATH=$SOURCE_PREFIX/$SOURCE_SUBDIR

"$SCRIPT_DIR/unpack-tarball.bash" -a "$ARCHIVE_PATH" -b "$BUILD_PATH" -s "$SOURCE_SUBDIR" -v "$DEBUG"

mkdir -p "$BUILD_PATH/$SOURCE_SUBDIR"/build
pushd "$BUILD_PATH/$SOURCE_SUBDIR"/build
/usr/local/bin/cmake "$SOURCE_PATH" -DCMAKE_INSTALL_PREFIX:PATH=/usr/local
make
make install
