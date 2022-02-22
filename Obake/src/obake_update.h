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

#ifndef OBAKE_UPDATE_H
#define OBAKE_UPDATE_H

#include "obake_analysis.h"

class Obake_Update{
private:
    int &M_mark_number_count, &M_pass_cycle,
        &M_previous_ball_keeper, &M_current_ball_keeper,
        &M_current_ball_keeper_count, &M_current_ball_keeper_total;
    bool M_kickable;
public:
    Obake_Update(rcsc::PlayerAgent * agent,
                 int mark_number_count,
                 int pass_cycle,
                 int previous_ball_keeper,
                 int current_ball_keeper,
                 int current_ball_keeper_count,
                 int current_ball_keeper_total,
                 const bool kickable)
        : M_mark_number_count(mark_number_count)
        , M_pass_cycle(pass_cycle)
        , M_previous_ball_keeper(previous_ball_keeper)
        , M_current_ball_keeper(current_ball_keeper)
        , M_current_ball_keeper_count(current_ball_keeper_count)
        , M_current_ball_keeper_total(current_ball_keeper_total)
        , M_kickable(kickable)
        {}
    bool execute(rcsc::PlayerAgent * agent);
};

#endif
