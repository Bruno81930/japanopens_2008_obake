// -*-c++-*-

/*!
  \file body_hold_ball.h
  \brief stay there and keep the ball from opponent players.
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

#ifndef RCSC_ACTION_BODY_HOLD_BALL_H
#define RCSC_ACTION_BODY_HOLD_BALL_H

#include <rcsc/player/soccer_action.h>
#include <rcsc/geom/vector_2d.h>

namespace rcsc {

class PlayerObject;

/*!
  \class Body_HoldBall
  \brief stay there and keep the ball from opponent players.
*/
class Body_HoldBall
    : public BodyAction {
private:
    //! if true, agent will try to face to the target point
    const bool M_do_turn;
    //! face target point
    const Vector2D M_turn_target_point;

public:
    /*!
      \brief construct with all parameters
      \param do_turn if true, agent will try to face to the target point
      \param turn_target_point face target point
    */
    Body_HoldBall( const bool do_turn = false,
                   const Vector2D & turn_target_point = Vector2D( 0.0, 0.0 ) )
        : M_do_turn( do_turn )
        , M_turn_target_point( turn_target_point )
      { }

    /*!
      \brief execute action
      \param agent pointer to the agent itself
      \return true if action is performed
    */
    bool execute( PlayerAgent * agent );

private:
    /*!
      \brief kick the ball to avoid opponent
      \param agent pointer to agent itself
      \param front_keep_dist ball distance at body front
      \return true if action is performed
    */
    bool avoidOpponent( PlayerAgent * agent,
                        const double & front_keep_dist );

    /*!
      \brief kick the ball to the point where opponent never reach
      \param agent agent pointer to agent itself
      \param opponent const pointer to the considered opponent
      \return true if action is performed
    */
    bool avoidOpponentLine( PlayerAgent * agent,
                            const PlayerObject * opponent );

    /*!
      \brief if possible, turn to face target point
      \param agent agent pointer to agent itself
      \return true if action is performed
    */
    bool turnToPoint( PlayerAgent * agent );

    /*!
      \brief keep the ball at body front
      \param agent agent pointer to agent itself
      \param front_keep_dist ball distance at body front
      \return true if action is performed
    */
    bool keepFront( PlayerAgent * agent,
                    const double & front_keep_dist );

};

}

#endif
