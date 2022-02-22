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

#include <rcsc/common/logger.h>
#include <rcsc/player/player_agent.h>
#include <rcsc/action/basic_actions.h>
#include <rcsc/geom/vector_2d.h>
#include <rcsc/player/intercept_table.h>
#include <rcsc/common/server_param.h>

#include "bhv_basic_move.h"
#include "obake_analysis.h"

#include "bhv_obake_mark.h"                            

bool
Bhv_ObakeMark::execute(rcsc::PlayerAgent * agent)
{
    rcsc::dlog.addText( rcsc::Logger::TEAM,
                        "%s:%d: Bhv_ObakeMark"
                        ,__FILE__, __LINE__ );
    const rcsc::WorldModel & wm = agent->world();
    Obake_Analysis().setRole(wm.self().unum(),
			     M_role_side_or_center_back,
			     M_role_defensive_half,
			     M_role_offensive_half,
			     M_role_side_or_center_forward);
    const int previous_mark_number  = M_mark_number;
    const double max_pass_dist = 25.0;
    const double max_mark_dist = 6.5;
    rcsc::Vector2D target_pos = M_home_pos;
    if(M_role_side_or_center_back) 
    {
        M_mark_number = getMarkNumber(agent);
    }
    else if(M_role_defensive_half)
    {
        /*if(wm.self().unum() == 6)
        {
            std::cout<<"DH 6 execute "<<M_mark_number;
            }*/
        M_mark_number = getMarkNumber(agent);
        /*if(wm.self().unum() == 6)
        {
            std::cout<<M_mark_number<<std::endl;
            }*/
    }
    else if(M_role_side_or_center_forward
            && wm.self().pos().dist(wm.ball().pos()) <= max_pass_dist + 5.0)
    {
        M_mark_number = getMarkNumber(agent);
    }
    else if(M_role_offensive_half)
    {
        if(M_home_pos.x <= (rcsc::ServerParam::i().ourPenaltyAreaLineX() + 3.0))
        {
            M_mark_number = getNearestMarkNumber(agent, max_mark_dist);
        }
        else
        {
            M_mark_number = getMarkNumber(agent);
        }
    }
    
    if(previous_mark_number != M_mark_number)
    {
        M_mark_number_count = 0;
    }
    
    if((M_role_side_or_center_back
        || (M_role_defensive_half
            && M_home_pos.x >= (rcsc::ServerParam::i().ourPenaltyAreaLineX() + 3.0)))
       && M_mark_number != 0)
    {
        target_pos = getMarkPosition(agent, M_mark_number);
        target_pos.x = M_home_pos.x;
        M_mark_position = target_pos;
    }
    else if((M_role_side_or_center_forward
             || M_role_offensive_half)
            && M_mark_number != 0
            && wm.self().pos().dist(wm.ball().pos()) <= max_pass_dist + 5.0)
    {
        target_pos = getMarkPosition(agent, M_mark_number);
        M_mark_position = target_pos;
    }
    else if((M_role_offensive_half || M_role_defensive_half)
            && M_mark_number != 0
            && M_home_pos.x <= (rcsc::ServerParam::i().ourPenaltyAreaLineX() + 3.0))
    {
        const rcsc::AbstractPlayerObject * mark_target =  wm.opponent(M_mark_number);
        bool offensive = false;
        if(mark_target)
        {
            const rcsc::Vector2D left_top(((mark_target)->pos().x - 5.0), ((mark_target)->pos().y - 3.0));
            const double length = 5.0;
            const double width = 6.0;
            const rcsc::Rect2D check_area(left_top, length, width);
            if(wm.existTeammateIn(check_area, 10, false))
            {
                offensive = true;
            }

        }
        target_pos = getMarkPosition(agent, M_mark_number, offensive);
        M_mark_position = target_pos;
    }
    rcsc::dlog.addText(rcsc::Logger::TEAM,
                       "%s:%d: mark target_number:%d target_pos(%f, %f)"
                       ,__FILE__, __LINE__,M_mark_number,
                       target_pos.x, target_pos.y );
    Bhv_BasicMove(target_pos).execute(agent);                                                   
    return true;
}

