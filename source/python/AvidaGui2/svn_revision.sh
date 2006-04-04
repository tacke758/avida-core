#!/bin/bash

SCRIPT_DIR=`dirname "$0"`

revision_string=`svnversion "$SCRIPT_DIR"` || (echo "Can't get Avida-ED's svn revision number."; exit 1)
echo "svn_revision_string = '$revision_string'" > "$SCRIPT_DIR"/Avida_ED_version.py
