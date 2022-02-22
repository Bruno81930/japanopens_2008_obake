// -*-c++-*-

/*
*Copyright:

Copyright Copyright (C) Hidehisa AKIYAMA

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

/*
In this file, the code which is between 
"begin of the added code" and "end of the added code" 
has been added by Shogo TAKAGI
*/

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

#include <rcsc/common/server_param.h>

#include <rcsc/common/logger.h>
#include <rcsc/player/player_agent.h>
#include <rcsc/player/debug_client.h>
#include <rcsc/player/intercept_table.h>

#include <rcsc/action/basic_actions.h>
#include <rcsc/action/body_go_to_point.h>
#include <rcsc/action/body_intercept.h>
#include <rcsc/action/neck_turn_to_ball_or_scan.h>
#include <rcsc/action/neck_turn_to_low_conf_teammate.h>

#include "bhv_basic_tackle.h"
#include "obake_stamina_control.h"
#include "obake_fuzzy_grade.h"
#include "obake_analysis.h"
#include "bhv_basic_move.h"

/*-------------------------------------------------------------------*/
/*!
  execute action
*/
bool
Bhv_BasicMove::execute( rcsc::PlayerAgent * agent )
{
    rcsc::dlog.addText( rcsc::Logger::TEAM,
                        "%s:%d: Bhv_BasicMove"
                        ,__FILE__, __LINE__ );

    // tackle

    if ( Bhv_BasicTackle( 0.9, 80.0 ).execute( agent ) )
    {
        return true;
    }

    const rcsc::WorldModel & wm = agent->world();

    // check ball owner
    Obake_Analysis().setRole(wm.self().unum(),
			     M_role_side_or_center_back,
			     M_role_defensive_half,
			     M_role_offensive_half,
			     M_role_side_or_center_forward);
    const int self_min = wm.interceptTable()->selfReachCycle();
    const int mate_min = wm.interceptTable()->teammateReachCycle();
    const int opp_min = wm.interceptTable()->opponentReachCycle();
    if(checkInterceptSituation(agent))
    {
        rcsc::dlog.addText( rcsc::Logger::TEAM,
                            "%s:%d: intercept"
                            ,__FILE__, __LINE__ );
//        std::cout<<"number"<<wm.self().unum()<<", intercept"<<std::endl;
        rcsc::Body_Intercept().execute( agent );
/*        if(wm.ball().pos().x > wm.offsideLineX() - 5.0
           && wm.ball().pos().dist(wm.self().pos()) <= 12.5
           && wm.ball().vel().y * (wm.self().pos().y - wm.ball().pos().y) >= 0)
        {
            std::cout<<"intercept vel"<<std::endl;
         && wm.ball().pos().dist(wm.self().pos()) <= 12.5
           && std::abs(wm.ball().pos().y - wm.self().pos().y) <= 5.0)
        {
            std::cout<<"intercept y"<<std::endl;
        }
*/
        if ( wm.ball().distFromSelf()
             < rcsc::ServerParam::i().visibleDistance() )
        {
            agent->setNeckAction( new rcsc::Neck_TurnToLowConfTeammate() );;
        }
        else
        {
            agent->setNeckAction( new rcsc::Neck_TurnToBallOrScan() );
        }
        return true;
    }

    // go to home position

    const double dash_power = getDashPower( agent, M_home_pos );

    double dist_thr = wm.ball().distFromSelf() * 0.1;
    if(wm.ball().pos().x >= rcsc::ServerParam::i().theirPenaltyAreaLineX()
       && M_home_pos.x >= rcsc::ServerParam::i().theirPenaltyAreaLineX())
    {
        dist_thr = wm.ball().distFromSelf() * 0.07;
    }
    if ( dist_thr < 1.0 ) dist_thr = 1.0;

    agent->debugClient().addMessage( " BasicMove%.0f", dash_power );
    //begin of the added code
    /********************************************/
//    if(mate_min >= opp_min && self_min != mate_min)
    if(mate_min >= opp_min && self_min > opp_min)
    {
        M_home_pos = getAdvancedDeffensivePostion(agent);
    }
    /********************************************/
    //end of the added code
    agent->debugClient().setTarget( M_home_pos );
    if ( ! rcsc::Body_GoToPoint( M_home_pos, dist_thr, dash_power
                                 ).execute( agent ) )
    {
        if(Obake_Analysis().checkExistOurPenaltyAreaIn(wm.self().pos()))
        {
            rcsc::Vector2D face_point(rcsc::ServerParam::i().pitchHalfLength() - 9.5,
                                      0.0);
            rcsc::Body_TurnToPoint(face_point).execute(agent);
        }
        else if(Obake_Analysis().checkExistOppPenaltyAreaIn(wm.self().pos())
                && mate_min < opp_min)
        {
            rcsc::Body_TurnToBall().execute( agent );
        }
        else if(wm.self().pos().x >= wm.offsideLineX() - 7.0
           || Obake_Analysis().checkExistOurPenaltyAreaIn(wm.self().pos()))
        {
            rcsc::Vector2D face_point(100.0, wm.self().pos().y);            
            rcsc::Body_TurnToPoint(face_point).execute(agent);
        }
        else if(wm.self().pos().dist(wm.ball().pos()) <= 15.0)
        {
            rcsc::Vector2D face_point(100.0, wm.self().pos().y);
            rcsc::Body_TurnToPoint(face_point).execute(agent);
        }
        else
        {
            rcsc::Body_TurnToBall().execute( agent );
        }
    }

    if(wm.existKickableOpponent()
       && wm.ball().distFromSelf() < 18.0 
       && self_min < mate_min)
    {
        agent->setNeckAction(new rcsc::Neck_TurnToBall());
    }
    else if(wm.existKickableOpponent()
            && wm.ball().distFromSelf() < 10.0)             
    {
        agent->setNeckAction(new rcsc::Neck_TurnToBall());
    }
    else if(mate_min < self_min 
            && mate_min < opp_min
            && wm.ball().pos().x >= rcsc::ServerParam::i().theirPenaltyAreaLineX()
            && wm.ball().distFromSelf() < 25.0
            && (!Obake_Analysis().checkExistOppPenaltyAreaIn(wm.self().pos())
                || wm.self().pos().x >= rcsc::ServerParam::i().theirPenaltyAreaLineX()))
    {
        agent->setNeckAction(new rcsc::Neck_TurnToBall());
    }
    else
    {
        agent->setNeckAction( new rcsc::Neck_TurnToBallOrScan() );
    }

    return true;
}