/*!
  This function retuns the nearest opponent from self
  for marking
*/
int
Bhv_ObakeMark::getNearestMarkNumber(rcsc::PlayerAgent * agent,
                                    const double &max_mark_dist)
{
    const rcsc::WorldModel & wm = agent->world();
    int mark_number = 0;
    if(M_mark_number >= 2 && M_mark_number <= 11)
    {
        mark_number = M_mark_number;
        const rcsc::AbstractPlayerObject * previous_mark_opp = wm.opponent(mark_number);
        if(previous_mark_opp)
        {
            if((previous_mark_opp)->pos().dist(M_home_pos) <= max_mark_dist + 5.0)
            {
                return mark_number;
            }
        }
    }
    const rcsc::PlayerObject * nearest_self_opp = wm.getOpponentNearestToSelf(10);
    if(nearest_self_opp)
    {
         if((nearest_self_opp)->pos().dist(M_home_pos) <= max_mark_dist)
         {
             mark_number = (nearest_self_opp)->unum();
         }
     }
     return mark_number;
 }

int
Bhv_ObakeMark::getMarkNumber(rcsc::PlayerAgent * agent)
{
    const rcsc::WorldModel & wm = agent->world();
    double max_pass_dist = 25.0;
    if(M_role_defensive_half)
    {
        max_pass_dist = 30.0;
    }
    if(M_role_side_or_center_forward
       && wm.ball().pos().x < rcsc::ServerParam::i().ourPenaltyAreaLineX())
    {
        max_pass_dist = 20.0;
    }
    double max_mark_dist = 10.0;
    if(M_role_defensive_half)
    {
        max_mark_dist += 5.0;
    }
    else if(M_role_offensive_half)
    {
        max_mark_dist += 3.0;
    }
    const int mate_min = wm.interceptTable()->teammateReachCycle();
    const int self_min = wm.interceptTable()->selfReachCycle();
    int mark_number = 0;
    rcsc::Vector2D left_top((wm.self().pos().x - 5.0), -rcsc::ServerParam::i().pitchHalfWidth());
    double length = 5.0 + std::min(std::abs(wm.self().pos().x - wm.ball().pos().x), 10.0);
    const double width = rcsc::ServerParam::i().pitchWidth();
    int mate_number = Obake_Analysis().getMateNumber(agent, left_top, length, width);
    int opp_number = Obake_Analysis().getOppNumber(agent, left_top, length, width);
    const rcsc::PlayerPtrCont::const_iterator mate_end = wm.teammatesFromBall().end();
    if(M_mark_number >= 2 && M_mark_number <= 11 
       && (!M_role_side_or_center_back ||
           (M_role_side_or_center_back && wm.ball().pos().x <= rcsc::ServerParam::i().ourPenaltyAreaLineX())))
    {
        mark_number = M_mark_number;
        const rcsc::AbstractPlayerObject * previous_mark_opp = wm.opponent(mark_number);
        if(previous_mark_opp)
        {
            const double dist_from_self = (previous_mark_opp)->pos().dist(wm.self().pos());
            double dist = 5.0;
            if(wm.self().unum() == 4 || wm.self().unum() == 5)
            {
                dist = 4.0;
            }
            if(M_home_pos.x <= rcsc::ServerParam::i().ourPenaltyAreaLineX())
            {
                dist =  2.5;
            }
            if(M_role_side_or_center_back
               && self_min > mate_min
               && std::abs(M_home_pos.y - (previous_mark_opp)->pos().y) <= dist
               && std::abs(M_home_pos.x - (previous_mark_opp)->pos().x) <= dist
               && mate_number >= opp_number
               && wm.self().pos().x >= wm.ball().pos().x)
            {
                return mark_number;
            }
            else if(M_role_side_or_center_back
                    && self_min > mate_min
                    && (previous_mark_opp)->pos().x >= -rcsc::ServerParam::i().pitchHalfLength()
                    && (previous_mark_opp)->pos().x <= (-rcsc::ServerParam::i().pitchHalfLength()
                                                        + rcsc::ServerParam::i().goalAreaLength() + 5.5)
                    && (previous_mark_opp)->pos().absY() <= rcsc::ServerParam::i().goalAreaWidth())
            {
                return mark_number;
            }
            else if(M_role_defensive_half
                    &&  dist_from_self <= max_pass_dist
                    && std::abs(M_home_pos.y -(previous_mark_opp)->pos().y) <= 17.0)

            {
                const double mark_dist = std::min(dist_from_self, 4.0);
                rcsc::Circle2D circle((previous_mark_opp)->pos(), mark_dist); 
                if(!wm.existTeammateIn(circle, 10, false) 
                   || (previous_mark_opp)->pos().x <= wm.ball().pos().x
                   || Obake_Analysis().checkExistOurPenaltyAreaIn((previous_mark_opp)->pos()))
                {
                    return mark_number;
                }
            }
            else if(!M_role_side_or_center_back
                    && (previous_mark_opp)->distFromBall() <= max_pass_dist
                    && dist_from_self <= max_mark_dist + 5.0)
            {
                const double mark_dist = std::min(dist_from_self, 4.0);
                rcsc::Circle2D circle((previous_mark_opp)->pos(), mark_dist); 
                if(!wm.existTeammateIn(circle, 10, false)
                   || (previous_mark_opp)->pos().x <= wm.ball().pos().x
                   || Obake_Analysis().checkExistOurPenaltyAreaIn((previous_mark_opp)->pos()))
                {
                    return mark_number;
                }
            }
        }
    }
    if(M_role_offensive_half)
    {
        mark_number = getMarkNumberForOffensiveHalf(agent);
    }
    else if(M_role_side_or_center_back)
    {
/*        if(M_home_pos.x <= rcsc::ServerParam::i().ourPenaltyAreaLineX())
        {
            mark_number = getMarkNumberInPenaltyArea(agent);
        }
*/

    }
    else if(M_role_defensive_half)
    {
        /*if(wm.self().unum() == 6)
        {
            std::cout<<"DH 6 bhv obake mark numeber = "<<mark_number;
            }*/
        mark_number = getMarkNumberForDefensiveHalf(agent);
        /*if(wm.self().unum() == 6)
        {
            std::cout<<" "<<mark_number<<std::endl;
            }*/
    }
    else
    {
        if(wm.ball().pos().x > rcsc::ServerParam::i().ourPenaltyAreaLineX())
	{
	    mark_number = getMarkNumberForForward(agent);
	}
	else
	{
	    mark_number = getMarkNumberForForwardInPenaltyArea(agent);
	}
    }

    return mark_number;
}

