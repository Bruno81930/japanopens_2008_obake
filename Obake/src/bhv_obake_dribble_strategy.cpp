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

#include <rcsc/player/logger.h>
#include <rcsc/common/server_param.h>
#include <rcsc/geom/angle_deg.h>
#include <rcsc/geom/vector_2d.h>
#include <rcsc/player/player_agent.h>
#include <rcsc/player/action_effector.h>
#include <rcsc/action/body_dribble.h>

#include "obake_fuzzy_grade.h"

#include "bhv_obake_action_strategy.h"

bool
Bhv_ObakeActionStrategy::execute(rcsc::PlayerAgent * agent)
{
    rcsc::dlog.addText( rcsc::Logger::TEAM,
                        "%s:%d: Bhv_ObakeActionStrategy"
                        ,__FILE__, __LINE__ );
    const rcsc::WorldModel & wm = agent->world();
    const rcsc::Vector2D front_opp_goal(rcsc::ServerParam::i().pitchHalfLength() 
                                        -rcsc::ServerParam::i().goalAreaLength()/*- 9.5*/,
                                        0.0);
    const rcsc::Vector2D front_top_opp_goal(rcsc::ServerParam::i().pitchHalfLength()
                                            -rcsc::ServerParam::i().goalAreaLength(),//- 9.5,
                                            -rcsc::ServerParam::i().goalHalfWidth());
    const rcsc::Vector2D front_bottom_opp_goal(rcsc::ServerParam::i().pitchHalfLength()
                                               -rcsc::ServerParam::i().goalAreaLength(),//- 9.5,
                                               rcsc::ServerParam::i().goalHalfWidth());
    const rcsc::Vector2D first_apex = wm.self().pos();
    double angle = 35.0;
    double dash_power;
    rcsc::Vector2D vector_goal = front_opp_goal - wm.self().pos();
    rcsc::Vector2D dribble_target, second_apex, third_apex;
    const double near_rate = Obake_FuzzyGrade().degreeNearOppGoalX(wm.ball().pos().x);
    const double far_rate = Obake_FuzzyGrade().degreeFarOppGoalX(wm.ball().pos().x);
    const double near_base_dist = 1.5;//2.0;
    const double far_base_dist = 4.0;
    double dist = (near_rate * near_base_dist + far_rate * far_base_dist)
                        / (near_rate + far_rate);
    double front = 5.0;
    bool dribble_area = false;
    dribble_target = front_opp_goal;
    /******/
    if(wm.ball().pos().x >= rcsc::ServerParam::i().theirPenaltyAreaLine())
    {
/*
        std::vector<double> score_target_vector;
        const std::vector<rcsc::Vector2D> target_point_vector = getTargetPoint(agent, score_target_vector);
        const std::vector<double> score_target_vector = getTargetPointScore(agent,
                                                                            target_point_vector);
*/
/*
        const rcsc::Vector2D target_point = getBestTargetPoint(target_point_vector,
                                                               score_target_vector);
*/


/*
        const rcsc::Vector2D target_point = getBestTargetPoint(agent);
        const double dash_power = getDashPower(agent, target_point);
        rcsc::Body_Dribble(target_point,
                           1.0,
                           dash_power,
                           1
            ).execute(agent);
*/
    }
    /********/
    if(wm.self().pos().x >= rcsc::ServerParam::i().theirPenaltyAreaLine()
       ||
       (wm.self().pos().x >= 30.0 && wm.self().pos().absY() >= rcsc::ServerParam::i().penaltyAreaHalfWidth())
        || ((wm.self().pos().x >= rcsc::ServerParam::i().theirPenaltyAreaLine() - 16.0
	     && wm.self().pos().x >= (wm.offsideLineX() -  7.0))
       && wm.self().pos().x < front_opp_goal.x
	     && wm.self().pos().absY() <= rcsc::ServerParam::i().penaltyAreaHalfWidth()))
    {
        while(front >= 3.0)
        {
            
            if(wm.self().pos().y <= 0
               && checkDribbleArea(agent,
                                   front_top_opp_goal,
                                   dist,
                                   2.0,
                                   angle))
            {
                dribble_area = true;
                dribble_target = front_top_opp_goal;
            }
            else if(wm.self().pos().y >= 0
                    && checkDribbleArea(agent,
                                        front_bottom_opp_goal,
                                        dist,
                                        2.0,
                                        angle))
            {
                dribble_area = true;
                dribble_target = front_bottom_opp_goal;
            }
            else if(checkDribbleArea(agent,
                                     front_opp_goal,
                                     dist,
                                     2.0,
                                     angle))
            {
                dribble_area = true;
            }
            front -= 1.0;
        }
        if(dribble_area
           && dribble_target.x < rcsc::ServerParam::i().pitchHalfLength() - 1.0
           && dribble_target.absY() < rcsc::ServerParam::i().pitchHalfWidth() - 1.0)
        {
            dash_power = getDashPower(agent, dribble_target);
            rcsc::dlog.addText( rcsc::Logger::TEAM,
                                "%s:%d: dribble to (%.1f, %.1f) dash power=%.1f"
                                ,__FILE__, __LINE__,
                                dribble_target.x, dribble_target.y,
                                dash_power);
            rcsc::Body_Dribble(dribble_target,
                               1.0,
                               dash_power,
                               1
                ).execute(agent);
            return true;
        }
        
    }
    
    if(wm.self().pos().x >= wm.offsideLineX() - 10.0
/*       && (wm.self().pos().x <= (rcsc::ServerParam::i().theirPenaltyAreaLine() + 11.0))
         && wm.self().pos().absY() <= penalty_area_half_width*/)
    {        
        dribble_target = wm.self().pos();
        dribble_target.x += 3.0;
        dist = 3.0;
        front = 2.0;
        if(checkDribbleArea(agent,
                            dribble_target,
                            dist,
                            front,
                            angle)
           && dribble_target.x < rcsc::ServerParam::i().pitchHalfLength() - 1.0
           && dribble_target.absY() < rcsc::ServerParam::i().pitchHalfWidth() - 1.0)
        {
            dash_power = getDashPower(agent, dribble_target);
            rcsc::dlog.addText( rcsc::Logger::TEAM,
                                "%s:%d: dribble to (%.1f, %.1f) dash power=%.1f"
                                ,__FILE__, __LINE__,
                                dribble_target.x, dribble_target.y,
                                dash_power);
            rcsc::Body_Dribble(dribble_target,
                               1.0,
                               dash_power,
                               1
                ).execute(agent);
            return true;
        }
    }

    if(wm.self().pos().x >front_opp_goal.x
       && wm.self().pos().absY() <= rcsc::ServerParam::i().goalHalfWidth())
    {
        dribble_target.x = 1.5;
        dist = 1.5;
        front = 1.5;
        angle = 30.0;
        const double differecnce = - wm.self().pos().y / wm.self().pos().absY() * 4.0;
        while(dribble_target.absY() <= rcsc::ServerParam::i().goalHalfWidth())
        {

            if(checkDribbleArea(agent,
                                dribble_target,
                                dist,
                                front,
                                angle)
               && dribble_target.x < rcsc::ServerParam::i().pitchHalfLength() - 1.0
               && dribble_target.absY() < rcsc::ServerParam::i().pitchHalfWidth() - 1.0)
            {
                dash_power = getDashPower(agent, dribble_target);
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                                    "%s:%d: dribble to (%.1f, %.1f) dash power=%.1f"
                                    ,__FILE__, __LINE__,
                                    dribble_target.x, dribble_target.y,
                                    dash_power);
                rcsc::Body_Dribble(dribble_target,
                                   1.0,
                                   dash_power,
                                   1
                    ).execute(agent);
                return true;
            }
            dribble_target.y += differecnce;
        }
    }

    if(wm.self().pos().x >= rcsc::ServerParam::i().theirPenaltyAreaLine()
       && wm.self().pos().absY() <= rcsc::ServerParam::i().penaltyAreaHalfWidth())
    {
        dribble_target = wm.self().pos();
        dribble_target.y = 1.5;
        dist = 1.5;
        front = 1.5;
        angle = 30.0;
        const double differecnce = 1.0;
        int i;
        const int max_size = 4;
        const int max_count = 2;
        int count;
        for(i=0; i<max_size; i++)
        {
            count = 0;
            while(count < max_count)
            {
                rcsc::Vector2D new_dribble_target = dribble_target;
                if(count== 0)
                {
                    dribble_target.x += differecnce * i;
                }
                else
                {
                    dribble_target.x -= differecnce * i * 0.5;
                }
                if(checkDribbleArea(agent,
                                    new_dribble_target,
                                    dist,
                                    front,
                                    angle)
                   && dribble_target.x < rcsc::ServerParam::i().pitchHalfLength() - 1.0
                   && dribble_target.absY() < rcsc::ServerParam::i().pitchHalfWidth() - 1.0)
                {
                    dash_power = getDashPower(agent, dribble_target);
                    rcsc::dlog.addText( rcsc::Logger::TEAM,
                                        "%s:%d: dribble to (%.1f, %.1f) dash power=%.1f"
                                        ,__FILE__, __LINE__,
                                        dribble_target.x, dribble_target.y,
                                        dash_power);
                    rcsc::Body_Dribble(dribble_target,
                                       1.0,
                                       dash_power,
                                       1
                        ).execute(agent);
                    std::cout<<"side dribble"<<std::endl;
                    return true;
                }
                count++;
            }
        }
    }
    return false;
}

