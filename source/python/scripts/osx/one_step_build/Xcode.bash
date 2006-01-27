#!/bin/bash

# Installs the Xcode package from the specified disk image.
SCRIPT_DIR=`dirname "$0"`
"$SCRIPT_DIR/pkg-install.bash" "$@"
