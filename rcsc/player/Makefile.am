## Process this file with automake to produce Makefile.in

noinst_LTLIBRARIES = librcsc_player.la

librcsc_player_la_SOURCES = \
	abstract_player_object.cpp \
	action_effector.cpp \
	audio_sensor.cpp \
	ball_object.cpp \
	body_sensor.cpp \
	debug_client.cpp \
	freeform_parser.cpp \
	fullstate_sensor.cpp \
	interception.cpp \
	intercept_table.cpp \
	localization.cpp \
	object_table.cpp \
	penalty_kick_state.cpp \
	player_command.cpp \
	player_agent.cpp \
	player_config.cpp \
	player_intercept.cpp \
	player_object.cpp \
	say_message_builder.cpp \
	see_state.cpp \
	self_intercept.cpp \
	self_object.cpp \
	stamina_model.cpp \
	view_mode.cpp \
	visual_sensor.cpp \
	world_model.cpp

librcsc_playerincludedir = $(includedir)/rcsc/player

librcsc_playerinclude_HEADERS = \
	abstract_player_object.h \
	action_effector.h \
	audio_sensor.h \
	ball_object.h \
	body_sensor.h \
	debug_client.h \
	free_message.h \
	freeform_parser.h \
	fullstate_sensor.h \
	interception.h \
	intercept_table.h \
	localization.h \
	object_table.h \
	penalty_kick_state.h \
	player_command.h \
	player_agent.h \
	player_config.h \
	player_intercept.h \
	player_object.h \
	player_predicate.h \
	say_message_builder.h \
	see_state.h \
	self_intercept.h \
	self_object.h \
	soccer_action.h \
	soccer_intention.h \
	stamina_model.h \
	view_mode.h \
	visual_sensor.h \
	world_model.h

AM_CPPFLAGS = -I$(top_srcdir)
AM_CFLAGS = -Wall
AM_CXXFLAGS = -Wall
AM_LDLAGS =

CLEANFILES = *~
