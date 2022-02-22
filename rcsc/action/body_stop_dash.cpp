// -*-c++-*-

/*!
  \file body_stop_dash.cpp
  \brief try to change the agent's velocity to 0.
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

#include "body_stop_dash.h"

#include "body_go_to_point.h"

#include <rcsc/player/player_agent.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_StopDash::execute( PlayerAgent * agent )
{
    dlog.addText( Logger::ACTION,
                  "%s:%d: Body_StopDash"
                  ,__FILE__, __LINE__ );

    if ( ! agent->world().self().velValid() )
    {
        dlog.addText( Logger::ACTION,
                      "%s:%d: self vel is invalid"
                      ,__FILE__, __LINE__ );
        agent->doTurn( 0.0 );
        return false;
    }

    // velocity relative to body angle
    Vector2D rotated_vel = agent->world().self().vel();
    rotated_vel.rotate( - agent->world().self().body() );

    // calc the power to reduce current speed
    double power = - ( rotated_vel.x
                       / agent->world().self().dashRate() );

    power = ServerParam::i().normalizePower( power );

    if ( M_save_recovery )
    {
        power = agent->world().self().getSafetyDashPower( power );
    }

    dlog.addText( Logger::ACTION,
                  "%s:%d: stop dash power= %.1f, accel=%.3f"
                  ,__FILE__, __LINE__,
                  power, power * agent->world().self().dashRate() );

    return agent->doDash( power );
}

}
