// -*-c++-*-

/*!
  \file ball_object.cpp
  \brief ball object class Source File
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

#include "ball_object.h"

#include "action_effector.h"
#include "self_object.h"
#include "player_command.h"

#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include <rcsc/game_mode.h>

#include <iostream>

namespace rcsc {

int BallObject::S_pos_count_thr = 10;
int BallObject::S_rpos_count_thr = 5;
int BallObject::S_vel_count_thr = 10;

/*-------------------------------------------------------------------*/
/*!

*/
BallObject::BallObject()
    : M_dist_from_self( 1000.0 )
    , M_angle_from_self( 0.0 )
    , M_pos( 0.0, 0.0 )
    , M_pos_error( 0.0, 0.0 )
    , M_pos_count( 1000 )
    , M_rpos( Vector2D::INVALIDATED )
    , M_rpos_error( 0.0, 0.0 )
    , M_rpos_count( 1000 )
    , M_rpos_prev( Vector2D::INVALIDATED )
    , M_seen_pos( 0.0, 0.0 )
    , M_seen_pos_count( 1000 )
    , M_seen_rpos( Vector2D::INVALIDATED )
    , M_heard_pos( 0.0, 0.0 )
    , M_heard_pos_count( 1000 )
    , M_vel( 0.0, 0.0 )
    , M_vel_error( 0.0, 0.0 )
    , M_vel_count( 1000 )
    , M_heard_vel( 0.0, 0.0 )
    , M_heard_vel_count( 1000 )
    , M_lost_count( 0 )
    , M_ghost_time( -1, 0 )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
