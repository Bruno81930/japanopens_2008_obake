
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
#include <rcsc/geom/vector_2d.h>
#include <rcsc/common/logger.h>
#include <rcsc/player/player_agent.h>
#include <rcsc/player/intercept_table.h>
#include <rcsc/player/debug_client.h>
#include <rcsc/formation/formation.h>

#include "bhv_basic_offensive_kick.h"
#include "bhv_basic_move.h"

#include "obake_analysis.h"
#include "obake_fuzzy_grade.h"

#include "strategy.h"

#include "bhv_obake_mark.h"
#include "bhv_obake_receive.h"
#include "obake_update.h"
#include "bhv_obake_receive_test.h"
#include "role_center_forward.h"

/*-------------------------------------------------------------------*/
/*!

*/
//begin of the added code
/********************************************/
int RoleCenterForward::S_mark_number = 0;
int RoleCenterForward::S_mark_number_count = 0;
int RoleCenterForward::S_pass_cycle = 0;
int RoleCenterForward::S_previous_ball_keeper = 0;
int RoleCenterForward::S_current_ball_keeper = 0;
int RoleCenterForward::S_current_ball_keeper_count = 0;
int RoleCenterForward::S_current_ball_keeper_total = 0;
rcsc::Vector2D RoleCenterForward::S_mark_position = rcsc::Vector2D(0.0, 0.0);
/********************************************/
//end of the added code

void
RoleCenterForward::execute( rcsc::PlayerAgent * agent )
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
       /*
          if(!Bhv_ObakeReceiveTest().checkExistOpponentTest(agent))
        {
            std::cout<<"error"<<std::endl;
        }
        */
        /*
        if(!Bhv_ObakeReceiveTest().putNewVectorIntoSearchOrderTest(agent))
        {
            std::cout<<"error"<<std::endl;
        }
        */
        //Bhv_ObakeReceiveTest().getBestMiddleTest(agent);
        //begin of the added code
/********************************************/
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
        doMove( agent );
    }
}







/*-------------------------------------------------------------------*/
/*!

*/
void
RoleCenterForward::doKick( rcsc::PlayerAgent * agent )
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
RoleCenterForward::doMove( rcsc::PlayerAgent * agent )
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
/*    if(opp_min < mate_min && self_min == mate_min
       && S_mark_number_count < 15)
    {
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
        home_pos = ball_pos;
        if(home_pos.x < wm.self().pos().x)
        {
            home_pos.x -= 5.0;
        }
        else if(std::abs(ball_pos.y - wm.self().pos().y) > 3.5
            || std::abs(ball_pos.y - wm.self().pos().y) >
            std::abs(ball_pos.x - wm.self().pos().x))
        {
            home_pos = wm.self().pos();
            home_pos.y = ball_pos.y;
        }
        else
        {
            home_pos.x -= 2.5;
        }
    }*/
    const double decay = rcsc::ServerParam::i().ballDecay();
    rcsc::Vector2D ball_pos, v;
    ball_pos = wm.ball().pos();
    if(wm.ball().velCount() < 3)
    {
        v = wm.ball().vel();
        v.setLength((1-std::pow(decay, (wm.ball().velCount()+1))) / (1 - decay));
        ball_pos += v;
    }
/*    if(opp_min < mate_min && self_min == mate_min 
       && !(self_min == mate_min && S_mark_number_count < 15))
    {
        
        home_pos = ball_pos;
        if(home_pos.x < wm.self().pos().x)
        {
            home_pos.x -= 5.0;
        }
        else if(std::abs(ball_pos.y - wm.self().pos().y) > 3.5)
        {
            home_pos = wm.self().pos();
            home_pos.y = ball_pos.y;
        }
        else
        {
            home_pos.x -= 2.5;
        }
        }*/
    if(!wm.existKickableTeammate()
       && self_min == mate_min
       && self_min <= opp_min)
    {
        Bhv_BasicMove(home_pos).execute(agent);
    }
    else if(opp_min < mate_min 
            && wm.self().pos().x >= ball_pos.x
            && home_pos.dist(ball_pos) <= 15.0
            && (wm.ball().pos().x > 0.0
		|| wm.ball().pos().dist(wm.self().pos()) <= 10.0))//rcsc::ServerParam::i().ourPenaltyAreaLineX())
    {
        home_pos = ball_pos;
        Bhv_BasicMove( home_pos ).execute( agent );
    }
    else if(opp_min < mate_min 
            && wm.self().pos().x < ball_pos.x
            && home_pos.dist(ball_pos) <= 10.0
            && wm.ball().pos().x > rcsc::ServerParam::i().ourPenaltyAreaLineX())
    {
        home_pos = ball_pos;
        Bhv_BasicMove( home_pos ).execute( agent );
    }
    else if(opp_min < mate_min 
            && !(self_min == mate_min && S_mark_number_count < 15)
            && wm.ball().pos().x > 0.0)
    {
        Bhv_ObakeMark(home_pos,
                      S_mark_number,
                      S_mark_number_count,
                      S_mark_position).execute(agent);
    }
/*    else if(mate_min < opp_min && mate_min < self_min)
    {
        Bhv_ObakeReceive(home_pos).execute(agent);
    }
*/
    else
    {
        if(!Obake_Analysis().checkExistOppPenaltyAreaIn(home_pos))
        {
            home_pos.x = wm.offsideLineX() - 1.0;
            Bhv_BasicMove(home_pos).execute(agent);
        }
        else 
        {
            Bhv_ObakeReceive(home_pos).execute(agent);
        }
    }
    /********************************************/
    //end of the added code
}

