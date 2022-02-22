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

#include <rcsc/geom/triangle_2d.h>

#include <rcsc/action/basic_actions.h>
#include <rcsc/action/body_pass.h>
#include <rcsc/action/neck_scan_field.h>
#include <rcsc/action/neck_turn_to_low_conf_teammate.h>

#include "obake_fuzzy_grade.h"
#include "obake_analysis.h"
#include "obake_strategy.h"
#include "body_obake_pass.h"
#include "bhv_obake_pass.h"

bool
Bhv_ObakePass::execute(rcsc::PlayerAgent * agent)
{
    rcsc::dlog.addText( rcsc::Logger::TEAM,
                        "%s:%d: Bhv_ObakePass"
                        ,__FILE__, __LINE__ );
    const rcsc::WorldModel & wm = agent->world();
    /*test*/
/*    rcsc::Triangle2D test = Obake_Analysis().getTriangle(wm.self().pos(),
                                        rcsc::Vector2D(wm.self().pos().x+10.0, wm.self().pos().y),
                                        90.0);
    std::cout<<"self = "<<wm.self().pos()<<std::endl;
    std::cout<<"triangle = "<<test.a()<<","<<test.b()<<","<<test.c()<<std::endl;
*/
    /**********************/
    

    /**/
    const double goal_half_width = rcsc::ServerParam::i().goalWidth() / 2;
    const double decay = rcsc::ServerParam::i().defaultPlayerDecay();
    const double angle = 35.0;
    rcsc::Vector2D pass_point;
    const rcsc::PlayerObject * opp_goalie = wm.getOpponentGoalie();
    const rcsc::PlayerPtrCont & opps = wm.opponentsFromSelf();
    double score;
    bool can_assist, can_shoot;
    if(Body_ObakePass::get_best_pass(agent, &pass_point, NULL, NULL, &score, &can_shoot, &can_assist))
    {   
        if(Obake_Analysis().checkExistOppPenaltyAreaIn(pass_point)
           && wm.ball().pos().x >= rcsc::ServerParam::i().pitchHalfLength() 
           - rcsc::ServerParam::i().goalAreaLength())
        {
            rcsc::dlog.addText( rcsc::Logger::TEAM,
                                "%s:%d: do obake pass"
                                ,__FILE__, __LINE__ );
            agent->debugClient().addMessage( "OffKickPass(1)" );
            Body_ObakePass().execute(agent);
            agent->setNeckAction(new rcsc::Neck_TurnToLowConfTeammate());
            return true;
        }
/*
        if(Obake_Analysis().checkExistOppPenaltyAreaIn(pass_point)
           && wm.ball().pos().absY() >= rcsc::ServerParam::i().penaltyAreaHalfWidth())
        {
            Body_ObakePass().execute(agent);
            agent->setNeckAction(new rcsc::Neck_TurnToLowConfTeammate());
            return true;
        }
*/
        if(pass_point.x > wm.offsideLineX() - 5.0
           && pass_point.absY() <= rcsc::ServerParam::i().goalAreaHalfWidth())
        {
            Body_ObakePass().execute(agent);
            agent->setNeckAction(new rcsc::Neck_TurnToLowConfTeammate());
            return true;
        }
        if(pass_point.x > wm.offsideLineX() - 3.5
           && pass_point.absY() >= rcsc::ServerParam::i().pitchHalfWidth() - 8.0
           && wm.ball().pos().x <= wm.offsideLineX() - 5.0)
        {
            Body_ObakePass().execute(agent);
            agent->setNeckAction(new rcsc::Neck_TurnToLowConfTeammate());
            return true;
        }
        if((can_shoot || can_assist)
           && !(wm.ball().pos().x >= rcsc::ServerParam::i().theirPenaltyAreaLineX()
                && wm.ball().pos().absY() <= rcsc::ServerParam::i().goalAreaHalfWidth()))
        {
            Body_ObakePass().execute(agent);
            agent->setNeckAction(new rcsc::Neck_TurnToLowConfTeammate());
            return true;
        }
        if(/*Obake_Analysis().checkExistOppPenaltyAreaIn(pass_point)
           || (pass_point.absY() <= wm.ball().pos().absY()
               && pass_point.x >= wm.ball().pos().x)
           || (wm.ball().pos().x < rcsc::ServerParam::i().theirPenaltyAreaLineX()
           && pass_point.x > wm.offsideLineX())*/
            (((pass_point.x >= wm.self().pos().x - 1.0 
                  && wm.self().pos().x < wm.offsideLineX() - 7.0)
                 || pass_point.x >= wm.self().pos().x + 3.0
              || (pass_point.x > wm.self().pos().x
                  && Obake_Analysis().checkExistOurPenaltyAreaIn(wm.self().pos())))
              && !(std::abs(wm.offsideLineX() - wm.self().pos().x) <= 6.5
                     && pass_point.x <= (wm.self().pos().x + 3.5)
                     && wm.self().pos().absY() <= 12.0))
            || ObakeStrategy::getArea(pass_point) == ObakeStrategy::ShootChance
            || (pass_point.x >= rcsc::ServerParam::i().theirPenaltyAreaLineX() - 5.0
                && pass_point.y <= (rcsc::ServerParam::i().penaltyAreaHalfWidth() 
                                   + rcsc::ServerParam::i().goalAreaHalfWidth()) / 2.0
                && (pass_point.absY() <= wm.self().pos().absY()
                   || pass_point.absY() <= goal_half_width)))
        {
            bool safety = true;
            double dist = 4.0; 
            const double length = 12.0;
            const double width = rcsc::ServerParam::i().goalAreaHalfWidth();
            rcsc::Vector2D left_top((rcsc::ServerParam::i().pitchHalfLength() - length),
                                    -width);
            const int opp_number = Obake_Analysis().getOppNumber(agent,
                                                                 left_top,
                                                                 length,
                                                                 width);
            const double r = 2.5;
            if(Obake_Analysis().checkExistShootCourse(agent,
                                                      pass_point,
                                                      angle,
                                                      r)
               && pass_point.x >= rcsc::ServerParam::i().theirPenaltyAreaLineX())
            {
                dist -= 1.5;
            }
            if(can_shoot)
            {
                dist = 1.5;
                const double top_range = (rcsc::ServerParam::i().goalAreaHalfWidth() 
                                          + rcsc::ServerParam::i().penaltyAreaHalfWidth()) / 2;
                const rcsc::Vector2D check_area_left_top(wm.ball().pos().x, -rcsc::ServerParam::i().goalAreaHalfWidth());
                const double check_area_length = std::abs(rcsc::ServerParam::i().pitchHalfLength()-wm.ball().pos().x);
                const double check_area_width = rcsc::ServerParam::i().goalAreaWidth();
                const int opp_number_in_check_area = Obake_Analysis().getOppNumber(agent,
                                                                                   check_area_left_top,
                                                                                   check_area_length,
                                                                                   check_area_width);
                const double angle = 30.0;
                if(opp_number_in_check_area >= 4
                   /*|| checkExistShootCourse(agent,
                                             pass_point,
                                             angle)*/)
                {
                    dist -= 0.3;
//                    std::cout<<"ShootChance:numberStrategy"<<std::endl;
                }
                else if(wm.ball().pos().absY() >= top_range)
                {
                    dist -= 0.3;
                }
            }   
            else if(pass_point.x > wm.offsideLineX() - 5.0
                    && pass_point.absY() <= rcsc::ServerParam::i().penaltyAreaHalfWidth()/*15.0*/
                    && evaluatePassPoint(agent, pass_point, angle))
            {
		dist = (pass_point.absY() < wm.self().pos().absY()) ? 2.0 : 2.5;
	    }
            else if(wm.self().pos().x <= 0.0
                    && pass_point.absY() <= 12.0
                    && pass_point.x >= wm.offsideLineX() - 12.5)
            {
                dist = 2.7;
            }

            if(wm.self().pos().x > rcsc::ServerParam::i().theirPenaltyAreaLineX())
            {

                if(opp_goalie)
                {
                    double opp_y = opp_goalie->pos().y;
                    if(opp_goalie->velCount() < 3)
                    {
                        opp_y += opp_goalie->vel().y * (1-std::pow(decay, opp_goalie->velCount()+1) / (1 - decay));
                    }
                    if(pass_point.y * opp_y <= 0.0)
                    {
                       dist -= 0.3;
                    }
                }
            }                
            const int mate_number = Obake_Analysis().getMateNumber(agent,
                                                                   pass_point);
//            std::cout<<"is_shoot_chance = "<<is_shoot_chance<<std::endl;
            if(!can_shoot && !can_assist)
            {
                if(checkDribbleIsBetter(agent,
                                        pass_point))
                {
                    return false;
                }
            }
            if(mate_number >= 1 
               && mate_number <= 11
               && !can_shoot
               && wm.offsideLineX() >= rcsc::ServerParam::i().theirPenaltyAreaLineX())
            {
                const rcsc::AbstractPlayerObject * mate = wm.teammate(mate_number);
                if(mate)
                {
                    const double pass_angle = 30.0;
                    const double max_pass_dist = 25.0;
//                    std::cout<<"self = "<<wm.self().unum();
                    if(can_assist)
                    {
                        dist = 2.0;
                    }
                }
            }

            
            const rcsc::PlayerPtrCont::const_iterator opps_end = opps.end();
            for(rcsc::PlayerPtrCont::const_iterator it = opps.begin();
                it != opps_end;
                ++it)
            {
                if((*it)->pos().dist(pass_point) < dist 
                   /*|| !evaluatePassPoint(agent, pass_point, angle)*/)
                {
                    safety = false;
                    break;
                }
            }
            if(safety)
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                                    "%s:%d: do obake pass"
                                    ,__FILE__, __LINE__ );
                agent->debugClient().addMessage( "OffKickPass(1)" );
                Body_ObakePass().execute(agent);
                agent->setNeckAction(new rcsc::Neck_TurnToLowConfTeammate());
                return true;
            }
        }
    }
    return false;
}


