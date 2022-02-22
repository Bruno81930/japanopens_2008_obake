## Process this file with automake to produce Makefile.in

noinst_LTLIBRARIES = librcsc_formation.la

librcsc_formation_la_SOURCES = \
	formation.cpp \
	formation_factory.cpp \
	formation_bpn.cpp \
	formation_dt.cpp \
	formation_knn.cpp \
	formation_ngnet.cpp \
	formation_rbf.cpp \
	formation_sbsp.cpp \
	formation_static.cpp \
	formation_uva.cpp

librcsc_formationincludedir = $(includedir)/rcsc/formation

librcsc_formationinclude_HEADERS = \
	formation.h \
	formation_factory.h \
	formation_bpn.h \
	formation_dt.h \
	formation_knn.h \
	formation_ngnet.h \
	formation_rbf.h \
	formation_sbsp.h \
	formation_static.h \
	formation_uva.h

AM_CPPFLAGS =	-I$(top_srcdir)
AM_CFLAGS = -Wall
AM_CXXFLAGS = -Wall
AM_LDLAGS =

CLEANFILES = *~
