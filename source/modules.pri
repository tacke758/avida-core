
CPU_CC    = $$AVIDA_SRC_DIR/cpu
EVENT_CC  = $$AVIDA_SRC_DIR/event
MAIN_CC   = $$AVIDA_SRC_DIR/main
TRIO_C    = $$AVIDA_SRC_DIR/third-party/trio-1.9
TOOLS_CC  = $$AVIDA_SRC_DIR/tools

CPU_HH    = $$CPU_CC
EVENT_HH  = $$EVENT_CC
MAIN_HH   = $$MAIN_CC
TRIO_H    = $$TRIO_C
TOOLS_HH  = $$TOOLS_CC

DEPENDPATH += ;$$AVIDA_SRC_DIR
DEPENDPATH += ;$$CPU_HH
DEPENDPATH += ;$$EVENT_HH
DEPENDPATH += ;$$MAIN_HH
DEPENDPATH += ;$$TRIO_H
DEPENDPATH += ;$$TOOLS_HH

INCLUDEPATH += $$DEPENDPATH

include($$CPU_CC/cpu.pri)
include($$EVENT_CC/event.pri)
include($$MAIN_CC/main_sub.pri)
include($$TRIO_C/trio.pri)
include($$TOOLS_CC/tools.pri)