bool
Bhv_ObakePass::evaluatePassPoint(rcsc::PlayerAgent * agent,
                                 const rcsc::Vector2D &pass_point,
                                 const double &angle)
{
    const rcsc::WorldModel & wm = agent->world();    
    const double penalty_area_half_width = rcsc::ServerParam::i().penaltyAreaWidth() / 2;
    const double goal_ara_half_width = rcsc::ServerParam::i().goalAreaWidth() / 2;
    if((wm.ball().pos().absY() <= penalty_area_half_width 
        &&  pass_point.absY() > penalty_area_half_width)
       || (wm.ball().pos().absY() <= goal_ara_half_width 
            && pass_point.y * wm.ball().pos().y > 0.0
            && pass_point.absY() > goal_ara_half_width))
    {
        return false;
    }
    const rcsc::Vector2D front_opp_goal(rcsc::ServerParam::i().pitchHalfLength() - 9.5, 0.0);
    const double half_angle = angle / 2;
    rcsc::Vector2D vector_goal = front_opp_goal - pass_point;
    double length, tmp;
    length = 15.0;
    vector_goal.setLength(length);
    tmp = std::abs(length * std::tan(half_angle * 3.14 / 180));
    const rcsc::Vector2D first_apex = pass_point;
    rcsc::Vector2D second_apex  = pass_point + vector_goal;
    rcsc::Vector2D third_apex = pass_point + vector_goal;
    rcsc::Vector2D v = vector_goal;
    v.setLength(tmp);
    v.rotate(90.0);
    second_apex += v;
    v.rotate(180.0);
    third_apex += v;
    const rcsc::Triangle2D dribble_area(first_apex, second_apex, third_apex);
    if(!wm.existOpponentIn(dribble_area, 10, true))
    {
      return true;
    }
    return false;   
}


