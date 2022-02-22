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

#ifndef BHV_OBAKE_MARK_H
#define BHV_OBAKE_MARK_H
#include <rcsc/player/soccer_action.h>
#include <rcsc/geom/vector_2d.h>

class Bhv_ObakeMark : public rcsc::SoccerBehavior{
private:
    rcsc::Vector2D M_home_pos;
    int & M_mark_number;
    int & M_mark_number_count;
    rcsc::Vector2D & M_mark_position;
    bool M_role_side_or_center_back;
    bool M_role_defensive_half;
    bool M_role_offensive_half;
    bool M_role_side_or_center_forward;
public:
    Bhv_ObakeMark(const rcsc::Vector2D &home_pos,
                  int &mark_number,
                  int &mark_number_count,
                  rcsc::Vector2D &mark_position)
        : M_home_pos(home_pos)
        , M_mark_number(mark_number)
        , M_mark_number_count(mark_number_count)
        , M_mark_position(mark_position)
        , M_role_side_or_center_back(false)
        , M_role_defensive_half(false)
        , M_role_offensive_half(false)
        , M_role_side_or_center_forward(false)
        {}
    bool execute(rcsc::PlayerAgent * agent);
    int getNearestMarkNumber(rcsc::PlayerAgent * agent,
                             const double &max_mark_dist);
    int getMarkNumberInPenaltyArea(rcsc::PlayerAgent * agent);
    int getMarkNumber(rcsc::PlayerAgent * agent);
    int getMarkNumberForBack(rcsc::PlayerAgent * agent);
    int getMarkNumberForDefensiveHalf(rcsc::PlayerAgent * agent);
    int getMarkNumberForOffensiveHalf(rcsc::PlayerAgent * agent);
    int getMarkNumberForForward(rcsc::PlayerAgent * agent);
    int getMarkNumberForForwardInPenaltyArea(rcsc::PlayerAgent * agent);
    rcsc::Vector2D getMarkPosition(rcsc::PlayerAgent * agent,
                                   const bool offensive = false);
    rcsc::Vector2D getMarkPosition(rcsc::PlayerAgent * agent, 
                                   const int mark_number,
                                   const bool offensive = false);
};
 
#endif