bool
Bhv_BasicMove::checkInterceptSituation(rcsc::PlayerAgent * agent)
{
    const rcsc::WorldModel & wm = agent->world();
    const int self_min = wm.interceptTable()->selfReachCycle();
    const int mate_min = wm.interceptTable()->teammateReachCycle();
    const int opp_min = wm.interceptTable()->opponentReachCycle();
    if(wm.existKickableTeammate())
    {
	return false;
    }
    if(self_min < mate_min
	&& self_min <= opp_min)
    {
	return true;
    }
    if(self_min <= 3 && !M_role_side_or_center_back
       && self_min < mate_min)
    {
	return true;
    }
    if(self_min <= 3 && M_role_side_or_center_back
       && self_min <= mate_min)
    {
	return true;
    }
    if(self_min <= mate_min
       && std::abs(wm.offsideLineX()-wm.ball().pos().x) <= 15.0
       && M_role_side_or_center_forward)
    {
	return true;
    }
    /*  if( self_min < mate_min + 1
	&& self_min < opp_min + 4 
	&& opp_min < mate_min) 
    {
	return true;
        }*/
/*
    const double r = 3.5;
    const rcsc::Circle2D circle(wm.ball().pos(), r);
*/
    /* if(!wm.existKickableOpponent() 
       && !wm.existTeammateIn(circle, 10, true)
       &&(wm.ball().pos().x > wm.offsideLineX() - 5.0
	  && wm.ball().pos().dist(wm.self().pos()) <= 12.5
	  && (wm.ball().vel().y * (wm.self().pos().y - wm.ball().pos().y) >= 0
	      || std::abs(wm.ball().pos().y - wm.self().pos().y) <= 5.0)))
    {
	return true;
    }
    */
    return false;
}
/*-------------------------------------------------------------------*/
/*!

*/
double
Bhv_BasicMove::getDashPower( rcsc::PlayerAgent * agent,
                             const rcsc::Vector2D & target_point )
{
    static bool s_recover_mode = false;
    const rcsc::WorldModel & wm = agent->world();
    /*--------------------------------------------------------*/
    // check recover
    if (checkRecoverMode(agent, target_point, s_recover_mode))
    {
        s_recover_mode = true;
    }
    else
    {
        s_recover_mode = false;
    }

    /*--------------------------------------------------------*/
    double dash_power = 100.0;
    const int mate_min = wm.interceptTable()->teammateReachCycle();
    const int opp_min = wm.interceptTable()->opponentReachCycle();
    const int self_min = wm.interceptTable()->selfReachCycle();
    const double my_inc
        = wm.self().playerType().staminaIncMax()
        * wm.self().recovery();
    const bool except_near_opp_defender = false;
    const bool except_nearest_mate = false;
    const bool except_role_center_or_side_back = false;
    const bool except_role_goalie = true;
        int ball_reach_step = 0;
    if(!wm.existKickableTeammate()
        && !wm.existKickableOpponent())
    {
        ball_reach_step
            = std::min(wm.interceptTable()->teammateReachCycle(),
                        wm.interceptTable()->opponentReachCycle() );
    }
    const rcsc::Vector2D base_pos
        = wm.ball().inertiaPoint( ball_reach_step );
    
    if ((wm.defenseLineX() > wm.self().pos().x
         && wm.ball().pos().x < wm.defenseLineX() + 20.0)
        || (!wm.self().isKickable()
            && self_min < mate_min
            && self_min <= opp_min)
        ||  (self_min < mate_min
            && self_min >= opp_min)
        || (!wm.self().isKickable()
            && wm.self().pos().dist(base_pos) <= Obake_Analysis().getDistFromNearestMate(agent,
                                                                                         base_pos,
                                                                                         except_near_opp_defender,
                                                                                         except_role_center_or_side_back,
                                                                                         except_role_goalie)))
    {
        rcsc::dlog.addText( rcsc::Logger::TEAM,
                            "%s:%d: correct DF line. keep max power"
                            ,__FILE__, __LINE__ );
        // keep max power
        dash_power = rcsc::ServerParam::i().maxPower();
    }
    else if(s_recover_mode)
    {
        dash_power = my_inc - 25.0; // preffered recover value
        if ( dash_power < 0.0 ) dash_power = 0.0;

        rcsc::dlog.addText( rcsc::Logger::TEAM,
                            "%s:%d: recovering"
                            ,__FILE__, __LINE__ );
    }
    // in offside area
    else if ( wm.self().pos().x > wm.offsideLineX() )
    {
        dash_power = rcsc::ServerParam::i().maxPower();
        rcsc::dlog.addText( rcsc::Logger::TEAM,
                            "%s:%d: in offside area. dash_power= %f"
                            ,__FILE__, __LINE__,
                            dash_power );
    }
    // normal
    else
    {
/*
        dash_power = std::min( my_inc * 1.7,
                               rcsc::ServerParam::i().maxPower() );
*/
        //begin of the added code
        /********************************************/
	dash_power = getBestDashPower(agent,
				      target_point);
        /********************************************/
        //end of the added code
        rcsc::dlog.addText( rcsc::Logger::TEAM,
                            "%s:%d: normal mode dash_power= %f"
                            ,__FILE__, __LINE__,
                            dash_power );
    }

    return dash_power;
}

