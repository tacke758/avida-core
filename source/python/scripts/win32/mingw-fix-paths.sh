#!/bin/sh
# Run in build/source/bindings/Boost.Python/CMakeFiles/AvidaCore

# Fix linking errors with libboost_python-mgw.lib
perl -i -pe 's/-llibboost_python-mgw.lib/-llibboost_python-mgw/g' */build.make

# Fix linking errors with python24.lib
perl -i -pe 's/-lpython24.lib/-lpython24/g' */build.make

# Fix path issues in makefiles
# Fixes "No rule to make target `/cygwin/..."
perl -i -pe 's/ \/cygwin/ c:\/cygwin/g' */build.make
