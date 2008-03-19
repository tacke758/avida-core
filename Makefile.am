CXXFLAGS   = -Wall
MAKEFLAGS = -j2 -e CXXFLAGS -e VIEW -e VIEW_LIBS
VIEW = @VIEW@
VIEW_LIBS = @VIEW_LIBS@

export VIEW
export CXXFLAGS
export VIEW_LIBS

SUBDIRS = source

EXTRA_DIST = avida_doc.rules

primitive:
	VIEW=primitive; VIEW_LIBS= ; $(MAKE)

curses:
	VIEW=curses; VIEW_LIBS=-lcurses; $(MAKE)

ncurses:
	VIEW=ncurses; VIEW_LIBS=-lncurses; $(MAKE)

viewer-clean:
	$(MAKE) clean -C source/viewers; $(MAKE) clean -C source/main



