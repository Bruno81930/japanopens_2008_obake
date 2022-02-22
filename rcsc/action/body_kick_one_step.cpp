// -*-c++-*-

/*!
  \file body_kick_one_step.cpp
  \brief one step kick behavior.
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

#include "body_kick_one_step.h"

#include "body_stop_ball.h"
#include "body_kick_to_relative.h"

#include <rcsc/player/player_agent.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include <rcsc/geom/circle_2d.h>
#include <rcsc/geom/ray_2d.h>

#include <algorithm>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_KickOneStep::execute( PlayerAgent * agent )
{
    dlog.addText( Logger::KICK,
                  "%s:%d: Body_KickOneStep"
                  ,__FILE__, __LINE__ );

    const WorldModel & wm = agent->world();

    if ( ! wm.self().isKickable() )
    {
        std::cerr << __FILE__ << ": " << __LINE__
                  << " not ball kickable!"
                  << std::endl;
        dlog.addText( Logger::ACTION,
                      "%s:%d:  not kickable"
                      ,__FILE__, __LINE__ );
        return false;
    }

    if ( ! wm.ball().velValid() )
    {
        dlog.addText( Logger::KICK,
                      "%s:%d. unknown ball vel"
                      ,__FILE__, __LINE__ );
        return Body_StopBall().execute( agent );
    }

    M_first_speed = std::min(M_first_speed, ServerParam::i().ballSpeedMax());

    const AngleDeg target_angle
        = ( M_target_point - wm.ball().pos() ).th();

    Vector2D first_vel = get_max_possible_vel( target_angle,
                                              wm.self().kickRate(),
                                              wm.ball().vel() );
    if ( first_vel.r() > M_first_speed )
    {
        first_vel.setLength( M_first_speed );
    }
    else
    {
        dlog.addText( Logger::KICK,
                      "%s:%d: cannot get required vel. only angle adjusted"
                      ,__FILE__, __LINE__ );
    }

    // first_vel.r() may be less than M_first_speed ...


    const Vector2D real_accel = first_vel - wm.ball().vel();

    const double kick_power = real_accel.r() / wm.self().kickRate();
    const AngleDeg kick_dir = real_accel.th() - wm.self().body();

    if ( kick_power > ServerParam::i().maxPower() + 0.01 )
    {
        dlog.addText( Logger::KICK,
                      "%s:%d: why power over??"
                      ,__FILE__, __LINE__ );
        double keep_dist = wm.self().playerType().playerSize()
            + wm.self().playerType().kickableMargin() * 0.6;
        // keep current angle
        AngleDeg keep_angle = wm.ball().angleFromSelf()
            - wm.self().body();

        return Body_KickToRelative( keep_dist,
                                    keep_angle,
                                    false
                                    ).execute( agent );
    }

    dlog.addText( Logger::KICK,
                  "%s:%d: first_speed= %.3f, angle= %.1f, power= %.1f, dir= %.1f "
                  ,__FILE__, __LINE__,
                  first_vel.r(), first_vel.th().degree(),
                  kick_power, kick_dir.degree() );

    M_ball_result_pos = wm.ball().pos() + first_vel;
    M_ball_result_vel = first_vel * ServerParam::i().ballDecay();
    M_kick_step = 1;

    return agent->doKick( kick_power, kick_dir );
}

/*-------------------------------------------------------------------*/
/*!

*/
Vector2D
Body_KickOneStep::get_max_possible_vel( const AngleDeg & target_angle,
                                        const double & krate,
                                        const Vector2D & ball_vel )
{
    // ball info may be the estimated value..

    const double max_accel
        = std::min( ServerParam::i().maxPower() * krate,
                    ServerParam::i().ballAccelMax() );

    // origin point is current ball pos
    Ray2D desired_ray( Vector2D( 0.0, 0.0 ), target_angle );
    // center point is next ball pos relative to current ball pos
    Circle2D next_reachable_circle( ball_vel, max_accel );


    // sol is ball vel (= current ball vel + accel)
    Vector2D sol1, sol2; // rel to current ball pos
    int num = next_reachable_circle.intersection( desired_ray, &sol1, &sol2 );

    if ( num == 0 )
    {
        dlog.addText( Logger::KICK,
                      "%s:%d: get_max_possible_vel: angle=%.1f. No solution"
                      ,__FILE__, __LINE__,
                      target_angle.degree() );
        return Vector2D( 0.0, 0.0 );
    }

    if ( num == 1 )
    {
        if ( sol1.r() > ServerParam::i().ballSpeedMax() )
        {
            // next inertia ball point is within reachable circle.
            if ( next_reachable_circle.contains( Vector2D( 0.0, 0.0 ) ) )
            {
                // can adjust angle at least
                sol1.setLength( ServerParam::i().ballSpeedMax() );

                dlog.addText( Logger::KICK,
                              "%s:%d: get_max_possible_vel: angle=%.1f."
                              " 1 solution  adjust."
                              ,__FILE__, __LINE__,
                              target_angle.degree() );
            }
            else
            {
                // failed
                sol1.assign( 0.0, 0.0 );

                dlog.addText( Logger::KICK,
                              "%s:%d: get_max_possible_vel: angle=%.1f."
                              " 1 solution. failed."
                              ,__FILE__, __LINE__,
                              target_angle.degree() );
            }
        }
#ifdef DEBUG
        dlog.addText( Logger::KICK,
                      "one kick -----> angle=%.1f max_vel=(%.2f, %.2f)r=%.2f",
                      target_angle.degree(),
                      sol1.x, sol1.y, sol1.r() );
#endif
        return sol1;
    }


    // num == 2

    double length1 = sol1.r();
    double length2 = sol2.r();

    if ( length1 < length2 )
    {
        std::swap( sol1, sol2 );
        std::swap( length1, length2 );
        dlog.addText( Logger::KICK,
                      "%s:%d: get_max_possible_vel: swap"
                      ,__FILE__, __LINE__ );
    }

    if ( length1 > ServerParam::i().ballSpeedMax() )
    {
        if ( length2 > ServerParam::i().ballSpeedMax() )
        {
            sol1.assign( 0.0, 0.0 );

            dlog.addText( Logger::KICK,
                          "%s:%d: get_max_possible_vel: angle=%.1f."
                          " 2 solutions. but never reach"
                          ,__FILE__, __LINE__,
                          target_angle.degree() );
        }
        else
        {
            sol1.setLength( ServerParam::i().ballSpeedMax() );

            dlog.addText( Logger::KICK,
                          "%s:%d: get_max_possible_vel: angle=%.1f."
                          " 2 solutions. adjust to ballSpeedMax"
                          ,__FILE__, __LINE__,
                          target_angle.degree() );
        }
    }

    dlog.addText( Logger::KICK,
                  "one kick: 2 solutions: angle=%.1f max_vel=(%.2f, %.2f)r=%.2f",
                  target_angle.degree(),
                  sol1.x, sol1.y, sol1.r() );

    return sol1;
}

}
