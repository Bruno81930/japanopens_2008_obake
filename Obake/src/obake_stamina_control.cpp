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
#include "config.h"
#endif

#include <rcsc/common/logger.h>
#include <rcsc/player/player_agent.h>
#include <rcsc/common/server_param.h>
#include <rcsc/player/intercept_table.h>

#include "obake_analysis.h"
#include "obake_fuzzy_grade.h"
#include "obake_stamina_control.h"


/*!
This function returns dash power which is used
vwhen the player dashes
*/
double
Obake_StaminaControl::getOffensiveDashPower(rcsc::PlayerAgent * agent)
{
    const rcsc::WorldModel & wm = agent->world();
    const double first_rate = std::min(Obake_FuzzyGrade().degreeFullStamina(wm.self().stamina()),
                                       std::max(Obake_FuzzyGrade().degreeFarDestinationX(wm.self().pos().x,
                                                                                         M_target_point.x), 
                                                Obake_FuzzyGrade().degreeFarDestinationY(wm.self().pos().y,
                                                                                         M_target_point.y)));
    const double second_rate = std::min(Obake_FuzzyGrade().degreeNearOppPenaltyAreaX(wm.ball().pos().x),
                                        Obake_FuzzyGrade().degreeFarDestinationX(wm.self().pos().x,
                                                                                 M_target_point.x));
    const double third_rate = std::min(Obake_FuzzyGrade().degreeNearOffsideLine(agent,
                                                                                wm.ball().pos().x),
                                       (1 - Obake_FuzzyGrade().degreeLackStamina(wm.self().stamina())));

                                        
    double rate = std::max(first_rate, second_rate);
    rate = std::max(rate, third_rate);
    const int self_min = wm.interceptTable()->selfReachCycle();
    const int mate_min = wm.interceptTable()->teammateReachCycle();
    if(self_min <= mate_min
       && !(M_role_side_or_center_back
            || M_role_deffensive_half))
    {
        const double degree_very_near_offside_line = 
            std::pow(Obake_FuzzyGrade().degreeNearOffsideLine(agent,
                                                              wm.ball().pos().x), 2);
        const double fourth_rate = std::max(Obake_FuzzyGrade().degreeNearOppPenaltyAreaX(wm.ball().pos().x),
                                            degree_very_near_offside_line);
        rate = std::max(rate, fourth_rate);
    }  
    const double dash_power = rcsc::ServerParam::i().maxPower() * rate;
    return dash_power;
}


double 
Obake_StaminaControl::getDeffensiveDashPower(rcsc::PlayerAgent * agent)
{
    rcsc::dlog.addText( rcsc::Logger::TEAM,
                        "%s:%d:"
                        ,__FILE__, __LINE__);
    const rcsc::WorldModel & wm = agent->world();
    Obake_Analysis().setRole(wm.self().unum(),
			     M_role_side_or_center_back,
			     M_role_deffensive_half,
			     M_role_offensive_half,
			     M_role_side_or_center_forward);
    const double base_fast = 1.0;
    const double base_moderate = 0.5;
    const double base_slow = 0.3;
    const double fast_rate = getDeffensiveFastDashRate(agent);
    const double moderate_rate = getDeffensiveModerateDashRate(agent);
    const double slow_rate = getDeffensiveSlowDashRate(agent);
    const double sum = fast_rate + moderate_rate + slow_rate;
    if(sum == 0.0)
    {
        return rcsc::ServerParam::i().maxPower();
    }
    const double rate = (base_fast * fast_rate
                         + base_moderate * moderate_rate
                         + base_slow * slow_rate)
                        / sum;
    const double dash_power = rcsc::ServerParam::i().maxPower() * rate;
/*
    if(wm.self().unum()==6)
    {
	std::cout<<"fast:"<<fast_rate<<",moderate:"<<moderate_rate
                 <<"slow:"<<slow_rate<<"dash:"<<dash_power<<std::endl;
    }
*/
    rcsc::dlog.addText( rcsc::Logger::TEAM,
                        "%s:%d: dash_power=%.2f"
                        ,__FILE__, __LINE__,
                        dash_power);
    return dash_power;
}

