// -*-c++-*-

/*!
  \file coach_config.cpp
  \brief coach configuration Source File
*/

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA

 This code is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 *EndCopyright:
 */

/////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "coach_config.h"

#include <rcsc/param/param_map.h>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
CoachConfig::CoachConfig()
{
    setDefaultParam();
}

/*-------------------------------------------------------------------*/
/*!

*/
CoachConfig::~CoachConfig()
{

    // std::cerr << "delete CoachConfig" << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
CoachConfig::setDefaultParam()
{
    // basic setting
    M_team_name = "HELIOS_base";
    M_version = 9.4;

    M_coach_name = "Coach_base";
    M_use_coach_name = false;

    M_interval_msec = 50;
    M_server_wait_seconds = 5;

    M_rcssserver_host = "localhost";
    M_rcssserver_port = 6002;

    M_compression = -1;

    M_use_eye = true;
    M_hear_say = true;

    M_analyze_player_type = true;

    M_use_advise = true;
    M_use_freeform = true;

    M_use_hetero = true;

    M_use_team_graphic = false;

    M_max_team_graphic_per_cycle = 32;

    //
    // debug
    //
    M_log_dir = "/tmp";
    M_log_ext = ".log";

    M_debug = false;
    M_debug_system = false;
    M_debug_sensor = false;
    M_debug_world = false;
    M_debug_action = false;
    M_debug_intercept = false;
    M_debug_kick = false;
    M_debug_dribble = false;
    M_debug_pass = false;
    M_debug_cross = false;
    M_debug_shoot = false;
    M_debug_clear = false;
    M_debug_team = false;
    M_debug_role = false;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
CoachConfig::createParamMap( ParamMap & param_map )
{
    param_map.add()
        ( "team_name", "t", &M_team_name )
        ( "version", "v", &M_version )

        ( "coach_name", "", &M_coach_name )
        ( "use_coach_name", "", &M_use_coach_name )

        ( "interval_msec", "", &M_interval_msec )
        ( "server_wait_seconds", "", &M_server_wait_seconds )

        ( "host", "h",  &M_rcssserver_host )
        ( "port", "", &M_rcssserver_port )

        ( "compression", "", &M_compression )

        ( "use_eye", "", &M_use_eye )
        ( "hear_say", "", &M_hear_say )

        ( "analyze_player_type", "", &M_analyze_player_type )

        ( "use_advise", "", &M_use_advise )
        ( "use_freeform", "", &M_use_freeform )

        ( "use_hetero", "", &M_use_hetero )

        ( "use_team_graphic", "", &M_use_team_graphic )
        ( "max_team_graphic_per_cycle", "", &M_max_team_graphic_per_cycle )

        ( "log_dir", "", &M_log_dir )
        ( "log_ext", "", &M_log_ext )

        ( "debug", "", BoolSwitch( &M_debug ) )
        ( "debug_system", "", BoolSwitch( &M_debug_system ) )
        ( "debug_sensor", "", BoolSwitch( &M_debug_sensor ) )
        ( "debug_world", "", BoolSwitch( &M_debug_world ) )
        ( "debug_action", "", BoolSwitch( &M_debug_action ) )
        ( "debug_intercept", "", BoolSwitch( &M_debug_intercept ) )
        ( "debug_kick", "", BoolSwitch( &M_debug_kick ) )
        ( "debug_dribble", "", BoolSwitch( &M_debug_dribble ) )
        ( "debug_pass", "", BoolSwitch( &M_debug_pass ) )
        ( "debug_cross", "", BoolSwitch( &M_debug_cross ) )
        ( "debug_shoot", "", BoolSwitch( &M_debug_shoot ) )
        ( "debug_clear", "", BoolSwitch( &M_debug_clear ) )
        ( "debug_team", "", BoolSwitch( &M_debug_team ) )
        ( "debug_role", "", BoolSwitch( &M_debug_role ) );
        ;
}

}