bool
Bhv_ObakePass::checkDribbleIsBetter(rcsc::PlayerAgent * agent,
                                    const rcsc::Vector2D &pass_point)
{
    const rcsc::WorldModel & wm = agent->world();
    const rcsc::Vector2D self_next_pos = wm.self().pos() + wm.self().vel();
    const rcsc::Vector2D ball_next_pos = wm.ball().pos() + wm.ball().vel();
    const double r = 4.0;
    const int opp_number = Obake_Analysis().getOppNumber(agent,
                                                         self_next_pos,
                                                         r);
    if(Obake_Analysis().checkExistOppPenaltyAreaIn(ball_next_pos)
        && !Obake_Analysis().checkExistOppPenaltyAreaIn(self_next_pos))
    {
        return false;
    }
    if(Obake_Analysis().checkExistOppPenaltyAreaIn(self_next_pos)
       && (pass_point.absY() > rcsc::ServerParam::i().penaltyAreaHalfWidth()
           || pass_point.x <= rcsc::ServerParam::i().theirPenaltyAreaLineX() - 7.0)
       && opp_number < 3)
    {
        return true;
    }
    if(ObakeStrategy().getArea(wm.self().pos()) == ObakeStrategy::ShootChance
       && !Obake_Analysis().checkExistOppPenaltyAreaIn(pass_point))
    {
        return true;
    }
    

    if((Obake_Analysis().checkExistOppPenaltyAreaIn(self_next_pos)
        || (self_next_pos.x > wm.offsideLineX() - 8.0))
       && (pass_point.x < self_next_pos.x
           || !Obake_Analysis().checkExistOppPenaltyAreaIn(pass_point)))
    {
        const double dribble_angle = 30.0;
        const rcsc::Vector2D front_opp_goal(rcsc::ServerParam::i().pitchHalfLength() - 9.5,
                                            0.0);
       rcsc::Vector2D near_centering_point(rcsc::ServerParam::i().pitchHalfLength()
                                           - rcsc::ServerParam::i().goalAreaLength(),
                                           rcsc::ServerParam::i().goalAreaHalfWidth());
       rcsc::Vector2D far_centering_point(rcsc::ServerParam::i().pitchHalfLength(), 
                                          (rcsc::ServerParam::i().goalAreaHalfWidth()
                                           + rcsc::ServerParam::i().penaltyAreaHalfWidth()) / 2.0);
       if(self_next_pos.y < 0.0)
       {
           near_centering_point.y *= -1.0;
           far_centering_point.y *= -1.0;
       }
       rcsc::Vector2D additional_vector;
       std::vector<rcsc::Vector2D> strategic_additional_vector_vector;
       additional_vector = front_opp_goal - self_next_pos;
       strategic_additional_vector_vector.push_back(additional_vector);
       additional_vector = near_centering_point - self_next_pos;
       strategic_additional_vector_vector.push_back(additional_vector);
       additional_vector = far_centering_point - self_next_pos;
       strategic_additional_vector_vector.push_back(additional_vector);
       additional_vector.y = 0.0;
       additional_vector.x = 4.0;
       strategic_additional_vector_vector.push_back(additional_vector);
       rcsc::Vector2D target_point;
       const std::vector<rcsc::Vector2D>::const_iterator
           v_end = strategic_additional_vector_vector.end();
       for(std::vector<rcsc::Vector2D>::const_iterator
               v = strategic_additional_vector_vector.begin();
           v != v_end;
           v++)
       {
           additional_vector = (*v); 
           additional_vector.setLength(4.0);
           target_point = ball_next_pos + additional_vector;
           if(!Obake_Analysis().checkExistOpponent(agent,
                                                  ball_next_pos,
                                                  target_point,
                                                   dribble_angle))
           {
               return true;
           }
       }
       

    }
    if(self_next_pos.x >= rcsc::ServerParam::i().theirPenaltyAreaLineX())
    {
       const double r = 3.5;//4.5;
       const rcsc::Vector2D center(self_next_pos.x + 1.5,
                                    self_next_pos.y);
        rcsc::Circle2D circle(center, r);
        if(wm.existOpponentIn(circle, 10, true))
        {
            return false;
        }
        const rcsc::PlayerPtrCont::const_iterator end = wm.teammatesFromSelf().end();
        for(rcsc::PlayerPtrCont::const_iterator mate = wm.teammatesFromSelf().begin();
            mate != end;
            ++mate)
        {
            if(Obake_Analysis().checkExistOppPenaltyAreaIn((*mate)->pos()))
            {
                if(self_next_pos.absY() > rcsc::ServerParam::i().goalAreaHalfWidth())
                {
                    rcsc::Circle2D receiver_circle(rcsc::Vector2D ((*mate)->pos().x,(*mate)->pos().y), 
                                                   r);
                    if(!wm.existOpponentIn(receiver_circle, 10, true)
                       && (*mate)->pos().absY() < self_next_pos.absY())
                    {
                        return false;
                    }
                }
            }
        }
    }
    else
    {
        const double r = 4.0;//4.5;
        const rcsc::Vector2D center(self_next_pos.x + 1.5,
                                    self_next_pos.y);
        rcsc::Circle2D circle(center, r);
        const double degree_near_penalty_area = Obake_FuzzyGrade().degreeNearOppPenaltyAreaX(self_next_pos.x);
        const double degree_far_penalty_area = Obake_FuzzyGrade().degreeFarOppPenaltyAreaX(self_next_pos.x);
        const double sum = degree_near_penalty_area + degree_far_penalty_area;
        const double base_near_r = 5.0;//6.0;
        const double base_far_r = 10.0;
        const double offense_r = (base_near_r * degree_near_penalty_area
                                  + base_far_r * degree_far_penalty_area)
            / sum;
        
        if(wm.existOpponentIn(circle, 10, true)
           || Obake_Analysis().getOppNumber(agent,
                                            self_next_pos,
                                            offense_r) >= 2)
        {
            return false;
        }
        const rcsc::Vector2D pass_point_center(pass_point.x + 1.5,
                                               pass_point.y);
        const rcsc::Circle2D pass_point_circle(pass_point_center, r);
        if(pass_point.x >= self_next_pos.x  + 15.0
           && !Obake_Analysis().checkExistOppPenaltyAreaIn(self_next_pos)
           && !wm.existOpponentIn(pass_point_circle, 10, true))
        {
            return false;
        }
        const rcsc::PlayerPtrCont::const_iterator end = wm.teammatesFromSelf().end();
        for(rcsc::PlayerPtrCont::const_iterator mate = wm.teammatesFromSelf().begin();
            mate != end;
            ++mate)
        {
            if((*mate)->pos().x >= wm.self().pos().x 
                && self_next_pos.absY() <= (*mate)->pos().absY())
            {
                if(self_next_pos.absY() > rcsc::ServerParam::i().goalAreaHalfWidth())
                {
                    rcsc::Circle2D receiver_circle(rcsc::Vector2D ((*mate)->pos().x,(*mate)->pos().y), 
                                                   r-0.5);
                    if(!wm.existOpponentIn(receiver_circle, 10, true)
                       && (*mate)->pos().absY() < self_next_pos.absY())
                    {
                        return false;
                    }
                }
            }
        }

    }
    return true;
}




