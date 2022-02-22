// -*-c++-*-

/*!
  \file basic_actions.cpp
  \brief basic player actions Source File
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

#include "basic_actions.h"

#include "bhv_scan_field.h"
#include "neck_scan_field.h"

#include <rcsc/player/player_agent.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>

namespace rcsc {


/////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_TurnToAngle::execute( PlayerAgent * agent )
{
    dlog.addText( Logger::ACTION,
                  __FILE__": Body_TurnToAngle" );

    const SelfObject & self = agent->world().self();

    if ( ! self.faceValid() )
    {
        agent->doTurn( 0.0 );
        return false;
    }

    agent->doTurn( M_angle - self.body() );
    return true;
}

/////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_TurnToPoint::execute( PlayerAgent * agent )
{
    dlog.addText( Logger::ACTION,
                  __FILE__": Body_TurnToPoint" );

    const SelfObject & self = agent->world().self();

    if ( ! self.posValid() )
    {
        return agent->doTurn( 60.0 );
    }

    // relative angle from my predicted pos & body angle to point
    Vector2D my_point
        = self.playerType().inertiaPoint( self.pos(),
                                          self.vel(),
                                          M_cycle );
    AngleDeg target_rel_angle
        = ( M_point - my_point ).th() - self.body();


    // not consider about max effective turn (inertia moment)
    // try to face to point greedy

    agent->doTurn( target_rel_angle );

    if ( target_rel_angle.abs() < 1.0 )
    {
        return false;
    }
    return true;
}

/////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_TurnToBall::execute( PlayerAgent * agent )
{
    dlog.addText( Logger::ACTION,
                  __FILE__": Body_TurnToBall" );

    if ( ! agent->world().ball().posValid() )
    {
        return false;
    }

    Vector2D ball_point = agent->world().ball().inertiaPoint( M_cycle );

    return Body_TurnToPoint( ball_point, M_cycle ).execute( agent );
}

/////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_TackleToPoint::execute( PlayerAgent * agent )
{
    dlog.addText( Logger::ACTION,
                  __FILE__": Body_TackleToPoint" );

    if ( agent->world().self().tackleProbability() < 0.01 )
    {
        return false;
    }

    AngleDeg target_rel_angle
        = ( M_point - agent->world().ball().pos() ).th()
        - agent->world().self().body();

    if ( agent->config().version() < 12.0 )
    {
        if ( target_rel_angle.abs() < 90.0 )
        {
            return agent->doTackle( ServerParam::i().maxTacklePower() );
        }
        else if ( ServerParam::i().maxBackTacklePower() > 0.0 )
        {
            // backward case
            return agent->doTackle( - ServerParam::i().maxBackTacklePower() );
        }
        return false;
    }

    // TODO: calculate the effective tackle power


    return agent->doTackle( target_rel_angle.degree() );
}

/////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------*/
/*!

*/
bool
Neck_TurnToRelative::execute( PlayerAgent * agent )
{
    dlog.addText( Logger::ACTION,
                  __FILE__": Neck_TurnToRelative" );

    return agent->doTurnNeck( M_angle_rel_to_body
                              - agent->world().self().neck() );
}

/////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------*/
/*!

*/
bool
Neck_TurnToPoint::execute( PlayerAgent * agent )
{
    dlog.addText( Logger::ACTION,
                  __FILE__": Neck_TurnToPoint" );

    const Vector2D next_pos = agent->effector().queuedNextMyPos();
    const AngleDeg next_body = agent->effector().queuedNextMyBody();
    const double next_view_width = agent->effector().queuedNextViewWidth().width() * 0.5;

    for ( std::vector< Vector2D >::const_iterator p = M_points.begin();
          p != M_points.end();
          ++p )
    {
        Vector2D rel_pos = *p - next_pos;
        AngleDeg rel_angle = rel_pos.th() - next_body;

        if ( rel_angle.abs() < ServerParam::i().maxNeckAngle() + next_view_width - 5.0 )
        {
            dlog.addText( Logger::ACTION,
                          "%s:%d Neck_TurnToPoint (%.1f %.1f) rel_angle = %.1f"
                          ,__FILE__, __LINE__,
                          p->x, p->y, rel_angle.degree() );
            return agent->doTurnNeck( rel_angle - agent->world().self().neck() );
        }
    }

    dlog.addText( Logger::ACTION,
                  __FILE__": Neck_TurnToPoint. cannot turn neck to target points. scan" );
    Neck_ScanField().execute( agent );
    return true;
}