/*!
  This fuction return 0 when there isn't any opponent
  that the player shouldn't mark
*/
/*
int 
Bhv_ObakeMark::getMarkNumberInPenaltyArea(rcsc::PlayerAgent * agent)
{
    const rcsc::WorldModel & wm = agent->world();
    const double goal_area_length = rcsc::ServerParam::i().goalAreaLength();
    const double goal_area_width = rcsc::ServerParam::i().goalAreaWidth();
    const double goal_area_half_width = rcsc::ServerParam::i().goalAreaWidth() / 2;
    const int mate_min = wm.interceptTable()->teammateReachCycle();
    const int self_min = wm.interceptTable()->selfReachCycle();
    const rcsc::Vector2D left_top(-rcsc::ServerParam::i().pitchHalfLength(), -goal_area_half_width);
    const double length = goal_area_length + 3.5;
    const double width = goal_area_width;
*/
    /*
    const int opp_number = Obake_Analysis().getOppNumber(agent, left_top, length, width);
    const int mate_number = Obake_Analysis().getMateBackNumber(agent, left_top, length, width);
    */
/*
    int mark_number = 0;
    if(self_min != mate_min)
    {
        const rcsc::PlayerPtrCont::const_iterator end = wm.opponentsFromBall().end();
        for(rcsc::PlayerPtrCont::const_iterator opp = wm.opponentsFromBall().begin();
            opp != end;
            opp++)
        {
            const rcsc::Circle2D circle((*opp)->pos(), 3.0);
            const double opp_length = std::abs(-rcsc::ServerParam::i().pitchHalfLength() - (*opp)->pos().x);
            const double opp_width = (*opp)->pos().absY();
            rcsc::Rect2D rect(rcsc::Vector2D(-rcsc::ServerParam::i().pitchHalfLength(), 0.0), opp_length, opp_width);
            if((*opp)->pos().y < 0.0)
            {
                rect.setTopLeft((*opp)->pos());
            }
            if((*opp)->pos().x >= -rcsc::ServerParam::i().pitchHalfLength()
               && (*opp)->pos().x <= (-rcsc::ServerParam::i().pitchHalfLength() + length)
               && (*opp)->pos().absY() <= goal_area_half_width
               && !wm.existTeammateIn(circle, 10, false))
            {
                mark_number = (*opp)->unum();
                break;
            }
            else if((*opp)->pos().x >= -rcsc::ServerParam::i().pitchHalfLength()
                    && (*opp)->pos().x <= (-rcsc::ServerParam::i().pitchHalfLength() + length)
                    && !wm.existOpponentIn(rect, 10, false)
                    && !wm.existTeammateIn(circle, 10, false))
            {
                mark_number = (*opp)->unum();
                break;
            }

        }
    }
    return mark_number;
}
*/
int 
Bhv_ObakeMark::getMarkNumberInPenaltyArea(rcsc::PlayerAgent * agent)
{
    const rcsc::WorldModel & wm = agent->world();
    const int mate_min = wm.interceptTable()->teammateReachCycle();
    const int self_min = wm.interceptTable()->selfReachCycle();
    const rcsc::Vector2D left_top(-rcsc::ServerParam::i().pitchHalfLength(), -rcsc::ServerParam::i().goalAreaHalfWidth());
    const double length =  rcsc::ServerParam::i().goalAreaLength() + 3.5;
    /*
    const double goal_area_width = rcsc::ServerParam::i().goalAreaWidth();
    const double width = goal_area_width;
    const int opp_number = Obake_Analysis().getOppNumber(agent, left_top, length, width);
    const int mate_number = Obake_Analysis().getMateBackNumber(agent, left_top, length, width);
    */
    int mark_number = 0;
   if(self_min > mate_min)
    {
        const rcsc::PlayerPtrCont::const_iterator end = wm.opponentsFromBall().end();
        for(rcsc::PlayerPtrCont::const_iterator opp = wm.opponentsFromBall().begin();
            opp != end;
            opp++)
        {
            const rcsc::Circle2D circle((*opp)->pos(), 3.0);
            const double opp_length = std::abs(-rcsc::ServerParam::i().pitchHalfLength() - (*opp)->pos().x);
            const double opp_width = (*opp)->pos().absY();
            rcsc::Rect2D rect(rcsc::Vector2D(-rcsc::ServerParam::i().pitchHalfLength(), 0.0), opp_length, opp_width);
            if((*opp)->pos().y < 0.0)
            {
                rect.setTopLeft((*opp)->pos());
            }
            if((*opp)->pos().x >= -rcsc::ServerParam::i().pitchHalfLength()
               && (*opp)->pos().x <= (-rcsc::ServerParam::i().pitchHalfLength() + length)
               && (*opp)->pos().absY() <= rcsc::ServerParam::i().goalAreaHalfWidth()
               && !wm.existTeammateIn(circle, 10, false))
            {
                mark_number = (*opp)->unum();
                break;
            }
            else if((*opp)->pos().x >= -rcsc::ServerParam::i().pitchHalfLength()
                    && (*opp)->pos().x <= (-rcsc::ServerParam::i().pitchHalfLength() + length)
                    && !wm.existOpponentIn(rect, 10, false)
                    && !wm.existTeammateIn(circle, 10, false))
            {
                mark_number = (*opp)->unum();
                break;
            }

        }
    }
    return mark_number;
}

 /*!
 This fuction return 0 when there isn't any opponent
 that the player shouldn't mark
 */
