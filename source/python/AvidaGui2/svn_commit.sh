#!/bin/bash

SCRIPT_DIR=`dirname "$0"`

svn commit || (echo "Can't 'svn commit' Avida-ED."; exit 1)
revision_string=`svnversion` || (echo "Can't get Avida-ED's svn revision number."; exit 1)
echo "svn_revision_string = 'Avida-ED v2.0.$revision_string'" > "$SCRIPT_DIR"/Avida_ED_version.py
