// -*-c++-*-

/*!
  \file abstract_player_object.cpp
  \brief abstract player object class Source File
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

#include "abstract_player_object.h"
#include "world_model.h"

#include <rcsc/common/server_param.h>
#include <rcsc/common/player_type.h>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
AbstractPlayerObject::AbstractPlayerObject()
    : M_side( NEUTRAL )
    , M_unum( Unum_Unknown )
    , M_goalie( false )
    , M_type( Hetero_Unknown )
    , M_player_type( static_cast< const PlayerType * >( 0 ) )
    , M_pos( Vector2D::INVALIDATED )
    , M_pos_count( 1000 )
    , M_seen_pos( Vector2D::INVALIDATED )
    , M_seen_pos_count( 1000 )
    , M_heard_pos( Vector2D::INVALIDATED )
    , M_heard_pos_count( 1000 )
    , M_vel( 0.0, 0.0 )
    , M_vel_count( 1000 )
    , M_body( 0.0 )
    , M_body_count( 1000 )
    , M_face( 0.0 )
    , M_face_count( 1000 )
    , M_dist_from_ball( 1000.0 )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
AbstractPlayerObject::AbstractPlayerObject( const SideID side,
                                            const Localization::PlayerT & p )
    : M_side( side )
    , M_unum( p.unum_ )
    , M_goalie( p.goalie_ )
    , M_type( Hetero_Unknown )
    , M_player_type( static_cast< const PlayerType * >( 0 ) )
    , M_pos( p.pos_ )
    , M_pos_count( 0 )
    , M_seen_pos( p.pos_ )
    , M_seen_pos_count( 0 )
    , M_heard_pos( Vector2D::INVALIDATED )
    , M_heard_pos_count( 1000 )
    , M_vel( 0.0, 0.0 )
    , M_vel_count( 1000 )
    , M_body( 0.0 )
    , M_body_count( 1000 )
    , M_face( 0.0 )
    , M_face_count( 1000 )
    , M_dist_from_ball( 1000.0 )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
void
AbstractPlayerObject::setPlayerType( const int id )
{
    M_type = id;
    M_player_type = PlayerTypeSet::i().get( id );
}

}
