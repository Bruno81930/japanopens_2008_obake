// -*-c++-*-

/*!
  \file abstract_player_object.h
  \brief abstract player object class Header File
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

#ifndef RCSC_PLAYER_ABSTRACT_PLAYER_OBJECT_H
#define RCSC_PLAYER_ABSTRACT_PLAYER_OBJECT_H

#include <rcsc/player/localization.h>

#include <rcsc/geom/vector_2d.h>
#include <rcsc/geom/angle_deg.h>
#include <rcsc/types.h>

namespace rcsc {

class PlayerType;

/*!
  \class AbstractPlayerObject
  \brief abstact player object class
*/
class AbstractPlayerObject {
protected:

    SideID M_side; //!< team side
    int  M_unum; //!< uniform number
    bool M_goalie; //!< goalie flag

    int M_type; //!< player type id
    const PlayerType * M_player_type; //!< player type reference

    Vector2D M_pos; //!< global coordinate
    int M_pos_count; //!< main accuracy counter

    Vector2D M_seen_pos; //! last seen global coordinate
    int M_seen_pos_count; //! count from last see

    Vector2D M_heard_pos; //! last heard global coordinate
    int M_heard_pos_count; //! count from last hear

    Vector2D M_vel; //!< global velocity
    int M_vel_count; //!< accuracy count

    AngleDeg M_body; //!< global body angle
    int M_body_count; //!< body angle accuracy
    AngleDeg M_face; //!< global neck angle
    int M_face_count; //!< face angle accuracy

    double M_dist_from_ball; //!< distance from ball

public:

    /*!
      \brief initialize member variables.
    */
    AbstractPlayerObject();

    /*!
      \brief initialize member variables using observed info
      \param side analyzed side info
      \param p analyzed seen player info
    */
    AbstractPlayerObject( const SideID side,
                          const Localization::PlayerT & p );

    /*!
      \brief destructor. nothing to do
    */
    virtual
    ~AbstractPlayerObject()
      { }


    // ------------------------------------------
    /*!
      \brief check if this player is self or not
      \return true if this player is self
     */
    virtual
    bool isSelf() const
      {
          return false;
      }

    /*!
      \brief check if this player is ghost object or not
      \return true if this player may be ghost object
     */
    virtual
    bool isGhost() const
      {
          return false;
      }

    // ------------------------------------------

    /*!
      \brief get team side id
      \return side id (LEFT,RIGHT,NEUTRAL)
    */
    SideID side() const
      {
          return M_side;
      }

    /*!
      \brief get player's uniform number
      \return uniform number. if unknown player, returned -1
    */
    int unum() const
      {
          return M_unum;
      }

    /*!
      \brief get goalie flag
      \return true if this player is goalie
    */
    bool goalie() const
      {
          return M_goalie;
      }

    /*!
      \brief get the player type id
      \return player type id
     */
    int type() const
      {
          return M_type;
      }

    /*!
      \brief get the player type as a pointer.
      \return player type pointer variable
     */
    const
    PlayerType * playerTypePtr() const
      {
          return M_player_type;
      }

    /*!
      \brief update player type id
      \param id new id
     */
    void setPlayerType( const int type );

    /*!
      \brief get global position
      \return const reference to the point object
    */
    const
    Vector2D & pos() const
      {
          return M_pos;
      }

    /*!
      \brief get global position accuracy
      \return count from last observation
    */
    int posCount() const
      {
          return M_pos_count;
      }

    /*!
      \brief get the last seen position
      \return const reference to the point object
     */
    const
    Vector2D & seenPos() const
      {
          return M_seen_pos;
      }

    /*!
      \brief get the number of cycles since last observation
      \return count since last seen
    */
    int seenPosCount() const
      {
          return M_seen_pos_count;
      }

    /*!
      \brief get the last heard position
      \return const reference to the point object
     */
    const
    Vector2D & heardPos() const
      {
          return M_heard_pos;
      }

    /*!
      \brief get the number of cycles since last observation
      \return count since last observation
    */
    int heardPosCount() const
      {
          return M_heard_pos_count;
      }

    /*!
      \brief get velocity
      \return const reference to the vector object
    */
    const
    Vector2D & vel() const
      {
          return M_vel;
      }

    /*!
      \brief get velocity accuracy
      \return count from last observation
    */
    int velCount() const
      {
          return M_vel_count;
      }

    /*!
      \brief get global body angle
      \return const reference to the angle object
    */
    const
    AngleDeg & body() const
      {
          return M_body; // global body angle
      }

    /*!
      \brief get global body angle accuracy
      \return count from last observation
    */
    int bodyCount() const
      {
          return M_body_count;
      }

    /*!
      \brief get global neck angle
      \return const reference to the angle object
    */
    const
    AngleDeg & face() const
      {
          return M_face; // global neck angle
      }

    /*!
      \brief get global neck angle accuracy
      \return count from last observation
    */
    int faceCount() const
      {
          return M_face_count;
      }

    /*!
      \brief get distance from ball
      \return distance value from ball
    */
    const
    double & distFromBall() const
      {
          return M_dist_from_ball;
      }

    // ------------------------------------------
    /*!
      \brief template method. check if player is in the region
      \param region template resion. REGION must have method contains()
      \return true if region contains player position
    */
    template < typename REGION >
    bool isWithin( const REGION & region ) const
      {
          return region.contains( this->pos() );
      }

};

//! typedef of the AbstractPlaeyrObject container
typedef std::vector< AbstractPlayerObject * > AbstractPlayerCont;

}

#endif