/////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------*/
/*!

*/
bool
Neck_TurnToBall::execute( PlayerAgent * agent )
{
    dlog.addText( Logger::ACTION,
                  __FILE__": Neck_TurnToBall" );

    if ( ! agent->world().ball().posValid() )
    {
        Neck_ScanField().execute( agent );
        return true;
    }

    const Vector2D my_next = agent->effector().queuedNextMyPos();
    const AngleDeg my_body_next = agent->effector().queuedNextMyBody();

    const Vector2D ball_next = agent->effector().queuedNextBallPos();
    const AngleDeg ball_angle_next = ( ball_next - my_next ).th();
    const AngleDeg ball_rel_angle_next = ball_angle_next - my_body_next;

    const double next_view_width = agent->effector().queuedNextViewWidth().width();

    dlog.addText( Logger::ACTION,
                  __FILE__":%d: ball_next=(%.2f, %.2f), ball_rel_angle=%.1f, next_view=%.1f"
                  , __LINE__,
                  ball_next.x, ball_next.y,
                  ball_rel_angle_next.degree(),
                  next_view_width );

    // never look the ball
    if ( ball_rel_angle_next.abs()
         > ServerParam::i().maxNeckAngle() + next_view_width * 0.5 )
    {
        Neck_ScanField().execute( agent );
        return true;
    }

    // ball is near, directly look the ball
    if ( my_next.dist( ball_next ) < 15.0 ) // [200803]
    {
        agent->doTurnNeck( ball_rel_angle_next
                           - agent->world().self().neck() );
        return true;
    }

    double angle_buf = std::max( 0.0, next_view_width * 0.5 - 10.0 ); //[200803] 15.0

    AngleDeg left_rel_angle = ball_rel_angle_next - angle_buf;
    AngleDeg right_rel_angle = ball_rel_angle_next + angle_buf;

    if ( left_rel_angle.isLeftOf( ServerParam::i().minNeckAngle() ) )
    {
        left_rel_angle = ServerParam::i().minNeckAngle();
    }

    if ( right_rel_angle.isRightOf( ServerParam::i().maxNeckAngle() ) )
    {
        right_rel_angle = ServerParam::i().maxNeckAngle();
    }

    int left_sum_count = 0;
    int right_sum_count = 0;

    agent->world().dirRangeCount( my_body_next + left_rel_angle,
                                  next_view_width,
                                  NULL, &left_sum_count, NULL );
    agent->world().dirRangeCount( my_body_next + right_rel_angle,
                                  next_view_width,
                                  NULL, &right_sum_count, NULL );

    dlog.addText( Logger::ACTION,
                  "%s:%d: angle_buf=%.0f  left_rel=%.0f right_rel=%.0f"
                  " left_sum=%d  right_sum=%d"
                  ,__FILE__, __LINE__,
                  angle_buf,
                  left_rel_angle.degree(), right_rel_angle.degree(),
                  left_sum_count, right_sum_count );


    if ( left_sum_count > right_sum_count )
    {
        agent->doTurnNeck( left_rel_angle - agent->world().self().neck() );
    }
    else
    {
        agent->doTurnNeck( right_rel_angle - agent->world().self().neck() );
    }

    return true;
}

/////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------*/
/*!

*/
bool
Bhv_BodyNeckToPoint::execute( PlayerAgent * agent )
{
    dlog.addText( Logger::ACTION,
                  __FILE__": Bhv_BodyNeckToPoint" );

    if ( ! agent->world().self().posValid() )
    {
        return Bhv_ScanField().execute( agent );
    }

    // relative angle from my predicted pos & body angle to point
    Vector2D mynext
        = agent->world().self().pos()
        + agent->world().self().vel();

    AngleDeg target_rel_angle
        = ( M_point - mynext ).th()
        - agent->world().self().body();

    // body & neck is already face to point.
    //if ( (agent->world().self().neck() - target_rel_angle).abs() < 1.0
    //     && target_rel_angle.abs() < 1.0 )
    //{
    //    return false;
    //}

    // calc the max turn angle with current my speed.
    double max_turn
        = effective_turn( ServerParam::i().maxMoment(),
                          agent->world().self().vel().r(),
                          agent->world().self().playerType().inertiaMoment());

    // can face only turn
    if ( target_rel_angle.abs() < max_turn )
    {
        Body_TurnToPoint( M_point ).execute( agent );
        agent->setNeckAction( new Neck_TurnToRelative( 0.0 ) );
        return true;
    }
    // cannot face only turn
    // turn_neck is required.
    agent->doTurn( target_rel_angle );

    if ( target_rel_angle.degree() > 0.0 )
    {
        target_rel_angle -= max_turn;
    }
    else
    {
        target_rel_angle += max_turn;
    }

    if ( target_rel_angle.degree() < ServerParam::i().minNeckAngle() )
    {
        target_rel_angle = ServerParam::i().minNeckAngle();
    }
    else if ( target_rel_angle.degree() > ServerParam::i().maxNeckAngle() )
    {
        target_rel_angle = ServerParam::i().maxNeckAngle();
    }

    agent->setNeckAction( new Neck_TurnToRelative( target_rel_angle ) );
    return true;
}

