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

#include <rcsc/soccer_math.h>
#include <rcsc/geom/line_2d.h>
#include <rcsc/common/server_param.h>
#include <rcsc/player/player_agent.h>
#include <rcsc/player/intercept_table.h>
#include <rcsc/player/debug_client.h>

#include <rcsc/common/logger.h>

#include <rcsc/action/basic_actions.h>
#include <rcsc/action/body_go_to_point.h>
#include <rcsc/action/body_stop_dash.h>
#include <rcsc/action/body_intercept.h>
#include <rcsc/action/neck_turn_to_ball.h>
#include <rcsc/action/bhv_go_to_point_look_ball.h>
#include "obake_analysis.h"
#include "bhv_goalie_basic_move.h"

/*-------------------------------------------------------------------*/
/*!
  execute action
*/
bool
Bhv_GoalieBasicMove::execute( rcsc::PlayerAgent * agent )
{
    const rcsc::WorldModel & wm = agent->world();
    rcsc::Vector2D move_point = M_home_pos;
    if(//wm.ball().pos().x < rcsc::ServerParam::i().ourPenaltyAreaLine() + 10.0
        Obake_Analysis().getMateDefenseLine(agent) <= rcsc::ServerParam::i().ourPenaltyAreaLineX() + 15.0
        && wm.gameMode().type() != rcsc::GameMode::FreeKick_
        && wm.gameMode().type() != rcsc::GameMode::KickIn_
        && wm.gameMode().type() != rcsc::GameMode::OffSide_
        && wm.gameMode().type() != rcsc::GameMode::CornerKick_
        && wm.gameMode().type() != rcsc::GameMode::GoalKick_)
    {
        move_point = getTargetPoint( agent );
    }
    rcsc::dlog.addText( rcsc::Logger::TEAM,
                        "%s: Bhv_GoalieBasicMove. move_point(%.2f %.2f)"
                        ,__FILE__,
                        move_point.x, move_point.y );
  
    double dist_thr = agent->world().ball().distFromSelf() * 0.1;

    if ( ! rcsc::Body_GoToPoint( move_point,
                                 dist_thr,
                                 getBasicDashPower( agent, move_point )
                                 ).execute( agent )
         )
    {
/*
        rcsc::Vector2D face_point(agent->world().self().pos().x,
                                  100.0);
        if ( agent->world().ball().pos().y < agent->world().self().pos().y )
        {
            face_point.y *= -1.0;
        }
*/
        const rcsc::Vector2D face_point = getFacePoint(agent);
        rcsc::Body_TurnToPoint( face_point ).execute( agent );
    }
    agent->setNeckAction( new rcsc::Neck_TurnToBall() );

    return true;
}

