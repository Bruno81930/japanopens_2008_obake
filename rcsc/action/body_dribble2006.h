// -*-c++-*-

/*!
  \file body_dribble2006.h
  \brief advanced dribble action. player agent can avoid opponent.
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

#ifndef RCSC_ACTION_BODY_DRIBBLE_2006_H
#define RCSC_ACTION_BODY_DRIBBLE_2006_H

#include <rcsc/player/soccer_action.h>
#include <rcsc/geom/vector_2d.h>

#include <functional>

namespace rcsc {

class WorldModel;

/*!
  \class Body_Dribble2006
  \brief advanced dribble action. player agent can avoid opponent.
 */
class Body_Dribble2006
    : public BodyAction {
public:

    /*!
      \struct KeepDribbleInfo
      \brief dribble object
     */
    struct KeepDribbleInfo {
        Vector2D first_ball_vel_; //!< first ball velocity
        int dash_count_; //!< queued dash count
        double min_opp_dist_; //!< estimated distance to the nearest opponent while dashing

        /*!
          \brief construct with all variables
          \param first_ball_vel first ball velocity
          \param dash_count queued dash count
          \param min_opp_dist estimated distance to the nearest opponent while dashing
         */
        KeepDribbleInfo( const Vector2D & first_ball_vel,
                         const int dash_count,
                         const double & min_opp_dist )
            : first_ball_vel_( first_ball_vel )
            , dash_count_( dash_count )
            , min_opp_dist_( min_opp_dist )
          { }
    };

    /*!
      \struct KeepDribbleCmp
      \brief function object to evaluate the keep dribble.
     */
    struct KeepDribbleCmp
        : public std::binary_function< KeepDribbleInfo,
                                       KeepDribbleInfo,
                                       bool > {
        /*!
          \brief compare the dribble score
          \return true if lhs is better than rhs
         */
        result_type operator()( const first_argument_type & lhs,
                                const second_argument_type & rhs ) const
          {
              if ( lhs.dash_count_ > rhs.dash_count_ )
              {
                  return true;
              }
              if ( lhs.dash_count_ == rhs.dash_count_ )
              {
                  return ( lhs.min_opp_dist_ < rhs.min_opp_dist_ );
              }
              return false;
          }
    };

private:
    //! target point to be reached
    const Vector2D M_target_point;
    //! distance threshond to the target point
    const double M_dist_thr;
    //! power parameter for dash command
    double M_dash_power;
    //! the number of dash command after kick
    int M_dash_count;
    //! switch that determines whether player agent avoid opponents or not
    const bool M_dodge_mode;
public:
    /*!
      \brief construct with parameters
      \param target_point target point to be reached
      \param dist_thr distance threshond to the target point
      \param dash_power power parameter for dash command. if this is negative
      value, backward dribble will be performed.
      \param dash_count the number of dash command after kick
      \param dodge switch that determines whether player agent avoid opponents
      or not
    */
    Body_Dribble2006( const Vector2D & target_point,
                      const double & dist_thr,
                      const double & dash_power,
                      const int dash_count,
                      const bool dodge = true )
        : M_target_point( target_point )
        , M_dist_thr( dist_thr )
        , M_dash_power( dash_power )
        , M_dash_count( dash_count )
        , M_dodge_mode( dodge )
      { }

    /*!
      \brief execute action
      \param agent pointer to the agent itself
      \return true if action is performed
    */
    bool execute( PlayerAgent* agent );

private:

    /*!
      \brief called from execute() and doDodge(). check condition and filter
      dash power.
      \param agent pointer to the agent itself
      \param dash_power parameter for dash command
      \param dash_count the number of dash command after kick
      \param dodge switch that determines whether player agent avoid opponents
      or not
      \return true if planning is done successfully
    */
    bool doAction( PlayerAgent * agent,
                   const Vector2D & target_point,
                   const double & dash_power,
                   const int dash_count,
                   const bool dodge );

    /*!
      \brief if possible perform turn action only
      \param agent pointer to the agent itself
      \param target_point target point to be reached
      \param dash_power parameter for dash command
      \return true if turn action is performed
    */
    bool doTurnOnly( PlayerAgent * agent,
                     const Vector2D & target_point,
                     const double & dash_power );

    /*!
      \brief kick the ball to the center of agent itself
      \param agent pointer to the agent itself
      \return true if action is performed
    */
    bool doCollideWithBall( PlayerAgent * agent );

    /*!
      \brief try intentional collision if agent never face to target point
      in one step.
      \param agent pointer to the agent itself
      \param dir_diff_abs absolute difference value between target and agent's
      body
      \param kick_first true if kick is done at first
      \return true if action is performed
    */
    bool doCollideForTurn( PlayerAgent * agent,
                           const double & dir_diff_abs,
                           const bool kick_first );

    /*!
      \brief try to perform "kick [-> turn]+ [-> dash]+"
      \param agent pointer to the agent itself
      \param target_point target point to be reached
      \param dir_margin_abs absolute turn margin value
      \param dash_power power parameter for dash command
      \return true if action is performed
    */
    bool doKickTurnsDash( PlayerAgent * agent,
                          const Vector2D & target_point,
                          const double & dir_margin_abs,
                          const double & dash_power );

    /*!
      \brief try to perform "kick [-> dash]+"
      \param agent pointer to the agent itself
      \param target_point target point to be reached
      \param dir_margin_abs absolute turn margin value
      \param dash_power power parameter for dash command
      \param dash_count the number of dash command after kick
      \return true if action is performed
    */
    bool doKickDashes( PlayerAgent * agent,
                       const Vector2D & target_point,
                       const double & dash_power,
                       const int dash_count );

    /*!
      \brief try to perform "kick [-> dash]+"
      but ball should be always keep.
     */
    bool doKickDashesWithBall( PlayerAgent * agent,
                               const double & dash_power );

    bool existKickableOpponent( const WorldModel & wm,
                                const Vector2D & ball_pos,
                                double * min_opp_dist ) const;

    /*!
      \brief try to perform new dribble to avoid opponent
      \param agent pointer to the agent itself
      \param target_point original target point to be reached
      \return true if action is performed
    */
    bool doDodge( PlayerAgent * agent,
                  const Vector2D & target_point );

    /*!
      \brief kick action for oppponent avoidance
      \param agent pointer to the agent itself
      \param avoid_angle avoid target angle
      \return true if action is performed
    */
    bool doAvoidKick( PlayerAgent * agent,
                      const AngleDeg & avoid_angle );
    /*!
      \brief check wheter now agent should avoid opponent or not
      \param agent const pointer to the agent itself
      \param target_point original target point to be reached
      \return true if agent should perform dodge action
    */
    bool isDodgeSituation( const PlayerAgent * agent,
                           const Vector2D & target_point );

    /*!
      \brief check wheter agent can kick the ball after one dash.
      if necessary dash power is normalized.
      \param agent const pointer to the agent itself
      \param dash_power power parameter for dash command. if necessary,
      value is normalized.
      \return true if agent can kick the ball after dash
    */
    bool canKickAfterDash( const PlayerAgent * agent,
                           double * dash_power );

    /*!
      \brief check wheter near oppopnent exists or not, calculate ball keep
      angle & return true
      \param agent const pointer to the agent itself
      \param keep_angle ball keep angle is stored to thie as global value.
      \return true if near opponent exists
    */
    bool existCloseOpponent( const PlayerAgent * agent,
                             AngleDeg * keep_angle );

    /*!
      \brief calculate avoid target angle.
      \param agent const pointer to the agent itself
      \param target_angle original target angle
      \return calculated result
    */
    AngleDeg getAvoidAngle( const PlayerAgent * agent,
                            const AngleDeg & target_angle );

};

}

#endif