double
Bhv_ObakeActionStrategy::getDashPower(rcsc::PlayerAgent * agent,
                                       const rcsc::Vector2D &target_point)
{
    const rcsc::WorldModel & wm = agent->world();
    const rcsc::AngleDeg target_angle = (target_point - wm.self().pos()).th();
    const rcsc::PlayerObject * nearest_self_opp = wm.getOpponentNearestToSelf(10);
    const double decay = rcsc::ServerParam::i().ballDecay();
    const rcsc::PlayerPtrCont::const_iterator end = wm.opponentsFromSelf().end();
    const double dribble_first_rate = 1.0;
    const double dribble_slow_rate = 0.10;
    double first_rate = Obake_FuzzyGrade().degreeNearOffsideLine(agent, wm.offsideLineX());;//dribble first
    double second_rate = 0.0;//dribble first
    double third_rate = 0.0;//dribble slow
    double dist;
    rcsc::Vector2D base_pos, v;
    double dist_max = 100.0;
    for(rcsc::PlayerPtrCont::const_iterator opp = wm.opponentsFromSelf().begin();
        opp != end;
        opp++)
    {
        if(dist_max < (*opp)->distFromSelf())
        {
            break;
        }
        base_pos = (*opp)->pos();
        if((*opp)->velCount() < 3)
        {
            v = (*opp)->vel();
            v.setLength((1-std::pow(decay, ((*opp)->velCount()+1))) / (1 - decay));
            base_pos += v;
        }
        v = base_pos - wm.self().pos();
        v.rotate(-target_angle.degree());
        if(v.x > 0.0)
        {
            dist = base_pos.dist(wm.self().pos());
            if(first_rate > (1 - Obake_FuzzyGrade().degreeExistNearestOpp(dist))) 
            {
                first_rate = 1 - Obake_FuzzyGrade().degreeExistNearestOpp(dist);
            }
            const double vel_count_max = 3;
            dist_max = dist + vel_count_max * rcsc::ServerParam::i().defaultPlayerSpeedMax();
        }
        
    }
    if(nearest_self_opp)
    {
        dist = (nearest_self_opp)->distFromSelf();
        second_rate = Obake_FuzzyGrade().degreeExistNearestOpp(dist);
        base_pos = (nearest_self_opp)->pos();
        if((nearest_self_opp)->velCount() < 3)
        {
            v = (nearest_self_opp)->vel();
            v.setLength((1-std::pow(decay, ((nearest_self_opp)->velCount()+1))) / (1 - decay));
            base_pos += v;
        }
        v = base_pos - wm.self().pos();
        v.rotate(-target_angle.degree());
        if(v.x <= 0.0)
        {
            second_rate = 0.0;
        }  
        if(v.x > 0.0)
        {
            if(std::abs(v.absX()) <= (v.absY() - rcsc::ServerParam::i().tackleDist()))
            {
                third_rate = Obake_FuzzyGrade().degreeExistNearestOpp(dist);
            }
        }
    }
    const double degree_near_goal_x = Obake_FuzzyGrade().degreeNearOppGoalX(wm.ball().pos().x);
    const double degree_near_goal_y = Obake_FuzzyGrade().degreeNearOppGoalY(wm.ball().pos().y);
    third_rate = std::max(third_rate, std::min(degree_near_goal_x, degree_near_goal_y));
    const double rate_max = std::max(first_rate, second_rate);
    const double sum = (rate_max + third_rate);
    double dash_power = rcsc::ServerParam::i().maxPower();
    if(sum != 0.0)
    {
        const double rate = (rate_max * dribble_first_rate 
                             + third_rate * dribble_slow_rate)
                             / sum;
        dash_power *= rate;
    }
    return dash_power;
}

