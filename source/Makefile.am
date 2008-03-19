
SUBDIRS_PRIMITIVE = support
if BUILD_PRIMITIVE
SUBDIRS_PRIMITIVE += third-party tools cpu event main
endif

if VIEWER
SUBDIRS_VIEWERS = $(SUBDIRS_PRIMITIVE) viewers
else
SUBDIRS_VIEWERS = $(SUBDIRS_PRIMITIVE)
endif

if QT_VIEWER
SUBDIRS = $(SUBDIRS_VIEWERS) qt-viewer
else
SUBDIRS = $(SUBDIRS_VIEWERS)
endif

EXTRA_DIST = defs.hh modules.pri
