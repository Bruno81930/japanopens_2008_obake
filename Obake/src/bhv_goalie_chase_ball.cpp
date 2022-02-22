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

#include <rcsc/geom/line_2d.h>
#include <rcsc/common/server_param.h>

#include <rcsc/common/logger.h>
#include <rcsc/player/player_agent.h>
#include <rcsc/player/intercept_table.h>
#include <rcsc/player/debug_client.h>

#include <rcsc/action/basic_actions.h>
#include <rcsc/action/body_go_to_point.h>
#include <rcsc/action/body_stop_dash.h>
#include <rcsc/action/body_intercept.h>

#include "bhv_goalie_basic_move.h"

#include "bhv_goalie_chase_ball.h"

/*-------------------------------------------------------------------*/
/*!
  execute action
*/
bool
Bhv_GoalieChaseBall::execute( rcsc::PlayerAgent * agent )
{
    rcsc::dlog.addText( rcsc::Logger::TEAM,
                        "%s:%d: Bhv_GoalieChaseBall"
                        ,__FILE__, __LINE__ );
    //----------------------------------------------------------
    // It is necessary to consider the situation
    //  which opponent keeps the ball or dribbles.

    const rcsc::WorldModel & wm = agent->world();

    ////////////////////////////////////////////////////////////////////////
    // get active interception catch point

    rcsc::Vector2D my_int_pos = wm.interceptTable()->selfInterceptPoint();
    rcsc::dlog.addText( rcsc::Logger::TEAM,
                        "%s: execute. intercept point=(%.2f %.2f)"
                        ,__FILE__,
                        my_int_pos.x, my_int_pos.y );

    agent->debugClient().addMessage( "Intercept" );
    rcsc::Body_Intercept( true ).execute( agent );
    agent->setNeckAction( new rcsc::Neck_TurnToBall() );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Bhv_GoalieChaseBall::is_ball_chase_situation( const rcsc::PlayerAgent * agent )
{
    const rcsc::WorldModel & wm = agent->world();
    if ( wm.gameMode().type() != rcsc::GameMode::PlayOn )
    {
        return false;
    }

    int self_goalie_min = wm.interceptTable()->selfReachCycle();
    int opp_min_cyc = wm.interceptTable()->opponentReachCycle();

    ////////////////////////////////////////////////////////////////////////
    // check shoot moving
    if ( is_ball_shoot_moving( agent )
         && self_goalie_min < opp_min_cyc )
    {
        rcsc::dlog.addText( rcsc::Logger::TEAM,
                            "%s:%d: shoot moving. chase ball"
                            ,__FILE__, __LINE__ );
        return true;
    }

    ////////////////////////////////////////////////////////////////////////
    // get active interception catch point

    const rcsc::Vector2D my_int_pos = wm.interceptTable()->selfInterceptPoint();

    double pen_thr = wm.ball().distFromSelf() * 0.1 + 1.0;
    if ( pen_thr < 1.0 ) pen_thr = 1.0;
    if ( my_int_pos.absY() > rcsc::ServerParam::i().penaltyAreaHalfWidth() - pen_thr
         || my_int_pos.x > rcsc::ServerParam::i().ourPenaltyAreaLineX() - pen_thr )
    {
        rcsc::dlog.addText( rcsc::Logger::TEAM,
                            "%s:%d: intercept point is out of penalty"
                            ,__FILE__, __LINE__ );
        return false;
    }

    ////////////////////////////////////////////////////////////////////////
    // Now, I can chase the ball
    // check the ball possessor

    if ( wm.existKickableTeammate()
         && ! wm.existKickableOpponent() )
    {
        rcsc::dlog.addText( rcsc::Logger::TEAM,
                            "%s:%d: exist kickable player"
                            ,__FILE__, __LINE__ );
        return false;
    }

    if ( opp_min_cyc <= self_goalie_min - 2 )
    {
        rcsc::dlog.addText( rcsc::Logger::TEAM,
                            "%s:%d: opponent reach the ball faster than me"
                            ,__FILE__, __LINE__ );
        return false;
    }

    rcsc::dlog.addText( rcsc::Logger::TEAM,
                        "%s:%d: exist interception point. try chase."
                        ,__FILE__, __LINE__ );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Bhv_GoalieChaseBall::is_ball_shoot_moving( const rcsc::PlayerAgent * agent )
{
    const rcsc::WorldModel& wm = agent->world();

    if ( wm.ball().distFromSelf() > 30.0 )
    {
        return false;
    }

    if ( wm.ball().pos().x > -34.5 )
    {
        return false;
    }

    // check opponent kicker
    if ( wm.existKickableOpponent() )
    {
        rcsc::dlog.addText( rcsc::Logger::TEAM,
                            "%s:%d: check shoot moving. opponent kickable "
                            ,__FILE__, __LINE__ );
        return false;
    }
    else if ( wm.existKickableTeammate() )
    {
        rcsc::dlog.addText( rcsc::Logger::TEAM,
                            "%s:%d: check shoot moving. teammate kickable"
                            ,__FILE__, __LINE__ );
        return false;
    }

    if ( wm.ball().vel().absX() < 0.1 )
    {
        if ( wm.ball().pos().x < -46.0
             && wm.ball().pos().absY() < rcsc::ServerParam::i().goalHalfWidth() + 2.0 )
        {
            rcsc::dlog.addText( rcsc::Logger::TEAM,
                                "%s:%d: check shoot moving. bvel.x(%f) is ZERO. but near to goal"
                                ,__FILE__, __LINE__,
                                wm.ball().vel().x );
            return true;
        }
        rcsc::dlog.addText( rcsc::Logger::TEAM,
                            "%s:%d: check shoot moving. bvel,x is small"
                            ,__FILE__, __LINE__ );
        return false;
    }


    const rcsc::Line2D ball_line( wm.ball().pos(), wm.ball().vel().th() );
    const double intersection_y = ball_line.getY( -rcsc::ServerParam::i().pitchHalfLength() );

    if ( std::fabs( ball_line.getB() ) > 0.1
         && std::fabs( intersection_y ) < rcsc::ServerParam::i().goalHalfWidth() + 2.0
         && wm.ball().pos().x < -40.0
         && wm.ball().pos().absY() < 15.0 )
    {
        const rcsc::Vector2D end_point
            = wm.ball().pos()
            + wm.ball().vel() / ( 1.0 - rcsc::ServerParam::i().ballDecay());
        if ( wm.ball().vel().r() > 0.5
             && end_point.x < -rcsc::ServerParam::i().pitchHalfLength() + 2.0 )
        {
            rcsc::dlog.addText( rcsc::Logger::TEAM,
                                "%s:%d: shoot to Y(%f). ball_line a=%f, b=%f, c=%f"
                                ,__FILE__, __LINE__,
                                intersection_y,
                                ball_line.getA(),
                                ball_line.getB(),
                                ball_line.getC() );
            return true;
        }
    }

    return false;
}
