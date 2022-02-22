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
v
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
#include <rcsc/player/intercept_table.h>
#include <rcsc/player/debug_client.h>
#include <rcsc/formation/formation.h>

#include "bhv_basic_offensive_kick.h"
#include "bhv_basic_move.h"
#include "bhv_obake_receive.h"
#include "bhv_obake_defend.h"
#include "bhv_obake_mark.h"
#include "obake_analysis.h"
#include "strategy.h"

#include "role_defensive_half.h"

/*-------------------------------------------------------------------*/
/*!

*/

//begin of the added code
/********************************************/
int RoleDefensiveHalf::S_mark_number = 0;
int RoleDefensiveHalf::S_mark_number_count = 0;
bool RoleDefensiveHalf::S_exist_intercept_target = false;
int RoleDefensiveHalf::S_intercept_target_number = 0;
rcsc::Vector2D RoleDefensiveHalf::S_mark_position = rcsc::Vector2D(0.0, 0.0);
/********************************************/
//end of the added code

void
RoleDefensiveHalf::execute( rcsc::PlayerAgent * agent )
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
//        update(agent);
        doMove( agent );
    }

}

/*-------------------------------------------------------------------*/
/*!

*/
void
RoleDefensiveHalf::doKick( rcsc::PlayerAgent * agent )
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
RoleDefensiveHalf::doMove( rcsc::PlayerAgent * agent )
{
    const rcsc::WorldModel & wm = agent->world();
    //std::cout<<"DH"<<wm.self().unum()<<std::endl;
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
    const bool except_near_opp_defender = false;
    const bool except_role_center_or_side_back = true;
    const bool except_role_goalie = false;
    const double nearest_dist = Obake_Analysis().getDistFromNearestMate(agent,
                                                                        ball_pos,
                                                                        except_near_opp_defender,
                                                                        except_role_center_or_side_back,
                                                                        except_role_goalie);
    rcsc::Circle2D circle(ball_pos, 5.0);
    if(!wm.existKickableTeammate()
       && self_min <= mate_min
       && self_min <= opp_min)
    {
        Bhv_BasicMove(home_pos).execute(agent);
    }
    if(opp_min < self_min
        && (self_min < mate_min
            || (wm.self().pos().dist(ball_pos) <= nearest_dist
                && wm.self().pos().dist(ball_pos) <= nearest_dist)))
    {
        home_pos = base_pos;
        if(wm.self().pos().dist(ball_pos) > 3.0)
        {
            home_pos.x -= 3.0;
        }
        Bhv_BasicMove( home_pos ).execute( agent );
    }
    else if(opp_min < mate_min 
            && wm.self().pos().x >= ball_pos.x -5.0
            && wm.self().pos().dist(ball_pos) <= 6.0
            && ((wm.self().pos().dist(ball_pos) <= nearest_dist
                && ball_pos.x >= rcsc::ServerParam::i().ourPenaltyAreaLineX())
                || ball_pos.x < rcsc::ServerParam::i().ourPenaltyAreaLineX())
            //|| home_pos.dist(ball_pos) <= 8.5)
//            && !wm.existTeammateIn(circle, 10, false)
            && wm.ball().pos().x > rcsc::ServerParam::i().ourPenaltyAreaLineX())
    {
/*        if(wm.self().unum() == 6)
        {
            std::cout<<"DH 6 move ball"<<std::endl;
        }
*/
        home_pos = ball_pos;
        Bhv_BasicMove( home_pos ).execute( agent );
    }
    else if(opp_min < mate_min
            && self_min > mate_min)
    {
        Bhv_ObakeMark(home_pos,
                      S_mark_number,
                      S_mark_number_count,
                      S_mark_position).execute(agent);
    }
    else
    {
/*        if(wm.self().unum() == 6)
        {
            std::cout<<"DH 6 move"<<std::endl;
        }
*/
        Bhv_BasicMove( home_pos ).execute( agent );
    }
    /********************************************/
    //end of the added code
}

//begin of the added code
/********************************************/
void
RoleDefensiveHalf::update(rcsc::PlayerAgent * agent)
{
    const rcsc::WorldModel & wm = agent->world();
    const double max_dist = 8.0;//5.5;
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

                }
            }
        }
        else
        {
            const int mate_min = wm.interceptTable()->teammateReachCycle();
            const int opp_min = wm.interceptTable()->opponentReachCycle();
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
        if(wm.existKickableOpponent()
           && wm.self().pos().dist(wm.ball().pos()) <= max_dist)
        {
            S_exist_intercept_target = true;
            const rcsc::PlayerObject * target = wm.getOpponentNearestToBall(10);
            if(target)
            {
                S_intercept_target_number = (target)->unum();
            }
        }
    }
    S_mark_number_count++;
}
/********************************************/
//end of the added code
