// -*-c++-*-

/*!
  \file player_object.cpp
  \brief player object class Source File
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

#include "player_object.h"
#include "fullstate_sensor.h"

#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include <rcsc/common/player_type.h>

namespace rcsc {

int PlayerObject::S_pos_count_thr = 30;
int PlayerObject::S_vel_count_thr = 5;
int PlayerObject::S_face_count_thr = 2;

/*-------------------------------------------------------------------*/
/*!

*/
PlayerObject::PlayerObject()
    : AbstractPlayerObject()
    , M_dist_from_self( 1000.0 )
    , M_angle_from_self( 0.0 )
    , M_ghost_count( 0 )
    , M_rpos( Vector2D::INVALIDATED )
    , M_rpos_count( 1000 )
    , M_pointto_angle( 0.0 )
    , M_pointto_count( 1000 )
    , M_tackle_count( 1000 )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
PlayerObject::PlayerObject( const SideID side,
                            const Localization::PlayerT & p )
    : AbstractPlayerObject( side, p )
    , M_dist_from_self( p.rpos_.r() )
    , M_angle_from_self( 0.0 )
    , M_ghost_count( 0 )
    , M_rpos( p.rpos_ )
    , M_pointto_angle( 0.0 )
    , M_pointto_count( 1000 )
    , M_tackle_count( 1000 )
{
    if ( p.hasVel() )
    {
        M_vel = p.vel_;
        M_vel_count = 0;
    }

    if ( p.hasAngle() )
    {
        M_body = p.body_;
        M_body_count = 0;
        M_face = p.face_;
        M_face_count = 0;
    }

    if ( p.isPointing() )
    {
        M_pointto_angle = p.arm_;
        M_pointto_count = 0;
    }

    if ( p.isTackling() )
    {
        M_tackle_count = 0;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerObject::set_count_thr( const int pos_thr,
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
bool
PlayerObject::isTackling() const
{
    return M_tackle_count <= ServerParam::i().tackleCycles();
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
PlayerObject::isKickable( const double & buf ) const
{
    if ( ! M_player_type )
    {
        return distFromBall() < ServerParam::i().defaultKickableArea();
    }

    return distFromBall() < M_player_type->kickableArea() - buf;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerObject::update()
{
    if ( velValid() )
    {
        M_pos += M_vel;
        // speed is not decayed in internal update.
    }
    /*
    // wind effect
    //vel += wind_effect( ServerParam::i().windRand(),
    //                          ServerParam::i().windForce(),
    //                          ServerParam::i().windDir());
    */
    M_pos_count = std::min( 1000, M_pos_count + 1 );
    M_rpos_count = std::min( 1000, M_rpos_count + 1 );
    M_seen_pos_count = std::min( 1000, M_seen_pos_count + 1 );
    M_heard_pos_count = std::min( 1000, M_heard_pos_count + 1 );
    M_vel_count = std::min( 1000, M_vel_count + 1 );
    M_body_count = std::min( 1000, M_body_count + 1 );
    M_face_count = std::min( 1000, M_face_count + 1 );
    M_pointto_count = std::min( 1000, M_pointto_count + 1 );
    M_tackle_count = std::min( 1000, M_tackle_count + 1 );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerObject::updateBySee( const SideID side,
                           const Localization::PlayerT & p )
{
    M_side = side;
    M_ghost_count = 0;

    // unum is updated only when unum is seen.
    if ( p.unum_ != Unum_Unknown )
    {
        M_unum = p.unum_;
        if ( ! p.goalie_ )
        {
            // when unum is seen, goalie info is also seen
            M_goalie = false;
        }
    }

    if ( p.goalie_ )
    {
        M_goalie = true;
    }

    if ( p.hasVel() )
    {
        M_vel = p.vel_;
        M_vel_count = 0;
        dlog.addText( Logger::WORLD,
                      "%s: updateBySee. unum=%d. pos=(%.2f, %.2f) vel=(%.2f, %.2f)"
                      ,__FILE__,
                      p.unum_, M_pos.x, M_pos.y, M_vel.x, M_vel.y );
    }
    else if ( 0 < M_pos_count
              && M_pos_count <= 2
              && p.rpos_.r() < 40.0 )
    {
        const double speed_max = ( M_player_type
                                   ? M_player_type->playerSpeedMax()
                                   : ServerParam::i().defaultPlayerSpeedMax() );
        const double decay = ( M_player_type
                               ? M_player_type->playerDecay()
                               : ServerParam::i().defaultPlayerSpeedMax() );

        M_vel = ( p.pos_ - M_pos ) / static_cast< double >( M_pos_count );
        double tmp = M_vel.r();
        if ( tmp > speed_max )
        {
            M_vel *= speed_max / tmp;
        }
        M_vel *= decay;
        M_vel_count = M_pos_count;
        dlog.addText( Logger::WORLD,
                      "%s: updateBySee. unum=%d. update vel by pos diff."
                      "prev_pos=(%.2f, %.2f) old_pos=(%.2f, %.2f) -> vel=(%.3f, %.3f)"
                      ,__FILE__,
                      p.unum_,
                      M_pos.x, M_pos.y, p.pos_.x, p.pos_.y, M_vel.x, M_vel.y );
    }
    else
    {
        M_vel.assign( 0.0, 0.0 );
        M_vel_count = 1000;
    }

    M_pos = p.pos_;
    M_rpos = p.rpos_;
    M_seen_pos = p.pos_;

    M_pos_count = 0;
    M_rpos_count = 0;
    M_seen_pos_count = 0;

    if ( p.hasAngle() )
    {
        M_body = p.body_;
        M_face = p.face_;
        M_body_count
            = M_face_count = 0;
    }
    else if ( velValid() )
    {
        M_body = vel().th();
        M_body_count = velCount();
        M_face = 0.0;
        M_face_count = 1000;
    }

    if ( p.isPointing()
         && M_pointto_count >= ServerParam::i().pointToBan() )
    {
        M_pointto_angle = p.arm_;
        M_pointto_count = 0;
    }

    if ( p.isTackling() )
    {
        if ( M_tackle_count > ServerParam::i().tackleCycles() ) // see tackling recently
        {
            M_tackle_count = 0;
        }
    }
    else
    {
        M_tackle_count = 1000;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerObject::updateByFullstate( const FullstateSensor::PlayerT & p,
                                 const Vector2D & self_pos,
                                 const Vector2D & ball_pos )
{
    M_ghost_count = 0;

    M_unum = p.unum_;
    M_goalie = p.goalie_;

    M_pos = p.pos_;
    M_rpos = p.pos_ - self_pos;

    M_pos_count = 0;
    M_rpos_count = 0;

    M_vel = p.vel_;
    M_vel_count = 0;

    M_body = p.body_;
    M_body_count = 0;
    M_face = p.body_ + p.neck_;
    M_face_count = 0;

    M_pointto_angle = M_face + p.pointto_dir_;
    M_pointto_count = 0;

    M_dist_from_self = M_rpos.r();
    M_angle_from_self = M_rpos.r();
    M_dist_from_ball = ( M_pos - ball_pos ).r();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerObject::updateByHear( const int heard_unum,
                            const bool goalie,
                            const Vector2D & heard_pos )
{
    M_heard_pos = heard_pos;
    M_heard_pos_count = 0;

    M_ghost_count = 0;

    if ( heard_unum != Unum_Unknown )
    {
        M_unum = heard_unum;
    }

    if ( goalie )
    {
        M_goalie = true;
    }

    if ( posCount() > 1
         || distFromSelf() > 20.0 )
    {
        M_pos = heard_pos;

        if ( posCount() > 1 )
        {
            M_pos_count = 1;
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerObject::updateByHear( const int heard_unum,
                            const bool goalie,
                            const Vector2D & heard_pos,
                            const AngleDeg & heard_body )
{
    updateByHear( heard_unum, goalie, heard_pos );

    if ( bodyCount() > 1 )
    {
        M_body = heard_body;
        M_body_count = 1;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerObject::updateSelfBallRelated( const Vector2D & self,
                                     const Vector2D & ball )
{
    M_dist_from_self = ( M_pos - self ).r();
    M_angle_from_self = ( M_pos - self ).th();
    M_dist_from_ball = ( M_pos - ball ).r();

    if ( M_rpos_count > 0 )
    {
        M_rpos = M_pos - self;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerObject::forget()
{
    M_pos_count
        = M_seen_pos_count
        = M_heard_pos_count
        = M_vel_count
        = M_face_count
        = M_pointto_count
        = M_tackle_count
        = 1000;
}

}
