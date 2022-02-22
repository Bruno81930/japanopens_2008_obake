## Process this file with automake to produce Makefile.in

PLAYERSOURCES = \
	bhv_basic_move.cpp \
	bhv_basic_offensive_kick.cpp \
	bhv_basic_tackle.cpp \
	bhv_go_to_static_ball.cpp \
	bhv_goalie_basic_move.cpp \
	bhv_goalie_chase_ball.cpp \
	bhv_goalie_free_kick.cpp \
	bhv_pre_process.cpp \
	bhv_prepare_set_play_kick.cpp \
	bhv_set_play.cpp \
	bhv_set_play_free_kick.cpp \
	bhv_set_play_goal_kick.cpp \
	bhv_set_play_kick_in.cpp \
	bhv_set_play_kick_off.cpp \
	bhv_their_goal_kick_move.cpp \
	bhv_penalty_kick.cpp \
	body_kick_to_corner.cpp \
	role_center_back.cpp \
	role_center_forward.cpp \
	role_defensive_half.cpp \
	role_goalie.cpp \
	role_offensive_half.cpp \
	role_sample.cpp \
	role_side_back.cpp \
	role_side_forward.cpp \
	sample_player.cpp \
	strategy.cpp \
	main_player.cpp \
	body_obake_clear.cpp \
	bhv_obake_defend.cpp \
	bhv_obake_action_strategy.cpp \
	bhv_obake_action_strategy_test.cpp \
	bhv_obake_mark.cpp \
	bhv_obake_pass.cpp \
	bhv_obake_receive.cpp \
	bhv_obake_receive_test.cpp \
	body_obake_pass.cpp \
	body_obake_shoot.cpp \
	obake_analysis.cpp \
	obake_fuzzy_grade.cpp \
	obake_fuzzy_grade_model.cpp \
	obake_stamina_control.cpp \
	obake_strategy.cpp \
	obake_update.cpp

PLAYERHEADERS = \
	bhv_basic_move.h \
	bhv_basic_offensive_kick.h \
	bhv_basic_tackle.h \
	bhv_go_to_static_ball.h \
	bhv_goalie_basic_move.h \
	bhv_goalie_chase_ball.h \
	bhv_goalie_free_kick.h \
	bhv_pre_process.h \
	bhv_prepare_set_play_kick.h \
	bhv_set_play.h \
	bhv_set_play_free_kick.h \
	bhv_set_play_goal_kick.h \
	bhv_set_play_kick_in.h \
	bhv_set_play_kick_off.h \
	bhv_their_goal_kick_move.h \
	bhv_penalty_kick.h \
	body_kick_to_corner.h \
	role_center_back.h \
	role_center_forward.h \
	role_defensive_half.h \
	role_goalie.h \
	role_offensive_half.h \
	role_sample.h \
	role_side_back.h \
	role_side_forward.h \
	sample_player.h \
	soccer_role.h \
	strategy.h \
	bhv_obake_defend.h \
	bhv_obake_action_strategy.h \
	bhv_obake_action_strategy_test.h \
	bhv_obake_mark.h \
	bhv_obake_pass.h \
	bhv_obake_receive.h \
	bhv_obake_receive_test.h \
	body_obake_clear.h \
	body_obake_pass.h \
	body_obake_shoot.h \
	obake_analysis.h \
	obake_fuzzy_grade.h \
	obake_fuzzy_grade_model.h \
	obake_stamina_control.h \
	obake_strategy.h \
	obake_update.h

COACHSOURCES = \
	sample_coach.cpp \
	main_coach.cpp

COACHHEADERS = \
	sample_coach.h

TRAINERSOURCES = \
	sample_trainer.cpp \
	main_trainer.cpp

TRAINERHEADERS = \
	sample_trainer.h


noinst_PROGRAMS = sample_player sample_coach sample_trainer

noinst_DATA = \
	start.sh.in \
	train.sh.in \
	player.conf \
	formations-bpn/before-kick-off.conf \
	formations-bpn/defense-formation.conf \
	formations-bpn/defense-formation.dat \
	formations-bpn/goal-kick-opp.conf \
	formations-bpn/goal-kick-our.conf \
	formations-bpn/goalie-catch-opp.conf \
	formations-bpn/goalie-catch-our.conf \
	formations-bpn/kickin-our-formation.conf \
	formations-bpn/kickin-our-formation.dat \
	formations-bpn/offense-formation.conf \
	formations-bpn/offense-formation.dat \
	formations-bpn/setplay-our-formation.conf \
	formations-bpn/setplay-our-formation.dat \
	formations-dt/before-kick-off.conf \
	formations-dt/defense-formation.conf \
	formations-dt/goal-kick-opp.conf \
	formations-dt/goal-kick-our.conf \
	formations-dt/goalie-catch-opp.conf \
	formations-dt/goalie-catch-our.conf \
	formations-dt/kickin-our-formation.conf \
	formations-dt/kickin-our-formation.dat \
	formations-dt/normal-formation.conf \
	formations-dt/normal-formation.dat \
	formations-dt/offense-formation.conf \
	formations-dt/setplay-our-formation.conf \
	formations-dt/setplay-our-formation.dat \
	formations-dt/4-2-3-1-offense-formation.conf \
	formations-dt/4-2-3-1-offense-formation.dat \
	formations-uva/before-kick-off.conf \
	formations-uva/defense-formation.conf \
	formations-uva/goal-kick-opp.conf \
	formations-uva/goal-kick-our.conf \
	formations-uva/goalie-catch-opp.conf \
	formations-uva/goalie-catch-our.conf \
	formations-uva/kickin-our-formation.conf \
	formations-uva/offense-formation.conf \
	formations-uva/setplay-our-formation.conf

sample_player_SOURCES = \
	$(PLAYERSOURCES)

sample_player_LDFLAGS =

sample_player_LDADD =

sample_coach_SOURCES = \
	$(COACHSOURCES)

sample_coach_LDFLAGS =

sample_coach_LDADD =

sample_trainer_SOURCES = \
	$(TRAINERSOURCES)

sample_trainer_LDFLAGS =

sample_trainer_LDADD =

noinst_HEADERS = \
	$(PLAYERHEADERS) \
	$(COACHHEADERS) \
	$(TRAINERHEADERS)

AM_CPPFLAGS =
AM_CFLAGS = -Wall
AM_CXXFLAGS = -Wall
AM_LDLAGS =

CLEANFILES = start.sh train.sh *~

EXTRA_DIST = $(noinst_DATA)
