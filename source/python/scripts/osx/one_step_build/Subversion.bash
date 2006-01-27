#!/bin/bash

# Installs the Subversion Client package from the specified disk image.
SCRIPT_DIR=`dirname "$0"`
"$SCRIPT_DIR/pkg-install.bash" "$@"
