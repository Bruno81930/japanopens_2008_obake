// -*-c++-*-

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA

 This code is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2, or (at your option)
 any later version.

 This code is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this code; see the file COPYING.  If not, write to
 the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

 *EndCopyright:
 */

/////////////////////////////////////////////////////////////////////

#ifndef AGENT2D_BHV_BASIC_MOVE_H
#define AGENT2D_BHV_BASIC_MOVE_H

#include <rcsc/geom/vector_2d.h>
#include <rcsc/player/soccer_action.h>

class Bhv_BasicMove
    : public rcsc::SoccerBehavior {
private:
    rcsc::Vector2D M_home_pos;
    bool M_role_side_or_center_back;
    bool M_role_defensive_half;
    bool M_role_offensive_half;
    bool M_role_side_or_center_forward;
public:
    explicit
    Bhv_BasicMove(const rcsc::Vector2D &home_pos)
        : M_home_pos(home_pos)
        , M_role_side_or_center_back(false)
	, M_role_defensive_half(false)
	, M_role_offensive_half(false)
	, M_role_side_or_center_forward(false)
	{}
    bool execute( rcsc::PlayerAgent * agent );

private:
    bool checkInterceptSituation(rcsc::PlayerAgent * agent);
    bool checkRecoverMode(rcsc::PlayerAgent * agent,
                          const rcsc::Vector2D &target_point,
                          bool is_recover_mode);    
    rcsc::Vector2D getAdvancedDeffensivePostion(rcsc::PlayerAgent * agent);
    double getDashPower(rcsc::PlayerAgent * agent,
                        const rcsc::Vector2D & target_point );
    double getBestDashPower(rcsc::PlayerAgent * agent,
			    const rcsc::Vector2D &target_point);
};

#endif
