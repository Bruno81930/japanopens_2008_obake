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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include<rcsc/player/player_agent.h>

#include "obake_update.h"

bool
Obake_Update::execute(rcsc::PlayerAgent * agent)
{
   const rcsc::WorldModel & wm = agent->world();
   if(!M_kickable)
   {
       M_mark_number_count++;
       
   }
   int keeper = Obake_Analysis().getBallKeeperNumber(agent, 10);
   if(M_kickable)
   {
       keeper = wm.self().unum();
   }
   if(M_current_ball_keeper == keeper)
   {
       M_pass_cycle = 0;
   }
   if(M_current_ball_keeper != keeper
      && keeper >= 1 && keeper <= 11)
   {
       if(M_current_ball_keeper >= 1)
       {
           const double min_probability = 1 / 3;
           const double probability  = (double)M_current_ball_keeper_count / (M_current_ball_keeper_total - M_pass_cycle);
           if(probability >= min_probability)
           {
               M_previous_ball_keeper = M_current_ball_keeper;
           }
       }
       M_current_ball_keeper = keeper;
       M_current_ball_keeper_total = 0;
       M_current_ball_keeper_count = 0;
       M_pass_cycle = 0;
   }
   else if(M_current_ball_keeper < 0
           && keeper == 0)
   {
       M_current_ball_keeper = keeper;
       M_current_ball_keeper_total = 0;
       M_current_ball_keeper_count = 0;
       M_pass_cycle = 0;
   }
   if(keeper == M_current_ball_keeper)
   {
       M_current_ball_keeper_count++;
   }
   if(M_current_ball_keeper >= 1 
      && M_current_ball_keeper <= 11
      && keeper == 0)
   {
       M_pass_cycle++;
   }
   M_current_ball_keeper_total++;
   if(keeper < 0)
   {
       M_current_ball_keeper = keeper;
       M_current_ball_keeper_total = 0;
       M_current_ball_keeper_count = 0;
       M_pass_cycle = 0;
   }
   return true;
}
