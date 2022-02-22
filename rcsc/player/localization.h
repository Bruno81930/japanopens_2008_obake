// -*-c++-*-

/*!
  \file localization.h
  \brief localization module Header File
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

#ifndef RCSC_PLAYER_LOCALIZER_H
#define RCSC_PLAYER_LOCALIZER_H

#include <rcsc/player/visual_sensor.h>
#include <rcsc/geom/vector_2d.h>
#include <rcsc/types.h>

#include <boost/scoped_ptr.hpp>

namespace rcsc {

class LocalizeImpl;

/*!
  \class Localization
  \brief localization module
*/
class Localization {
public:

    /*!
      \brief localized player info
    */
    struct PlayerT {
        int unum_; //!< uniform number
        bool goalie_; //!< true if goalie
        Vector2D pos_; //!< global coordinate
        Vector2D rpos_; //!< relative coordinate
        Vector2D vel_; //!< global velocity
        double body_; //!< body angle
        double face_; //!< face angle
        bool has_face_; //!< true if face angle is seen
        double arm_; //!< global pointing angle
        bool pointto_; //!< true if pointing is seen
        bool tackle_; //!< true if tackling is seen

        /*!
          \brief init member variables by error value
        */
        PlayerT()
            : unum_( Unum_Unknown )
            , goalie_( false )
            , pos_( Vector2D::INVALIDATED )
            , rpos_( Vector2D::INVALIDATED )
            , vel_( Vector2D::INVALIDATED )
            , body_( 0.0 )
            , face_( 0.0 )
            , has_face_( false )
            , arm_( 0.0 )
            , pointto_( false )
            , tackle_( false )
          { }

        /*!
          \brief reset all data
        */
        void reset()
          {
              pos_.invalidate();
              rpos_.invalidate();
              unum_ = Unum_Unknown;
              has_face_ = pointto_ = tackle_ = false;
          }

        /*!
          \brief check if velocity is estimated
          \return true if this player has velocity info
        */
        bool hasVel() const
          {
              return vel_.valid();
          }

        /*!
          \brief check if angle is estimated
          \return true if this player has angle info
        */
        bool hasAngle() const
          {
              return has_face_;
          }

        /*!
          \brief check if this player is pointing
          \return true if this player is pointing somewhere
        */
        bool isPointing() const
          {
              return pointto_;
          }

        /*!
          \brief check if this player is tackling
          \return true if this player is freezed by tackle effect
        */
        bool isTackling() const
          {
              return tackle_;
          }
    };

private:

    //! implemantion
    boost::scoped_ptr< LocalizeImpl > M_impl;

public:
    /*!
      \brief create internal implementation
      \param ourside our team side
    */
    explicit
    Localization( const SideID ourside );

    /*!
      \brief implicitly delete internal impl
    */
    ~Localization();

public:
    /*!
      \brief localize self.
      \param see analyzed see info
      \param self_face pointer to the variable to store the localized face angle
      \param self_face_err pointer to the variable to store the localized face angle error
      \param self_pos pointer to the variable to store the localized self position
      \param self_pos_err pointer to the variable to store the localized self position error
    */
    void localizeSelf( const VisualSensor & see,
                       double * self_face,
                       double * self_face_err,
                       Vector2D * self_pos,
                       Vector2D * self_pos_err ) const;

    /*!
      \brief localze ball relative info
      \param see analyzed see info
      \param self_face localized self face angle
      \param self_face_err localized self face angle error
      \param rpos pointer to the variable to store the localized relative position
      \param rpos_err pointer to the variable to store the localized relative position error
      \param rvel pointer to the variable to store the localized relative velocity
      \param rvel_err pointer to the variable to store the localized relative velocity error
    */
    void localizeBallRelative( const VisualSensor & see,
                               const double & self_face,
                               const double & self_face_err,
                               Vector2D * rpos,
                               Vector2D * rpos_err,
                               Vector2D * rvel,
                               Vector2D * rvel_err ) const;

    /*!
      \brief localze other player
      \param from seen player info
      \param self_face localized self face angle
      \param self_face_err localized self face angle error
      \param self_pos localized self position
      \param self_vel localized self velocity
      \param to pointer to the variable to store the localized result
    */
    void localizePlayer( const VisualSensor::PlayerT & from,
                         const double & self_face,
                         const double & self_face_err,
                         const Vector2D & self_pos,
                         const Vector2D & self_vel,
                         PlayerT * to ) const;
};

}

#endif
