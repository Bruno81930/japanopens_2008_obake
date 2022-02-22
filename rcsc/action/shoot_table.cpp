// -*-c++-*-

/*!
  \file shoot_table.cpp
  \brief shoot plan search and holder class
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

#include "shoot_table.h"

#include <rcsc/action/body_kick_one_step.h>

#include <rcsc/player/player_agent.h>
#include <rcsc/player/interception.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include <rcsc/math_util.h>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
void
ShootTable::search( const PlayerAgent * agent )
{
    /////////////////////////////////////////////////////////////////////
    const WorldModel & wm = agent->world();

    if ( M_time == wm.time() )
    {
        return;
    }

    M_time = wm.time();
    M_shots.clear();

    /////////////////////////////////////////////////////////////////////
    static const Rect2D
        shootable_area( ServerParam::i().theirPenaltyAreaLineX() - 5.0, // left
                        -ServerParam::i().penaltyAreaHalfWidth(), // top
                        ServerParam::i().penaltyAreaLength() + 5.0, // length
                        ServerParam::i().penaltyAreaWidth() ); // width

    if ( ! wm.self().isKickable()
         || ! shootable_area.contains( wm.ball().pos() ) )
    {
        return;
    }

    Vector2D goal_l( ServerParam::i().pitchHalfLength(),
                     -ServerParam::i().goalHalfWidth() );
    Vector2D goal_r( ServerParam::i().pitchHalfLength(),
                     ServerParam::i().goalHalfWidth() );

    goal_l.y += std::min( 1.5,
                          0.6 + goal_l.dist( wm.ball().pos() ) * 0.042 );
    goal_r.y -= std::min( 1.5,
                          0.6 + goal_r.dist( wm.ball().pos() ) * 0.042 );

    const int DIST_DIVS = 15;
    const double dist_step = std::fabs( goal_l.y - goal_r.y ) / ( DIST_DIVS - 1 );


    dlog.addText( Logger::SHOOT,
                  "Shoot search range=(%.1f %.1f)-(%.1f %.1f) dist_step=%.1f",
                  goal_l.x, goal_l.y, goal_r.x, goal_r.y, dist_step );

    const PlayerObject * goalie = agent->world().getOpponentGoalie();

    Vector2D shot_point = goal_l;

    for ( int i = 0;
          i < DIST_DIVS;
          ++i, shot_point.y += dist_step )
    {
        calculateShotPoint( wm, shot_point, goalie );
    }

    //std::sort( M_shots.begin(),
    //           M_shots.end(),
    //           ScoreCmp() );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ShootTable::calculateShotPoint( const WorldModel & wm,
                                const Vector2D & shot_point,
                                const PlayerObject * goalie )
{
    Vector2D shot_rel = shot_point - wm.ball().pos();
    AngleDeg shot_angle = shot_rel.th();

    int goalie_count = 1000;
    if ( goalie )
    {
        goalie_count = goalie->posCount();
    }

    if ( 5 < goalie_count
         && goalie_count < 30
         && wm.dirCount( shot_angle ) > 3 )
    {
        return;
    }

    double shot_dist = shot_rel.r();

    Vector2D one_step_vel
        = Body_KickOneStep::get_max_possible_vel( shot_angle,
                                                  wm.self().kickRate(),
                                                  wm.ball().vel() );
    double max_one_step_speed = one_step_vel.r();

    double shot_first_speed
        = ( shot_dist + 5.0 ) * ( 1.0 - ServerParam::i().ballDecay() );
    shot_first_speed = std::max( max_one_step_speed, shot_first_speed );
    shot_first_speed = std::max( 1.5, shot_first_speed );

    // gaussian function, distribution = goal half width
    //double y_rate = std::exp( - std::pow( shot_point.y, 2.0 )
    //                          / ( 2.0 * ServerParam::i().goalHalfWidth() * 3.0 ) );
    double y_dist = std::max( 0.0, shot_point.absY() - 4.0 );
    double y_rate = std::exp( - std::pow( y_dist, 2.0 )
                              / ( 2.0 * ServerParam::i().goalHalfWidth() ) );

    bool over_max = false;
    while ( ! over_max )
    {
        if ( shot_first_speed > ServerParam::i().ballSpeedMax() - 0.001 )
        {
            over_max = true;
            shot_first_speed = ServerParam::i().ballSpeedMax();
        }

        Shot shot( shot_point, shot_first_speed, shot_angle );
        shot.score_ = 0;

        if ( canScore( wm, &shot ) )
        {
            shot.score_ += 100;
            bool one_step = ( shot_first_speed <= max_one_step_speed );
            if ( one_step )
            {   // one step kick
                shot.score_ += 100;
            }

            double goalie_rate = -1.0;
            if ( shot.goalie_never_reach_ )
            {
                shot.score_ += 100;
            }

            if ( goalie )
            {
                AngleDeg goalie_angle = ( goalie->pos() - wm.ball().pos() ).th();
                double angle_diff = ( shot.angle_ - goalie_angle ).abs();
                goalie_rate = 1.0 - std::exp( - std::pow( angle_diff * 0.1, 2.0 )
                                              / ( 2.0 * 90.0 * 0.1 ) );
                shot.score_ = static_cast< int >( shot.score_ * goalie_rate );
#if 0
                dlog.addText( Logger::SHOOT,
                              "--- apply goalie rate. angle_diff=%.1f rate=%.2f",
                              angle_diff, goalie_rate );
#endif
            }

            shot.score_ = static_cast< int >( shot.score_ * y_rate );

            dlog.addText( Logger::SHOOT,
                          "<<< Shoot score=%d pos(%.1f %.1f)"
                          " angle=%.1f speed=%.1f"
                          " y_rate=%.2f g_rate=%.2f"
                          " one_step=%d GK_never_reach=%d",
                          shot.score_,
                          shot_point.x, shot_point.y,
                          shot_angle.degree(),
                          shot_first_speed,
                          y_rate, goalie_rate,
                          ( one_step ? 1 : 0 ),
                          ( shot.goalie_never_reach_ ? 1 : 0 ) );

            M_shots.push_back( shot );
        }
#if 0
        else
        {
            dlog.addText( Logger::SHOOT,
                          ">>> Shoot failed to(%.1f %.1f)"
                          " angle=%.1f speed=%.1f",
                          shot_point.x, shot_point.y,
                          shot_angle.degree(),
                          shot_first_speed );
        }
#endif
        shot_first_speed += 0.5;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
ShootTable::canScore( const WorldModel & wm,
                      Shot * shot )
{
    static const double opp_x_thr = ServerParam::i().theirPenaltyAreaLineX() - 5.0;
    static const double opp_y_thr = ServerParam::i().penaltyAreaHalfWidth();

    static const double player_max_speed
        = ServerParam::i().defaultPlayerSpeedMax();
    static const double player_control_area
        = ServerParam::i().defaultKickableArea();

    // estimate required ball travel step
    const double ball_reach_step
        = calc_length_geom_series( shot->speed_,
                                   wm.ball().pos().dist( shot->point_ ),
                                   ServerParam::i().ballDecay() );

    if ( ball_reach_step < 1.0 )
    {
        shot->score_ += 100;
        return true;
    }

    // estimate opponent interception
    const Interception util( wm.ball().pos() + shot->vel_,
                             shot->speed_ * ServerParam::i().ballDecay(),
                             shot->angle_ );

    const PlayerPtrCont::const_iterator end = wm.opponentsFromSelf().end();
    for ( PlayerPtrCont::const_iterator it = wm.opponentsFromSelf().begin();
          it != end;
          ++it )
    {
        // outside of penalty
        if ( (*it)->pos().x < opp_x_thr ) continue;
        if ( (*it)->pos().absY() > opp_y_thr ) continue;
        // behind of shoot course
        if ( ( shot->angle_ - (*it)->angleFromSelf() ).abs() > 90.0 )
        {
            continue;
        }

        if ( (*it)->goalie() )
        {
            if ( maybeGoalieCatch( wm, *it, shot ) )
            {
                return false;
            }
        }
        else
        {
#if 1
            if ( (*it)->posCount() > 10
                 || ( (*it)->isGhost() && (*it)->posCount() > 5 ) )
            {
                continue;
            }
#endif

            double cycle = ( std::ceil( util.getReachCycle( (*it)->pos(),
                                                            &((*it)->vel()),
                                                            NULL,
                                                            std::min( 2, (*it)->posCount() ),
                                                            player_control_area,
                                                            player_max_speed ) ) );
            if ( cycle < ball_reach_step )
            {
                return false;
            }
        }
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
ShootTable::maybeGoalieCatch( const WorldModel & wm,
                              const PlayerObject * goalie,
                              Shot * shot )
{
    static const
        Rect2D penalty_area( ServerParam::i().theirPenaltyAreaLineX(), // left
                             -ServerParam::i().penaltyAreaHalfWidth(), // top
                             ServerParam::i().penaltyAreaLength(), // length
                             ServerParam::i().penaltyAreaWidth() ); // width
    static const double catchable_area = ServerParam::i().catchableArea();

    const double dash_accel_mag = ( ServerParam::i().maxPower()
                                    * ServerParam::i().defaultDashPowerRate()
                                    * ServerParam::i().defaultEffortMax() );
    const double seen_dist_noise = goalie->distFromSelf() * 0.05;

    int min_cycle = 1;
    {
        Line2D shot_line( wm.ball().pos(), shot->point_ );
        double goalie_line_dist
            = std::max( 0.0, shot_line.dist( goalie->pos() ) - catchable_area );
        goalie_line_dist -= seen_dist_noise;
        min_cycle = static_cast< int >
            ( std::ceil( goalie_line_dist / ServerParam::i().defaultRealSpeedMax() ) ) ;
        min_cycle -= std::min( 5, goalie->posCount() );
        min_cycle = std::max( 1, min_cycle );
    }
    Vector2D ball_pos = inertia_n_step_point( wm.ball().pos(),
                                              shot->vel_,
                                              min_cycle,
                                              ServerParam::i().ballDecay() );
    Vector2D ball_vel = ( shot->vel_
                          * std::pow( ServerParam::i().ballDecay(), min_cycle ) );


    int cycle = min_cycle;
    while ( ball_pos.x < ServerParam::i().pitchHalfLength() )
    {
        // estimate the required turn angle
        Vector2D goalie_pos
            = inertia_n_step_point( goalie->pos(),
                                    goalie->vel(),
                                    cycle,
                                    ServerParam::i().defaultPlayerDecay() );
        Vector2D ball_relative = ball_pos - goalie_pos;
        double ball_dist = ball_relative.r() - seen_dist_noise;

        if ( ball_dist < catchable_area )
        {
            return true;
        }

        if ( ball_dist < catchable_area + 1.0 )
        {
            shot->goalie_never_reach_ = false;
        }

        AngleDeg ball_angle = ball_relative.th();
        AngleDeg goalie_body = ( goalie->bodyCount() <= 5
                                 ? goalie->body()
                                 : ball_angle );

        // simulate turn
        int n_turn = 0;

        double angle_diff = ( ball_angle - goalie_body ).abs();
        if ( angle_diff > 90.0 )
        {
            angle_diff = 180.0 - angle_diff; // back dash
            goalie_body -= 180.0;
        }

        double turn_margin
            = std::max( AngleDeg::asin_deg( catchable_area / ball_dist ),
                        15.0 );

#if 0
        dlog.addText( Logger::SHOOT,
                      "_ t cycle=%d ball_pos(%.1f %.1f) g_body=%.0f b_angle=%.1f angle_diff=%.1f turn_margin=%.1f",
                      cycle,
                      ball_pos.x, ball_pos.y,
                      goalie_body.degree(), ball_angle.degree(),
                      angle_diff, turn_margin );
#endif

        Vector2D goalie_vel = goalie->vel();

        while ( angle_diff > turn_margin )
        {
            double max_turn
                = effective_turn( 180.0,
                                  goalie_vel.r(),
                                  ServerParam::i().defaultInertiaMoment() );
            angle_diff -= max_turn;
            goalie_vel *= ServerParam::i().defaultPlayerDecay();
            ++n_turn;
        }

        // simulate dash
        goalie_pos
            = inertia_n_step_point( goalie->pos(),
                                    goalie->vel(),
                                    n_turn,
                                    ServerParam::i().defaultPlayerDecay() );

        Vector2D dash_accel = Vector2D::polar2vector( dash_accel_mag,
                                                      ball_angle );
        {
            goalie_vel += dash_accel;
            if ( goalie_vel.r() > ServerParam::i().defaultRealSpeedMax() )
            {
                goalie_vel.setLength( ServerParam::i().defaultRealSpeedMax() );
                goalie_vel *= ServerParam::i().ballDecay();
            }
            else
            {
                goalie_vel -= dash_accel;
            }
        }

        const int max_dash = ( cycle - 1
                               - n_turn
                               + std::min( 5, goalie->posCount() ) );
        double goalie_travel = 0.0;
        for ( int i = 0; i < max_dash; ++i )
        {
            goalie_vel += dash_accel;
            goalie_pos += goalie_vel;
            goalie_travel += goalie_vel.r();
            goalie_vel *= ServerParam::i().defaultPlayerDecay();

            double d = goalie_pos.dist( ball_pos ) - seen_dist_noise;
#if 0
            dlog.addText( Logger::SHOOT,
                          "_   cycle=%d ball(%.1f %.1f) angle=%.0f goalie pos(%.1f %.1f) travel=%.1f dist=%.1f turn=%d dash=%d",
                          cycle,
                          ball_pos.x, ball_pos.y,
                          ball_angle.degree(),
                          goalie_pos.x, goalie_pos.y,
                          goalie_travel,
                          d,
                          n_turn, i + 1 );
#endif
            if ( d < catchable_area + 1.0 + ( goalie_travel * 0.04 ) )
            {
                shot->goalie_never_reach_ = false;
            }
        }

        // check distance
        if ( goalie->pos().dist( goalie_pos ) * 1.05
             > goalie->pos().dist( ball_pos ) - seen_dist_noise
             - catchable_area - 0.2 )
        {
            return true;
        }

        // update ball position & velocity
        ++cycle;
        ball_pos += ball_vel;
        ball_vel *= ServerParam::i().ballDecay();
    }

    return false;
}

}