int
Bhv_ObakeMark::getMarkNumberForBack(rcsc::PlayerAgent * agent)
{
    const rcsc::WorldModel & wm = agent->world();
    const double length = 4.0 + std::min((wm.ball().pos().x - wm.self().pos().x), 4.5);
    const double width = rcsc::ServerParam::i().pitchWidth();
    const rcsc::Vector2D left_top((wm.self().pos().x - 4.0), -rcsc::ServerParam::i().pitchHalfWidth());
    int mark_number = 0;
    const int opp_number = Obake_Analysis().getOppNumber(agent, left_top, length, width);
    const int mate_number = Obake_Analysis().getMateBackNumber(agent, left_top, length, width);
    if(wm.self().pos().x < wm.ball().pos().x)
    {
        if(mate_number >= opp_number
           && Obake_Analysis().checkDefenseLine(agent))
        {
            int max = 0;
            switch(wm.self().unum()){
                case 4:
                    max = 1;
                case 2:
                    max = 2;
                case 3:
                    max = 3;
                case 5:
                    max = 4;
            }
            std::vector<double> opp_y_vector = Obake_Analysis().getOppYVector(agent, left_top, length, width);
            //const double max_x_range = 4.5;
            double max_y_range = 3.5;
            std::list<int>::size_type count;
            bool exist_mark_target = false;
            count = 1;
            const std::vector<double>::const_iterator end = opp_y_vector.end();
            for(std::vector<double>::const_iterator p = opp_y_vector.begin();
                p != end;
                p++)
            {
                if(std::abs(M_home_pos.y - (*p)) <= max_y_range)
                {
                    exist_mark_target = true;
                    break;
                }
                count++;
            }
            if(exist_mark_target)
            {
                std::list<int> opp_number_list = Obake_Analysis().getOppNumberYList(agent,left_top, length, width);
                std::list<int>::const_iterator opp_number = opp_number_list.begin();
                while(count != 0)
                {
                    opp_number++;
                    count--;
                }
                if(count <= opp_number_list.size())
                {
                    const rcsc::AbstractPlayerObject * mark_target = wm.opponent((*opp_number));
                    if(mark_target)
                    {
                        mark_number = (*opp_number);
                    }
                }
            }
        }

    }
    return mark_number;
}

