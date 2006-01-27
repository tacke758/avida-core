#!/bin/bash

# Installs the package from the specified disk image.

usage()
{
	echo "Use: $0 -a <archive_path> [-v Y]" >&2
	echo "  -a <archive_path> -- full path to disk image." >&2
	echo "  -p <pkg_name> -- name of package to search for." >&2
	echo "  -v Y -- be verbose." >&2
	exit 1
}

INSTALL_DONE=N

# Parse command-line arguments
for arg
do
	case "$arg" in
		-a )	ARCHIVE_PATH=$2; shift 2;;
		-p )	PKG_NAME=$2; shift 2;;
		-v )	DEBUG=$2; shift 2;;
	esac
done

if [ "${DEBUG}" == "Y" ]; then echo "I'm going to be verbose, to make debugging easier..."; set -x; fi
if [ $# != 0 ]; then usage; fi
if [ "$ARCHIVE_PATH" == "" ]; then usage; fi
if [ "$PKG_NAME" == "" ]; then usage; fi

# Mount archive
echo "Trying to mount archive..."
PUPPETSTRINGS=`hdiutil attach "$ARCHIVE_PATH" -puppetstrings`
if [ $? != 0 ]
then
	echo "... Failed."
	echo "Couldn't mount archive \"$ARCHIVE_PATH\"."
	exit 1
else
	echo "... Okay."
fi

# Locate and install package
MOUNTPOINT=`echo $PUPPETSTRINGS | sed -n 's/.*\(\/Volumes\/.*$\)/\1/p'`
echo "Trying to locate package..."
if [ $? != 0 ]
then
	echo "... Failed."
	echo "Couldn't find package under mountpoint \"$MOUNTPOINT\"."
else
	echo "... Okay."

	PACKAGE=`find "$MOUNTPOINT" -path "$PKG_NAME"`
	echo "Trying to install package..."
	installer -pkg "$PACKAGE" -target /
	if [ $? != 0 ]
	then
		echo "... Failed."
		echo "Couldn't install package \"$PACKAGE\""
	else
		echo "... Okay."
# Success!
		INSTALL_DONE=Y
	fi

fi

# Unmount archive
FILESYSTEM=`echo $PUPPETSTRINGS | awk '/\/dev\/disk?/ { print $1 }'`
echo "Trying to unmount archive..."
hdiutil detach "$FILESYSTEM"
if [ $? != 0 ]
then
	echo "... Failed."
	echo "Couldn't unmount archive \"$ARCHIVE_PATH\""
	echo "at filesystem \"$FILESYSTEM\"."
	exit 1
else
	echo "... Okay."
fi

if [ "${INSTALL_DONE}" == "Y" ]
then
	echo "Install succeeded."
	exit 0
else
	echo "Install failed."
	exit 1
fi

