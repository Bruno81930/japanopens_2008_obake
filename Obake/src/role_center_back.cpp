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

#include "strategy.h"

#include "obake_analysis.h"



#include "role_center_back.h"

/*-------------------------------------------------------------------*/
/*!

*/
//begin of the added code
/********************************************/
int RoleCenterBack::S_mark_number = 0;
int RoleCenterBack::S_mark_number_count = 0;
rcsc::Vector2D RoleCenterBack::S_mark_position = rcsc::Vector2D(0.0, 0.0);
/********************************************/
//end of the added code

void
RoleCenterBack::execute( rcsc::PlayerAgent * agent )
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
        update();
        doMove( agent );
    }

}

/*-------------------------------------------------------------------*/
/*!

*/
void
RoleCenterBack::doKick( rcsc::PlayerAgent * agent )
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
RoleCenterBack::doMove( rcsc::PlayerAgent * agent )
{
    const rcsc::WorldModel & wm = agent->world();

    int ball_reach_step = 0;
    if ( ! wm.self().isKickable()
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
    //begin of the added code
    /********************************************/
    const int mate_min = wm.interceptTable()->teammateReachCycle();
    const int opp_min = wm.interceptTable()->opponentReachCycle();
    const int self_min = wm.interceptTable()->selfReachCycle();
    rcsc::dlog.addText( rcsc::Logger::ROLE,
                        "%s: HOME POSITION=(%.2f, %.2f) base_point(%.1f %.1f)"
                        ,__FILE__,
                        home_pos.x, home_pos.y,
                        base_pos.x, base_pos.y );
    bool exist_defender = true;
    double dist_y = 7.5;
    if(wm.ball().pos().x <= rcsc::ServerParam::i().ourPenaltyAreaLineX())
    {
        dist_y -= 2.5;
    }
    
    if(/*(wm.ball().pos().x >= rcsc::ServerParam::i().ourPenaltyAreaLineX()
	 */ Obake_Analysis().checkExistOurPenaltyAreaIn(wm.ball().pos())//)
       && opp_min < mate_min //&& self_min != mate_min
       && wm.self().pos().x <= wm.ball().pos().x
       && wm.ball().pos().absY() >= rcsc::ServerParam::i().penaltyAreaHalfWidth()
       && std::abs(wm.ball().pos().y - wm.self().pos().y) <= 15.0//dist_y
       && std::abs(wm.ball().pos().x - wm.self().pos().x) <= 10.0)
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
    bool exist_another_center_back = true;
    int number = 2;
    if(wm.self().unum() == 2)
    {
        number = 3;
    }
    const rcsc::PlayerObject * nearest_ball_mate = wm.getTeammateNearestToBall(10);
    if(nearest_ball_mate)
    {
        if((nearest_ball_mate)->unum() == number
            && (nearest_ball_mate)->pos().x > wm.ball().pos().x)
        {
            exist_another_center_back = false;
        }
    }
    const double top_range = (rcsc::ServerParam::i().goalAreaHalfWidth() 
                              + rcsc::ServerParam::i().penaltyAreaHalfWidth()) 
                              / 2;
    const rcsc::Vector2D self_next_pos = wm.self().pos() + wm.self().vel();
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
    if(!wm.existKickableTeammate()
       && ((self_min < mate_min
           && self_min <= opp_min + 2)
       || wm.self().pos().dist(wm.ball().pos()) <= Obake_Analysis().getDistFromNearestMate(agent,
                                                                                     wm.ball().pos(),
                                                                                     except_opp_defender,
                                                                                     except_role_center_or_side_back,
                                                                                     except_role_goalie)))
    {
        home_pos = base_pos;
        Bhv_BasicMove(home_pos).execute(agent);
    }
    else if(opp_min < mate_min && self_min <= mate_min + 1
            || (!exist_another_center_back 
                && wm.self().pos().x < wm.ball().pos().x
                && (wm.ball().pos().absY() <= top_range 
                    ||(Obake_Analysis().checkExistOurPenaltyAreaIn(wm.ball().pos())
                       && wm.ball().pos().x <= (-rcsc::ServerParam::i().pitchHalfLength()
                                                -rcsc::ServerParam::i().ourPenaltyAreaLineX())/2))))
    {     
        if(ball_pos.x <= rcsc::ServerParam::i().ourPenaltyAreaLineX())
        {
            /*
            home_pos = ball_pos;
            if(home_pos.x < self_next_pos.x)
            {
                home_pos.x -= 5.0;
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
                    if(std::abs(ball_pos.y - self_next_pos.y) > 1.2)
                    {
                        home_pos = self_next_pos;
                        home_pos.y = ball_pos.y;
                    }
                    else
                    {
                        home_pos.x -= 3.5;
                        if(std::abs(wm.ball().pos().x - self_next_pos.x) <= 3.5)
                        {
                            home_pos.x = self_next_pos.x + 0.5;
                        }
                    }
                }
            }
            if(home_pos.x <= -rcsc::ServerParam::i().pitchHalfLength() + 1.0)
            {
                home_pos.x = -rcsc::ServerParam::i().pitchHalfLength() + 2.0;
            }
            Bhv_BasicMove(home_pos).execute( agent );
            */
            const bool exist_intercept_target = true;
            Bhv_ObakeDefend(agent,
                            home_pos,
                            exist_intercept_target).execute(agent);
        }
        else 
        {
            const bool exist_intercept_target = true;
            Bhv_ObakeDefend(agent,
                            home_pos,
                            exist_intercept_target).execute(agent);
        }
    }
    else if(!exist_defender
            && wm.self().pos().x <= wm.ball().pos().x
            && Obake_Analysis().checkExistOurPenaltyAreaIn(base_pos))
    {
        home_pos.y = ball_pos.y;
        if(std::abs(self_next_pos.x-ball_pos.x) < std::abs(self_next_pos.y-ball_pos.y))
        {
            rcsc::Vector2D defense_pos = ball_pos;
            const rcsc::PlayerObject * target = wm.getOpponentNearestToBall(10);

            bool exist_best_defensepoint = false;                                                           
            double min_angel = 60.0;
            const double r = 10.0;
            rcsc::Circle2D circle(ball_pos, r);
            if(target
               && wm.existTeammateIn(circle, 10, false))
            {
                if(((target)->body().degree() < -min_angel
                    && (target)->body().degree() >= -180.0)
                   ||((target)->body().degree() > min_angel
                      && (target)->body().degree() <= 180.0))

                {
                    
                    const rcsc::Vector2D intersectoin = Bhv_ObakeDefend(agent,
                                                                        home_pos,
                                                                        true).getOppBodyIntersection(agent,
                                                                                                 target);
                    const rcsc::Vector2D target_next_pos = Bhv_ObakeDefend(agent,
                                                                           home_pos,
                                                                           true).getTargetNextPos(agent,
                                                                                              target);
                    const rcsc::Vector2D target_to_intersection = intersectoin - target_next_pos;
                    const rcsc::Vector2D self_to_intersection = intersectoin - self_next_pos;
                    if(self_to_intersection.length() > target_to_intersection.length() - 2.0)
                    {
                        const double difference = std::abs(self_to_intersection.length()
                                                           - target_to_intersection.length());
                        rcsc::Vector2D block_vector = target_to_intersection;
                        block_vector.setLength(difference);
                        defense_pos = intersectoin + block_vector;
                    }
                    exist_best_defensepoint = true;
                }
            }
            if(!exist_best_defensepoint)
            {
                defense_pos.y -= std::abs(self_next_pos.y-ball_pos.y) - 5.0;
            }

            if(!(ball_pos.x <=rcsc::ServerParam::i().ourPenaltyAreaLineX()
                 && ball_pos.absY() <= rcsc::ServerParam::i().goalAreaHalfWidth()))
            {
                home_pos = defense_pos;
            }
        }
        Bhv_BasicMove(home_pos).execute(agent);
    }
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
}

//begin of the added code
/********************************************/
void
RoleCenterBack::update()
{
    S_mark_number_count++;
}
/********************************************/
//end of the added code