/*!
  This fuction return 0 when there isn't any opponent
  that the player shouldn't mark
*/
int
Bhv_ObakeMark::getMarkNumberForDefensiveHalf(rcsc::PlayerAgent * agent)
{
    const rcsc::WorldModel & wm = agent->world();
    int mark_number = 0;
    rcsc::Vector2D v;
    bool exist_target = false;
    const bool except_near_opp_defender = false;
    const bool except_role_side_or_center_back = true;
    const rcsc::PlayerPtrCont::const_iterator end = wm.opponentsFromBall().end();
    for(rcsc::PlayerPtrCont::const_iterator opp = wm.opponentsFromBall().begin();
        opp != end;
        opp++)
    {
        if(((*opp)->pos().x <= wm.ball().pos().x + 5.0 
            || ((*opp)->pos().x <= rcsc::ServerParam::i().ourPenaltyAreaLineX()
                && ((*opp)->pos().absY() <= wm.ball().pos().absY() 
                    || (*opp)->pos().absY() <= rcsc::ServerParam::i().goalHalfWidth())))
           && std::abs(M_home_pos.y - (*opp)->pos().y) <= 10.0
           && (*opp)->distFromSelf() <= Obake_Analysis().getDistFromNearestMate(agent,
                                                                                (*opp)->pos(),
                                                                                except_near_opp_defender,
                                                                                except_role_side_or_center_back)
            && M_home_pos.dist((*opp)->pos()) < 12.5)
        {
            if(exist_target)
            {
                if(((*opp)->pos().x <= v.x 
                    || ((*opp)->pos().x <= rcsc::ServerParam::i().ourPenaltyAreaLineX()
                        && wm.ball().pos().x <= rcsc::ServerParam::i().ourPenaltyAreaLineX()))
                   && (*opp)->pos().absY() < v.absY())
                {
                    v = (*opp)->pos();
                    mark_number = (*opp)->unum();
                }
            }
            else
            {
                v = (*opp)->pos();
                mark_number = (*opp)->unum();
                exist_target = true;
            }
        }
    }
    if(!exist_target)
    {
        const rcsc::PlayerObject * nearest_self_opp = wm.getOpponentNearestToSelf(10);        
        if(nearest_self_opp)
        {
            if(std::abs(M_home_pos.y - (nearest_self_opp)->pos().y) <= 10.0)
            {
                mark_number = (nearest_self_opp)->unum();
            }
        }
    }
    return mark_number;
}