bool
Bhv_ObakeActionStrategy::checkDribbleArea(rcsc::PlayerAgent * agent,
                                           const rcsc::Vector2D &dribble_target,
                                           const double &dist,
                                           const double &front,
                                           const double &angle)
{
    const rcsc::WorldModel & wm = agent->world();
    const double half_angle = angle / 2;
    rcsc::Vector2D vector_target = dribble_target - wm.self().pos();
    double tmp;
    vector_target.setLength(dist);
    rcsc::Vector2D vertical_vector = vector_target;
    vector_target.setLength(dist + front);
    rcsc::Vector2D first_apex = wm.self().pos();
    rcsc::Vector2D second_apex  = wm.self().pos() + vector_target;
    rcsc::Vector2D third_apex = wm.self().pos() + vector_target;
    tmp = (dist + front) * std::abs(std::tan(half_angle * 3.14 / 180));
    vertical_vector.setLength(tmp);
    vertical_vector.rotate(90.0);
    second_apex += vertical_vector;
    vector_target.rotate(180.0);
    third_apex += vertical_vector;
    const rcsc::Triangle2D first_dribble_area(first_apex, second_apex, third_apex);
    if(!wm.existOpponentIn(first_dribble_area, 10, true))
    {
        return true;
    }
    return false;
}

