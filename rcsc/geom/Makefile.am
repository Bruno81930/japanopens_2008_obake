## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = librcsc_geom.la

librcsc_geom_la_SOURCES = \
	angle_deg.cpp \
	circle_2d.cpp \
	delaunay_triangulation.cpp \
	line_2d.cpp \
	matrix_2d.cpp \
	polygon_2d.cpp \
	ray_2d.cpp \
	rect_2d.cpp \
	sector_2d.cpp \
	segment_2d.cpp \
	triangle_2d.cpp \
	vector_2d.cpp

librcsc_geomincludedir = $(includedir)/rcsc/geom

##pkginclude_HEADERS
librcsc_geominclude_HEADERS = \
	angle_deg.h \
	circle_2d.h \
	delaunay_triangulation.h \
	line_2d.h \
	matrix_2d.h \
	polygon_2d.h \
	ray_2d.h \
	rect_2d.h \
	sector_2d.h \
	size_2d.h \
	segment_2d.h \
	triangle_2d.h \
	vector_2d.h

librcsc_geom_la_LDFLAGS = -version-info 3:0:0
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

AM_CPPFLAGS = -I$(top_srcdir)
AM_CFLAGS = -Wall
AM_CXXFLAGS = -Wall
AM_LDFLAGS =

CLEANFILES = *~

#EXTRA_DIST =

if UNIT_TEST
TESTS = triangle_2d_test segment_2d_test polygon_2d_test \
	matrix_2d_test
LDADD = librcsc_geom.la $(BOOST_UNIT_TEST_FRAMEWORK_LIB)
endif

check_PROGRAMS = $(TESTS)

triangle_2d_test_SOURCES = triangle_2d_test.cpp
segment_2d_test_SOURCES = segment_2d_test.cpp
polygon_2d_test_SOURCES = polygon_2d_test.cpp
matrix_2d_test_SOURCES = matrix_2d_test.cpp
