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

#include <rcsc/math_util.h>
#include <rcsc/common/server_param.h>

#include <rcsc/player/player_agent.h>

#include <rcsc/action/basic_actions.h>
#include <rcsc/action/body_go_to_point.h>
#include <rcsc/action/body_kick_one_step.h>
#include <rcsc/action/body_kick_two_step.h>
#include <rcsc/action/intention_kick.h>
#include <rcsc/action/neck_scan_field.h>

#include "bhv_go_to_static_ball.h"
#include "bhv_set_play.h"
#include "bhv_prepare_set_play_kick.h"

#include "bhv_set_play_kick_off.h"

/*-------------------------------------------------------------------*/
/*!

*/
bool
Bhv_SetPlayKickOff::execute( rcsc::PlayerAgent * agent )
{
    // check kicker
    if ( isKicker( agent ) )
    {
        doKick( agent );
    }
    else
    {
        doMove( agent );
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Bhv_SetPlayKickOff::isKicker( const rcsc::PlayerAgent * agent )
{
    if ( ! agent->world().teammatesFromBall().empty()
         && ( agent->world().teammatesFromBall().front()->distFromBall()
              < agent->world().ball().distFromSelf() ) )
    {
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
Bhv_SetPlayKickOff::doKick( rcsc::PlayerAgent * agent )
{
    static int S_scan_count = -5;

    // go to ball
    if ( Bhv_GoToStaticBall( 0.0 ).execute( agent ) )
    {
        return;
    }

    // already kick point
    if ( agent->world().self().body().abs() < 175.0 )
    {
        rcsc::Vector2D face_point( -rcsc::ServerParam::i().pitchHalfLength(),
                                   0.0 );
        rcsc::Body_TurnToPoint( face_point ).execute( agent );
        agent->setNeckAction( new rcsc::Neck_ScanField() );
        return;
    }

    if ( S_scan_count < 0 )
    {
        S_scan_count++;
        rcsc::Body_TurnToAngle( 180.0 ).execute( agent );
        agent->setNeckAction( new rcsc::Neck_ScanField() );
        return;
    }

    if ( S_scan_count < 10
         && agent->world().teammatesFromSelf().empty() )
    {
        S_scan_count++;
        rcsc::Body_TurnToAngle( 180.0 ).execute( agent );
        agent->setNeckAction( new rcsc::Neck_ScanField() );
        return;
    }

    S_scan_count = -5;


    rcsc::Vector2D target_point;
    double ball_speed;

    // teammate not found
    if ( agent->world().teammatesFromSelf().empty() )
    {
        target_point.assign( rcsc::ServerParam::i().pitchHalfLength(),
                             static_cast< double >
                             ( -1 + 2 * agent->world().time().cycle() % 2 )
                             * 0.8 * rcsc::ServerParam::i().goalHalfWidth() );
        ball_speed = rcsc::ServerParam::i().ballSpeedMax();
    }
    else
    {
        double dist = agent->world().teammatesFromSelf().front()->distFromSelf();
        // too far
        if ( dist > 35.0 )
        {
            target_point.assign( rcsc::ServerParam::i().pitchHalfLength(),
                                 static_cast< double >
                                 ( -1 + 2 * agent->world().time().cycle() % 2 )
                                 * 0.8 * rcsc::ServerParam::i().goalHalfWidth() );
            ball_speed = rcsc::ServerParam::i().ballSpeedMax();
        }
        else
        {
            target_point = agent->world().teammatesFromSelf().front()->pos();
            ball_speed
                = rcsc::calc_first_term_geom_series_last
                ( 0.85,
                  dist,
                  rcsc::ServerParam::i().ballDecay() );
            ball_speed = std::min( ball_speed, rcsc::ServerParam::i().ballSpeedMax() );
        }
    }

    rcsc::Body_KickTwoStep( target_point,
                            ball_speed,
                            true // enforce kick
                            ).execute( agent );
    agent->setNeckAction( new rcsc::Neck_ScanField() );

    // set enforce kick intention
    agent->setIntention( new rcsc::IntentionKick( target_point,
                                                  ball_speed,
                                                  1, true,
                                                  agent->world().time() ) );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
Bhv_SetPlayKickOff::doMove( rcsc::PlayerAgent * agent )
{
    rcsc::Vector2D target_pos = M_home_pos;
    target_pos.x = std::min( -0.5, target_pos.x );

    double dash_power
        = Bhv_SetPlay::get_set_play_dash_power( agent );
    double dist_thr = agent->world().ball().distFromSelf() * 0.07;
    if ( dist_thr < 1.0 ) dist_thr = 1.0;

    if ( ! rcsc::Body_GoToPoint( target_pos,
                                 dist_thr,
                                 dash_power
                                 ).execute( agent ) )
    {
        // already there
        rcsc::Body_TurnToBall().execute( agent );
    }
    agent->setNeckAction( new rcsc::Neck_ScanField() );
}
