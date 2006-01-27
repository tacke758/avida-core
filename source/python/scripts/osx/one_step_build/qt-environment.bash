#!/bin/bash

# echos environment variables for accessing Qt.

usage()
{
	echo "Use: $0 -q <qt_dir> [-v Y]" >&2
	echo "  -q <qt_dir> -- full path to qt." >&2
	echo "  -v Y -- be verbose." >&2
	exit 1
}

# Parse command-line arguments
for arg
do
	case "$arg" in
		-q )	QTDIR=$2; shift 2;;
		-v )	DEBUG=$2; shift 2;;
	esac
done

if [ "${DEBUG}" == "Y" ]; then set -x; fi
if [ $# != 0 ]; then usage; fi
if [ "$QTDIR" == "" ]; then usage; fi

echo export QTDIR="$QTDIR"
echo export PATH=$QTDIR/bin:$PATH
if [ "$DYLD_LIBRARY_PATH" == "" ]
then
	echo export DYLD_LIBRARY_PATH=$QTDIR/lib
else
	echo export DYLD_LIBRARY_PATH=$QTDIR/lib:$DYLD_LIBRARY_PATH
fi

