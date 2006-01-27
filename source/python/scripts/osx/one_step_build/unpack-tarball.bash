#!/bin/bash

# Installs from the specified archive.

usage()
{
	echo "Use: $0 -a <archive_path> -b <build_path> -s <source_subdir> [-v Y]" >&2
	echo "  -a <archive_path> -- full path to archive." >&2
	echo "  -b <build_path> -- full path to directory where build should take place." >&2
	echo "  -s <source_subdir> -- name of subdirectory containing source code." >&2
	echo "  -v Y -- be verbose." >&2
	exit 1
}

INSTALL_DONE=N

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

pushd "$SOURCE_PREFIX"
if [ -d "$SOURCE_PATH" ]
then
	echo "Trying to rename existing source subdirectory..."
	DATE_STR=`date +%m%d.%H%M%S`
	mv "$SOURCE_PATH" "$HOME/.Trash/src.$SOURCE_SUBDIR.$DATE_STR"
fi
if [ -d "$BUILD_PATH/$SOURCE_SUBDIR" ]
then
	echo "Trying to rename existing source subdirectory..."
	DATE_STR=`date +%m%d.%H%M%S`
	mv "$BUILD_PATH/$SOURCE_SUBDIR" "$HOME/.Trash/bld.$SOURCE_SUBDIR.$DATE_STR"
fi


ARCHIVE_UNPACKED=N
echo "Trying to upack archive as though it's a gzipped tarball..."
tar xfz $ARCHIVE_PATH
if [ $? != 0 ]
then
	echo "... Failed."
	echo "\"$ARCHIVE_PATH\" doesn't appear to be a gzipped tarball."
else
	echo "... Okay."
	ARCHIVE_UNPACKED=Y
fi
if [ "$ARCHIVE_UNPACKED" == "N" ]
then
	echo "Trying to upack archive as though it's a bzipped tarball..."
	tar xfj $ARCHIVE_PATH
	if [ $? != 0 ]
	then
		echo "... Failed."
		echo "\"$ARCHIVE_PATH\" doesn't appear to be a bzipped tarball."
	else
		echo "... Okay."
		ARCHIVE_UNPACKED=Y
	fi
fi
if [ "$ARCHIVE_UNPACKED" == "N" ]
then
	echo "Trying to upack archive using OSX's \"BOMArchiveHelper\" application..."
	pushd "$HOME/.Trash"
	/System/Library/CoreServices/BOMArchiveHelper.app/Contents/MacOS/BOMArchiveHelper "$ARCHIVE_PATH"
	if [ $? != 0 ]
	then
		echo "... Failed."
		echo "\"BOMArchiveHelper\" application can't figure out how to open \"$ARCHIVE_PATH\"."
	else
		echo "... Okay."
		ARCHIVE_UNPACKED=Y
	fi
	popd
fi


if [ "$ARCHIVE_UNPACKED" == "N" ]
then
	echo "Could not unpack tarball."
	exit 1
else
	echo "Tarball unpacked."
	exit 0
fi
