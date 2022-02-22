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

#ifndef BHV_OBAKE_PASS_H
#define BHV_OBAKE_PASS_H

#include <rcsc/geom/vector_2d.h>
#include <rcsc/player/soccer_action.h>

class Bhv_ObakePass : public rcsc::SoccerBehavior{
private:
    int M_previous_ball_keeper;
    rcsc::Vector2D M_ball_next_pos;
    rcsc::Vector2D M_self_next_pos;
public:
    Bhv_ObakePass(rcsc::PlayerAgent * agent,
                  int previous_ball_keeper)
        : M_previous_ball_keeper(previous_ball_keeper)
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
    bool evaluatePassPoint(rcsc::PlayerAgent * agent,
                           const rcsc::Vector2D &pass_point,
                           const double &angle);
    bool checkDribbleIsBetter(rcsc::PlayerAgent * agent,
                              const rcsc::Vector2D &pass_point);
};

#endif