/////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------*/
/*!

*/
bool
Bhv_BodyNeckToBall::execute( PlayerAgent * agent )
{
    dlog.addText( Logger::ACTION,
                  __FILE__": Bhv_BodyNeckToBall" );

    if ( agent->world().ball().posValid() )
    {
        Vector2D ballnext
            = agent->world().ball().pos()
            + agent->world().ball().vel();
        return Bhv_BodyNeckToPoint( ballnext ).execute( agent );
    }
    else
    {
        return Bhv_ScanField().execute( agent );
    }
}

/////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------*/
/*!

*/
bool
Bhv_NeckBodyToPoint::execute( PlayerAgent * agent )
{
    dlog.addText( Logger::ACTION,
                  __FILE__": Bhv_NeckBodyToPoint.(%.1f, %.1f)",
                  M_point.x, M_point.y );

    // relative angle from my predicted pos & body angle to point
    Vector2D mynext
        = agent->world().self().pos()
        + agent->world().self().vel();

    AngleDeg target_rel_angle
        = ( M_point - mynext ).th()
        - agent->world().self().body();

    //if ( ( agent->world().self().neck() - target_rel_angle ).abs() < 1.0 )
    //{
    //    dlog.addText( Logger::ACTION,
    //                  "%s:%d: alerady facing"
    //                  ,__FILE__, __LINE__ );
    //    return false;
    //}

    // can face to point only by turn_neck.
    if ( ServerParam::i().minNeckAngle() + 5.0 < target_rel_angle.degree()
         && target_rel_angle.degree() < ServerParam::i().maxNeckAngle() - 5.0 )
    {
        agent->doTurn( 0.0 );
        agent->setNeckAction( new Neck_TurnToRelative( target_rel_angle ) );
        return true;
    }
    // calc the max turn angle with current my speed
    double max_turn
        = effective_turn( ServerParam::i().maxMoment(),
                          agent->world().self().vel().r(),
                          agent->world().self().playerType().inertiaMoment() );

    // can face only by turn
    if ( target_rel_angle.abs() < max_turn )
    {
        dlog.addText( Logger::ACTION,
                      "%s:%d: can face only turn"
                      ,__FILE__, __LINE__ );
        agent->doTurn( target_rel_angle );
        agent->setNeckAction( new Neck_TurnToRelative( 0.0 ) );
        return true;
    }

    // cannot face only turn
    // turn_neck is required.
    agent->doTurn( target_rel_angle );

    if ( target_rel_angle.degree() > 0.0 )
    {
        target_rel_angle -= max_turn;
    }
    else
    {
        target_rel_angle += max_turn;
    }

    dlog.addText( Logger::ACTION,
                  "%s:%d: turn & turn_neck"
                  ,__FILE__, __LINE__ );
    // moment is justified automatically.
    agent->setNeckAction( new Neck_TurnToRelative( target_rel_angle ) );
    return true;
}

/////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------*/
/*!

*/
bool
Bhv_NeckBodyToBall::execute( PlayerAgent * agent )
{
    dlog.addText( Logger::ACTION,
                  __FILE__": Bhv_NeckBodyToBall" );

    if ( agent->world().ball().posValid() )
    {
        Vector2D ball_next
            = agent->world().ball().pos()
            + agent->world().ball().vel();

        return Bhv_NeckBodyToPoint( ball_next ).execute( agent );
    }
    else
    {
        return Bhv_ScanField().execute( agent );
    }
}

/////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------*/
/*!

*/
bool
View_Wide::execute( PlayerAgent * agent )
{
    dlog.addText( Logger::ACTION,
                  __FILE__": View_Wide" );

    return agent->doChangeView( ViewWidth::WIDE );
}

/////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------*/
/*!

*/
bool
View_Normal::execute( PlayerAgent * agent )
{
    dlog.addText( Logger::ACTION,
                  __FILE__": View_Normal" );

    return agent->doChangeView( ViewWidth::NORMAL );
}

/////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------*/
/*!

*/
bool
Arm_PointToPoint::execute( PlayerAgent * agent )
{
    dlog.addText( Logger::ACTION,
                  __FILE__": Arm_PointToPoint" );

    if ( agent->world().self().armMovable() > 0 )
    {
        dlog.addText( Logger::ACTION,
                      "Arm_PointToPoint. arm is not movable." );
        return false;
    }

    return agent->doPointto( M_point.x, M_point.y );
}

/////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------*/
/*!

*/
bool
Arm_Off::execute( PlayerAgent * agent )
{
    dlog.addText( Logger::ACTION,
                  __FILE__": Arm_Off" );

    if ( agent->world().self().armMovable() > 0 )
    {
        dlog.addText( Logger::ACTION,
                      "Arm_Off. arm is not movable." );
        return false;
    }

    return agent->doPointtoOff();
}

/////////////////////////////////////////////////////////////////////

}