bool
Bhv_ObakeActionStrategy::checkAvailableTatgetPoint(rcsc::PlayerAgent * agent,
                                                    const rcsc::Vector2D &target_point)
{

    return true;
}

bool
Bhv_ObakeActionStrategy::checkAvoidanceFromNearestOpp(rcsc::PlayerAgent * agent,
						       const rcsc::Vector2D &target_pos)
{
    const rcsc::WorldModel &wm =agent->world();
    const rcsc::PlayerObject * nearest_from_ball = wm.getOpponentNearestToBall(10);
    if(nearest_from_ball)
    {
	const rcsc::Vector2D nearest_next_pos = (nearest_from_ball)->pos() + (nearest_from_ball)->vel();
	const rcsc::Vector2D ball_next_pos = wm.ball().pos() + wm.ball().vel();
	const double dist_from_next_opp_to_ball = nearest_next_pos.dist(ball_next_pos);
	const double dist_from_next_opp_to_target = nearest_next_pos.dist(target_pos);
/*	std::cout<<"opp "<<nearest_next_pos<<std::endl;
	std::cout<<"ball"<<ball_next_pos<<std::endl;
	std::cout<<"opp to ball = "<<dist_from_next_opp_to_ball<<std::endl;
	std::cout<<"opp to target = "<<dist_from_next_opp_to_target<<std::endl;
*/
	if(dist_from_next_opp_to_ball < dist_from_next_opp_to_target)
	{
	    return true;
	}
    }
    return false;
}

bool
Bhv_ObakeActionStrategy::checkBestAction(rcsc::PlayerAgent * agent,
                                          const rcsc::Vector2D &target_point,
                                          double &score,
                                          const double &max_score)
{
    
}

/*
bool
Bhv_ObakeActionStrategy::checkExistShootCourse(rcsc::PlayerAgent * agent)
{

}
*/
bool
Bhv_ObakeActionStrategy::checkDribbleToBodyDir()
{


}


bool
Bhv_ObakeActionStrategy::checkDribbleToNearGoalX(rcsc::PlayerAgent * agent,
						 const double &target_pos_x)
{
    const rcsc::WorldModel &wm = agent->world();
    const double ball_next_pos_x = wm.ball().pos().x + wm.ball().vel().x;
    if(target_pos_x > ball_next_pos_x)
    {
	return true;
    }
    return false;
}

