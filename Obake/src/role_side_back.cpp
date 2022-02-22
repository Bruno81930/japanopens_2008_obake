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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <rcsc/common/server_param.h>
#include <rcsc/common/logger.h>
#include <rcsc/player/player_agent.h>
#include <rcsc/player/intercept_table.h>
#include <rcsc/player/debug_client.h>
#include <rcsc/formation/formation.h>

#include "bhv_basic_offensive_kick.h"
#include "bhv_basic_move.h"
#include "bhv_obake_defend.h"
#include "bhv_obake_mark.h"
#include "bhv_obake_receive.h"
#include "obake_analysis.h"
#include "strategy.h"

#include "role_side_back.h"

/*-------------------------------------------------------------------*/
/*!

*/

//begin of the added code
/********************************************/
int RoleSideBack::S_mark_number = 0;
int RoleSideBack::S_mark_number_count = 0;
bool RoleSideBack::S_exist_intercept_target = false;
int RoleSideBack::S_intercept_target_number = 0;
rcsc::Vector2D RoleSideBack::S_mark_position = rcsc::Vector2D(0.0, 0.0);
/********************************************/
//end of the added code

void
RoleSideBack::execute( rcsc::PlayerAgent * agent )
{
    bool kickable = agent->world().self().isKickable();
    if ( agent->world().existKickableTeammate()
         && agent->world().teammatesFromBall().front()->distFromBall()
         < agent->world().ball().distFromSelf() )
    {
        kickable = false;
    }

    if ( kickable )
    {
        doKick( agent );
    }
    else
    {
        update(agent);
        doMove(agent);
    }

}

