
AVIDA_SRC_DIR = ..

TEMPLATE    = app
TARGET      = ncurses-viewer
CONFIG      *= warn-on
CONFIG      *= cpu event main trio tools
LIBS        += -lncurses

include($$AVIDA_SRC_DIR/modules.pri)

HEADERS +=  ansi.hh \
            bar_screen.hh \
            hist_screen.hh \
            map_screen.hh \
            menu.hh \
            ncurses.hh \
            options_screen.hh \
            stats_screen.hh \
            symbol_util.hh \
            text_screen.hh \
            text_window.hh \
            view.hh \
            viewer.hh \
            zoom_screen.hh

SOURCES +=  ansi.cc \
            bar_screen.cc \
            hist_screen.cc \
            map_screen.cc \
            menu.cc \
            ncurses.cc \
            options_screen.cc \
            stats_screen.cc \
            symbol_util.cc \
            text_screen.cc \
            view.cc \
            viewer.cc \
            zoom_screen.cc \