//begin of the added code
/********************************************/
double
Bhv_BasicMove::getBestDashPower(rcsc::PlayerAgent * agent, 
				const rcsc::Vector2D &target_point)
{
    const rcsc::WorldModel & wm = agent->world();
    const int self_min = wm.interceptTable()->selfReachCycle();
    const int mate_min = wm.interceptTable()->teammateReachCycle();
    const int opp_min = wm.interceptTable()->opponentReachCycle();
    double dash_power = rcsc::ServerParam::i().maxPower();
    if(M_role_side_or_center_back
       || M_role_defensive_half)
    {
        if(mate_min < (opp_min - 1)
           || self_min <= opp_min)
	{
	    dash_power = Obake_StaminaControl(target_point).getOffensiveDashPower(agent);
	}
	else
	{
	    dash_power = Obake_StaminaControl(target_point).getDeffensiveDashPower(agent);
	}
    }
    else
    {
	//if(mate_min < opp_min)
        if(self_min < opp_min
           && (target_point.absY() >= rcsc::ServerParam::i().pitchHalfWidth() - 8.0
               /*|| wm.ball().pos().x > wm.offsideLineX() - 6.5*/)
           && target_point.x > 0.0
           && M_role_side_or_center_forward)
        {
            dash_power = rcsc::ServerParam::i().maxPower();
        }
        else if((M_role_side_or_center_forward || M_role_offensive_half)
                && self_min > mate_min
                && mate_min < opp_min
                && (wm.ball().pos().x >= rcsc::ServerParam::i().theirPenaltyAreaLineX()
                || wm.ball().pos().x > wm.offsideLineX() - 8.0))
        {
            dash_power = rcsc::ServerParam::i().maxPower();
        }
        else if(M_role_offensive_half
                && self_min > mate_min
                && mate_min < opp_min
                && Obake_Analysis().checkExistOppPenaltyAreaIn(target_point))
        {
            dash_power = rcsc::ServerParam::i().maxPower();
        }
        else if(mate_min < opp_min
                || self_min <= opp_min)
	{
	    dash_power = Obake_StaminaControl(target_point).getOffensiveDashPower(agent);
	}
	else
	{
	    dash_power = Obake_StaminaControl(target_point).getDeffensiveDashPower(agent);
	}
    }

    return dash_power;
}

