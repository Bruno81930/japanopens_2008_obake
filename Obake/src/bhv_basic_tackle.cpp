// -*-c++-*-

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA

 This code is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2, or (at your option)
 any later version.

 This code is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this code; see the file COPYING.  If not, write to
 the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

 *EndCopyright:
 */

/////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "bhv_basic_tackle.h"

#include <rcsc/action/neck_turn_to_ball_or_scan.h>

#include <rcsc/player/player_agent.h>
#include <rcsc/player/debug_client.h>

#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>

/*-------------------------------------------------------------------*/
/*!
  execute action
*/
bool
Bhv_BasicTackle::execute( rcsc::PlayerAgent * agent )
{
    const rcsc::WorldModel & wm = agent->world();

    if ( wm.existKickableOpponent()
         && wm.self().tackleProbability() > M_min_prob )
    {
        if ( agent->config().version() >= 12.0 )
        {
            if ( wm.self().body().abs() < M_body_thr )
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                                    "%s:%d: Bhv_BasicTackle. to body dir"
                                    ,__FILE__, __LINE__ );

                agent->debugClient().addMessage( "Tackle+" );
                agent->doTackle( 0.0 ); // forward tackle
                agent->setNeckAction( new rcsc::Neck_TurnToBallOrScan() );
                return true;
            }

            if ( wm.self().body().abs() > 180.0 - M_body_thr )
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                                    "%s:%d: Bhv_BasicTackle. to back dir"
                                    ,__FILE__, __LINE__ );

                agent->debugClient().addMessage( "Tackle-" );
                agent->doTackle( 180.0 ); // backward tackle
                agent->setNeckAction( new rcsc::Neck_TurnToBallOrScan() );
                return true;
            }

        }
        else // v11 or older
        {
            double tackle_power = rcsc::ServerParam::i().maxTacklePower();
            if ( wm.self().body().abs() < M_body_thr )
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                                    "%s:%d: Bhv_BasicTackle. to body dir"
                                    ,__FILE__, __LINE__ );

                agent->debugClient().addMessage( "Tackle+" );
                agent->doTackle( tackle_power );
                agent->setNeckAction( new rcsc::Neck_TurnToBallOrScan() );
                return true;
            }

            tackle_power = - rcsc::ServerParam::i().maxBackTacklePower();
            if ( tackle_power < 0.0
                 && wm.self().body().abs() > 180.0 - M_body_thr )
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                                    "%s:%d: Bhv_BasicTackle. to body reverse dir"
                                    ,__FILE__, __LINE__ );

                agent->debugClient().addMessage( "Tackle-" );
                agent->doTackle( tackle_power );
                agent->setNeckAction( new rcsc::Neck_TurnToBallOrScan() );
                return true;
            }
        }
    }

    return false;
}
