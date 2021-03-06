// -*-c++-*-

/*!
  \file self_object.cpp
  \brief self object class Source File
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

#include "self_object.h"

#include "action_effector.h"
#include "ball_object.h"
#include "debug_client.h"
#include "body_sensor.h"
#include "fullstate_sensor.h"

#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include <rcsc/soccer_math.h>

#include <algorithm>
#include <iostream>
#include <cmath>

namespace rcsc {

int SelfObject::S_pos_count_thr = 20;
int SelfObject::S_vel_count_thr = 10;
int SelfObject::S_face_count_thr = 5;

/*-------------------------------------------------------------------*/
/*!

*/
SelfObject::SelfObject()
    : AbstractPlayerObject()
    , M_time( -1, 0 )
    , M_sense_body_time( -1, 0 )
    , M_pos_error( 0.0, 0.0 )
    , M_pos_prev( Vector2D::INVALIDATED )
    , M_vel_error( 0.0, 0.0 )
    , M_neck( 0.0 )
    , M_face_error( 0.0 )
    , M_view_width( ViewWidth::NORMAL )
    , M_view_quality( ViewQuality::HIGH )
    , M_last_catch_time( 0, 0 )
    , M_tackle_expires( 0 )
    , M_arm_movable( 0 )
    , M_pointto_pos( Vector2D::INVALIDATED )
    , M_last_pointto_time( 0, 0 )
    , M_attentionto_side( NEUTRAL )
    , M_attentionto_unum( 0 )
    , M_last_move( 0.0, 0.0 )
    , M_collision_estimated( false )
    , M_collides_with_none( false )
    , M_collides_with_ball( false )
    , M_collides_with_player( false )
    , M_collides_with_post( false )
    , M_kickable( false )
    , M_kick_rate( 0.0 )
    , M_tackle_probability( 0.0 )
{
    M_type = Hetero_Default;
    M_player_type = PlayerTypeSet::i().get( Hetero_Default );

    for ( int i = 0; i < 4; ++i )
    {
        M_last_moves[i].assign( 0.0, 0.0 );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
SelfObject::set_count_thr( const int pos_thr,
                           const int vel_thr,
                           const int face_thr )
{
    S_pos_count_thr = pos_thr;
    S_vel_count_thr = vel_thr;
    S_face_count_thr = face_thr;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
SelfObject::init( const SideID side,
                  const int unum,
                  const bool goalie )
{
    M_side = side;
    M_unum = unum;
    M_goalie = goalie;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
SelfObject::posValid() const
{
    return M_pos_count < S_pos_count_thr;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
SelfObject::velValid() const
{
    return M_vel_count < S_vel_count_thr;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
SelfObject::faceValid() const
{
    return M_face_count < S_face_count_thr;
}

/*-------------------------------------------------------------------*/
/*!

*/
double
SelfObject::kickableArea() const
{
    return ( this->playerType().kickableMargin()
             + this->playerType().playerSize()
             + ServerParam::i().ballSize() );
}

/*-------------------------------------------------------------------*/
/*!

*/
double
SelfObject::catchableArea() const
{
    //double t = atan2(0.5, 2.0); // atan2(w * 0.5, l);
    //double len = 2.0 / cos(t);

    return std::sqrt( std::pow( ServerParam::i().catchAreaWidth() * 0.5, 2.0 )
                      + std::pow( ServerParam::i().catchAreaLength(), 2.0 ) );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
SelfObject::update( const ActionEffector & act,
                    const GameTime & current )
{
    if ( M_time == current )
    {
        return;
    }

    M_time = current;

    M_pos_prev = M_pos;

    Vector2D accel( 0.0, 0.0 );
    double dash_power = 0.0;
    double turn_moment = 0.0, turn_err = 0.0;
    double neck_moment = 0.0;

    /////////////////////////////////////////
    // base command
    switch ( act.lastBodyCommandType() ) {
    case PlayerCommand::DASH:
        act.getDashInfo( &accel, &dash_power );
        break;
    case PlayerCommand::TURN:
        act.getTurnInfo( &turn_moment, &turn_err );
        break;
    case PlayerCommand::TACKLE:
        M_tackle_expires = ServerParam::i().tackleCycles();
        break;
    case PlayerCommand::MOVE:
        M_pos = act.getMovePos();
        //M_vel.assign( 0.0, 0.0 );
        //M_vel_error.assign( 0.0, 0.0 );
        break;
    case PlayerCommand::CATCH:
        M_last_catch_time = act.lastActionTime();
        break;
    case PlayerCommand::KICK:
        break;
    default:
        //std::cerr << current << " self update : no command!!"
        //          << std::endl;
        break;
    }

    /////////////////////////////////////////
    // support command
    // turn neck
    if ( act.doneTurnNeck() )
    {
        neck_moment = act.getTurnNeckMoment();
    }

    // effect of other support commans are immediately applied.
    // so those effect is set when commends is sent.
    // SEE: ActionEffector.makeCommand

    // neck update
    M_neck += neck_moment;
    if ( M_neck.degree() > ServerParam::i().maxNeckAngle() )
    {
        M_neck = ServerParam::i().maxNeckAngle();
    }
    if ( M_neck.degree() < ServerParam::i().minNeckAngle() )
    {
        M_neck = ServerParam::i().minNeckAngle();
    }

    // stamina
    M_stamina.update( playerType(), dash_power );


    /////////////////////////////////////////////////////////////////
    // turn
    M_body += turn_moment;

    /////////////////////////////////////////////////////////////////
    // face
    M_face = M_body + M_neck;
    M_face_error += turn_err;

    /////////////////////////////////////////////////////////////////
    // vel
    if ( velValid() )
    {
        M_vel += accel;
    }

    /////////////////////////////////////////////////////////////////
    // pos
    if ( posValid() )
    {
        M_pos += M_vel;
    }

    // rcssserver/src/object.C
    // PVector MPObject::noise()
    //  {
    //    Value maxrnd = randp * vel.r() ;
    //    return PVector(drand(-maxrnd,maxrnd),drand(-maxrnd,maxrnd)) ;
    //  }
    //
    // vel += acc;
    // vel += noise();
    // vel += wind();
    // new_pos = pos + vel
    // goal post check (new_pos)
    // pos = new_pos;
    // vel *= decay;
    // acc *= Zero;

    /////////////////////////////////////////////////////////////////
    // update error
    if ( velValid() )
    {
        const double velrnd = ServerParam::i().playerRand() * M_vel.r();

        M_pos_error.add( velrnd, velrnd ); // add new vel rand

        M_vel_error.add( velrnd, velrnd ); // add new vel rand

        dlog.addText( Logger::WORLD,
                      __FILE__": update. pos=(%.2f, %.2f) pos_err(%.3f, %.3f) "
                      "vel=(%.2f, %.2f) vel_err=(%.3f, %.3f)",
                      M_pos.x, M_pos.y, M_pos_error.x, M_pos_error.y,
                      M_vel.x, M_vel.y, M_vel_error.x, M_vel_error.y );
    }

    /////////////////////////////////////////////////////////////////
    updateWindEffect();

    /////////////////////////////////////////////////////////////////
    // vel decay, also error
    M_vel *= this->playerType().playerDecay();
    M_vel_error *= this->playerType().playerDecay();

    /////////////////////////////////////////////////////////////////
    // update accuracy count
    ++M_pos_count;
    ++M_seen_pos_count;
    ++M_vel_count;
    ++M_body_count;
    ++M_face_count;

    // update action effect count
    M_tackle_expires = std::max( 0, M_tackle_expires - 1 );
    M_arm_movable = std::max( 0, M_arm_movable - 1 );

    /////////////////////////////////////////////////////////////////
    // last move vector
    M_last_move = M_vel / this->playerType().playerDecay();
    for ( int i = 2; i > 0; --i )
    {
        M_last_moves[i] = M_last_moves[i-1];
    }
    M_last_moves[0] = M_last_move;

    /////////////////////////////////////////////////////////////////
    // reset collision state
    M_collision_estimated = false;
    M_collides_with_none = false;
    M_collides_with_ball = false;
    M_collides_with_player = false;
    M_collides_with_post = false;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
SelfObject::updateWindEffect()
{
#if 0
    // rcssserver/src/field.C
    void Weather::init(Stadium *)
        {
            if (ServerParam::instance().win_no) {
                wind_vector = PVector(0.0, 0.0) ;
                wind_rand = 0.0 ;
            }
            else if (ServerParam::instance().win_random) {
                wind_vector = Polar2PVector(drand(0, 100), Deg2Rad(drand(-180, 180))) ;
                wind_rand = drand(0, 0.5) ;
            }
            else {
                wind_vector = Polar2PVector(ServerParam::instance().winforce,
                                            Deg2Rad(ServerParam::instance().windir)) ;
                wind_rand = ServerParam::instance().winrand ;
            }
        }

    // rcssserver/src/object.C
    PVector MPObject::wind()
        {
            return PVector( vel.r()
                            * ( weather->wind_vector.x +
                                drand(-weather->wind_rand, weather->wind_rand) )
                            / (weight * WIND_WEIGHT),
                            vel.r()
                            * ( weather->wind_vector.y +
                                drand(-weather->wind_rand, weather->wind_rand) )
                            / (weight * WIND_WEIGHT) );
        }
    // rcssserver/srd/param.h
    //#define WIND_WEIGHT		10000.0
#endif

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
                                 / (ServerParam::i().playerWeight()
                                    * ServerParam::i().windWeight()),
                                 speed * wind_vector.y * ServerParam::i().windRand()
                                 / (ServerParam::i().playerWeight()
                                    * ServerParam::i().windWeight()) );
            M_vel_error.add( wind_error, wind_error );
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
SelfObject::updateAfterSense( const BodySensor & sense,
                              const ActionEffector & act,
                              const GameTime & current )
{
    if ( M_sense_body_time == current )
    {
        std::cerr << current
                  << "self : updateAfterSense : called twice ??"
                  << std::endl;
        return;
    }

    M_sense_body_time = current;

    // ------------------------------------------------
    // internal update
    update( act, current );

    // ------------------------------------------------
    // use sense_body

    ////////////////////////////////////////////////////
    // view mode
    M_view_width = sense.viewWidth();
    M_view_quality = sense.viewQuality();

    ////////////////////////////////////////////////////
    // stamina
    M_stamina.updateAfterSense( playerType(),
                                sense.stamina(), sense.effort(),
                                current );

    ////////////////////////////////////////////////////
    // neck relative
    if ( std::fabs( M_neck.degree() - sense.neckDir() ) > 0.5 )
    {
        M_neck = sense.neckDir();
    }

    //
    // collision
    //
    if ( sense.noneCollided()
         || sense.ballCollided()
         || sense.playerCollided()
         || sense.postCollided() )
    {
        M_collision_estimated = false; // !!! important !!!

        if ( sense.noneCollided() )
        {
            M_collides_with_none = true;
            dlog.addText( Logger::WORLD,
                          __FILE__": self.updateAfterSense. no collision" );
        }

        if ( sense.ballCollided() )
        {
            M_collides_with_ball = true;
            dlog.addText( Logger::WORLD,
                          __FILE__": self.updateAfterSense. collided with ball" );
        }

        if ( sense.playerCollided() )
        {
            M_collides_with_player = true;
            dlog.addText( Logger::WORLD,
                          __FILE__": self.updateAfterSense. collided with player" );
        }

        if ( sense.postCollided() )
        {
            M_collides_with_post = true;
            dlog.addText( Logger::WORLD,
                          __FILE__": self.updateAfterSense. collided with post" );
        }
    }

    ////////////////////////////////////////////////////
    // face & vel
    if ( faceValid() )
    {
        ////////////////////////////////////////////////////
        // face
        M_face = M_body + M_neck;

        ////////////////////////////////////////////////////
        // vel
        const Vector2D estimate_vel = M_vel;

        // sensed speed dir is rounded by  "Rad2IDeg(a) ((int)(Rad2Deg(a)))"
        double sensed_speed_dir = sense.speedDir();
        double sensed_speed_dir_error = 0.5;
        if ( sensed_speed_dir == 0.0 )
        {
            sensed_speed_dir_error = 1.0;
        }
        else if ( sensed_speed_dir > 0.0 )
        {
            sensed_speed_dir = AngleDeg::normalize_angle( sensed_speed_dir + 0.5 );
        }
        else if ( sensed_speed_dir < 0.0 )
        {
            sensed_speed_dir = AngleDeg::normalize_angle( sensed_speed_dir - 0.5 );
        }

        const AngleDeg vel_ang = face() + sensed_speed_dir;

        M_vel.setPolar( sense.speedMag(), vel_ang );

        // vel error
        double mincos, maxcos, minsin, maxsin;
        // vel angle error is face error + sensed speed dir error

        vel_ang.cosMinMax( faceError() + sensed_speed_dir_error,
                           &mincos,
                           &maxcos );
        vel_ang.sinMinMax( faceError() + sensed_speed_dir_error,
                           &minsin,
                           &maxsin );
        // sensed speed is rounded by 0.01
        M_vel_error.assign( ( maxcos - mincos ) * ( sense.speedMag() + 0.005 ),
                            ( maxsin - minsin ) * ( sense.speedMag() + 0.005 ) );
        dlog.addText( Logger::WORLD,
                      __FILE__": updateAfterSense. vel=(%.2f %.2f) vel_err=(%.3f, %.3f)  faceErr = %.3f",
                      M_vel.x, M_vel.y,
                      M_vel_error.x, M_vel_error.y,
                      faceError() );

        ////////////////////////////////////////////////////
        // collision check
        if ( ! hasSensedCollision()
             && velValid()
             && sense.speedMag() < ( playerType().realSpeedMax()
                                     * playerType().playerDecay()
                                     * 0.11 )
             )
        {
            dlog.addText( Logger::WORLD,
                          __FILE__": updateAfterSense. check collisiion. estimate_vel=(%.2f %.2f)",
                          estimate_vel.x, estimate_vel.y );
            // decay is very small
            //   default decay = 0.4
            //   collision decay = 0.1
            // Magic Numuber. less than player_decay
            if ( estimate_vel.r() > 0.01
                 && sense.speedMag() < estimate_vel.r() * 0.2 // too big decay
                 && ( estimate_vel.absX() < 0.08
                      || estimate_vel.x * M_vel.x < 0.0 )  // vel dir is reversed
                 && ( estimate_vel.absY() < 0.08
                      || estimate_vel.y * M_vel.y < 0.0 )  // vel dir is reversed
                 )
            {
                M_collision_estimated = true;
                dlog.addText( Logger::WORLD,
                              __FILE__": self.updateAfterSense. *COLLISION* estimated" );
            }
        }

        ////////////////////////////////////////////////////
        // vel accuracy is face angle conf
        M_vel_count = faceCount();

        //////////////////////////////////////////////////////
        // pointto
        if ( sense.armExpires() != 0
             && posValid() )
        {
            M_pointto_pos
                = M_pos
                + Vector2D::polar2vector( sense.pointtoDist(),
                                          face() + sense.pointtoDir() );
        }
        else
        {
            M_pointto_pos.invalidate();
        }

        ////////////////////////////////////////////////////
        // last move
        if ( ! M_collision_estimated )
        {
            M_last_move = M_vel / this->playerType().playerDecay();
        }
        else if ( velValid() )
        {
            // use estimated vel !!!
            M_last_move = estimate_vel / this->playerType().playerDecay();
        }
        else
        {
            dlog.addText( Logger::WORLD,
                          __FILE__": updateAfterSense. XXX reset last move" );
            M_last_move.invalidate();
        }
        M_last_moves[0] = M_last_move;

        if ( collisionEstimated()
             || collidesWithBall() )
        {
            dlog.addText( Logger::WORLD,
                          __FILE__": updateAfterSense. collision. reset last move" );
            M_last_moves[0].invalidate();
        }
    }

    // attentionto
    M_attentionto_side = sense.attentiontoSide();
    M_attentionto_unum = sense.attentiontoUnum();

    // tackle
    if ( M_tackle_expires > 0 )
    {
        dlog.addText( Logger::WORLD,
                      __FILE__": updateAfterSense internal_tackle_exp=%d, sense_body=%d",
                      M_tackle_expires, sense.tackleExpires() );
    }

    if ( M_tackle_expires != sense.tackleExpires() )
    {
        std::cerr << current
                  << "self : updateAfterSense. internal tackle expire has invalie value."
                  << std::endl;
        dlog.addText( Logger::WORLD,
                      __FILE__": updateAfterSense internal_tackle_exp is not match to sense_body" );
    }

    M_tackle_expires = sense.tackleExpires();
    M_arm_movable = sense.armMovable();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
SelfObject::updateAfterFullstate( const FullstateSensor::PlayerT & my_state,
                                  const ActionEffector & act,
                                  const GameTime & current )
{
    update( act, current );

    M_pos = my_state.pos_;
    M_pos_error.assign( 0.0, 0.0 );
    M_pos_count = 0;

    M_seen_pos = my_state.pos_;
    M_seen_pos_count = 0;

    M_vel = my_state.vel_;
    M_vel_error.assign( 0.0, 0.0 );
    M_vel_count = 0;

    M_body = my_state.body_;
    M_body_count = 0;
    M_neck = my_state.neck_;
    M_face = M_body + M_neck;
    M_face_error = 0.0;
    M_face_count = 0;

    M_stamina.updateAfterFullstate( my_state.stamina_,
                                    my_state.effort_,
                                    my_state.recovery_ );

    M_pointto_pos
        = pos()
        + Vector2D::polar2vector( my_state.pointto_dist_,
                                  face() + my_state.pointto_dir_ );

}


/*-------------------------------------------------------------------*/
/*!

*/
void
SelfObject::updatePos( const Vector2D & pos,
                       const Vector2D & pos_err,
                       const double & face,
                       const double & face_err,
                       const GameTime & current)
{
    // other param is updated in update or update_after_sense
    M_time = current;

    // I saw my position in last cycle
    if ( M_pos_count == 1 )
    {
        Vector2D new_pos = pos;
        Vector2D new_error = pos_err;
        // and last error is smaller than now
        if ( M_pos_error.x < pos_err.x )
        {
            new_pos.x
                = pos.x
                + ( M_pos.x - pos.x ) * ( pos_err.x / ( M_pos_error.x + pos_err.x ) );
            new_error.x = ( M_pos_error.x + pos_err.x ) * 0.5;
            dlog.addText( Logger::WORLD,
                          __FILE__": updatePos. X modified [%f] < [%f]",
                          M_pos_error.x, pos_err.x );
        }
        if  ( M_pos_error.y < pos_err.y )
        {
            new_pos.y
                = pos.y
                + ( M_pos.y - pos.y ) * ( pos_err.y / ( M_pos_error.y + pos_err.y ) );
            new_error.y = ( M_pos_error.y + pos_err.y ) * 0.5;
            dlog.addText( Logger::WORLD,
                          __FILE__": updatePos. Y modified [%f] < [%f]",
                          M_pos_error.y, pos_err.y );
        }
        M_pos = new_pos;
        M_pos_error = new_error;

        dlog.addText( Logger::WORLD,
                      __FILE__": updatePos. seen_pos=(%.3f, %.3f) average_pos=(%.3f, %.3f) "
                      " error=(%.4f, %.4f)",
                      pos.x, pos.y, new_pos.x, new_pos.y,
                      M_pos_error.x, M_pos_error.y );
    }
    else
    {
        M_pos = pos;
        M_pos_error = pos_err;
    }

    M_pos_count = 0;
    M_seen_pos = M_pos;
    M_seen_pos_count = 0;

    M_face = face;
    M_body = face - M_neck.degree();
    M_body_count = 0;
    M_face_error = face_err;
    M_face_count = 0;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
SelfObject::updateByCollision( const Vector2D & pos,
                               const Vector2D & pos_error )
{
    M_pos = pos;
    M_pos_error = pos_error;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
SelfObject::updateAngle( const double & face,
                         const double & face_err,
                         const GameTime & current )
{
    M_time = current;
    M_face = face;
    M_body = face - M_neck.degree();
    M_body_count = 0;
    M_face_error = face_err;
    M_face_count = 0;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
SelfObject::updateVelDirAfterSee( const BodySensor & sense,
                                  const GameTime & current )
{
    // I don't have current sense_body.
    if ( sense.time() != current )
    {
        return;
    }

    // I got the visual sensory at current cycle.
    // I could get my current face global angle.
    if ( faceCount() == 0 )
    {
        // sensed speed dir is rounded by  "Rad2IDeg(a) ((int)(Rad2Deg(a)))"
        double sensed_speed_dir = sense.speedDir();
        double sensed_speed_dir_error = 0.5;
        if ( sensed_speed_dir == 0.0 )
        {
            sensed_speed_dir_error = 1.0;
        }
        else if ( sensed_speed_dir > 0.0 )
        {
            sensed_speed_dir
                = AngleDeg::normalize_angle( sensed_speed_dir + 0.5 );
        }
        else if ( sensed_speed_dir < 0.0 )
        {
            sensed_speed_dir
                = AngleDeg::normalize_angle( sensed_speed_dir - 0.5 );
        }

        const AngleDeg vel_ang = face() + sensed_speed_dir;

        M_vel.setPolar( sense.speedMag(), vel_ang );
        M_vel_count = 0;

        // vel error
        double mincos, maxcos, minsin, maxsin;
        // vel angle error is face error + sensed speed dir error
        vel_ang.cosMinMax( faceError() + sensed_speed_dir_error,
                           &mincos,
                           &maxcos );
        vel_ang.sinMinMax( faceError() + sensed_speed_dir_error,
                           &minsin,
                           &maxsin );
        // sensed speed is rounded by 0.01
        M_vel_error.assign( ( maxcos - mincos ) * ( sense.speedMag() + 0.005 ),
                            ( maxsin - minsin ) * ( sense.speedMag() + 0.005 ) );

        // set last move
        M_last_move = M_vel / this->playerType().playerDecay();
        M_last_moves[0] = M_last_move;

        dlog.addText( Logger::WORLD,
                      __FILE__": updateVelDirAfterSee. face_error=%.2f, sensed_dir_error=%.2f."
                      " vel=(%f %f) vel_err=(%f %f) last_move=(%.4f %.4f)",
                      faceError(), sensed_speed_dir_error,
                      M_vel.x, M_vel.y,
                      M_vel_error.x, M_vel_error.y,
                      M_last_move.x, M_last_move.y );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
SelfObject::updateBallInfo( const BallObject & ball )
{
    M_kickable = false;
    M_kick_rate = 0.0;
    M_tackle_probability = 0.0;

    if ( ! ball.posValid() && ball.rposCount() != 0 )
    {
        return;
    }

    M_dist_from_ball = ball.distFromSelf();

    //------------------------------------------------------
    // check kickable

    if ( ball.distFromSelf() <= this->kickableArea() - 0.06 )
    {
        M_kickable = true;

        M_kick_rate
            = kick_rate( ball.distFromSelf(),
                         ( ball.angleFromSelf() - this->body() ).degree(),
                         ServerParam::i().kickPowerRate(),
                         ServerParam::i().ballSize(),
                         this->playerType().playerSize(),
                         this->playerType().kickableMargin() );
    }

    //------------------------------------------------------
    // check tackle
    Vector2D player2ball = ball.pos() - this->pos();
    player2ball.rotate( - this->body() );

    double tackle_dist;
    if ( player2ball.x > 0.0 )
    {
        tackle_dist = ServerParam::i().tackleDist();
    }
    else
    {
        tackle_dist = ServerParam::i().tackleBackDist();
    }

    double fail_prob
        = ( std::pow( std::fabs( player2ball.x ) / tackle_dist,
                      ServerParam::i().tackleExponent() )
            + std::pow( std::fabs( player2ball.y )
                        / ServerParam::i().tackleWidth(),
                        ServerParam::i().tackleExponent() ) );

    if ( fail_prob < 1.0 )
    {
        // success_prob = 1.0 - fail_prob
        // r = random(); <- [0,1]
        // if (r <= success_prob) SUCCESS;
        // else FAILURE;
        M_tackle_probability = 1.0 - fail_prob;
    }

    dlog.addText( Logger::WORLD,
                  __FILE__": updateBallInfo. ball_dist=%.3f.  my_kickable=%.3f.  tackle_prob=%.3f",
                  ball.distFromSelf(), kickableArea(), M_tackle_probability );
}

/*-------------------------------------------------------------------*/
/*!

*/
double
SelfObject::getSafetyDashPower( const double & dash_power ) const
{
    // required stamina value
    double required_stamina = ( dash_power > 0.0
                                ? dash_power
                                : dash_power * -2.0 );

    if ( required_stamina < 0.0 )
    {
        std::cerr << "required stamina should be positive now "
                  << " input dash power = " << dash_power
                  << std::endl;
        dlog.addText( Logger::ACTION,
                      __FILE__": getSafetyDashPower. required stamina should be positive now" );
    }

    // check rest stamina

    const double safety_stamina
        = stamina() - ServerParam::i().recoverDecThrValue() - 3.0;

    double available_stamina = std::max( 0.0, safety_stamina );

    double result_power = std::min( required_stamina, available_stamina );
    // backward dash case
    if ( dash_power < 0.0 )
    {
        result_power *= -0.5;
    }

    if ( std::fabs( result_power ) > std::fabs( dash_power ) )
    {
        return dash_power;
    }

    dlog.addText( Logger::ACTION,
                  __FILE__": getSafetyDashPower. cur_stamina=%.1f, required=%.1f, result=%.2f",
                  stamina(), required_stamina, result_power );


    return result_power;
}


/*-------------------------------------------------------------------*/
/*!

*/
bool
SelfObject::canTurn( const double & moment ) const
{
    return std::fabs( moment )
        <= playerType().effectiveTurn( ServerParam::i().maxMoment(),
                                       vel().r() );
}

}