void
BallObject::set_count_thr( const int pos_thr,
                           const int rpos_thr,
                           const int vel_thr )
{
    S_pos_count_thr = pos_thr;
    S_rpos_count_thr = rpos_thr;
    S_vel_count_thr = vel_thr;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
BallObject::setGhost( const GameTime & current )
{
    M_dist_from_self = 1000.0;
    M_pos_count = 1000;
    M_rpos_count = 1000;
    M_lost_count = 0;
    M_ghost_time = current;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
BallObject::update( const ActionEffector & act,
                    const GameMode & game_mode,
                    const GameTime & current )
{
    Vector2D new_vel( 0.0, 0.0 );

    ////////////////////////////////////////////////////////////////////////
    // vel
    if ( velValid() )
    {
        Vector2D accel( 0.0, 0.0 );
        Vector2D accel_err( 0.0, 0.0 );
        double tmp = 0.0;

        new_vel = M_vel;

        /////////////////////////////////////////////////////////////
        // kicked in last cycle
        // get info from stored action param
        if ( act.lastBodyCommandType() == PlayerCommand::KICK )
        {
            act.getKickInfo( &accel, &accel_err );
            dlog.addText( Logger::WORLD,
                          "Ball update. get queued kick accel (%f, %f)",
                          accel.x, accel.y );

            // check max accel
            tmp = accel.r();
            if ( tmp > ServerParam::i().ballAccelMax() )
            {
                accel *= ( ServerParam::i().ballAccelMax() / tmp );
            }

            new_vel += accel;
        }


        // check max vel
        tmp = new_vel.r();
        if ( tmp > ServerParam::i().ballSpeedMax() )
        {
            new_vel *= ( ServerParam::i().ballSpeedMax() / tmp );
            tmp = ServerParam::i().ballSpeedMax();
        }

        // add move noise.
        // ball speed max is not considerd, therefore value of tmp is not changed.
        M_vel_error.add( tmp * ServerParam::i().ballRand(),
                         tmp * ServerParam::i().ballRand() );
        // add kick noise
        M_vel_error += accel_err;
    }

    ////////////////////////////////////////////////////////////////////////
    // wind effect
    updateWindEffect();

    ////////////////////////////////////////////////////////////////////////

    const GameMode::Type pmode = game_mode.type();

    if ( pmode == GameMode::PlayOn
         || pmode == GameMode::GoalKick_
         || pmode == GameMode::GoalieCatch_
         || pmode == GameMode::PenaltyTaken_ )
    {
        // ball position may change.
        M_pos_count = std::min( 1000, M_pos_count + 1 );
    }
    else
    {
        // if setplay playmode, ball does not move until playmode change to playon.

        if ( pmode == GameMode::BeforeKickOff )
        {
            M_pos.assign( 0.0, 0.0 );
            M_pos_count = 0;
            M_seen_pos.assign( 0.0, 0.0 );
            dlog.addText( Logger::WORLD,
                          "Ball update. before_kick_off. set to center." );
        }
        // if I didin't see the ball in this setplay playmode,
        // we must check the ball first.
        else if ( M_pos_count > 1
                  || ( M_rpos_count > 0
                       && distFromSelf() < ServerParam::i().visibleDistance() )
                  )
        {
            // NOT seen at last cycle, but internal info means ball visible.
            // !!! IMPORTANT to check the ghost
            dlog.addText( Logger::WORLD,
                          "Ball update. SetPlay. but not seen. gconf= %d."
                          "  rconf= %d.  distFromSelf= %f",
                          posCount(), rposCount(), distFromSelf() );
            M_pos_count = 1000;
        }
        else
        {
            dlog.addText( Logger::WORLD,
                          "Ball update. SetPlay. seen once. gconf= %d."
                          "  rconf= %d.  distFromSelf= %f",
                          posCount(), rposCount(), distFromSelf() );
            M_pos_count = 1;
        }

        // in SetPlay mode, ball velocity must be Zero.
        new_vel.assign( 0.0, 0.0 );
        M_vel_error.assign( 0.0, 0.0 );
        M_vel_count = 0;
    }

    // update position with velocity
    if ( posValid() )
    {
        M_pos += new_vel;
        M_pos_error += M_vel_error;
    }

    // vel decay
    M_vel = new_vel;
    M_vel *= ServerParam::i().ballDecay();
    M_vel_error *= ServerParam::i().ballDecay();

    // update accuracy counter
    M_rpos_count = std::min( 1000, M_rpos_count + 1 );
    M_seen_pos_count = std::min( 1000, M_seen_pos_count + 1 );
    M_heard_pos_count = std::min( 1000, M_heard_pos_count + 1 );
    M_vel_count = std::min( 1000, M_vel_count + 1 );
    M_heard_vel_count = std::min( 1000, M_heard_vel_count + 1 );
    M_lost_count = std::min( 1000, M_lost_count + 1 );

    // set previous rpos
    M_rpos_prev = M_rpos;

    // M_rpos is updated using visual info or self info
}

/*-------------------------------------------------------------------*/
/*!

*/
void
BallObject::updateByFullstate( const Vector2D & pos,
                               const Vector2D & vel,
                               const Vector2D & self_pos )
{
    M_pos = pos;
    M_pos_error.assign( 0.0, 0.0 );
    M_pos_count = 0;

    M_rpos = pos - self_pos;
    M_rpos_error.assign( 0.0, 0.0 );
    M_rpos_count = 0;

    M_vel = vel;
    M_vel_error.assign( 0.0, 0.0 );
    M_vel_count = 0;

    M_lost_count = 0;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
BallObject::updateWindEffect()
{
    // ball_speed_max is not considerd in rcssserver
    // wind effect
#if 0
    if ( ! ServerParam::i().windNone() ) // use wind
    {
        if ( ! ServerParam::i().useWindRandom() ) // but static initialization
        {
            Vector2D wind_vector( 1,
                                  ServerParam::i().windForce(),
                                  ServerParam::i().windDir() );
            double speed = M_vel.r();

            Vector2D wind_effect( speed * wind_vector.x / (weight * WIND_WEIGHT),
                                  speed * wind_vector.y / (weight * WIND_WEIGHT) );
            M_vel += wind_effect;

            Vector2D wind_error( speed * wind_vector.x * ServerParam::i().windRand()
                                 / (ServerParam::i().playerWeight() * WIND_WEIGHT),
                                 speed * wind_vector.y * ServerParam::i().windRand()
                                 / (ServerParam::i().playerWeight() * WIND_WEIGHT) );
            M_vel_error.add(wind_error, wind_error);
        }
        else
        {
            // it is necessary to estimate wind force & dir

        }
    }
#endif
}

/*-------------------------------------------------------------------*/
/*!

*/
void
BallObject::updateByCollision( const Vector2D & pos,
                               const int pos_count,
                               const Vector2D & rpos,
                               const int rpos_count,
                               const Vector2D & vel,
                               const int vel_count )
{
    M_pos = pos;
    M_pos_count = pos_count;
    M_rpos = rpos;
    M_rpos_count = rpos_count;
    M_vel = vel;
    M_vel_count = vel_count;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
BallObject::updateOnlyRelativePos( const Vector2D & rpos,
                                   const Vector2D & rpos_err )
{
    M_rpos = rpos;
    M_rpos_error = rpos_err;
    M_rpos_count = 0;

    M_seen_rpos = rpos;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
BallObject::updateOnlyVel( const Vector2D & vel,
                           const Vector2D & vel_err,
                           const int vel_count )
{
    M_vel = vel;
    M_vel_error = vel_err;
    M_vel_count = vel_count;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
BallObject::updatePos( const Vector2D & pos,
                       const Vector2D & pos_err,
                       const int pos_count,
                       const Vector2D & rpos,
                       const Vector2D & rpos_err )
{
    M_pos = pos;
    M_pos_error = pos_err;
    M_pos_count = pos_count;
    M_rpos = rpos;
    M_rpos_error = rpos_err;
    M_rpos_count = 0;
    M_seen_pos = pos;
    M_seen_pos_count = 0;
    M_seen_rpos = rpos;
    M_lost_count = 0;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
BallObject::updateAll( const Vector2D & pos,
                       const Vector2D & pos_err,
                       const int pos_count,
                       const Vector2D & rpos,
                       const Vector2D & rpos_err,
                       const Vector2D & vel,
                       const Vector2D & vel_err,
                       const int vel_count )
{
    updatePos( pos, pos_err, pos_count, rpos, rpos_err );
    updateOnlyVel( vel, vel_err, vel_count );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
BallObject::updateByHear( const Vector2D & heard_pos,
                          const Vector2D & heard_vel,
                          const GameTime & current )
{
    M_heard_pos = heard_pos;
    M_heard_pos_count = 0;
    M_heard_vel = heard_vel;
    M_heard_vel_count = 0;

    if ( M_ghost_time == current )
    {
        if ( heard_pos.dist( M_pos ) < 3.0 )
        {
            dlog.addText( Logger::WORLD,
                          __FILE__": Ball. updateByHeardInfo. ghost detected. heard_pos=(%.2f, %.2f)"
                          " heard_vel=(%.2f, %.2f)",
                          heard_pos.x, heard_pos.y,
                          heard_vel.x, heard_vel.y );
            return;
        }
    }

    if ( posCount() >= 2 )
    {
        dlog.addText( Logger::WORLD,
                      __FILE__": Ball. updateByHeardInfo. heard_pos=(%.2f, %.2f)"
                      " heard_vel=(%.2f, %.2f)",
                      heard_pos.x, heard_pos.y,
                      heard_vel.x, heard_vel.y );
        M_pos = heard_pos;
        M_pos_count = 1;
        M_vel = heard_vel;
        M_vel_count = 1;
    }
    else if ( velCount() >= 2 )
    {
        dlog.addText( Logger::WORLD,
                      __FILE__": Ball. updateByHeardInfo. vel only"
                      " heard_vel=(%.2f, %.2f)",
                      heard_vel.x, heard_vel.y );
        M_vel = heard_vel;
        M_vel_count = 1;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
BallObject::updateSelfRelated( const SelfObject & self )
{
    // seen
    if ( rposCount() == 0 )
    {
        // M_rpos is already updated
        M_dist_from_self = M_rpos.r();
        M_angle_from_self = M_rpos.th();
    }
    // not seen
    else
    {
        // update rpos
        if ( M_rpos_prev.valid()
             && self.lastMove().valid() )
        {
            // M_rpos_prev is updated in update()
            M_rpos
                = M_rpos_prev
                + ( M_vel / ServerParam::i().ballDecay() )
                - self.lastMove();
            M_rpos_error += M_vel_error;
            M_rpos_error += ( self.velError() / self.playerType().playerDecay() );
        }
        // it is not necessary to consider other case.

        // update dist & angle

        // at least, rpos is valid
        if ( M_rpos.valid()
             && posCount() >= rposCount() )
        {
            M_pos = self.pos() + this->rpos();
            M_pos_error = self.posError() + this->rposError();
            M_dist_from_self = rpos().r();
            M_angle_from_self = rpos().th();
        }
        else if ( posValid() )
        {
            M_rpos = pos() - self.pos();
            M_rpos_error = posError() + self.posError();
            M_dist_from_self = rpos().r();
            M_angle_from_self = rpos().th();
        }
        else
        {
            M_dist_from_self = 1000.0;
            M_angle_from_self = 0.0;
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
Vector2D
BallObject::inertiaTravel( const int cycle ) const
{
    return inertia_n_step_travel( vel(),
                                  cycle,
                                  ServerParam::i().ballDecay() );
}

/*-------------------------------------------------------------------*/
/*!

*/
Vector2D
BallObject::inertiaPoint( const int cycle ) const
{
    return inertia_n_step_point( pos(),
                                 vel(),
                                 cycle,
                                 ServerParam::i().ballDecay() );
}

/*-------------------------------------------------------------------*/
/*!

*/
Vector2D
BallObject::inertiaFinalPoint() const
{
    return inertia_final_point( pos(),
                                vel(),
                                ServerParam::i().ballDecay() );
}

}
