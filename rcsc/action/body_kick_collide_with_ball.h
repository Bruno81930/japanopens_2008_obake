// -*-c++-*-

/*!
  \file body_kick_collide_with_ball.h
  \brief intentional kick action to collide with ball
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

#ifndef RCSC_ACTION_BODY_KICK_COLLIDE_WITH_BALL_H
#define RCSC_ACTION_BODY_KICK_COLLIDE_WITH_BALL_H

#include <rcsc/player/soccer_action.h>

namespace rcsc {

/*!
  \class Body_KickCollideWithBall
  \brief intentional kick action to collide with ball
 */
class Body_KickCollideWithBall
    : public BodyAction {
private:

public:
    /*!
      \brief accessible from global.
     */
    Body_KickCollideWithBall()
      { }

    /*!
      \brief execute action
      \param agent pointer to the agent itself
      \return true if action is performed
     */
    bool execute( PlayerAgent * agent );
};

}

#endif