bool
Bhv_ObakeActionStrategy::checkDribbleToNearGoalY(rcsc::PlayerAgent * agent,
						  const double &target_pos_y)
{
    const rcsc::WorldModel & wm = agent->world();
    const double ball_next_pos_y = wm.ball().pos().y + wm.ball().vel().y;
    if(std::abs(target_pos_y) < std::abs(ball_next_pos_y))    
    {
	return true;
    }
    return false;
}

double 
Bhv_ObakeActionStrategy::getPositionMultiplier(const rcsc::Vector2D &target_point)
{
    double multiplier;
    return multiplier;
}


std::vector<rcsc::Vector2D>
Bhv_ObakeActionStrategy::getTargetPointVector(rcsc::PlayerAgent * agent)
{
    const rcsc::WorldModel & wm = agent->world();
    const rcsc::Vector2D self_next_pos = wm.self().pos() + wm.self().vel();
    const double base_near_dribble_dist = 3.0;
    const double base_far_dribble_dist = 5.0;
    const double degree_near_penalty_area_x = Obake_FuzzyGrade().degreeNearOppPenaltyAreaX(self_next_pos.x);
    const double degree_far_penalty_area_x =  Obake_FuzzyGrade().degreeFarOppPenaltyAreaX(self_next_pos.x);
    const double sum = degree_near_penalty_area_x + degree_far_penalty_area_x;
    const double dribble_dist = (base_near_dribble_dist * degree_near_penalty_area_x
                                 + base_far_dribble_dist * degree_far_penalty_area_x)
        / sum;
    const int dir_size = 8;
    const double angle = 360.0 / dir_size;
    const double safe_dist = 2.5;
 
   std::vector<rcsc::Vector2D> target_point_vector;
   rcsc::Vector2D target_point, additional_vector;
   int i;

   additional_vector = rcsc::Vector2D::polar2vector(dribble_dist, wm.self().body());
   for(i=0; i<dir_size; i++)
   {
       target_point = self_next_pos + additional_vector;

       /* change the range of the vector into a range
          that is adjusted the range of the pitch*/
       if(target_point.x > rcsc::ServerParam::i().pitchHalfWidth() - safe_dist)
       {
           target_point.x = rcsc::ServerParam::i().pitchHalfWidth() - safe_dist;
       }
       else if(target_point.x < -rcsc::ServerParam::i().pitchHalfWidth() + safe_dist)
       {
           target_point.x = -rcsc::ServerParam::i().pitchHalfWidth() + safe_dist;
       }
       if(target_point.y < -rcsc::ServerParam::i().pitchHalfWidth() + safe_dist)
       {
           target_point.y = -rcsc::ServerParam::i().pitchHalfWidth() + safe_dist; 
       }
       else if(target_point.y > rcsc::ServerParam::i().pitchHalfWidth() - safe_dist)
       {
           target_point.y = rcsc::ServerParam::i().pitchHalfWidth() - safe_dist; 
       }

       target_point_vector.push_back(target_point);
       additional_vector.rotate(angle);
   }
   
   if(self_next_pos.x >= rcsc::ServerParam::i().theirPenaltyAreaLine())
   {
       const rcsc::Vector2D front_opp_goal(rcsc::ServerParam::i().pitchHalfLength() - 9.5, 0.0);
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
       
       std::vector<rcsc::Vector2D> strategic_additional_vector_vector;
       additional_vector = front_opp_goal - self_next_pos;
       strategic_additional_vector_vector.push_back(additional_vector);
       additional_vector = near_centering_point - self_next_pos;
       strategic_additional_vector_vector.push_back(additional_vector);
       additional_vector = far_centering_point - self_next_pos;
       strategic_additional_vector_vector.push_back(additional_vector);
       
       const std::vector<rcsc::Vector2D>::const_iterator
           v_end = strategic_additional_vector_vector.end();
       for(std::vector<rcsc::Vector2D>::const_iterator
               v = strategic_additional_vector_vector.begin();
           v != v_end;
           v++)
       {
           additional_vector = (*v); 
           additional_vector.setLength(dribble_dist);
           target_point = self_next_pos + additional_vector;
           target_point_vector.push_back(target_point);
       }
   }
   return target_point_vector;
}

