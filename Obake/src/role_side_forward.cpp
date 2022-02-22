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
#include <rcsc/player/intercept_table.h>
#include <rcsc/player/debug_client.h>
#include <rcsc/formation/formation.h>

#include "bhv_basic_offensive_kick.h"
#include "bhv_basic_move.h"

#include "strategy.h"

#include "bhv_obake_receive.h"
#include "bhv_obake_mark.h"
#include "obake_analysis.h"
#include "obake_update.h"
#include "bhv_obake_receive_test.h"
#include "role_side_forward.h"

/*-------------------------------------------------------------------*/
/*!

*/
//begin of the added code
/********************************************/
int RoleSideForward::S_mark_number = 0;
int RoleSideForward::S_mark_number_count = 0;
int RoleSideForward::S_pass_cycle = 0;
int RoleSideForward::S_previous_ball_keeper = 0;
int RoleSideForward::S_current_ball_keeper = 0;
int RoleSideForward::S_current_ball_keeper_count = 0;
int RoleSideForward::S_current_ball_keeper_total;
rcsc::Vector2D RoleSideForward::S_mark_position = rcsc::Vector2D(0.0, 0.0);
/********************************************/
//end of the added code

void
RoleSideForward::execute( rcsc::PlayerAgent * agent )
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
//begin of the added code
/********************************************/
        Obake_Update(agent,
                     S_mark_number_count,
                     S_pass_cycle,
                     S_previous_ball_keeper,
                     S_current_ball_keeper,
                     S_current_ball_keeper_count,
                     S_current_ball_keeper_total,
                     true).execute(agent);
/********************************************/
//end of the added code
    }
    else
    {
//begin of the added code
/********************************************/
        if(agent->world().self().unum() == 9)
        {
            //Bhv_ObakeReceiveTest().getBestMiddleTest(agent);
/*            if(!Bhv_ObakeReceiveTest().putNewVectorIntoSearchOrderTest(agent))
            {
                std::cout<<"error"<<std::endl;
            }
*/

        }
        Obake_Update(agent,
                     S_mark_number_count,
                     S_pass_cycle,
                     S_previous_ball_keeper,
                     S_current_ball_keeper,
                     S_current_ball_keeper_count,
                     S_current_ball_keeper_total,
                     false).execute(agent);
/********************************************/
//end of the added code
        doMove(agent);
    }

}

/*-------------------------------------------------------------------*/
/*!

*/
void
RoleSideForward::doKick( rcsc::PlayerAgent * agent )
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
        Bhv_BasicOffensiveKick(S_previous_ball_keeper).execute( agent );
        break;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
RoleSideForward::doMove( rcsc::PlayerAgent * agent )
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
//    double difference;
    const rcsc::Vector2D ball_next_pos = wm.ball().pos() + wm.ball().vel();
    const rcsc::Vector2D self_next_pos = wm.self().pos() + wm.self().vel();
    if(opp_min < self_min && self_min <= mate_min + 1)
    {
        home_pos = ball_next_pos;
        if(home_pos.x < wm.self().pos().x)
        {
            home_pos.x -= 5.0;
        }
        else if(std::abs(ball_next_pos.y - self_next_pos.y) > 3.5
                || std::abs(ball_next_pos.y - self_next_pos.y) >
                std::abs(ball_next_pos.x - self_next_pos.x))
        {
            home_pos = self_next_pos;
            home_pos.y = ball_next_pos.y;
        }
        else
        {
            home_pos.x -= 2.5;
        }
    }
    if(!wm.existKickableTeammate()
       && self_min <= opp_min)
    {
        Bhv_BasicMove(home_pos).execute(agent);
    }
    else if(opp_min < mate_min
            && (wm.ball().pos().x > 0.0
		|| wm.ball().pos().dist(home_pos) <= 15.0
                || wm.ball().pos().dist(wm.self().pos()) <= 12.5))
//            && !(self_min == mate_min && S_mark_number_count < 15))
    {
        const bool except_near_opp_defender = false;
        const bool except_role_side_or_center_back = true;
        if( self_next_pos.x >= ball_next_pos.x
            && self_next_pos.dist(ball_next_pos) <= 15.0
            && rcsc::ServerParam::i().ourPenaltyAreaLineX())
        {
            home_pos = ball_next_pos;
            Bhv_BasicMove( home_pos ).execute( agent );
        }
        else if( self_next_pos.x < ball_next_pos.x
                 && (self_next_pos.dist(ball_next_pos) <= 5.0
                     || self_next_pos.dist(ball_next_pos)
                     <= Obake_Analysis().getDistFromNearestMate(agent,
                                                                ball_next_pos,
                                                                except_near_opp_defender,
                                                                except_role_side_or_center_back))
                     && ball_next_pos.x > rcsc::ServerParam::i().ourPenaltyAreaLineX())
        {
            home_pos = ball_next_pos;
            Bhv_BasicMove( home_pos ).execute( agent );
        }
        else
        {
            Bhv_ObakeMark(home_pos,
                          S_mark_number,
                          S_mark_number_count,
                          S_mark_position).execute(agent);
        }
    }
    else if(ball_next_pos.x < 0.0
            && ball_next_pos.dist(self_next_pos) >= 25.0)
    {
        if(ball_next_pos.x <= 0.0)
	{
            home_pos.x = wm.offsideLineX() - 0.5;
	}
/*
        if(ball_next_pos.x <=rcsc::ServerParam::i().theirPenaltyAreaLineX()
           && ball_next_pos.y * self_next_pos.y < 0.0
           && ball_next_pos.absY() >= rcsc::ServerParam::i().goalHalfWidth())
	{
            double difference = 25.0665;
	    home_pos.y =(wm.self().unum() == 10) ? difference : -difference;
	}
*/
        Bhv_BasicMove(home_pos).execute(agent);
    }
    else if(mate_min < opp_min && mate_min < self_min)
    {
/*	   home_pos.y = 8.0;
	    if(wm.self().unum()==9)
	    {
		home_pos.y = -8.0;
	    }
*/
/*
        if(wm.ball().pos().x <=rcsc::ServerParam::i().theirPenaltyAreaLineX())
	{
            double difference = 13.3756;
	    home_pos.y =(wm.self().unum() == 10) ? difference : -difference;
	}
*/
        if(home_pos.y * wm.ball().pos().y > 0.0
           && wm.ball().pos().x >= rcsc::ServerParam::i().theirPenaltyAreaLineX())
        {
            home_pos.y = rcsc::ServerParam::i().penaltyAreaHalfWidth();
            if(home_pos.y < 0.0)
            {
                home_pos.y *= -1.0;
            }
        }
        Bhv_ObakeReceive(home_pos).execute(agent);
    }
    else
    {
        if(ball_next_pos.x <= 0.0)
	{
            home_pos.x = wm.offsideLineX() - 0.5;
	}
	/*if(ball_next_pos.x <=rcsc::ServerParam::i().theirPenaltyAreaLine()
           && ball_next_pos.y * self_next_pos.y < 0.0)
	{
	    home_pos.y = 8.5;
	    if(wm.self().unum()==9)
	    {
		home_pos.y = -8.5;
	    }
	}
        else if(ball_next_pos.x <=rcsc::ServerParam::i().theirPenaltyAreaLine()
                && ball_next_pos.y * self_next_pos.y > 0.0)
	{
            home_pos.y = 14.0;
	    if(wm.self().unum()==9)
	    {
		home_pos.y = -14.0;
	    }
        }
        */
        Bhv_BasicMove(home_pos).execute(agent);
    }
    /********************************************/
    //end of the added code
}

