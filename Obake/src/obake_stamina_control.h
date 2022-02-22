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

#ifndef OBAKE_STAMINA_CONTROL_H
#define OBAKE_STAMINA_CONTROL_H
#include<rcsc/geom/vector_2d.h>

class Obake_StaminaControl{
private:
    bool M_role_side_or_center_back;
    bool M_role_deffensive_half;
    bool M_role_offensive_half;
    bool M_role_side_or_center_forward;
    const rcsc::Vector2D M_target_point;
public:
    explicit
	Obake_StaminaControl(const rcsc::Vector2D &target_point)
	: M_role_side_or_center_back(false)
	, M_role_deffensive_half(false)
	, M_role_offensive_half(false)
	, M_role_side_or_center_forward(false)
	, M_target_point(target_point)
	{}
    double getOffensiveDashPower(rcsc::PlayerAgent * agent);
    double getDeffensiveDashPower(rcsc::PlayerAgent * agent);
    double getDeffensiveFastDashRate(rcsc::PlayerAgent * agent);
    double getDeffensiveModerateDashRate(rcsc::PlayerAgent * agent);
    double getDeffensiveSlowDashRate(rcsc::PlayerAgent * agent);
};

#endif