int
Bhv_ObakeMark::getMarkNumberForOffensiveHalf(rcsc::PlayerAgent * agent)
{
    const rcsc::WorldModel & wm = agent->world();
    const rcsc::PlayerObject * nearest_ball_opp = wm.getOpponentNearestToBall(10);
    double max_pass_dist = 25.0;
    double max_mark_dist = 15.0;
    int mark_number = 0;
    if(wm.ball().pos().x < rcsc::ServerParam::i().ourPenaltyAreaLineX())
    {
        const rcsc::PlayerPtrCont::const_iterator end = wm.teammatesFromBall().end();
        for(rcsc::PlayerPtrCont::const_iterator mate = wm.teammatesFromBall().begin();
            mate != end;
            mate++)
        {
            if((*mate)->pos().x < wm.ball().pos().x) 
            {
                rcsc::Circle2D circle((*mate)->pos(), 4.5);
                if(wm.self().pos().dist(wm.ball().pos()) < (*mate)->distFromBall())
                {
                    if(nearest_ball_opp)
                    {
                        mark_number = (nearest_ball_opp)->unum();
                        return mark_number;
                    }
                }
                if(!wm.existOpponentIn(circle, 10.0, false))
                {
                    break;
                }
            }
        }
    }
    const rcsc::PlayerObject * nearest_self_opp = wm.getOpponentNearestToSelf(10);
    if(nearest_self_opp)
    {
        mark_number = (nearest_self_opp)->unum();
    }
    const bool except_near_opp_defender = false;
    const bool except_role_side_or_center_back = true;
    const rcsc::PlayerPtrCont::const_iterator opp_end = wm.opponentsFromBall().end();
    for(rcsc::PlayerPtrCont::const_iterator opp = wm.opponentsFromBall().begin();
        opp != opp_end;
        opp++)
    {
        if((wm.ball().pos().x >= rcsc::ServerParam::i().ourPenaltyAreaLineX()
            || (*opp)->distFromBall() <= 15.0)
           && ((*opp)->distFromBall() <= max_pass_dist
               || M_home_pos.y * wm.ball().pos().y < 0.0)
           && ((*opp)->distFromSelf() <= max_mark_dist 
               /*|| (M_home_pos.y * wm.ball().pos().y < 0.0)*/)
           && (*opp)->distFromSelf() <= Obake_Analysis().getDistFromNearestMate(agent,
                                                                                (*opp)->pos(),
                                                                                except_near_opp_defender,
                                                                                except_role_side_or_center_back))
            
        {
            mark_number = (*opp)->unum();
            return mark_number;
        }
    }
    return mark_number;
}