/*-------------------------------------------------------------------*/
/*!

*/
void
RoleSideBack::doKick( rcsc::PlayerAgent * agent )
{
    switch ( Strategy::get_ball_area( agent->world().ball().pos() ) ) {
    case Strategy::BA_CrossBlock:
    case Strategy::BA_Stopper:
    case Strategy::BA_Danger:
    case Strategy::BA_DribbleBlock:
    case Strategy::BA_DefMidField:
    case Strategy::BA_DribbleAttack:
    case Strategy::BA_OffMidField:
    case Strategy::BA_Cross:
    case Strategy::BA_ShootChance:
    default:
        Bhv_BasicOffensiveKick().execute( agent );
        break;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
RoleSideBack::doMove( rcsc::PlayerAgent * agent )
{
    const rcsc::WorldModel & wm = agent->world();

    int ball_reach_step = 0;
    if ( ! wm.existKickableTeammate()
         && ! wm.existKickableOpponent() )
    {
        ball_reach_step
            = std::min( wm.interceptTable()->teammateReachCycle(),
                        wm.interceptTable()->opponentReachCycle() );
    }
    const rcsc::Vector2D base_pos
        = wm.ball().inertiaPoint( ball_reach_step );

    rcsc::Vector2D home_pos
        = formation().getPosition( agent->config().playerNumber(),
                                   base_pos );
    if ( rcsc::ServerParam::i().useOffside() )
    {
        home_pos.x = std::min( home_pos.x, wm.offsideLineX() - 1.0 );
    }

    rcsc::dlog.addText( rcsc::Logger::ROLE,
                        "%s: HOME POSITION=(%.2f, %.2f) base_point(%.1f %.1f)"
                        ,__FILE__,
                        home_pos.x, home_pos.y,
                        base_pos.x, base_pos.y );
    //begin of the added code
    /********************************************/
    const int mate_min = wm.interceptTable()->teammateReachCycle();
    const int opp_min = wm.interceptTable()->opponentReachCycle();
    const int self_min = wm.interceptTable()->selfReachCycle();
    bool exist_defender = true;
    if((wm.ball().pos().x >= rcsc::ServerParam::i().ourPenaltyAreaLineX()
	|| Obake_Analysis().checkExistOurPenaltyAreaIn(wm.ball().pos()))
       && opp_min < mate_min// && self_min != mate_min
       && wm.self().pos().x <= wm.ball().pos().x
       && wm.self().pos().y * wm.ball().pos().y > 0.0
       && wm.ball().pos().absY() >= rcsc::ServerParam::i().penaltyAreaHalfWidth()
       && std::abs(wm.self().pos().x - wm.self().pos().x) <= 15.0)
    {
        const rcsc::PlayerPtrCont::const_iterator mate_end = wm.teammatesFromBall().end();
        for(rcsc::PlayerPtrCont::const_iterator mate = wm.teammatesFromBall().begin();
            mate != mate_end;
            mate++)
        {
            if((*mate)->distFromBall() > wm.self().pos().dist(wm.ball().pos()))
            {
                exist_defender = false;
                break;
            }
            else
            {
                if((*mate)->pos().x < wm.ball().pos().x
                   && (std::abs((*mate)->pos().y - wm.ball().pos().y)
                   < std::abs(wm.self().pos().y - wm.ball().pos().y)
                       || std::abs((*mate)->pos().x - wm.ball().pos().x) 
                       > std::abs((*mate)->pos().y - wm.ball().pos().y)))
                {
                    break;
                }
            }

        }
    }
    const double decay = rcsc::ServerParam::i().ballDecay();

    rcsc::Vector2D ball_pos, v;
    ball_pos = wm.ball().pos();
    if(wm.ball().velCount() < 3)
    {
        v = wm.ball().vel();
        const double speed = wm.ball().vel().length();
        v.setLength(speed * (1-std::pow(decay, (wm.ball().velCount()+1))) / (1 - decay));
        ball_pos += v;
    }
    const bool except_opp_defender = false;
    const bool except_role_center_or_side_back = false;
    const bool except_role_goalie = false;
    if(!wm.self().isKickable()
       && ((self_min < mate_min
            && self_min <= opp_min)
           || wm.self().pos().dist(wm.ball().pos()) <= Obake_Analysis().getDistFromNearestMate(agent,
                                                                                     wm.ball().pos(),
                                                                                     except_opp_defender,
                                                                                     except_role_center_or_side_back,
                                                                                                   except_role_goalie)))
    {
        home_pos = base_pos;
        Bhv_BasicMove(home_pos).execute(agent);
    }
    else if(/*(opp_min <= mate_min && self_min == mate_min)
	      ||*/ self_min <= mate_min
        && opp_min <=self_min )
    {
/*
        const double decay = rcsc::ServerParam::i().ballDecay();
        const rcsc::Vector2D self_next_pos = wm.self().pos() + wm.self().vel();
        rcsc::Vector2D ball_pos, v;
        ball_pos = wm.ball().pos();
        if(wm.ball().velCount() < 3)
        {
            v = wm.ball().vel();
            const double speed = wm.ball().vel().length();
            v.setLength(speed * (1-std::pow(decay, (wm.ball().velCount()+1))) / (1 - decay));
            ball_pos += v;
        }
        home_pos = ball_pos;
        if(home_pos.x < self_next_pos.x)
        {
            home_pos.x -= 6.5;
        }
        else
        {
            const double difference_x = std::abs(self_next_pos.x - wm.ball().pos().x);
            const double difference_y = std::abs(self_next_pos.y - wm.ball().pos().y);
            if(difference_x > difference_y + 2.0)
            {
                home_pos.x -= difference_x * 1.5;
            }
            else
            {
                if(std::abs(ball_pos.y - self_next_pos.y) > 1.5)
                {
                    home_pos = self_next_pos;
                    home_pos.y = ball_pos.y;
                }
                else
                {
                    if(std::abs(ball_pos.x - self_next_pos.x) <= 5.5)
                    {
                        home_pos.x = self_next_pos.x + 1.5;
                    }
                    home_pos.x -= 5.5;
                }
            }
        }
        Bhv_BasicMove(home_pos).execute( agent );
*/

        if(Obake_Analysis().checkSafeDefenseSituation(agent)
           && (((std::abs(wm.ball().pos().x -  wm.self().pos().x) < 6.0
                 && std::abs(wm.self().pos().x - wm.ball().pos().x)
                   > std::abs(wm.self().pos().y - wm.ball().pos().y) + 2.0))
               || wm.self().pos().x > wm.ball().pos().x))
        {
            Bhv_ObakeDefend(agent,
                            home_pos,
                            S_exist_intercept_target).execute(agent);
        }
        else 
        {
            home_pos.y = ball_pos.y;
            Bhv_BasicMove(home_pos).execute( agent );
        }
    }
/*    else if(!exist_defender)
    {
        home_pos.y = ball_pos.y;
        Bhv_BasicMove(home_pos).execute(agent);
        }*/
    else
    {
        Bhv_BasicMove(home_pos).execute(agent);
/*
        Bhv_ObakeMark(home_pos,
                      S_mark_number,
                      S_mark_number_count,
                      S_mark_position).execute(agent);
*/
    }
    /********************************************/
    //end of the added code
}

//begin of the added code
/********************************************/
void
RoleSideBack::update(rcsc::PlayerAgent * agent)
{
    const rcsc::WorldModel & wm = agent->world();
    const double max_dist = 5.5;//8.0;//5.5;
    const int mate_min = wm.interceptTable()->teammateReachCycle();
    const int opp_min = wm.interceptTable()->opponentReachCycle();
    const int self_min = wm.interceptTable()->selfReachCycle();
    if(S_exist_intercept_target)
    {
        if(wm.existKickableOpponent())
        {
            const rcsc::PlayerObject * target =  wm.getOpponentNearestToBall(10);
            if(target)
            {
                if((target)->unum() != S_intercept_target_number)
                {
                    S_intercept_target_number = (target)->unum();

//                    std::cout<<"target1"<<(target)->unum()<<std::endl;
//                    std::cout<<"intercept_number"<<S_intercept_target_number<<std::endl;
                }
            }
        }
        else
        {
            if(mate_min < opp_min)
            {
                S_exist_intercept_target = false;
            }
            const rcsc::AbstractPlayerObject * previous_target = wm.opponent(S_intercept_target_number);
            if(previous_target)
            {
                if((previous_target)->distFromBall() >= 4.0)
                {
                    S_exist_intercept_target = false;
                }
                const double dist_from_self = (previous_target)->pos().dist(wm.self().pos());
                if(dist_from_self >= 12.5)
                {
                    S_exist_intercept_target = false;
                }
            }
        }
    }
    else
    {
        if((wm.existKickableOpponent()
           && wm.self().pos().dist(wm.ball().pos()) <= max_dist)
	    || (opp_min < self_min && self_min <= mate_min))
        {
            S_exist_intercept_target = true;
            const rcsc::PlayerObject * target = wm.getOpponentNearestToBall(10);
            if(target)
            {
                S_intercept_target_number = (target)->unum();
//                std::cout<<"target2"<<(target)->unum()<<std::endl;
//                std::cout<<"intercept_number"<<S_intercept_target_number<<std::endl;
            }
        }
    }
    
    S_mark_number_count++;
/*    if(S_exist_intercept_target)
    {
        std::cout<<"exist";
    }
    else
    {
        std::cout<<"unexist";
    }
    std::cout<<", number="<<S_intercept_target_number<<std::endl;
*/
}
/********************************************/
//end of the added code
