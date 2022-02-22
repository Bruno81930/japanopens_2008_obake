// -*-c++-*-

/*!
  \file neck_turn_to_ball_or_scan.cpp
  \brief check the ball or scan field with neck evenly
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

#include "neck_turn_to_ball_or_scan.h"

#include "basic_actions.h"
#include "bhv_scan_field.h"
#include "neck_scan_field.h"

#include <rcsc/player/player_agent.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
bool
Neck_TurnToBallOrScan::execute( PlayerAgent * agent )
{
    dlog.addText( Logger::ACTION,
                  "%s:%d: Neck_TurnToBallOrScan"
                  ,__FILE__, __LINE__ );


    const Vector2D ball_next = agent->effector().queuedNextBallPos();
    const Vector2D my_next = agent->effector().queuedNextMyPos();
    const AngleDeg my_next_body = agent->effector().queuedNextMyBody();
    const double next_view_width = agent->effector().queuedNextViewWidth().width();

    bool can_face = false;

    if ( ( (ball_next - my_next).th() - my_next_body ).abs()
         < ServerParam::i().maxNeckAngle() + next_view_width * 0.5 - 2.0 )
    {
        can_face = true;
    }

    if ( ! can_face )
    {
        dlog.addText( Logger::ACTION,
                      "%s:%d: never face to ball"
                      ,__FILE__, __LINE__ );
        return Neck_ScanField().execute( agent );
    }

    // ball is not seen at current
    if ( can_face
         && agent->world().ball().posCount() != 0
         && ( agent->effector().queuedNextMyPos().dist( agent->effector().queuedNextBallPos() )
              > 2.5 ) )
    {
        dlog.addText( Logger::ACTION,
                      "%s:%d: simply face ball"
                      ,__FILE__, __LINE__ );
        return Neck_TurnToBall().execute( agent );
    }

    // ball is seen at current

    // not exist kickable player
    if ( ! agent->world().existKickableTeammate()
         && ! agent->world().existKickableOpponent() )
    {
        dlog.addText( Logger::ACTION,
                      "%s:%d: no kickable player. only scan field"
                      ,__FILE__, __LINE__ );
        return Neck_ScanField().execute( agent );
    }

    // exist kickable player
    // they may accerelate the ball to max speed.

    if ( agent->world().ball().distFromSelf() < 20.0 )
    {
        dlog.addText( Logger::ACTION,
                      "%s:%d: ball close. check ball."
                      ,__FILE__, __LINE__ );
        return Neck_TurnToPoint( ball_next ).execute( agent );
    }

    // consider ball reachable dist after 2step
    const double narrow_width = ServerParam::i().visibleAngle() * 0.25; // half width

    double reachable_angle = std::fabs( AngleDeg::atan2_deg
                                        ( ServerParam::i().ballSpeedMax() * 2.0,
                                          ( ball_next - my_next ).r() ) );

    // I can see the ball after 2 cycle even if ball is kicked.
    if ( reachable_angle < narrow_width - 5.0 )
    {
        dlog.addText( Logger::ACTION,
                      "%s:%d: exist kickable. but can see a fter 2 cycles."
                      " scan field.  ball reachable angle = %.1f"
                      ,__FILE__, __LINE__,
                      reachable_angle );
        return Neck_ScanField().execute( agent );
    }

    dlog.addText( Logger::ACTION,
                  "%s:%d: ball reachable angle is over view range %.1f"
                  " face to ball_next=(%.2f, %.2f)"
                  ,__FILE__, __LINE__,
                  reachable_angle, ball_next.x, ball_next.y );

    return Neck_TurnToPoint( ball_next ).execute( agent );
}

}