/*-------------------------------------------------------------------*/
rcsc::Vector2D
Bhv_GoalieBasicMove::getFacePoint(rcsc::PlayerAgent * agent)
{
    const rcsc::WorldModel & wm = agent->world();
    rcsc::Vector2D face_point, v;
    face_point = rcsc::Vector2D(agent->world().self().pos().x,
                                100.0);
    if(wm.ball().pos().y < wm.self().pos().y )
    {
        face_point.y *= -1.0;
    }
    const rcsc::PlayerObject * target = wm.getOpponentNearestToBall(10);
    if(target)
    {
        const rcsc::Vector2D ball_next_pos = wm.ball().pos() + wm.ball().vel();
        const rcsc::Vector2D self_next_pos = wm.self().pos() + wm.self().vel();
        const double difference_y = std::abs(ball_next_pos.y - self_next_pos.y);
        if(difference_y <= 3.5)
        {
            if((target)->body().degree() >= 70.0
                && (target)->body().degree() <= 150.0)
            {
                face_point.y = 100.0;
            }
            else if((target)->body().degree() <= -70.0
                    && (target)->body().degree() >= -150.0)
            {
                face_point.y = -100.0;
            }
        }
    }
    return face_point;
}
/*!
*/
rcsc::Vector2D
Bhv_GoalieBasicMove::getTargetPoint( rcsc::PlayerAgent * agent )
{
    const double base_move_x = -50.5;
    const double danger_move_x = -51.0;
    const rcsc::WorldModel & wm = agent->world();

    int ball_reach_step = 0;
    if ( ! wm.existKickableTeammate()
         && ! wm.existKickableOpponent() )
    {
        ball_reach_step
            = std::min( wm.interceptTable()->teammateReachCycle(),
                        wm.interceptTable()->opponentReachCycle() );
    }
    const rcsc::Vector2D base_pos = wm.ball().inertiaPoint( ball_reach_step );


    //---------------------------------------------------------//
    
    // angle is very dangerous
    if ( base_pos.y > rcsc::ServerParam::i().goalHalfWidth() + 3.0 )
    {
        rcsc::Vector2D right_pole( - rcsc::ServerParam::i().pitchHalfLength(),
                                   rcsc::ServerParam::i().goalHalfWidth() );
        rcsc::AngleDeg angle_to_pole = ( right_pole - base_pos ).th();

        if ( -140.0 < angle_to_pole.degree()
             && angle_to_pole.degree() < -90.0 )
        {
            agent->debugClient().addMessage( "RPole" );
            return rcsc::Vector2D( danger_move_x, rcsc::ServerParam::i().goalHalfWidth() + 0.5 );
        }
    }
    else if ( base_pos.y < -rcsc::ServerParam::i().goalHalfWidth() - 3.0 )
    {
        rcsc::Vector2D left_pole( - rcsc::ServerParam::i().pitchHalfLength(),
                                  - rcsc::ServerParam::i().goalHalfWidth() );
        rcsc::AngleDeg angle_to_pole = ( left_pole - base_pos ).th();

        if ( 90.0 < angle_to_pole.degree()
             && angle_to_pole.degree() < 140.0 )
        {
            agent->debugClient().addMessage( "LPole" );
            return rcsc::Vector2D( danger_move_x, - rcsc::ServerParam::i().goalHalfWidth() - 0.5 );
        }
    }

    //---------------------------------------------------------//
    // ball is near to goal line
    if ( base_pos.x < -rcsc::ServerParam::i().pitchHalfLength() + 8.0
         && base_pos.absY() > rcsc::ServerParam::i().goalHalfWidth() + 2.0 )
    {
        rcsc::Vector2D target_point( base_move_x, rcsc::ServerParam::i().goalHalfWidth() + 0.2 );
        if ( base_pos.y < 0.0 )
        {
            target_point.y *= -1.0;
        }

        rcsc::dlog.addText( rcsc::Logger::TEAM,
                            "%s:%d: getTarget. target is goal pole"
                            ,__FILE__, __LINE__ );
        agent->debugClient().addMessage( "Pos(1)" );

        return target_point;
    }

    //---------------------------------------------------------//
    {
        const double x_back = 6.0; // tune this!!
        int ball_pred_cycle = 5; // tune this!!
        const double y_buf = 0.5; // tune this!!
        const rcsc::Vector2D base_point( - rcsc::ServerParam::i().pitchHalfLength() - x_back,
                                         0.0 );
        rcsc::Vector2D ball_point;
        if ( wm.existKickableOpponent() )
        {
            ball_point = base_pos;
            agent->debugClient().addMessage( "Pos(2)" );
        }
        else
        {
            int opp_min = wm.interceptTable()->opponentReachCycle();
            if ( opp_min < ball_pred_cycle )
            {
                ball_pred_cycle = opp_min;
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                                    "%s:%d: opp may reach near future. cycle = %d"
                                    ,__FILE__, __LINE__, opp_min );
            }

            ball_point
                = rcsc::inertia_n_step_point( base_pos,
                                              wm.ball().vel(),
                                              ball_pred_cycle,
                                              rcsc::ServerParam::i().ballDecay() );
            agent->debugClient().addMessage( "Pos(3)" );
        }

        rcsc::Line2D ball_line( ball_point, base_point );
        double move_y = ball_line.getY( base_move_x );
        if ( move_y > rcsc::ServerParam::i().goalHalfWidth() - y_buf )
        {
            move_y = rcsc::ServerParam::i().goalHalfWidth() - y_buf;
        }
        if ( move_y < - rcsc::ServerParam::i().goalHalfWidth() + y_buf )
        {
            move_y = - rcsc::ServerParam::i().goalHalfWidth() + y_buf;
        }
        return rcsc::Vector2D( base_move_x, move_y );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
double
Bhv_GoalieBasicMove::getBasicDashPower( rcsc::PlayerAgent * agent,
                                        const rcsc::Vector2D & move_point )
{
    const rcsc::WorldModel & wm = agent->world();
    const rcsc::PlayerType & mytype = wm.self().playerType();

    const double my_inc = mytype.staminaIncMax() * wm.self().recovery();

    if ( std::fabs( wm.self().pos().x - move_point.x ) > 3.0 )
    {
        return rcsc::ServerParam::i().maxPower();
    }

    if ( wm.ball().pos().x > -30.0 )
    {
        if ( wm.self().stamina() < rcsc::ServerParam::i().staminaMax() * 0.9 )
        {
            return my_inc * 0.5;
        }
        agent->debugClient().addMessage( "P1" );
        return my_inc;
    }
    else if ( wm.ball().pos().x > rcsc::ServerParam::i().ourPenaltyAreaLineX() )
    {
        if ( wm.ball().pos().absY() > 20.0 )
        {
            // penalty area
            agent->debugClient().addMessage( "P2" );
            return my_inc;
        }
        if ( wm.ball().vel().x > 1.0 )
        {
            // ball is moving to opponent side
            agent->debugClient().addMessage( "P2.5" );
            return my_inc * 0.5;
        }

        int opp_min = wm.interceptTable()->opponentReachCycle();
        if ( opp_min <= 3 )
        {
            agent->debugClient().addMessage( "P2.3" );
            return rcsc::ServerParam::i().maxPower();
        }

        if ( wm.self().stamina() < rcsc::ServerParam::i().staminaMax() * 0.7 )
        {
            agent->debugClient().addMessage( "P2.6" );
            return my_inc * 0.7;
        }
        agent->debugClient().addMessage( "P3" );
        return rcsc::ServerParam::i().maxPower() * 0.6;
    }
    else
    {
        if ( wm.ball().pos().absY() < 15.0
             || wm.ball().pos().y * wm.self().pos().y < 0.0 ) // opposite side
        {
            agent->debugClient().addMessage( "P4" );
            return rcsc::ServerParam::i().maxPower();
        }
        else
        {
            agent->debugClient().addMessage( "P5" );
            return my_inc;
        }
    }
}
