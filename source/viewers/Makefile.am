
CPPFLAGS = @CPPFLAGS@
#LDFLAGS = -lm $(VIEW_LIBS)
LDFLAGS = -lm -lncurses

viewer_SOURCES = ansi.cc		ansi.hh			\
		bar_screen.cc		bar_screen.hh		\
		environment_screen.cc	environment_screen.hh	\
		hist_screen.cc		hist_screen.hh		\
		map_screen.cc		map_screen.hh		\
		menu.cc			menu.hh			\
		ncurses.cc		ncurses.hh		\
		options_screen.cc	options_screen.hh	\
		stats_screen.cc		stats_screen.hh 	\
		symbol_util.cc		symbol_util.hh		\
		text_screen.cc		text_screen.hh		\
		view.cc			view.hh			\
		viewer.cc		viewer.hh		\
		zoom_screen.cc		zoom_screen.hh		\
					text_window.hh		


viewer_LDADD = ../main/libmain.a		\
		../cpu/libcpu.a   		\
		../event/libevent.a          	\
		../main/libmain.a		\
		../tools/libtools.a \
		../third-party/trio-1.9/libtrio.a


bin_PROGRAMS = viewer

EXTRA_DIST = viewers.pro
