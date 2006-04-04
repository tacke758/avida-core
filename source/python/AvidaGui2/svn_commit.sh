#!/bin/bash

SCRIPT_DIR=`dirname "$0"`

svn commit || (echo "Can't 'svn commit' Avida-ED."; exit 1)
${SCRIPT_DIR}/svn_revision.sh
