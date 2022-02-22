// -*-c++-*-

/*!
  \file neck_turn_to_goalie_or_scan.cpp
  \brief check opponent goalie or scan field with neck evenly
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

#include "neck_turn_to_goalie_or_scan.h"

#include "basic_actions.h"
#include "bhv_scan_field.h"
#include "neck_scan_field.h"
#include "neck_turn_to_ball_or_scan.h"

#include <rcsc/player/player_agent.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
bool
Neck_TurnToGoalieOrScan::execute( PlayerAgent * agent )
{
    dlog.addText( Logger::TEAM,
                  "%s:%d: Neck_TurnToGoalieOrScan"
                  ,__FILE__, __LINE__ );

    const WorldModel & wm = agent->world();

    const PlayerObject * opp_goalie = wm.getOpponentGoalie();

    if ( ! opp_goalie
         || opp_goalie->posCount() <= std::max( 0, M_count_thr ) )
    {
        dlog.addText( Logger::ACTION,
                      "%s:%d: goalie not found, or already valid"
                      ,__FILE__, __LINE__ );
        return Neck_TurnToBallOrScan().execute( agent );
    }

    if ( opp_goalie->isGhost()
         && opp_goalie->posCount() >= 3 )
    {
        dlog.addText( Logger::ACTION,
                      "%s:%d: detect ghost goalie"
                      ,__FILE__, __LINE__ );
        return Neck_TurnToBallOrScan().execute( agent );
    }

    const ViewWidth vwidth = agent->effector().queuedNextViewWidth();
    const double next_view_width = vwidth.width();
    const Vector2D my_next = agent->effector().queuedNextMyPos();
    const AngleDeg my_next_body = agent->effector().queuedNextMyBody();

    const Vector2D goalie_next = opp_goalie->pos() + opp_goalie->vel();

    if ( ( ( goalie_next - my_next ).th() - my_next_body ).abs()
         > ServerParam::i().maxNeckAngle() + next_view_width * 0.5 - 10.0 )
    {
        dlog.addText( Logger::ACTION,
                      "%s:%d: never face to goalie. next_width=%f"
                      ,__FILE__, __LINE__,
                      next_view_width );
        return Neck_TurnToBallOrScan().execute( agent );
    }

    dlog.addText( Logger::ACTION,
                  "%s:%d: neck to goalie"
                  ,__FILE__, __LINE__ );
    return Neck_TurnToPoint( goalie_next ).execute( agent );
}

}
