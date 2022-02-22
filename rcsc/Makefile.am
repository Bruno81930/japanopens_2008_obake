# Process this file with automake to produce Makefile.in

SUBDIRS = \
	geom \
	gz \
	param \
	rcg \
	ann
if BUILD_AGENT
SUBDIRS += \
	util \
	formation \
	net \
	common \
	monitor \
	player \
	action \
	coach \
	trainer \
	.
endif

if BUILD_AGENT
BUILD_LIBS = librcsc_agent.la
endif

lib_LTLIBRARIES = $(BUILD_LIBS)

librcsc_agent_la_SOURCES =
librcsc_agent_la_LIBADD = \
	util/librcsc_util.la \
	formation/librcsc_formation.la \
	net/librcsc_net.la \
	common/librcsc_common.la \
	monitor/librcsc_monitor.la \
	player/librcsc_player.la \
	action/librcsc_action.la \
	coach/librcsc_coach.la \
	trainer/librcsc_trainer.la

librcsc_agent_la_LDFLAGS = -version-info 3:0:0
#libXXXX_la_LDFLAGS = -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)
#    1. Start with version information of `0:0:0' for each libtool library.
#
#    2. Update the version information only immediately before a public
#       release of your software.  More frequent updates are unnecessary,
#       and only guarantee that the current interface number gets larger
#       faster.
#
#    3. If the library source code has changed at all since the last
#       update, then increment REVISION (`C:R:A' becomes `C:r+1:A').
#
#    4. If any interfaces have been added, removed, or changed since the
#       last update, increment CURRENT, and set REVISION to 0.
#
#    5. If any interfaces have been added since the last public release,
#       then increment AGE.
#
#    6. If any interfaces have been removed since the last public release,
#       then set AGE to 0.

librcsc_agentincludedir = $(includedir)/rcsc
librcsc_agentinclude_HEADERS =

rcscincludedir = $(includedir)/rcsc
rcscinclude_HEADERS = \
	types.h \
	geom.h \
	gz.h \
	rcg.h \
	game_mode.h \
	game_time.h \
	math_util.h \
	random.h \
	soccer_math.h \
	timer.h

CLEANFILES = *~
