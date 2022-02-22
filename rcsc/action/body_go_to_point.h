// -*-c++-*-

/*!
  \file body_go_to_point.h
  \brief run behavior which has target point.
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

#ifndef RCSC_ACTION_BODY_GO_TO_POINT_H
#define RCSC_ACTION_BODY_GO_TO_POINT_H

#include <rcsc/player/soccer_action.h>
#include <rcsc/common/player_type.h>
#include <rcsc/geom/vector_2d.h>

namespace rcsc {

/*!
  \class Body_GoToPoint
  \brief run behavior which has target point.
 */
class Body_GoToPoint
    : public BodyAction {
private:
    //! target point to be reached
    const Vector2D M_target_point;
    //! distance threshold to the target point
    const double M_dist_thr;
    //! power parameter for dash command. should be positive value.
    const double M_dash_power;
    //! recommended reach cycle
    const int M_cycle;
    //! if this is true, agent should dash backward.
    bool M_back_mode;
    //! if this is true, agent must save recover parameter.
    const bool M_save_recovery;
    //! minimal turn buffer
    const double M_dir_thr;
public:
    /*!
      \brief construct with all paramters
      \param point target point to be reached
      \param dist_thr distance threshold to the target point
      \param dash_power power parameter for dash command. should be
      positive value.
      \param cycle recommended reach cycle
      \param back_mode if this is true, agent should dash backward.
      \param save_recovery if this is true, agent must save recover
      parameter.
      \param dir_thr when intercept mode, specify this.
    */
    Body_GoToPoint( const Vector2D & point,
                    const double & dist_thr,
                    const double & dash_power,
                    const int cycle = 100,
                    const bool back_mode = false,
                    const bool save_recovery = true,
                    const double & dir_thr = 12.0 )
        : M_target_point( point )
        , M_dist_thr( dist_thr )
        , M_dash_power( std::fabs( dash_power ) )
        , M_cycle( cycle )
        , M_back_mode( back_mode )
        , M_save_recovery( save_recovery )
        , M_dir_thr( dir_thr )
      { }

    /*!
      \brief execute action
      \param agent pointer to the agent itself
      \return true if action is performed
    */
    bool execute( PlayerAgent * agent );

private:
    /*!
      \brief if necesarry, perform turn action and return true
      \param agent pointer to agent itself
      \param target_rel relative coordinate value of target point
      \param accel_angle dash direction
      \return true if turn is performed
    */
    bool doTurn( PlayerAgent * agent,
                 const Vector2D & target_rel,
                 AngleDeg * accel_angle );

    /*!
      \brief if necesarry, perform dash action and return true
      \param agent pointer to agent itself
      \param target_rel relative coordinate value of target point
      \param accel_angle dash direction
      \return true if turn is performed
    */
    bool doDash( PlayerAgent * agent,
                 Vector2D target_rel,
                 const AngleDeg & accel_angle );
};

}

#endif
