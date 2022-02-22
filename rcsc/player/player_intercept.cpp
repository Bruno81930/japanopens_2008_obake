// -*-c++-*-

/*!
  \file player_intercept.cpp
  \brief intercept predictor for other players Source File
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

#include "player_intercept.h"
#include "world_model.h"
#include "ball_object.h"
#include "player_object.h"

#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include <rcsc/common/player_type.h>
#include <rcsc/soccer_math.h>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
int
PlayerIntercept::predict( const PlayerObject & player,
                          const PlayerType & player_type,
                          const int max_cycle ) const
{
    const double control_area = ( player.goalie()
                                  ? ServerParam::i().catchableArea()
                                  : player_type.kickableArea() );

    Vector2D ball_to_player = player.pos() - M_world.ball().pos();
    ball_to_player.rotate( - M_world.ball().vel().th() );
    int min_cycle = static_cast< int >
        ( std::ceil( ball_to_player.absY() / player_type.realSpeedMax() ) );

    if ( min_cycle < 1 )
    {
        min_cycle = 1;
    }
#ifdef DEBUG
    dlog.addText( Logger::INTERCEPT,
                  "Intercept Player %d (%.1f %.1f)---- start_cycle=%d max_cycle=%d",
                  player.unum(), player.pos().x, player.pos().y,
                  min_cycle, max_cycle );
#endif
    if ( min_cycle > max_cycle )
    {
        return predictFinal( player, player_type );
    }

    const std::size_t MAX_LOOP = std::min( static_cast< std::size_t >( max_cycle ),
                                           M_ball_pos_cache.size() );

    for ( std::size_t cycle = static_cast< std::size_t >( min_cycle );
          cycle < MAX_LOOP;
          ++cycle )
    {
        const Vector2D & ball_pos = M_ball_pos_cache.at( cycle );
#if 0
        dlog.addText( Logger::INTERCEPT,
                      "____cycle=%d  ball(%f %f)",
                      cycle, ball_pos.x, ball_pos.y );
#endif
        if ( control_area + ( player_type.realSpeedMax() * cycle )
             < player.pos().dist( ball_pos ) )
        {
            // never reach
            continue;
        }

        if ( player.goalie()
             && ( ball_pos.absX() < ServerParam::i().pitchHalfLength() - ServerParam::i().penaltyAreaLength()
                  || ball_pos.absY() > ServerParam::i().penaltyAreaHalfWidth() )
             )
        {
            continue;
        }

        if ( canReachAfterTurnDash( cycle,
                                    player, player_type,
                                    control_area,
                                    ball_pos ) )
        {
#ifdef DEBUG
            dlog.addText( Logger::INTERCEPT,
                          "--->cycle=%d  Sucess! ball(%f %f)",
                          cycle, ball_pos.x, ball_pos.y );
#endif
            return cycle;
        }
    }

    return predictFinal( player, player_type );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
PlayerIntercept::canReachAfterTurnDash( const int cycle,
                                        const PlayerObject & player,
                                        const PlayerType & player_type,
                                        const double & control_area,
                                        const Vector2D & ball_pos ) const
{
    int n_turn = predictTurnCycle( cycle,
                                   player,
                                   player_type,
                                   control_area,
                                   ball_pos );
#ifdef DEBUG
    dlog.addText( Logger::INTERCEPT,
                  "______ loop %d  turn step = %d",
                  cycle, n_turn );
#endif

    int n_dash = cycle - n_turn;
    if ( n_dash <= 0 )
    {
        return false;
    }

    return canReachAfterDash( n_turn,
                              n_dash,
                              player,
                              player_type,
                              control_area,
                              ball_pos );
}

/*-------------------------------------------------------------------*/
/*!

*/
int
PlayerIntercept::predictTurnCycle( const int cycle,
                                   const PlayerObject & player,
                                   const PlayerType & player_type,
                                   const double & control_area,
                                   const Vector2D & ball_pos ) const
{
    if ( player.bodyCount() >= 4 )
    {
        return 0;
    }

    Vector2D inertia_pos = player_type.inertiaPoint( player.pos(),
                                                     player.vel(),
                                                     cycle );
    Vector2D target_rel = ball_pos - inertia_pos;
    double target_dist = target_rel.r();
    double turn_margin = 180.0;
    if ( control_area < target_dist )
    {
        turn_margin = AngleDeg::asin_deg( control_area / target_dist );
    }
    turn_margin = std::max( turn_margin, 12.0 );

    double angle_diff = ( target_rel.th() - player.body() ).abs();
    // assume back dash
    if ( target_dist < 5.0 // XXX magic number XXX
         && angle_diff > 90.0 )
    {
        angle_diff = 180.0 - angle_diff;
    }

    if ( angle_diff > turn_margin )
    {
        return 1;
    }

    return 0;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
PlayerIntercept::canReachAfterDash( const int n_turn,
                                    const int n_dash,
                                    const PlayerObject & player,
                                    const PlayerType & player_type,
                                    const double & control_area,
                                    const Vector2D & ball_pos ) const
{
    const Vector2D first_pos
        = player_type.inertiaPoint( player.pos(), player.vel(), n_turn );
    Vector2D tmp_pos = first_pos;
    Vector2D tmp_vel( 0.0, 0.0 ); // if n_turn > 0 , assume that vel == 0
    if ( player.velCount() < 4 )
    {
        tmp_vel = player.vel();
    }
    else
    {
        tmp_vel.setPolar( player_type.realSpeedMax(),
                          ( ball_pos - first_pos ).th() );
    }
    tmp_vel *= std::pow( player_type.playerDecay(), n_turn );

    AngleDeg dash_angle = ( ball_pos - tmp_pos ).th();

    double dash_accel_mag = ( ServerParam::i().maxPower()
                              * player_type.dashRate( player_type.effortMax() ) );
    const Vector2D dash_accel
        = Vector2D::polar2vector( dash_accel_mag, dash_angle );

    const int total_dash = n_dash + std::min( 3, player.posCount() - n_turn );

    for ( int i = 0; i < total_dash; ++i )
    {
        tmp_vel += dash_accel;
        // reach max speed
        if ( tmp_vel.r() > player_type.realSpeedMax() - 0.005 )
        {
            double required_dist
                = ( ball_pos - tmp_pos ).r() - control_area;
            if ( required_dist
                 < player_type.realSpeedMax() * ( total_dash - i ) )
            {
#ifdef DEBUG
                dlog.addText( Logger::INTERCEPT,
                              "________run over by %d dash... p(%f %f) dash_dist=%f",
                              total_dash - i, tmp_pos.x, tmp_pos.y, required_dist );
#endif
                return true;
            }
            return false;
        }

        tmp_pos += tmp_vel;
        tmp_vel *= player_type.playerDecay();
    }

    if ( first_pos.dist( tmp_pos )
         > first_pos.dist( ball_pos ) - control_area )
    {
#ifdef DEBUG
        dlog.addText( Logger::INTERCEPT,
                      "______dash finised %d. run over... b(%f %f) p(%f %f)",
                      total_dash,
                      ball_pos.x, ball_pos.y,
                      tmp_pos.x, tmp_pos.y );
#endif
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------*/
/*!

*/
int
PlayerIntercept::predictFinal( const PlayerObject & player,
                               const PlayerType & player_type ) const
{
    // not found
    Vector2D inertia_pos = player_type.inertiaFinalPoint( player.pos(),
                                                          player.vel() );
    double dash_dist = inertia_pos.dist( M_ball_pos_cache.back() );
    int cycle = std::max( player_type.cyclesToReachDistance( dash_dist ),
                          static_cast< int >( M_ball_pos_cache.size() - 1 ) );
#ifdef DEBUG
    dlog.addText( Logger::INTERCEPT,
                  "____No Solution. calculate final point(%.2f %.2f) dash_dist=%.2f cycle = %d",
                  M_ball_pos_cache.back().x,
                  M_ball_pos_cache.back().y,
                  dash_dist,
                  cycle );
#endif
    return cycle;
}

}
