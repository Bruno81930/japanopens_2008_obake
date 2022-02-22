/*
*Copyright:

Copyright (C) Shogo TAKAGI

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*EndCopyright:
*/

/////////////////////////////////////////////////////////////////////

#ifndef BHV_OBAKE_DEFEND_H
#define BHV_OBAKE_DEFEND_H

#include <rcsc/geom/vector_2d.h>
#include <rcsc/player/soccer_action.h>


class Bhv_ObakeDefend : public rcsc::SoccerBehavior{
private:
    rcsc::Vector2D M_home_pos;
    bool  M_exist_intercept_target;
    rcsc::Vector2D M_ball_next_pos;
    rcsc::Vector2D M_self_next_pos;

public:
    Bhv_ObakeDefend(rcsc::PlayerAgent * agent,
                    const rcsc::Vector2D &home_pos,
                    const bool exist_intercept_target)
        :M_home_pos(home_pos),
        M_exist_intercept_target(exist_intercept_target)
        {
            const rcsc::WorldModel & wm = agent->world();
            M_self_next_pos = wm.self().pos() + wm.self().vel();
            M_ball_next_pos = wm.ball().pos();
            if(wm.ball().velCount() < 3)
            {
                const double ball_decay = rcsc::ServerParam::i().ballDecay();
                rcsc::Vector2D v = wm.ball().vel();
                const double speed = wm.ball().vel().length();
                v.setLength(speed * (1-std::pow(ball_decay, (wm.ball().velCount()+1))) / (1 - ball_decay));
                M_ball_next_pos += v;
            }
        }
    bool execute(rcsc::PlayerAgent * agent);
    bool checkTurnIsNeeded(rcsc::PlayerAgent * agent);
    rcsc::Vector2D getFacePoint(rcsc::PlayerAgent * agent);
    rcsc::Vector2D getDefensePosition(rcsc::PlayerAgent * agent);
    rcsc::Vector2D getOppBodyIntersection(rcsc::PlayerAgent * agent, 
                                          const rcsc::PlayerObject * target);
    rcsc::Vector2D getTargetNextPos(rcsc::PlayerAgent * agent,
                                    const rcsc::PlayerObject * target);
};

#endif