/*!
  This fuction return 0 when there isn't any opponent
  that the player shouldn't mark
*/
int
Bhv_ObakeMark::getMarkNumberForForward(rcsc::PlayerAgent * agent)
{
    const rcsc::WorldModel & wm = agent->world();
    const rcsc::PlayerObject * nearest_ball_opp = wm.getOpponentNearestToBall(10);
    const double max_pass_dist = 30.0;//25.0;
    const double max_mark_dist =(M_home_pos.y * wm.self().pos().y > 0.0)
        ? 15.0 : 17.5;//10.0;
    const double dist_self_from_ball = wm.self().pos().dist(wm.ball().pos());
    int mark_number = 0;
    const rcsc::PlayerPtrCont::const_iterator end = wm.teammatesFromBall().end();
    if(wm.ball().pos().x >= rcsc::ServerParam::i().ourPenaltyAreaLineX())
    {
        if(wm.ball().pos().x >= wm.self().pos().x)
        {
            for(rcsc::PlayerPtrCont::const_iterator mate = wm.teammatesFromBall().begin();
                mate != end;
                mate++)
            {
                if((*mate)->distFromBall() > dist_self_from_ball)
                {
                    if(nearest_ball_opp)
                    {
                        mark_number = (nearest_ball_opp)->unum();
                        return mark_number;
                    }
                }
                if((*mate)->pos().x < wm.ball().pos().x) 
                {
                    break;
                }
            }
        }
    }
    const rcsc::PlayerObject * nearest_opp_from_self = wm.getOpponentNearestToSelf(10);
    if(nearest_opp_from_self)
    {
        mark_number = (nearest_opp_from_self)->unum();
    }
    const bool except_near_opp_defender = true;
    const bool except_role_side_or_center_back = true;
    const double back_dist = (M_home_pos.y * wm.ball().pos().y < 0.0)
        ? 7.0 : 3.0;
    double nearest_dist;
    const rcsc::PlayerPtrCont::const_iterator opp_end = wm.opponentsFromBall().end();
    for(rcsc::PlayerPtrCont::const_iterator opp = wm.opponentsFromBall().begin();
        opp != opp_end;
        opp++)
    {
        if((((*opp)->pos().x < wm.ball().pos().x + back_dist //+ 10.0 
//             && wm.ball().pos().x >= rcsc::ServerParam::i().ourPenaltyAreaLineX()
             && !Obake_Analysis().checkExistOppPenaltyAreaIn((*opp)->pos()) )
            || wm.ball().pos().x < rcsc::ServerParam::i().ourPenaltyAreaLineX())
           && (*opp)->distFromBall() <= max_pass_dist
           && (*opp)->distFromSelf() <= max_mark_dist)

        {
            if((*opp)->pos().y * wm.ball().pos().y < 0.0
               && M_home_pos.y * wm.ball().pos().y < 0.0)
            {
                mark_number = (*opp)->unum();
                return mark_number;
            }
            nearest_dist = Obake_Analysis().getDistFromNearestMate(agent,
                                                                   (*opp)->pos(),
                                                                   except_near_opp_defender,
                                                                   except_role_side_or_center_back);
            if(wm.self().pos().dist((*opp)->pos()) < nearest_dist)
            {
                mark_number = (*opp)->unum();
                return mark_number;
            }
        }
    }
    const rcsc::PlayerObject * nearest_opp_from_ball = wm.getOpponentNearestToBall(10);
    if(nearest_opp_from_ball
       && dist_self_from_ball <= 20.0)
    {
        if(nearest_opp_from_self)
        {
            const double r = 3.0;
            const int mate_number = Obake_Analysis().getMateNumber(agent,
                                                                   wm.ball().pos(),
                                                                   r);
            if(dist_self_from_ball <= 10.0)
            {
                mark_number = (nearest_opp_from_ball)->unum();
            }
            else if(std::abs((nearest_opp_from_self)->pos().x - wm.offsideLineX()) >= 5.0)
            {
                mark_number = (nearest_opp_from_ball)->unum();
            }
            else if(std::abs(Obake_Analysis().getMateDefenseLine(agent) - wm.ball().pos().x) <= 15.0
                    && mate_number == 0
                    && dist_self_from_ball <= 15.0)
            {
                mark_number = (nearest_opp_from_ball)->unum();
            }
            /* else if(std::abs(Obake_Analysis().getMateDefenseLine(agent) - wm.ball().pos().x) <= 15.0
                    && mate_number < 2)
            {
                
                if(mate_number == 0)
                {
                    mark_number = (nearest_opp_from_ball)->unum();
                }
                else
                {
                    if(dist_self_from_ball <= 15.0)
                    {
                        mark_number = (nearest_opp_from_ball)->unum();
                    }
                }
                }*/
        }
        else
        {
            mark_number = (nearest_opp_from_ball)->unum();
        }
    }
    return mark_number;
}
int
Bhv_ObakeMark::getMarkNumberForForwardInPenaltyArea(rcsc::PlayerAgent * agent)
{
    const rcsc::WorldModel & wm = agent->world();
//    const rcsc::PlayerObject * nearest_ball_opp = wm.getOpponentNearestToBall(10);
    const double max_pass_dist = 25.0;
    const double max_mark_dist = 10.0;
    int mark_number = 0;
    bool front = false;
    const rcsc::PlayerPtrCont::const_iterator end = wm.teammatesFromBall().end();
    if(wm.ball().pos().x >= rcsc::ServerParam::i().ourPenaltyAreaLineX())
    {
        for(rcsc::PlayerPtrCont::const_iterator mate = wm.teammatesFromBall().begin();
            mate != end;
            mate++)
        {
            if((*mate)->pos().x < wm.ball().pos().x) 
            {
                if(wm.self().pos().dist(wm.ball().pos()) < (*mate)->distFromBall())
                {
                    front = true;
               
                }
                else
                {
                    break;
                }
            }
            if((*mate)->pos().x >= wm.ball().pos().x
               && front)
            {
                if(wm.self().pos().dist(wm.ball().pos()) <= (*mate)->distFromBall())
                {
                    mark_number = (*mate)->unum();
                    return mark_number;
                } 
                else
                {
                    break;
                }
            }
          
        }
    }
    const rcsc::PlayerObject * nearest_self_opp = wm.getOpponentNearestToSelf(10);
    if(nearest_self_opp)
    {
        mark_number = (nearest_self_opp)->unum();
    }
    const rcsc::PlayerPtrCont::const_iterator opp_end = wm.opponentsFromBall().end();
    for(rcsc::PlayerPtrCont::const_iterator opp = wm.opponentsFromBall().begin();
        opp != opp_end;
        opp++)
    {
        if((((*opp)->pos().x < wm.ball().pos().x + 10.0 
             && wm.ball().pos().x >= rcsc::ServerParam::i().ourPenaltyAreaLineX())
            || wm.ball().pos().x < rcsc::ServerParam::i().ourPenaltyAreaLineX())
           && (*opp)->distFromBall() <= max_pass_dist
           && (*opp)->distFromSelf() <= max_mark_dist)

        {
            mark_number = (*opp)->unum();
            return mark_number;
        }
    }
    return mark_number;
}

