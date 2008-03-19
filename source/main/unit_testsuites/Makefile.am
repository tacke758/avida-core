AUTOMAKE_OPTIONS = dejagnu
TESTSUITES =
EXTRA_DIST = dejagnu.h

EXTRA_DIST += demo.hello
noinst_PROGRAMS = hello
hello_SOURCES = demo.hello/hello.cc
if DEFAULT_TESTS
TESTSUITES += demo
endif

DEJATOOL = $(TESTSUITES)
AM_RUNTESTFLAGS = AVIDA_BLDDIR=`pwd`/..
