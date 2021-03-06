// -*-c++-*-

/*!
  \file global_object.h
  \brief The declaration of object types held by coach/trainer.
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

#ifndef RCSC_COACH_GLOBAL_OBJECT_H
#define RCSC_COACH_GLOBAL_OBJECT_H

#include <rcsc/geom/vector_2d.h>
#include <rcsc/types.h>

#include <list>
#include <vector>
#include <iostream>

namespace rcsc {

/*!
  \class GlobalBallObject
  \brief ball information for coath/trainer
*/
class GlobalBallObject {
private:

    Vector2D M_pos; //!< global coordinate value
    Vector2D M_vel; //!< velocity vector

public:

    /*!
      \brief initialize all data with 0.
     */
    GlobalBallObject()
        : M_pos( 0.0, 0.0 )
        , M_vel( 0.0, 0.0 )
      { }

    /*!
      \brief get global position
      \return const reference value
    */
    const
    Vector2D & pos() const
      {
          return M_pos;
      }

    /*!
      \brief get global velocity
      \return const reference value
    */
    const
    Vector2D & vel() const
      {
          return M_vel;
      }

    /*!
      \brief set position
      \param x coordinage x
      \param y coordinage y
     */
    void setPos( const double & x,
                 const double & y )
      {
          M_pos.assign( x, y );
      }

    /*!
      \brief set velocity
      \param vx velocity x
      \param vy velocity y
     */
    void setVel( const double & vx,
                 const double & vy )
      {
          M_vel.assign( vx, vy );
      }
};


/*-------------------------------------------------------------------*/

/*!
  \class GlobalPlayerObject
  \brief player information for coath/trainer
*/
class GlobalPlayerObject {
private:

    SideID M_side; //!< LEFT or RIGHT
    int M_unum; //!< uniform number
    bool M_goalie; //!< goalie or not
    int M_type; //!< plaeyr type id

    Vector2D M_pos; //!< global position
    Vector2D M_vel; //!< velocity

    AngleDeg M_body; //!< body angle
    AngleDeg M_face; //!< global neck angle

    int M_pointto_cycle; //< if player is pointing, this value in incremented
    AngleDeg M_pointto_angle; //!< player's global arm angle

    int M_tackle_cycle; //!< if player is tackling, this value is incremented

public:

    /*!
      \brief inititialize all value with 0 or invalid values
    */
    GlobalPlayerObject()
        : M_side( NEUTRAL )
        , M_unum( -1 )
        , M_goalie( false )
        , M_type( Hetero_Unknown )
        , M_pos( Vector2D::INVALIDATED )
        , M_vel( 0.0, 0.0 )
        , M_body( 0.0 )
        , M_face( 0.0 )
        , M_pointto_cycle( 0 )
        , M_pointto_angle( 0.0 )
        , M_tackle_cycle( 0 )
      { }

    /*!
      \brief get side info
      \return side ID
     */
    SideID side() const
      {
          return M_side;
      }

    /*!
      \brief get player's uniform number
      \return uniform number
     */
    int unum() const
      {
          return M_unum;
      }

    /*!
      \brief check if this player is goalie
      \return if goalie, true
     */
    bool goalie() const
      {
          return M_goalie;
      }

    /*!
      \brief get player type id
      \return player type id
     */
    int type() const
      {
          return M_type;
      }

    /*!
      \brief get player's position
      \return const reference to the position variable
     */
    const
    Vector2D & pos() const
      {
          return M_pos;
      }

    /*!
      \brief get player's velocity
      \return const reference to the velocity variable
     */
    const
    Vector2D & vel() const
      {
          return M_vel;
      }

    /*!
      \brief get player's body angle
      \return const reference to the body angle variable
     */
    const
    AngleDeg & body() const
      {
          return M_body;
      }

    /*!
      \brief get player's global face angle
      \return const reference to the face angle variable
     */
    const
    AngleDeg & face() const
      {
          return M_face;
      }

    /*!
      \brief get player's pointto status
      \return if the player is now pointing,
      return positive value as the continuous period.
      ir no pointing action, return 0.
     */
    int pointtoCycle() const
      {
          return M_pointto_cycle;
      }

    /*!
      \brief get player's global arm angle
      \return global pointing angle
     */
    const
    AngleDeg & pointtoAngle() const
      {
          return M_pointto_angle;
      }

    /*!
      \brief check if player is pointing or not
      \return true if player is pointing
     */
    bool isPointing() const
      {
          return ( M_pointto_cycle > 0 );
      }

    /*!
      \brief get player's tackle status
      \return if the player is now tackling,
      return positive value as the continuous period.
      ir no tackle action, return 0.
     */
    int tackleCycle() const
      {
          return M_tackle_cycle;
      }

    /*!
      \brief check if player is tackling or not
      \return true if player is tackling
     */
    bool isTackling() const
      {
          return ( M_tackle_cycle > 0 );
      }

    //////////////////////////////////////////////////////////////

    /*!
      \brief set team data with seen information
      \param side side ID
      \param unum uniform number
      \param goalie goalie flag
     */
    void setTeam( const SideID side,
                  const int unum,
                  const bool goalie )
      {
          M_side = side;
          M_unum = unum;
          M_goalie = goalie;
          if ( goalie )
          {
              M_type = Hetero_Default;
          }
      }

    /*!
      \brief set plaeyr type id
      \param type player type id
     */
    void setPlayerType( const int type )
      {
          M_type = type;
      }

    /*!
      \brief set position with seen information
      \param x seen x
      \param y seen y
     */
    void setPos( const double & x,
                 const double & y )
      {
          M_pos.assign( x, y );
      }

    /*!
      \brief set velocity with seen information
      \param vx seen velocity x
      \param vy seen velocity y
     */
    void setVel( const double & vx,
                 const double & vy )
      {
          M_vel.assign( vx, vy );
      }

    /*!
      \brief set body and neck angle with seen information
      \param b seen body angle
      \param n seen neck angle relative to body
     */
    void setAngle( const double & b,
                   const double & n )
      {
          M_body = b;
          M_face = b + n;
      }

    /*!
      \brief set arm status with seen information
      \param angle pointing global angle
     */
    void setArm( const double & angle )
      {
          M_pointto_cycle = 1;
          M_pointto_angle = angle;
      }

    /*!
      \brief set tackle status
     */
    void setTackle()
      {
          M_tackle_cycle = 1;
      }

    /*!
      \brief update with seen data
      \param p seen data
     */
    void update( const GlobalPlayerObject & p );

    /*!
      \brief put data to the output stream
      \param os reference cto the output stream
      \return os reference cto the output stream
     */
    std::ostream & print( std::ostream & os ) const;

};

} // namespace rcsc

#endif