double
Obake_StaminaControl::getDeffensiveFastDashRate(rcsc::PlayerAgent * agent)
{
    const rcsc::WorldModel & wm = agent->world();
    double fast_first_rate = 0.0;
    if(M_role_side_or_center_forward
       && (wm.self().pos().x > wm.ball().pos().x))
    {
	fast_first_rate = 1.0;
    }
    const double degree_near_destination = Obake_FuzzyGrade().degreeNearDestination(wm.self().pos(),
										    M_target_point);
    const double degree_not_near_destination = 1 - degree_not_near_destination;
    const double degree_full_stamina = Obake_FuzzyGrade().degreeFullStamina(wm.self().stamina());
    const double fast_second_rate = std::min(degree_not_near_destination,
				      degree_full_stamina);
    double fast_third_rate = 0.0;
    const double degree_moderate_stamina = Obake_FuzzyGrade().degreeModerateStamina(wm.self().stamina());
    if(wm.self().pos().x > wm.ball().pos().x
       && !(Obake_Analysis().checkExistOurPenaltyAreaIn(wm.ball().pos())))
    {
	fast_third_rate  = std::max(degree_moderate_stamina,
				    degree_full_stamina);
	fast_third_rate = std::min(fast_third_rate, degree_near_destination);
    }
    double fast_fourth_rate = 0.0;
    if(wm.self().pos().x >= wm.ball().pos().x)
    {
	fast_fourth_rate = degree_near_destination;
    }
    double fast_fifth_rate = 0.0;
    if(/*M_role_deffensive_half
         && M_role_offensive_half*/
        wm.self().unum() == 8)
    {
        if(Obake_Analysis().checkExistOurPenaltyAreaIn(wm.ball().pos()))
        {
            fast_fifth_rate = degree_not_near_destination;
        }
    }
    const double fast_rate = std::max(std::max(std::max(fast_first_rate, fast_second_rate),
                                              std::max(fast_third_rate, fast_fourth_rate)),
                                      fast_fifth_rate);
    rcsc::dlog.addText( rcsc::Logger::TEAM,
                        "%s:%d: fast_rate=%.2f"
                        ,__FILE__, __LINE__,
                        fast_rate);
    return fast_rate;
}

double 
Obake_StaminaControl::getDeffensiveModerateDashRate(rcsc::PlayerAgent * agent)
{
    const rcsc::WorldModel & wm = agent->world();
    double moderate_first_rate = 0.0;
    const double mate_dist = std::abs(wm.self().pos().y - wm.ball().pos().y);
    const double opp_dist = std::abs(wm.self().pos().x - wm.ball().pos().x) - rcsc::ServerParam::i().defaultKickableArea() * 2;
    const double degree_near_destination = Obake_FuzzyGrade().degreeNearDestination(wm.self().pos(),
										    M_target_point);
    const double degree_lack_stamina = Obake_FuzzyGrade().degreeLackStamina(wm.self().stamina());
    if(wm.self().pos().x <= wm.ball().pos().x
       && !(Obake_Analysis().checkExistOurPenaltyAreaIn(wm.ball().pos()))
       && mate_dist < opp_dist)
    {
	moderate_first_rate = Obake_FuzzyGrade().degreeLackStamina(wm.self().stamina());
    }
    const int self_min = wm.interceptTable()->selfReachCycle();
    const int mate_min = wm.interceptTable()->teammateReachCycle();
    double moderate_second_rate = 0.0;
    if(wm.self().pos().x >  wm.ball().pos().x
       && !(Obake_Analysis().checkExistOurPenaltyAreaIn(wm.ball().pos()))
       && self_min != mate_min)
    {
	moderate_second_rate = std::min(degree_near_destination,
					degree_lack_stamina);
    }
    const double moderate_rate = std::max(moderate_first_rate,
                                          moderate_second_rate);
    rcsc::dlog.addText( rcsc::Logger::TEAM,
                        "%s:%d: moderate_rate=%.2f"
                        ,__FILE__, __LINE__,
                        moderate_rate);
    return moderate_rate;
}

double
Obake_StaminaControl::getDeffensiveSlowDashRate(rcsc::PlayerAgent * agent)
{
    const rcsc::WorldModel & wm = agent->world();
    const double degree_far_destination = Obake_FuzzyGrade().degreeFarDestination(wm.self().pos(),
										  M_target_point);
    const double degree_lack_stamina = Obake_FuzzyGrade().degreeLackStamina(wm.self().stamina());
    double slow_first_rate = 0.0;
    if(/*self_min != mate_min
         &&*/ !((M_role_side_or_center_back 
                 || M_role_deffensive_half
                 || wm.self().unum() == 7)
                && wm.self().pos().x > wm.ball().pos().x))
    {
        slow_first_rate = std::min(degree_far_destination,
                                   degree_lack_stamina);  
    }
    const double slow_rate = slow_first_rate;
    rcsc::dlog.addText( rcsc::Logger::TEAM,
                        "%s:%d: slow_rate=%.2f"
                        ,__FILE__, __LINE__,
                        slow_rate);
    return slow_rate;
}
