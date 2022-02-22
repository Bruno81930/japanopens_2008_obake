// -*-c++-*-

/*!
  \file neck_turn_to_low_conf_teammate.cpp
  \brief check teammate player that has low confidence value
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

#include "neck_turn_to_low_conf_teammate.h"

#include "basic_actions.h"
#include "neck_scan_field.h"
#include "neck_turn_to_ball_or_scan.h"

#include <rcsc/player/player_agent.h>
#include <rcsc/player/debug_client.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

 */
bool
Neck_TurnToLowConfTeammate::execute( PlayerAgent * agent )
{
    const PlayerPtrCont & teammates = agent->world().teammatesFromSelf();

    dlog.addText( Logger::ACTION,
                  "%s:%d: Neck_TurnToLowConfTeammate"
                  ,__FILE__, __LINE__ );

    if ( teammates.empty() )
    {
        return Neck_ScanField().execute( agent );
    }

    ViewWidth vwidth = agent->effector().queuedNextViewWidth();
    double next_view_width = ServerParam::i().visibleAngle();
    if ( vwidth.type() == ViewWidth::WIDE )
    {
        if ( agent->world().seeTime() == agent->world().time() )
        {
            return false;
        }
        next_view_width *= 2.0;
    }
    else if ( vwidth.type() == ViewWidth::NARROW )
    {
        next_view_width *= 0.5;
    }

    const double next_neck_range_half
        = ServerParam::i().maxNeckAngle() + next_view_width * 0.5 - 5.0;

    const Vector2D my_next = agent->effector().queuedNextMyPos();
    const AngleDeg my_next_body = agent->effector().queuedNextMyBody();


    int pos_count = 2;
    Vector2D candidate_point( -52.5, 0.0 );
    int candidate_unum = 0;

    const double max_dist = 40.0;

    {
        const PlayerPtrCont::const_iterator end = teammates.end();
        for ( PlayerPtrCont::const_iterator it = teammates.begin();
              it != end;
              ++it )
        {
            if ( (*it)->isGhost() )
            {
                continue;
            }

            if ( (*it)->distFromSelf() > max_dist )
            {
                break;
            }

            if ( (*it)->posCount() >= pos_count
                 && candidate_point.x < (*it)->pos().x )
            {
                if ( (my_next_body - ( (*it)->pos() - my_next).th() ).abs()
                     < next_neck_range_half )
                {
                    // can face
                    candidate_unum = (*it)->unum();
                    candidate_point = (*it)->pos();
                    pos_count = (*it)->posCount();
                }
            }
        }
    }

    if ( pos_count <= 2 )
    {
        dlog.addText( Logger::ACTION,
                      "%s:%d: no candidate. pos_count=%d"
                      ,__FILE__, __LINE__,
                      pos_count );
        return Neck_TurnToBallOrScan().execute( agent );
    }

    AngleDeg neck_moment = ( candidate_point - my_next ).th(); // global
    neck_moment -= my_next_body; // relative to body
    neck_moment -= agent->world().self().neck(); // relative to face
    neck_moment = ServerParam::i().normalizeNeckMoment( neck_moment.degree() );

    dlog.addText( Logger::ACTION,
                  "%s:%d: look teammate %d (%.1f, %.1f). pos_count=%d"
                  ,__FILE__, __LINE__,
                  candidate_unum,
                  candidate_point.x, candidate_point.y,
                  pos_count );

    agent->debugClient().addMessage( "LookOur%d", candidate_unum );
    return agent->doTurnNeck( neck_moment );
}

}