bool
Bhv_BasicMove::checkRecoverMode(rcsc::PlayerAgent * agent,
                                const rcsc::Vector2D &target_point,
                                bool is_recover_mode)
{
    const rcsc::WorldModel & wm = agent->world();
    const int self_min = wm.interceptTable()->selfReachCycle();
    const int mate_min = wm.interceptTable()->teammateReachCycle();
    const int opp_min = wm.interceptTable()->opponentReachCycle();
    /*  if(wm.self().unum() == 4
       || wm.self().unum() == 5)
    {
        if(wm.self().unum()
        }*/
/*    if(mate_min >= opp_min && self_min == mate_min)
    {
        return true;
    }
    else*/
    int ball_reach_step = 0;
    if(!wm.existKickableTeammate()
        && !wm.existKickableOpponent())
    {
        ball_reach_step
            = std::min(wm.interceptTable()->teammateReachCycle(),
                        wm.interceptTable()->opponentReachCycle() );
    }
    const rcsc::Vector2D base_pos
        = wm.ball().inertiaPoint( ball_reach_step );
    
    if(M_role_side_or_center_back
       && wm.ball().pos().x < wm.self().pos().x 
       && wm.self().pos().x > rcsc::ServerParam::i().ourPenaltyAreaLineX()
       && ((self_min <= opp_min
           && self_min < mate_min
            && !wm.self().isKickable())
           || (self_min >= opp_min 
               && mate_min >= opp_min)))
    {
        return false;
    }

    if(mate_min < opp_min && self_min > mate_min
       && M_role_side_or_center_forward
       && (base_pos.x >= rcsc::ServerParam::i().theirPenaltyAreaLineX()
           || base_pos.x > wm.offsideLineX() - 10.0)
       && wm.self().stamina() >= rcsc::ServerParam::i().staminaMax() * 0.375)
    {
        return false;
    }
/*    else if(M_role_side_or_center_forward
      && )*/
    else if(mate_min < opp_min && self_min > mate_min
            && !(M_role_side_or_center_back && wm.self().pos().x >= wm.ball().pos().x))
    {
        if(wm.self().stamina() < rcsc::ServerParam::i().staminaMax() * 0.5)
        {
            return true;
        }
    }
    else if(mate_min < opp_min)
    {
        if(wm.self().stamina() < rcsc::ServerParam::i().staminaMax() * 0.4)
        {
            return true;
        }
    }
    else if(is_recover_mode)
    {
        if(mate_min < opp_min
           && (M_role_side_or_center_back
               || M_role_defensive_half
               || M_role_offensive_half)
           && wm.self().stamina() >= rcsc::ServerParam::i().staminaMax() * 0.55)
        {
            return false;
        }
        else if(wm.ball().pos().x <= rcsc::ServerParam::i().ourPenaltyAreaLineX()
                && wm.ball().pos().absY() <= rcsc::ServerParam::i().penaltyAreaHalfWidth())
        {
            return false;
        }
        else if(M_role_side_or_center_back 
                && wm.self().pos().x > wm.ball().pos().x)
        {
            return false;
        }
    }
    else
    {
        if(wm.self().stamina() < rcsc::ServerParam::i().staminaMax() * 0.5
           && target_point.dist(wm.self().pos()) >= 25.0)
        {
            return false;
        }
    }
    return false;
}
/********************************************/
//end of the added code

