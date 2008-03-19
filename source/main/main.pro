
AVIDA_SRC_DIR = ..

TEMPLATE    = app
TARGET      = primitive
CONFIG      *= warn-on release
CONFIG      *= cpu event main trio tools

include($$AVIDA_SRC_DIR/modules.pri)

SOURCES += primitive.cc
