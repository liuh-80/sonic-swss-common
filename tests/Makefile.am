INCLUDES = -I $(top_srcdir)

bin_PROGRAMS = tests

if DEBUG
DBGFLAGS = -ggdb -DDEBUG
else
DBGFLAGS = -g -DNDEBUG
endif

CFLAGS_GTEST =
LDADD_GTEST = -L/usr/src/gtest -lgtest -lgtest_main -lgmock -lgmock_main

tests_SOURCES = zmq_state_ut.cpp            \
                main.cpp

tests_CFLAGS = $(DBGFLAGS) $(AM_CFLAGS) $(CFLAGS_COMMON) $(CFLAGS_GTEST) $(LIBNL_CFLAGS)
tests_CPPFLAGS = $(DBGFLAGS) $(AM_CFLAGS) $(CFLAGS_COMMON) $(CFLAGS_GTEST) $(LIBNL_CFLAGS)
tests_LDADD = $(LDADD_GTEST) -lpthread -L$(top_srcdir)/common -lswsscommon $(LIBNL_LIBS) $(CODE_COVERAGE_LIBS) -L$(top_srcdir)/sonic-db-cli -lsonicdbcli -lzmq -luuid -lboost_serialization