/*
std::vector<rcsc::Vector2D> 
Bhv_ObakeActionStrategy::getTargetPoint(rcsc::PlayerAgent * agent,
                                         std::vector<double> &score_target_vector)
{
    std::vector<rcsc::Vector2D> target_point_vector;
    int i;
    double multiplier, score;
    //make target_point

    for(i=0; i<target_point_vector.size(); i++)
    {
        if(checkAvailableTatgetPoint(agent,
                                     target_point_vector.at(i)))
        {
            target_point_vector.push_back(target_point_vector.at(i));

            //make target_point score
            score = 1.0;
            score *=  getPositionMultiplier(target_point_vector.at(i));
            if(checkExistShootCourse(agent, 
                                     multiplier))
            {
                score *= multiplier;
            }
            if(checkExistPassCourse(agent,
                                    multiplier))
            {
                score *= multiplier;
            }
        }
    }
    return target_point_vector;
}
*/
/*std::vector<rcsc::Vector2D>
Bhv_ObakeActionStrategy::getAvailableTargetPoint(rcsc::PlayerAgent * agent,
                                                    const std::vector<rcsc::Vector2D> &target_point_vector)
{

}

std::vector<double>
Bhv_ObakeActionStrategy::getTargetPointScore(rcsc::PlayerAgent * agent,
					      const std::vector<rcsc::Vector2D> &target_point_vector)
{
    
    
}
*/

rcsc::Vector2D
Bhv_ObakeActionStrategy::getBestTargetPoint(rcsc::PlayerAgent * agent)
{
    rcsc::Vector2D best_target_point, target_point;
    const std::vector<rcsc::Vector2D> target_point_vector = getTargetPointVector(agent);
    double max_score, score;
    bool pass_score_is_better = true;
    const std::vector<rcsc::Vector2D>::const_iterator
        v_end = target_point_vector.end();
    for(std::vector<rcsc::Vector2D>::const_iterator
            v = target_point_vector.begin();
        v != v_end;
        v++)
    {
        if(checkBestAction(agent,
                           (*v),
                           score,
                           max_score))
        {
            if(score == max_score)
            {


            }
            else
            {
                best_target_point = target_point;
            }
        }
    }
    if(!pass_score_is_better)
    {

    }
    return best_target_point;
}

/*
rcsc::Vector2D
Bhv_ObakeActionStrategy::getBestTargetPoint(rcsc::PlayerAgent * agent)/*,
                                             const std::vector<rcsc::Vector2D> &target_point_vector,
                                             const std::vector<double> &target_point_score_vector)
{
    const rcsc::WorldModel & wm = agent->world();
    int count, i;
    count = 0;

//    const double a

    const std::vector<rcsc::Vector2D>::const_iterator
        t_end = target_point_vector.end();
    for(std::vector<rcsc::Vector2D>::const_iterator
        t = target_point_vector.begin();
        t != t_end)
    {
        
    }

    double max_score = target_point_score_vector.at(0);
    rcsc::Vector2D target_point = target_point_vector.at(0);
    for(i=1; i<target_point_vector.size(); i++)
    {
        if(target_point_score_vector.at(i) > max_score)
        {
            max_score = target_point_score_vector.at(i);
            target_point = target_point_vector.at(i);
            count = 0;
        }
        else if(target_point_score_vector.at(i) == max_score)
        {
            count++;
        }
    }

    if(count >= 1)
    {
        const int max = count + 1;
        count = (int)(rand() * (max + 1.0) /(1.0 + RAND_MAX));
        for(i=0; i<target_point_vector.size(); i++)
        {
            if(count == 0
               && target_point_score_vector.at(i) == max_score)
            {
                target_point = target_point_vector.at(i);
                break;
            }
            if(target_point_score_vector.at(i) == max_score)
            {
                count--;
            }
        }
    }

    return target_point;
}
*/

int 
Bhv_ObakeActionStrategy::getDashCount(rcsc::PlayerAgent * agent,
                                       const rcsc::Vector2D &dribble_target)
{
    const rcsc::WorldModel & wm = agent->world();
    int dash_count;
    return dash_count;
}