rcsc::Vector2D
Bhv_BasicMove::getAdvancedDeffensivePostion(rcsc::PlayerAgent * agent)
{
    const rcsc::WorldModel & wm = agent->world();
    rcsc::Vector2D adanced_pos = M_home_pos;
    const double base_near_pos_y = M_home_pos.y;
    const double base_middle_pos_y = (wm.self().pos().y + M_home_pos.y) / 2;
    const double base_far_pos_y = wm.self().pos().y;
    const double degree_near_destination_x = Obake_FuzzyGrade().degreeNearDestinationX(wm.self().pos().x,
                                                                                       M_home_pos.x);
    const double degree_far_destination_x = Obake_FuzzyGrade().degreeFarDestinationX(wm.self().pos().x,
                                                                                     M_home_pos.x);
    const double degree_full_stamina = Obake_FuzzyGrade().degreeFullStamina(wm.self().stamina());
    const double degree_moderate_stamina = Obake_FuzzyGrade().degreeModerateStamina(wm.self().stamina());
    const double degree_lack_stamina = Obake_FuzzyGrade().degreeLackStamina(wm.self().stamina());
    const double near_rate = degree_near_destination_x;
    double middle_rate = 0.0;
    double far_rate = 0.0;
    if(wm.self().pos().x > wm.ball().pos().x)
    {
        middle_rate = std::min(degree_far_destination_x,
                               degree_full_stamina);
                                      
        far_rate = std::min(degree_far_destination_x,
                            std::max(degree_lack_stamina,
                                     degree_moderate_stamina));
                               
    }
    else if(wm.self().pos().x <= wm.ball().pos().x)
    {
        middle_rate = degree_far_destination_x;
    }
    const double sum = near_rate + middle_rate +far_rate;
    adanced_pos.y = (base_near_pos_y * near_rate
                     + base_middle_pos_y * middle_rate
                     + base_far_pos_y * far_rate)
                    / sum;
    return adanced_pos;

}
