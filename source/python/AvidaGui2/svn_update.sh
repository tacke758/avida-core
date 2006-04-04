#!/bin/bash

SCRIPT_DIR=`dirname "$0"`

svn update || (echo "Can't 'svn update' Avida-ED."; exit 1)
${SCRIPT_DIR}/svn_revision.sh