/*!
  This fuction return 0 when there isn't any opponent
  that the player shouldn't mark
*/
/*
int
Bhv_ObakeMark::getMarkNumberForForward(rcsc::PlayerAgent * agent)
{
    const rcsc::WorldModel & wm = agent->world();
    const rcsc::PlayerObject * nearest_ball_opp = wm.getOpponentNearestToBall(10);
    const double max_pass_dist = 25.0;
    const double max_mark_dist = 10.0;
    int mark_number = 0;
    bool front = false;
    const rcsc::PlayerPtrCont::const_iterator end = wm.teammatesFromBall().end();
    if(wm.ball().pos().x >= rcsc::ServerParam::i().ourPenaltyAreaLineX())
    {
        for(rcsc::PlayerPtrCont::const_iterator mate = wm.teammatesFromBall().begin();
            mate != end;
            mate++)
        {
            if((*mate)->pos().x < wm.ball().pos().x) 
            {
                if(wm.self().pos().dist(wm.ball().pos()) < (*mate)->distFromBall())
                {
                    front = true;
               
                }
                else
                {
                    break;
                }
            }
            if((*mate)->pos().x >= wm.ball().pos().x
               && front)
            {
                if(wm.self().pos().dist(wm.ball().pos()) <= (*mate)->distFromBall())
                {
                    mark_number = (*mate)->unum();
                    return mark_number;
                } 
                else
                {
                    break;
                }
            }
          
        }
    }
    const rcsc::PlayerObject * nearest_self_opp = wm.getOpponentNearestToSelf(10);
    if(nearest_self_opp)
    {
        mark_number = (nearest_self_opp)->unum();
    }
    const rcsc::PlayerPtrCont::const_iterator opp_end = wm.opponentsFromBall().end();
    for(rcsc::PlayerPtrCont::const_iterator opp = wm.opponentsFromBall().begin();
        opp != opp_end;
        opp++)
    {
        if((((*opp)->pos().x < wm.ball().pos().x + 10.0 
             && wm.ball().pos().x >= rcsc::ServerParam::i().ourPenaltyAreaLineX())
            || wm.ball().pos().x < rcsc::ServerParam::i().ourPenaltyAreaLineX())
           && (*opp)->distFromBall() <= max_pass_dist
           && (*opp)->distFromSelf() <= max_mark_dist)

        {
            mark_number = (*opp)->unum();
            return mark_number;
        }
    }
    return mark_number;
}
*/
rcsc::Vector2D
Bhv_ObakeMark::getMarkPosition(rcsc::PlayerAgent * agent,
                               const int mark_number,
                               const bool offensive)
{
    const rcsc::WorldModel & wm = agent->world();
    int number;
    number = wm.self().unum();
    rcsc::Vector2D target_pos = M_home_pos;
    const rcsc::AbstractPlayerObject * mark_target = wm.opponent(mark_number);
    if(mark_target)
    {
        target_pos = (mark_target)->pos();
        /*if(wm.self().unum() == 6)
        {
            std::cout<<"target"<<target_pos;
            }*/
        const double decay = rcsc::ServerParam::i().defaultPlayerDecay();
        double block_dist = 2.0;
        if(M_role_side_or_center_back 
           && mark_target->pos().x > rcsc::ServerParam::i().ourPenaltyAreaLineX())
        {
            block_dist = 3.5;
        }
        if(wm.ball().pos().x < rcsc::ServerParam::i().ourPenaltyAreaLineX())
        {
            block_dist = 3.0;
        }
        rcsc::Vector2D block_vector(-block_dist, 0.0), v;
        if((mark_target)->velCount() < 3)
        {
            v = (mark_target)->vel();
            v.setLength((1-std::pow(decay, (mark_target)->velCount()+1)) / (1 - decay));
            target_pos += v;
        }
        /*if(wm.self().unum() == 6)
        {
            std::cout<<", + vel"<<target_pos;
            }*/
        if(offensive)
        {
            block_dist = 4.0;
            block_vector = wm.ball().pos() - target_pos;
            block_vector.setLength(block_dist);
        }
        target_pos += block_vector;
        /*if(wm.self().unum() == 6)
        {
            std::cout<<"mark_pos"<<target_pos<<std::endl;
            }*/
    }
    return target_pos;
}









