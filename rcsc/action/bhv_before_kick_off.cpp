// -*-c++-*-

/*!
  \file bhv_before_kick_off.cpp
  \brief behavior definition when playmode is BeforeKickOff
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

#include "bhv_before_kick_off.h"
#include "bhv_scan_field.h"
#include "neck_turn_to_relative.h"

#include <rcsc/player/player_agent.h>
#include <rcsc/common/server_param.h>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
bool
Bhv_BeforeKickOff::execute( PlayerAgent * agent )
{
    if ( agent->world().time().cycle() == 0
         && agent->world().time().stopped() < 5 )
    {
        agent->doTurn( 0.0 );
        agent->setNeckAction( new Neck_TurnToRelative( 0.0 ) );
        return false;
    }


    if ( ! agent->world().self().posValid() )
    {
        return Bhv_ScanField().execute( agent );
    }

    if ( agent->world().gameMode().type() == GameMode::AfterGoal_
         && agent->world().self().vel().r() > 0.05 )
    {
        agent->doTurn( 180.0 );
        agent->setNeckAction( new Neck_TurnToRelative( 0.0 ) );
        return true;
    }


    if ( ServerParam::i().kickoffOffside()
         && M_move_point.x >= 0.0 )
    {
        M_move_point.x = -1.0;
    }

    // check center circle
    SideID kickoff_side = NEUTRAL;

    if ( agent->world().gameMode().type() == GameMode::AfterGoal_ )
    {
        // after our goal
        if ( agent->world().gameMode().side() != agent->world().ourSide() )
        {
            kickoff_side = agent->world().ourSide();
        }
        else
        {
            kickoff_side = ( agent->world().isOurLeft()
                             ? RIGHT
                             : LEFT );
        }
    }
    else // before_kick_off
    {
        // check half_time count
        if ( ServerParam::i().halfTime() > 0 )
        {
            int time_flag = ( ( (agent->world().time().cycle() + 1)
                                / 3000 ) // ServerParam::i().halfTime() )
                              % 2 );
            kickoff_side = ( time_flag == 0
                             ? LEFT
                             : RIGHT );
        }
        else
        {
            kickoff_side = LEFT;
        }
    }

    if ( kickoff_side != agent->world().ourSide()
         && M_move_point.r() < ServerParam::i().centerCircleR() + 0.1 )
    {
        M_move_point *= ( ServerParam::i().centerCircleR() + 0.5 ) / M_move_point.r();
    }

    // move
    double tmpr = ( M_move_point - agent->world().self().pos() ).r();
    if ( tmpr > 1.0 )
    {
#if 0
        if ( tmpr > 1.5 && agent->world().time().cycle() == 0 )
        {
            double add_angle = 25.0;
            if ( agent->world().isOurRight() )
            {
                add_angle *= -1.0;
            }

            Vector2D add_pos = M_move_point - agent->world().self().pos();
            add_pos.normalize(1.0);
            add_pos.rotate(add_angle);
            if ( agent->world().self().pos().x + add_pos.x > -0.5 )
            {
                add_pos.rotate(-2.0 * add_angle);
            }
            M_move_point = agent->world().self().pos() + add_pos;

            if ( M_move_point.x > -0.5 )
            {
                M_move_point.x = -0.5;
            }
        }
#endif
        agent->doMove( M_move_point.x, M_move_point.y );
        agent->setNeckAction( new Neck_TurnToRelative( 0.0 ) );
        return true;
    }

    // field scan
    return Bhv_ScanField().execute( agent );
}

}
