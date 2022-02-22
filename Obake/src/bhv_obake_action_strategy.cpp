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

#include <rcsc/action/body_kick_multi_step.h>

#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include <rcsc/geom/angle_deg.h>
#include <rcsc/geom/vector_2d.h>
#include <rcsc/player/player_agent.h>
#include <rcsc/player/audio_sensor.h>
#include <rcsc/player/say_message_builder.h>
#include <rcsc/action/body_dribble.h>
#include <rcsc/action/neck_turn_to_low_conf_teammate.h>

#include "obake_fuzzy_grade.h"
#include "body_obake_pass.h"
#include "obake_analysis.h"
#include "obake_strategy.h"
#include "bhv_obake_action_strategy.h"
#include "bhv_obake_action_strategy_test.h"

bool
Bhv_ObakeActionStrategy::execute(rcsc::PlayerAgent * agent)
{
    rcsc::dlog.addText( rcsc::Logger::TEAM,
                        "%s:%d: Bhv_ObakeActionStrategy"
                        ,__FILE__, __LINE__ );
    const rcsc::WorldModel & wm = agent->world();
    Obake_Analysis().setRole(wm.self().unum(),
			     M_role_side_or_center_back,
			     M_role_defensive_half,
			     M_role_offensive_half,
			     M_role_side_or_center_forward);
    if(wm.ball().pos().x >= rcsc::ServerParam::i().theirPenaltyAreaLineX()
       && wm.ball().pos().absY() <= 20.0/*rcsc::ServerParam::i().goalAreaHalfWidth()*/)
    {
        if(checkStrategicDribble(agent))
        {
            return true;
        }
    }

    if(getBestAction(agent))
    {
	return true;
    }
    return false;
/*
*/
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
Bhv_ObakeActionStrategy::checkAvailableTargetPoint(rcsc::PlayerAgent * agent,
						   const rcsc::Vector2D &target_point)
{
    const rcsc::WorldModel & wm = agent->world();
    const rcsc::Vector2D ball_next_pos = wm.ball().pos() + wm.ball().vel();
    const rcsc::Vector2D ball_to_target_point = target_point - ball_next_pos;
    const double ball_to_target_point_dist = ball_to_target_point.length();
    const double degree_near_penalty_area_x = Obake_FuzzyGrade().degreeNearOppPenaltyAreaX(target_point.x);
    const double degree_near_offside_line = Obake_FuzzyGrade().degreeNearOffsideLine(agent,
                                                                                     target_point.x);
    const double degree_far_penalty_area_x =  Obake_FuzzyGrade().degreeFarOppPenaltyAreaX(target_point.x);
const double base_small_angle = 35.2853;
const double base_short_r = 1.90728;
const double base_large_angle = 78.4982;
    const double base_long_r = ball_to_target_point_dist;

    const double near_rate = std::max(degree_near_penalty_area_x,
                                       degree_near_offside_line);
    const double far_rate = degree_far_penalty_area_x;
    const double sum = near_rate + far_rate;
    const double best_r = (base_short_r * near_rate 
                           + base_long_r * far_rate) / sum;
    const double  best_angle = (base_small_angle * near_rate
                                + base_large_angle * far_rate) / sum;

    if(Obake_Analysis().checkExistOpponent(agent,
					   ball_next_pos,
					   target_point,
					   best_angle,
					   best_r))
    {
	return false;
    }

    return true;
}

bool
Bhv_ObakeActionStrategy::checkAvoidanceFromNearestOpp(rcsc::PlayerAgent * agent,
                                                      const rcsc::Vector2D &target_point,
                                                      const double &max_multiplier,
                                                      double &multiplier)
{
    const rcsc::WorldModel &wm =agent->world();
    const rcsc::PlayerObject * nearest_opp_from_ball = wm.getOpponentNearestToBall(10);
    if(nearest_opp_from_ball)
    {
	const rcsc::Vector2D nearest_opp_next_pos = (nearest_opp_from_ball)->pos() + (nearest_opp_from_ball)->vel();
	const rcsc::Vector2D ball_next_pos = wm.ball().pos() + wm.ball().vel();
	const double dist_from_next_opp_to_ball = nearest_opp_next_pos.dist(ball_next_pos);
	const double dist_from_next_opp_to_target = nearest_opp_next_pos.dist(target_point);
/*	std::cout<<"opp "<<nearest_next_pos<<std::endl;
	std::cout<<"ball"<<ball_next_pos<<std::endl;
	std::cout<<"opp to ball = "<<dist_from_next_opp_to_ball<<std::endl;
	std::cout<<"opp to target = "<<dist_from_next_opp_to_target<<std::endl;
*/
        multiplier = 1.0;
//	if(dist_from_next_opp_to_ball < dist_from_next_opp_to_target)
        rcsc::Vector2D ball_to_target_vector = target_point - ball_next_pos;
        rcsc::Vector2D opp_to_ball_vector = ball_next_pos - nearest_opp_next_pos;
        const double difference_angle = std::abs(ball_to_target_vector.th().degree() - opp_to_ball_vector.th().degree());
        if(difference_angle <= 90.0)
	{
            const double difference_dist = dist_from_next_opp_to_target - dist_from_next_opp_to_ball;
            const double degree_short_avoidance = Obake_FuzzyGrade().degreeShortAvoidanceDist(difference_dist);
            const double degree_long_avoidance = Obake_FuzzyGrade().degreeLongAvoidanceDist(difference_dist);
            const double base_short_avoidance_multiplier = 1.0;
            const double base_long_avoidance_multiplier = max_multiplier;
            const double sum = degree_short_avoidance + degree_long_avoidance;
            multiplier = (base_short_avoidance_multiplier * degree_short_avoidance 
                          + base_long_avoidance_multiplier * degree_long_avoidance) / sum;	 
            /*std::cout<<"dist = "<<difference_dist<<std::endl;
            std::cout<<"degree long = "<<degree_long_avoidance<<", degree short = "<<degree_short_avoidance;
            std::cout<<std::endl;
            */
            return true;
	}
    }
    return false;
}

bool
Bhv_ObakeActionStrategy::checkBetterAction(rcsc::PlayerAgent * agent,
                                           const rcsc::Vector2D &target_point,
                                           const double &max_score,
                                           double &score)
{
const double shoot_max_multiplier = 1.37533;
const double body_dir_multiplier = 1.03757;
const double stamina_max_multiplier = 1.10596;
const double avoidance_max_multiplier = 1.02147;
const double opp_dist_max_multiplier = 1.15577;
const double side_attack_max_multiplier = 1.12716;
const double middle_attack_max_multiplier = 1.23251;
const double position_max_multiplier = 1.22767;
const double near_goal_max_multiplier = 1.44628;
const double penetration_max_multiplier = 1.28585;
const double dangerous_position_max_multiplier = 1.16266;
    /* const double role_back_multiplier = 1.1;
      const double role_defensive_half_multiplier = 1.05;*/
    double multiplier, new_multiplier;
    const double degree_near_offside_line = Obake_FuzzyGrade().degreeNearOffsideLine(agent,
                                                                                     target_point.x);
    const double degree_near_goal_x = Obake_FuzzyGrade().degreeNearOppGoalX(target_point.x);
    const double degree_near_goal_y = Obake_FuzzyGrade().degreeNearOppGoalY(target_point.y);
    const double degree_far_goal_y = Obake_FuzzyGrade().degreeFarOppGoalY(target_point.y);
    const double small_rate = std::max(degree_near_offside_line,
                                       std::min(degree_near_goal_x,
                                                degree_near_goal_y));
    const double big_rate = std::max(std::min(degree_near_goal_x,
                                              degree_far_goal_y),
                                     (1 - small_rate));
    const double sum = small_rate + big_rate;
const double base_small_multiplier = 1.03948;
const double base_big_multiplier = 1.06905;
    const double pass_max_multiplier = (base_big_multiplier * big_rate + base_small_multiplier * small_rate) / sum;

    //check shoot
    if(ObakeStrategy().getArea(target_point) == ObakeStrategy::ShootChance)
    {
        const double shoot_angle = 30.0;
        const double goal_r = 2.5;
        if(!Obake_Analysis().checkExistShootCourse(agent,
                                                  target_point,
                                                  shoot_angle,
                                                  goal_r))
        {
            score /= shoot_max_multiplier;
        }
    }
    else
    {
           score /= shoot_max_multiplier;
    }
    if(score < max_score)
    {
	return false;
    }
    //check pass 
    
    multiplier = getPassCourseMultiplier(agent,
                                         target_point,
                                         pass_max_multiplier);
    new_multiplier = multiplier / position_max_multiplier;
    score  *= new_multiplier;
    
    //check penetration
    multiplier = getPenetrationMultiplier(agent,
                                          target_point,
                                          penetration_max_multiplier);
    new_multiplier = multiplier / penetration_max_multiplier;
    score  *= new_multiplier;
    //check stamina
    if(checkVeryLackStamina(agent,
                            stamina_max_multiplier,
                            multiplier))
    {
        new_multiplier = multiplier / stamina_max_multiplier;
        score *= new_multiplier;
    }

    if(score < max_score)
    {
	return false;
    }

    //check near goal
    multiplier = getNearOppGoalMultiplier(agent,
                                          target_point,
                                          near_goal_max_multiplier);
    new_multiplier = multiplier / near_goal_max_multiplier;
    score  *= new_multiplier;

    if(score < max_score)
    {
	return false;
    }
    
    //check avoidance
    if(checkAvoidanceFromNearestOpp(agent,
                                    target_point,
                                    avoidance_max_multiplier,
                                    multiplier))
    {
        new_multiplier = multiplier / avoidance_max_multiplier;
        score *= new_multiplier;
    }
    else
    {
        score /= avoidance_max_multiplier;
    }

    //check dangerous position
    multiplier = getDangerousPositionMultiplier(agent,
                                                target_point,
                                                dangerous_position_max_multiplier);
    score /= multiplier;

    if(score < max_score)
    {
	return false;
    }

    //check position
    multiplier = getPositionMultiplier(agent,
                                       target_point,
                                       position_max_multiplier);
    new_multiplier = multiplier / position_max_multiplier;
    score  *= new_multiplier;

    if(score < max_score)
    {
	return false;
    }
    
    //check middle attack
    multiplier = getMiddleAttackPositionMultiplier(agent,
                                                   target_point,
                                                   middle_attack_max_multiplier);
    new_multiplier = multiplier / middle_attack_max_multiplier;
    score *= new_multiplier;

    if(score < max_score)
    {
	return false;
    }

    //check side attack
    multiplier = getSideAttackPositionMultiplier(agent,
                                                 target_point,
                                                 side_attack_max_multiplier);
    new_multiplier = multiplier / side_attack_max_multiplier;
    score *= new_multiplier;

    if(score < max_score)
    {
	return false;
    }
    
    //check defensive role
/*
    if(M_role_side_or_center_back)
    {
        score /= role_back_multiplier;
    }
    else if(M_role_defensive_half)
    {
        score /= role_defensive_half_multiplier;
    }
    
    if(score < max_score)
    {
	return false;
    }
*/
    return true;
}

bool
Bhv_ObakeActionStrategy::checkDistFromNearestOpp(rcsc::PlayerAgent * agent,
                                                 const rcsc::Vector2D &target_point,
                                                 const double &max_multiplier,
                                                 double &multiplier)
{
    const rcsc::WorldModel & wm = agent->world();
    double opp_dist = 100.0;
    wm.getOpponentNearestTo(target_point, 10, &opp_dist );	
    const double base_big_multiplier = max_multiplier;
const double base_moderate_multiplier_rate = 0.86723;
    const double base_moderate_multiplier = max_multiplier * base_moderate_multiplier_rate;
const double base_small_multiplier_rate = 0.643863;
    const double base_small_multiplier = max_multiplier * base_small_multiplier_rate;
    const double degree_near_offside_line = Obake_FuzzyGrade().degreeNearOffsideLine(agent,
                                                                                     target_point.x);
    const double degree_far_offside_line = Obake_FuzzyGrade().degreeFarOffsideLine(agent,
                                                                                   target_point.x);
    const double degree_near_opp_goal_y = Obake_FuzzyGrade().degreeNearOppGoalY(target_point.y);
    const double degree_far_opp_goal_y = Obake_FuzzyGrade().degreeFarOppGoalY(target_point.y);
    const double degree_near_from_opp = Obake_FuzzyGrade().degreeNearFromOpp(opp_dist);
    const double degree_moderate_from_opp = Obake_FuzzyGrade().degreeModerateFromOpp(opp_dist);
    const double degree_far_from_opp = Obake_FuzzyGrade().degreeFarFromOpp(opp_dist);
    double big_rate = std::min(std::min(degree_near_offside_line,
                                        degree_near_opp_goal_y),
                               std::max(degree_moderate_from_opp,
                                        degree_far_from_opp));
    double moderate_rate = std::max(std::min(std::min(degree_near_offside_line,
                                                      degree_near_opp_goal_y),
                                             degree_near_from_opp),
                                    std::min(degree_far_offside_line,
                                             degree_far_from_opp));
    moderate_rate = std::max(moderate_rate,
                             std::min(std::min(degree_near_offside_line,
                                               degree_far_opp_goal_y),
                                      std::max(degree_moderate_from_opp,
                                               degree_far_from_opp)));
        
    const double small_rate = std::max(std::min(std::min(degree_near_offside_line,
                                                         degree_far_opp_goal_y),
                                                degree_near_from_opp),
                                       std::min(degree_far_offside_line,
                                                std::max(degree_near_from_opp,
                                                         degree_moderate_from_opp)));
    if(target_point.absY() >= rcsc::ServerParam::i().pitchHalfWidth() - 8.0)
    {
        big_rate = std::max(big_rate,
                            std::min(degree_near_offside_line,
                                     std::max(degree_moderate_from_opp,
                                              degree_far_from_opp)));
        moderate_rate = std::max(moderate_rate,
                                 std::min(degree_near_offside_line,
                                          degree_near_from_opp));
        
    }
    double sum = big_rate + moderate_rate + small_rate;
    const double opp_dist_rate = (base_big_multiplier * big_rate + base_moderate_multiplier * moderate_rate
                                  + base_small_multiplier * small_rate) / sum;
}

bool
Bhv_ObakeActionStrategy::checkDribbleToBodyDir(rcsc::PlayerAgent * agent,
                                               const rcsc::Vector2D &target_point)
{
    

}


bool
Bhv_ObakeActionStrategy::checkDribbleToNearGoalX(rcsc::PlayerAgent * agent,
						 const double &target_point_x)
{
    const rcsc::WorldModel &wm = agent->world();
    const double ball_next_pos_x = wm.ball().pos().x + wm.ball().vel().x;
    if(target_point_x > ball_next_pos_x)
    {
	return true;
    }
    return false;
}

bool
Bhv_ObakeActionStrategy::checkDribbleToNearGoalY(rcsc::PlayerAgent * agent,
						  const double &target_point_y)
{
    const rcsc::WorldModel & wm = agent->world();
    const double ball_next_pos_y = wm.ball().pos().y + wm.ball().vel().y;
    if(std::abs(target_point_y) < std::abs(ball_next_pos_y))    
    {
	return true;
    }
    return false;
}

bool
Bhv_ObakeActionStrategy::checkExistAssistCourse(rcsc::PlayerAgent * agent,
						const rcsc::Vector2D &base_pos,
						const double &pass_angle,
						const double &max_pass_dist,
						int &front_of_the_ball_mate,
						int &sum_of_the_ball_mate,
                                                int &sum_of_the_mate_in_penalty_area)
{
    const rcsc::WorldModel & wm = agent->world();
    const double r = 2.5;
    const double shoot_angle = 25.0;
    const double near_offside_line_opp_r = 3.5;
    const double far_offside_line_opp_r = 5.0;
/*
    const double degree_near_offside_line = Obake_FuzzyGrade().degreeNearOffsideLine(agent,
										     base_pos.x);
    const double degree_far_offside_line =  Obake_FuzzyGrade().degreeFarOffsideLine(agent,
										    base_pos.x);
    
    const double  sum =  degree_near_offside_line + degree_far_offside_line;
    const double opp_r = (near_offside_line_opp_r * degree_near_offside_line
			  + far_offside_line_opp_r * degree_far_offside_line) / sum;
*/
    double degree_near_offside_line, degree_far_offside_line,
        sum, opp_r;
    front_of_the_ball_mate = 0;
    sum_of_the_ball_mate = 0;
    sum_of_the_mate_in_penalty_area = 0;
    const rcsc::PlayerPtrCont::const_iterator end = wm.teammatesFromBall().end();
    for(rcsc::PlayerPtrCont::const_iterator mate = wm.teammatesFromBall().begin();
        mate != end;
        mate++)
    {
	if((*mate)->distFromBall() > max_pass_dist)
	{
	    break;
	}
        
        
	if(Obake_Analysis(). checkPassCourse(agent,
					     base_pos,
					     (*mate)->pos(),
					     pass_angle))
	{
	    if(ObakeStrategy::getArea((*mate)->pos()) == ObakeStrategy::ShootChance)
	    {
		if(Obake_Analysis().checkExistShootCourse(agent,
							  (*mate)->pos(),
							  shoot_angle,
							  r))
		{
		    return true;
		}
	    }
            degree_near_offside_line = Obake_FuzzyGrade().degreeNearOffsideLine(agent,
                                                                                (*mate)->pos().x);
            degree_far_offside_line =  Obake_FuzzyGrade().degreeFarOffsideLine(agent,
                                                                               (*mate)->pos().x);
    
            sum =  degree_near_offside_line + degree_far_offside_line;
            opp_r = (near_offside_line_opp_r * degree_near_offside_line
                     + far_offside_line_opp_r * degree_far_offside_line) / sum;
	    const rcsc::Circle2D circle((*mate)->pos(), opp_r);
	    if(!wm.existOpponentIn(circle, 10, true))
	    {
		sum_of_the_ball_mate++;
		if(wm.ball().pos().x < (*mate)->pos().x)
		{
		    front_of_the_ball_mate++;
		}
                if(Obake_Analysis().checkExistOppPenaltyAreaIn((*mate)->pos()))
                {
                    sum_of_the_mate_in_penalty_area++;
                }
	    }
	}
    }
    return false;
}

bool
Bhv_ObakeActionStrategy::checkExistShootCourse(rcsc::PlayerAgent * agent,
                                               const rcsc::Vector2D &target_point,
                                               const double &max_muliplier,
                                               double &multiplier)
{

}

double
Bhv_ObakeActionStrategy::getMiddleAttackPositionMultiplier(rcsc::PlayerAgent * agent,
                                                           const rcsc::Vector2D &target_point,
                                                           const double &max_multiplier)
{
    const rcsc::WorldModel & wm = agent->world();
    const double degree_near_offside_line = Obake_FuzzyGrade().degreeNearOffsideLine(agent,
                                                                                     target_point.x);
    const double degree_near_goal_y = Obake_FuzzyGrade().degreeNearOppGoalY(target_point.y);
    const double base_near_multiplier = max_multiplier;
    const double base_not_near_multiplier = 1.0;
    const double near_rate = std::max(degree_near_offside_line,
                                      degree_near_goal_y);
    const double not_near_rate = 1 - near_rate;
    const double multiplier = (near_rate * base_near_multiplier 
                               + not_near_rate * base_not_near_multiplier); 

    return multiplier;
}

double
Bhv_ObakeActionStrategy::getNearOppGoalMultiplier(rcsc::PlayerAgent * agent,
                                                  const rcsc::Vector2D &target_point,
                                                  const double &max_multiplier)
{
    const double base_near_opp_goal_multiplier = max_multiplier;
    const double base_not_near_opp_goal_multiplier = 1.0;
    const double degree_near_opp_goal = std::min(Obake_FuzzyGrade().degreeNearOppGoalX(target_point.x),
                                                 Obake_FuzzyGrade().degreeNearOppGoalY(target_point.y));
    const double degree_not_near_opp_goal = 1 - degree_near_opp_goal;
    const double multiplier = degree_near_opp_goal * base_near_opp_goal_multiplier
        + degree_not_near_opp_goal * base_not_near_opp_goal_multiplier;
    return multiplier;
}
double
Bhv_ObakeActionStrategy::getSideAttackPositionMultiplier(rcsc::PlayerAgent * agent,
                                                         const rcsc::Vector2D &target_point,
                                                         const double &max_multiplier)
{
    const rcsc::WorldModel & wm = agent->world();
    const rcsc::Vector2D self_next_pos = wm.self().pos() + wm.self().vel();
    double multiplier = 1.0;
    if(target_point.x <= rcsc::ServerParam::i().theirPenaltyAreaLineX()
       && target_point.absY() >= rcsc::ServerParam::i().pitchHalfWidth() - 8.0
       && self_next_pos.absY() >= rcsc::ServerParam::i().pitchHalfWidth() - 8.0
       && self_next_pos.x < target_point.x)
    {
        const double degree_near_offside_line = Obake_FuzzyGrade().degreeNearOffsideLine(agent,
                                                                                         target_point.x);
        const double degree_far_offside_line = Obake_FuzzyGrade().degreeFarOffsideLine(agent,
                                                                                       target_point.x);
        const double base_near_offside_line_multiplier = max_multiplier;
        const double base_far_offside_line_multiplier = 1.0;
        const double sum = degree_near_offside_line + degree_far_offside_line;
        multiplier = (base_near_offside_line_multiplier * degree_near_offside_line 
                      + base_far_offside_line_multiplier * degree_far_offside_line) / sum;
    }
    return multiplier;
}

bool
Bhv_ObakeActionStrategy::checkVeryLackStamina(rcsc::PlayerAgent * agent,
                                              const double &max_muliplier,
                                              double &multiplier)
{
    const rcsc::WorldModel & wm = agent->world();
    const double base_very_lack_stamina_multiplier = 1.0;
    const double base_not_very_lack_stamina_multiplier = max_muliplier;
    const double degree_very_lack_stamina = std::pow(Obake_FuzzyGrade().degreeLackStamina(wm.self().stamina()), 2.0);
    const double degree_not_very_lack_stamina = 1 - degree_very_lack_stamina;
    multiplier = base_very_lack_stamina_multiplier * degree_very_lack_stamina +
        base_not_very_lack_stamina_multiplier * degree_not_very_lack_stamina;
    if(degree_very_lack_stamina > 0.0)
    {
        return true;
    }
    return false;
}

double
Bhv_ObakeActionStrategy::getPassCourseMultiplier(rcsc::PlayerAgent * agent,
						 const rcsc::Vector2D &target_point,
						 const double &max_muliplier)
{
    const rcsc::WorldModel & wm = agent->world();
    const rcsc::Vector2D self_next_pos = wm.self().pos() + wm.self().vel();
    const double pass_angle = 30.0;
    const double max_pass_dist = 30.0;
    const double difference_multiplier = max_muliplier - 1.0;
    double rate;
    int front_of_the_ball_mate, sum_of_the_ball_mate,
        sum_of_the_mate_in_penalty_area;
    double multiplier = 1.0;
    rcsc::Vector2D base_point, additional_vector;
    additional_vector = target_point - self_next_pos;
    const double dribble_dist = additional_vector.length() / 2.5;
    additional_vector.setLength(dribble_dist);
    base_point = self_next_pos + additional_vector;
    
    if(checkExistAssistCourse(agent,
			      base_point,
			      pass_angle,
			      max_pass_dist,
			      front_of_the_ball_mate,
			      sum_of_the_ball_mate,
                              sum_of_the_mate_in_penalty_area))
    {
	multiplier = max_muliplier;
    }
    else if(sum_of_the_ball_mate >= 1)
    {
        if(wm.ball().pos().x >= rcsc::ServerParam::i().theirPenaltyAreaLineX()
           && sum_of_the_ball_mate >= 1)
        {
rate = 0.713208;
	    const double base_many_mate_number_multiplier = 1.0 + difference_multiplier * rate;
	    const double base_not_any_mate_number_multipler = 1.0;
	    const double degree_many_mate_number = Obake_FuzzyGrade().degreeManyMateNumber(sum_of_the_ball_mate);
	    const double degree_any_mate_number = 1 - degree_many_mate_number;
	    multiplier = base_many_mate_number_multiplier * degree_many_mate_number
		+ base_not_any_mate_number_multipler * degree_any_mate_number;
        }
	else if(front_of_the_ball_mate >= 1)
	{
rate = 0.824499;
	    const double base_many_mate_number_multiplier = 1.0 + difference_multiplier * rate;
	    const double base_not_any_mate_number_multipler = 1.0;
	    const double degree_many_mate_number = Obake_FuzzyGrade().degreeManyMateNumber(front_of_the_ball_mate);
	    const double degree_any_mate_number = 1 - degree_many_mate_number;
	    multiplier = base_many_mate_number_multiplier * degree_many_mate_number
		+ base_not_any_mate_number_multipler * degree_any_mate_number;
	}
	else
	{
const double rate = 0.269354;
	    const double base_many_mate_number_multiplier = 1.0 + difference_multiplier * rate;
	    const double base_not_any_mate_number_multipler = 1.0;
	    const double degree_many_mate_number = Obake_FuzzyGrade().degreeManyMateNumber(sum_of_the_ball_mate);
	    const double degree_any_mate_number = 1 - degree_many_mate_number;
	    multiplier = base_many_mate_number_multiplier * degree_many_mate_number
		+ base_not_any_mate_number_multipler * degree_any_mate_number;
	}
    }

    return multiplier;
}

double 
Bhv_ObakeActionStrategy::getPositionMultiplier(rcsc::PlayerAgent * agent,
                                               const rcsc::Vector2D &target_point,
                                               const double &max_multiplier)
{
    const rcsc::WorldModel & wm = agent->world();
    const rcsc::Vector2D self_next_pos = wm.self().pos() + wm.self().vel();
    double multiplier = 1.0;

    if(self_next_pos.x < rcsc::ServerParam::i().theirPenaltyAreaLineX())
    {
        const double degree_near_offside_line = Obake_FuzzyGrade().degreeNearOffsideLine(agent,
                                                                                         target_point.x);
        const double degree_far_offside_line = Obake_FuzzyGrade().degreeFarOffsideLine(agent,
                                                                                       target_point.x);
        const double base_near_offside_line_multiplier = max_multiplier;
        const double base_far_offside_line_multiplier = 1.0;
        const double sum = degree_near_offside_line + degree_far_offside_line;
        multiplier = (base_near_offside_line_multiplier * degree_near_offside_line 
                      + base_far_offside_line_multiplier * degree_far_offside_line) / sum;
    }
    else
    {
        const double difference_multiplier = max_multiplier - 1.0;
const double rate = 0.343315;
        const double degree_near_opp_goal_x = Obake_FuzzyGrade().degreeNearOppGoalX(target_point.x);
        const double degree_near_opp_goal_y = Obake_FuzzyGrade().degreeNearOppGoalY(target_point.y);
        const double big_rate = std::min(degree_near_opp_goal_x,
                                         degree_near_opp_goal_y);
        const double degree_near_mate_goal_x = Obake_FuzzyGrade().degreeNearMateGoalX(target_point.x);
        const double degree_near_mate_goal_y = Obake_FuzzyGrade().degreeNearMateGoalY(target_point.y);
        const double small_rate = std::min(degree_near_mate_goal_x,
                                           degree_near_mate_goal_y);
        const double moderate_rate = std::min((1 - big_rate),
                                              (1 - small_rate));
        const double base_big_multiplier = max_multiplier;
        const double base_moderate_multiplier = 1.0 + difference_multiplier * rate;
        const double base_small_multiplier = 1.0;
        const double sum = small_rate + moderate_rate + big_rate;
        multiplier = (small_rate * base_small_multiplier + moderate_rate * base_moderate_multiplier
                      + big_rate * base_big_multiplier) / sum;
    }
    return multiplier;
}

double
Bhv_ObakeActionStrategy::getShootMultiplier(rcsc::PlayerAgent * agent,
                                            const rcsc::Vector2D target_point,
                                            const double &max_multiplier)
{
    const double difference_multiplier = max_multiplier - 1.0;
const double rate = 0.642825;
    const double base_near_opp_goal_multiplier = max_multiplier;
    const double base_not_near_opp_goal_multiplier = 1 + difference_multiplier * rate;
    const double degree_near_opp_goal_x = Obake_FuzzyGrade().degreeNearOppGoalX(target_point.x);
    const double degree_near_opp_goal_y = Obake_FuzzyGrade().degreeNearOppGoalY(target_point.y);
    const double degree_near_opp_goal = std::min(degree_near_opp_goal_x,
                                                 degree_near_opp_goal_y);
    const double degree_not_near_opp_goal = 1 - degree_near_opp_goal;
    const double multiplier = (base_near_opp_goal_multiplier * degree_near_opp_goal 
                               + base_not_near_opp_goal_multiplier * degree_not_near_opp_goal);
    return multiplier;
}

std::vector<rcsc::Vector2D>
Bhv_ObakeActionStrategy::getTargetPointVector(rcsc::PlayerAgent * agent)
{
    const rcsc::WorldModel & wm = agent->world();
    const rcsc::Vector2D self_next_pos = wm.self().pos() + wm.self().vel();
const double base_near_target_point_dist = 3.48959;
const double base_moderate_target_point_dist = 9.36535;
const double base_far_target_point_dist = 17.063;
    const double degree_near_penalty_area_x = Obake_FuzzyGrade().degreeNearOppPenaltyAreaX(self_next_pos.x);
    const double degree_near_offside_line = Obake_FuzzyGrade().degreeNearOffsideLine(agent,
                                                                                     self_next_pos.x);
    const double degree_far_penalty_area_x =  Obake_FuzzyGrade().degreeFarOppPenaltyAreaX(self_next_pos.x);
    const double degree_near_goal_x = Obake_FuzzyGrade().degreeNearOppGoalX(self_next_pos.x);
    const double degree_near_goal_y = Obake_FuzzyGrade().degreeNearOppGoalY(self_next_pos.y);
    const double near_rate = std::min(degree_near_goal_x,
                                      degree_near_goal_y);
    const double moderate_rate = std::max(degree_near_penalty_area_x,
                                      degree_near_offside_line);
    const double far_rate = degree_far_penalty_area_x;
    const double sum = near_rate + moderate_rate + far_rate;
    const double dribble_dist = (base_near_target_point_dist * near_rate
                                 + base_moderate_target_point_dist * moderate_rate
                                 + base_far_target_point_dist * far_rate)
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
       if(target_point.x > rcsc::ServerParam::i().pitchHalfLength() - safe_dist)
       {
           target_point.x = rcsc::ServerParam::i().pitchHalfLength() - safe_dist;
       }
       else if(target_point.x < -rcsc::ServerParam::i().pitchHalfLength() + safe_dist)
       {
           target_point.x = -rcsc::ServerParam::i().pitchHalfLength() + safe_dist;
       }
       if(target_point.y < -rcsc::ServerParam::i().pitchHalfWidth() + safe_dist)
       {
           target_point.y = -rcsc::ServerParam::i().pitchHalfWidth() + safe_dist; 
       }
       else if(target_point.y > rcsc::ServerParam::i().pitchHalfWidth() - safe_dist)
       {
           target_point.y = rcsc::ServerParam::i().pitchHalfWidth() - safe_dist; 
       }
       if(!((Obake_Analysis().checkExistOurPenaltyAreaIn(wm.self().pos()) && additional_vector.x < 0.0)
              || (!Obake_Analysis().checkExistOurPenaltyAreaIn(wm.self().pos())
                  && Obake_Analysis().checkExistOurPenaltyAreaIn(target_point))))
       {
           target_point_vector.push_back(target_point);
       }
       additional_vector.rotate(angle);
   }
   
   if(self_next_pos.x >= rcsc::ServerParam::i().theirPenaltyAreaLineX())
   {
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
       
       std::vector<rcsc::Vector2D> strategic_additional_vector_vector;
       additional_vector = front_opp_goal - self_next_pos;
       if(wm.self().body() != additional_vector.th())
       {
           strategic_additional_vector_vector.push_back(additional_vector);
       }
       additional_vector = near_centering_point - self_next_pos;
       if(wm.self().body() != additional_vector.th())
       {
           strategic_additional_vector_vector.push_back(additional_vector);
       }
       additional_vector = far_centering_point - self_next_pos;
       if(wm.self().body() != additional_vector.th())
       {
           strategic_additional_vector_vector.push_back(additional_vector);
       }
       
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

bool
Bhv_ObakeActionStrategy::getBestAction(rcsc::PlayerAgent * agent)
{
    const rcsc::WorldModel & wm = agent->world();
    rcsc::Vector2D additional_vector, best_target_point, target_point, near_target_point;
    const rcsc::Vector2D self_next_pos = wm.self().pos() + wm.self().vel();
    double near_dribble_rate, far_dribble_rate, sum, dribble_dist;
const double near_base_dist = 1.48388;
const double far_base_dist = 4.68525;
    
    const std::vector<rcsc::Vector2D> target_point_vector = getTargetPointVector(agent);
const double dribble_to_body_dir_rate = 1.02687;
    int receiver_unum;
    double max_score, pass_score, first_speed, score;
    bool pass_score_is_better = true;
/*
    rcsc::Vector2D pass_point;
    if(!Body_ObakePass::get_best_pass(agent, 
                                     &pass_point,
                                     &first_speed, 
                                     &receiver_unum,
                                     &pass_score))
    {
        pass_score_is_better = false;
        pass_score = 0.0;
    }
*/  
//    std::cout<<"size = "<<target_point_vector.size();
//    max_score = pass_score;
    bool exist_target = false;
    max_score = 0.0;
    std::vector<rcsc::Vector2D> candidate_target_point_vector;
//    std::cout<<"self_next = "<<self_next_pos<<std::endl;
    const std::vector<rcsc::Vector2D>::const_iterator
        v_end = target_point_vector.end();
    for(std::vector<rcsc::Vector2D>::const_iterator
            v = target_point_vector.begin();
        v != v_end;
        v++)
    {
        
//	std::cout<<"pos = "<<(*v)<<std::endl;
        near_dribble_rate = Obake_FuzzyGrade().degreeNearOppGoalX((*v).x);
        far_dribble_rate = Obake_FuzzyGrade().degreeFarOppGoalX((*v).x);
        sum = near_dribble_rate + far_dribble_rate;
        dribble_dist = (near_dribble_rate * near_base_dist + far_dribble_rate * far_base_dist)
        / sum;
	score = 1000;
        additional_vector = (*v) - self_next_pos;
        near_target_point = self_next_pos + rcsc::Vector2D::polar2vector(dribble_dist, additional_vector.th());
	if(!checkAvailableTargetPoint(agent,
                                      near_target_point))
	{
            additional_vector.rotate(30.0);
            near_target_point = self_next_pos + rcsc::Vector2D::polar2vector(dribble_dist, additional_vector.th());
            if(!checkAvailableTargetPoint(agent,
                                          near_target_point))
            {
                additional_vector.rotate(-60.0);
                near_target_point = self_next_pos
                    + rcsc::Vector2D::polar2vector(dribble_dist, additional_vector.th());
                if(!checkAvailableTargetPoint(agent,
                                              near_target_point))
                {
                    continue;
                }
            }
	}
        exist_target = true;
	if(additional_vector.th() != ((*target_point_vector.begin()) - self_next_pos).th())
	{
	    score /= dribble_to_body_dir_rate;
	}

        if(checkBetterAction(agent,
                             (*v),
                             max_score,
                             score))
        {
//	    std::cout<<"better score = "<<score<<std::endl;
            if(score == max_score)
            {
		candidate_target_point_vector.push_back(near_target_point);
            }
            else 
            {
                best_target_point = near_target_point;
		max_score = score;
//                std::cout<<"change max_score into "<<max_score<<std::endl;
/*
		if(pass_score_is_better)
		{
		    pass_score_is_better = false;
		}
*/
		if(candidate_target_point_vector.size() >= 1)
		{
		    candidate_target_point_vector.clear();
		    candidate_target_point_vector.push_back(best_target_point);
		}
            }
        }
    }
/*
    std::cout<<std::endl;
    std::cout<<"max_score = "<<max_score<<", pass_score = "<<pass_score<<std::endl;
*/
/*
    if(pass_score_is_better)
    {
	if(candidate_target_point_vector.size() >= 1)
	{
	    std::srand((unsigned)std::time(NULL));
	    const int max = candidate_target_point_vector.size();
	    const int min = 0;
	    const int num = min + (int)(rand() * (max - min + 1.0) / (1.0 + RAND_MAX));
	    if(num < max)
	    {
		best_target_point = candidate_target_point_vector.at(num);
		const double dash_power = getDashPower(agent, best_target_point);
		rcsc::Body_Dribble(best_target_point,
				   1.0,
				   dash_power,
				   1
		    ).execute(agent);
	    }
	}
	rcsc::Body_KickMultiStep(target_point,
				 first_speed,
				 false
	    ).execute(agent);
        rcsc::dlog.addText(rcsc::Logger::ACTION,
                           "%s:%d:e xecute() set pass communication."
                           ,__FILE__, __LINE__ );
        if(agent->config().useCommunication()
           && receiver_unum != rcsc::Unum_Unknown)
        {
	    rcsc::Vector2D target_buf = pass_point - wm.self().pos();
	    target_buf.setLength(1.0);
        
	    agent->addSayMessage(new rcsc::PassMessage(receiver_unum,
						       target_point + target_buf,
						       agent->effector().queuedNextBallPos(),
						       agent->effector().queuedNextBallVel()));

        }
	agent->setNeckAction(new rcsc::Neck_TurnToLowConfTeammate());
    }
*/
    if(candidate_target_point_vector.size() > 1)
    {
        const int max = candidate_target_point_vector.size() - 1;
        const int min = 0;
        const int num = min + (int)(rand() * (max - min + 1.0) / (1.0 + RAND_MAX));
        best_target_point = candidate_target_point_vector.at(num);
    }
    const double dash_power = getDashPower(agent, best_target_point);
    rcsc::dlog.addText( rcsc::Logger::TEAM,
                            "%s:%d: dribble to (%.1f, %.1f) dash power=%.1f"
                            ,__FILE__, __LINE__,
                        best_target_point.x, best_target_point.y,
                        dash_power);
    if(!exist_target)
    {
        return false;
    }
//    std::cout<<"best target = "<<best_target_point<<std::endl;
    rcsc::Body_Dribble(best_target_point,
                       1.0,
                       dash_power,
                       1
        ).execute(agent);
    agent->setNeckAction( new rcsc::Neck_TurnToLowConfTeammate() );
//    }
    
    return true;
}

/*
rcsc::Vector2D
Bhv_ObakeActionStrategy::getBestTargetPoint(rcsc::PlayerAgent * agent),
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

double
Bhv_ObakeActionStrategy::getPenetrationMultiplier(rcsc::PlayerAgent * agent,
                                                  const rcsc::Vector2D &target_point,
                                                  const double &max_multiplier)
{
    const rcsc::WorldModel & wm = agent->world();
    const rcsc::Vector2D self_next_pos = wm.self().pos() + wm.self().vel();
    double multiplier = 1.0;
    const double dribble_dist = 8.0;
    rcsc::Vector2D additional_vector = target_point - self_next_pos;
    additional_vector.setLength(dribble_dist);
    const rcsc::Vector2D new_target_point = self_next_pos + additional_vector;
    if(self_next_pos.x < rcsc::ServerParam::i().pitchHalfLength() - rcsc::ServerParam::i().goalAreaLength()
       && new_target_point.x > wm.offsideLineX())
    {
        multiplier = max_multiplier;
    }
    return multiplier;
}

double
Bhv_ObakeActionStrategy::getDangerousPositionMultiplier(rcsc::PlayerAgent * agent,
                                                        const rcsc::Vector2D &target_point,
                                                        const double &max_multiplier)
{
    const double difference_multiplier = max_multiplier - 1.0;
    const double base_big_multiplier = max_multiplier;
double rate = 0.760034;
    const double base_moderate_multiplier = 1.0 + difference_multiplier * rate;
    const double base_small_multiplier = 1.0;
    const double degree_near_mate_goal_x = Obake_FuzzyGrade().degreeNearMateGoalX(target_point.x);
    const double degree_near_mate_goal_y = Obake_FuzzyGrade().degreeNearMateGoalY(target_point.y);
    const double degree_not_near_mate_goal_x = 1 - degree_near_mate_goal_x;
    const double degree_not_near_mate_goal_y = 1 - degree_near_mate_goal_y;
    double big_rate, moderate_rate, small_rate;
    big_rate = std::min(degree_near_mate_goal_x,
                        degree_near_mate_goal_y);
    moderate_rate = std::max(std::min(degree_near_mate_goal_x,
                                      degree_not_near_mate_goal_y),
                             std::min(degree_not_near_mate_goal_x,
                                      degree_near_mate_goal_y));
    small_rate = std::min(degree_not_near_mate_goal_x,
                          degree_not_near_mate_goal_y);
    const double sum = small_rate + moderate_rate + big_rate;
    const double multiplier = (base_small_multiplier * small_rate 
                               + base_moderate_multiplier * moderate_rate 
                               + base_big_multiplier * big_rate) / sum;
    return multiplier;
}

int 
Bhv_ObakeActionStrategy::getDashCount(rcsc::PlayerAgent * agent,
                                      const rcsc::Vector2D &dribble_target)
{
    const rcsc::WorldModel & wm = agent->world();
    int dash_count;
    return dash_count;
}

bool
Bhv_ObakeActionStrategy::checkStrategicDribble(rcsc::PlayerAgent * agent)
{
    const rcsc::WorldModel & wm = agent->world();
    const rcsc::Vector2D front_opp_goal(rcsc::ServerParam::i().pitchHalfLength() 
                                        -rcsc::ServerParam::i().goalAreaLength(),
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
    if(wm.self().pos().x >= rcsc::ServerParam::i().theirPenaltyAreaLineX()
       ||
       (wm.self().pos().x >= 30.0 && wm.self().pos().absY() >= rcsc::ServerParam::i().penaltyAreaHalfWidth())
        || ((wm.self().pos().x >= rcsc::ServerParam::i().theirPenaltyAreaLineX() - 16.0
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
       && (wm.self().pos().x <= (rcsc::ServerParam::i().theirPenaltyAreaLineX() + 11.0))
         && wm.self().pos().absY() <= rcsc::ServerParam::i().penaltyAreaHalfWidth())
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

    if(wm.self().pos().x >= rcsc::ServerParam::i().theirPenaltyAreaLineX()
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
                    return true;
                }
                count++;
            }
        }
    }
    return false;

}
