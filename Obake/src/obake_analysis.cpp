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
#include "config.h"
#endif

#include <rcsc/geom/angle_deg.h>
#include <rcsc/math_util.h>
#include <rcsc/player/player_agent.h>
#include <rcsc/common/server_param.h>

#include "obake_analysis.h"
#include "obake_strategy.h"


bool
Obake_Analysis::checkExistOurPenaltyAreaIn(const rcsc::Vector2D &check_pos)
{
    if(check_pos.x <= rcsc::ServerParam::i().ourPenaltyAreaLineX()
       && check_pos.absY() <= rcsc::ServerParam::i().penaltyAreaHalfWidth())
    {
        return true;
    }
    return false;
}

bool
Obake_Analysis::checkExistOppPenaltyAreaIn(const rcsc::Vector2D &check_pos)
{
    if(check_pos.x >= rcsc::ServerParam::i().theirPenaltyAreaLineX()
       && check_pos.absY() <= rcsc::ServerParam::i().penaltyAreaHalfWidth())
    {
        return true;
    }
    return false;
}

bool
Obake_Analysis::checkExistDefender(rcsc::PlayerAgent * agent)
{
    const rcsc::WorldModel & wm = agent->world();
    const double dist_self_to_ball = wm.ball().pos().dist(wm.self().pos());
    const rcsc::PlayerPtrCont::const_iterator end = wm.teammatesFromBall().end();
    for(rcsc::PlayerPtrCont::const_iterator mate = wm.teammatesFromBall().begin();
        mate != end;
        mate++)
    {
        if((*mate)->distFromBall() > dist_self_to_ball)
        {
            break;
        }
        else if(wm.ball().pos().x > rcsc::ServerParam::i().ourPenaltyAreaLineX())
        {
            if((*mate)->pos().x < wm.ball().pos().x
               && ((*mate)->distFromBall() < 7.0
                   || (std::abs((*mate)->pos().x - wm.ball().pos().x) 
                       > std::abs((*mate)->pos().y - wm.ball().pos().y))))
            {
                return true;
            }
        }
        else
        {
            if(wm.ball().pos().absY() <= rcsc::ServerParam::i().goalAreaHalfWidth())
            {
                if(std::abs((*mate)->pos().x - wm.ball().pos().x) 
                   > std::abs((*mate)->pos().y - wm.ball().pos().y))
                {
                    return true;
                }
            }
            else 
            {
                if((*mate)->pos().absY() < wm.ball().pos().absY()
                   && std::abs((*mate)->pos().x - wm.ball().pos().x) 
                   < std::abs((*mate)->pos().y - wm.ball().pos().y))
                {
                    return true;
                }
            }
        }
    }
    return false;
}
/*
bool
Obake_Analysis::checkOppCanControlBall(rcsc::PlayerAgent * agent)
{
    const rcsc::WorldModel & wm = agent->world();
    const double base_pass_dist = 25.0;
    const int cycle = 15;
    const double calc_pass_dist = rcsc::calc_sum_geom_series(wm.ball().vel(),
                                                             rcsc::ServerParam::i().ballDecay(),
                                                             cycle);
    double max_pass_dist = std::min(base_pass_dist,
                                    calc_pass_dist);
    const rcsc::Vector2D ball_next_pos = wm.ball().pos() + wm.ball().vel();
    const rcsc::PlayerObject * nearest_opp_from_ball = wm.getOpponentNearestToBall(10);
    if(nearest_opp_from_ball)
    {
        const rcsc::Vector2D nearest_opp_from_ball_next_pos =
            (nearest_opp_from_ball)->pos() + (nearest_opp_from_ball)->vel();
        if(nearest_opp_from_ball_next_pos.dist(ball_next_pos) <= rcsc::ServerParam::i().defaultKickableArea() + 1.0)
        {
            return true;
        }
    }
    const rcsc::Line2D ball_line(wm.ball().pos(), ball_next_pos);
    rcsc::Vector2D ball_to_opp, intersection;
    double diffrence_angle;
    bool exist_opp_receiver = false;
    const rcsc::PlayerPtrCont::const_iterator
        opp_end = wm.opponentsFromSelf().end();
    for(rcsc::PlayerPtrCont::const_iterator
        opp = wm.opponentsFromBall().begin();
        opp != opp_end;
        opp++)
    {
        if ((*opp)->posCount() > 10)
        {
            continue;
        }
        if((*opp)->isGhost() && (*opp)->posCount() >= 4)
        {
            continue;
        }
        ball_to_opp = (*opp)->pos() - wm.ball().pos();
        diffrence_angle = std::abs(wm.ball().vel().th().degree()
                                   - ball_to_opp.th().degree());
        if (diffrence_angle > 90.0 )
        {
            continue;
        }
        if((*opp)->distFromBall() > max_pass_dist)
        {
            continue;
        }
        const rcsc::Line2D perpendicular_opp_ball_line =
            ball_line.perpendicular((*opp)->pos());
        intersection = rcsc::Line2D::intersection(ball_line,
                                                  perpendicular_opp_ball_line);
        if((*opp)->pos().dist2(intersection) <= 3.0 * 3.0)
        {
            exist_opp_receiver = true;
            max_pass_dist = intersection.dist(wm.ball().pos());
            break;
        }
    }
    if(exist_opp_receiver)
    {
        const double player_dash_speed = 1.0;
        int ball_step, mate_step;
        double ball_dist, ball_to_intersection_dist, mate_to_intersection_dist;
        const rcsc::PlayerPtrCont::const_iterator
            mate_end = wm.teammatesFromBall().end();
        for(rcsc::PlayerPtrCont::const_iterator
            mate = wm.teammatesFromBall().begin();
            mate != mate_end;
            mate++)
        {
            if ((*mate)->posCount() > 10)
            {
                continue;
            }
            if((*mate)->isGhost() && (*mate)->posCount() >= 4)
            {
                continue;
            }
            ball_to_opp = (*mate)->pos() - wm.ball().pos();
            diffrence_angle = std::abs(wm.ball().vel().th().degree()
                                       - ball_to_opp.th().degree());
            if (diffrence_angle > 90.0 )
            {
                continue;
            }

            const rcsc::Line2D perpendicular_mate_ball_line =
                ball_line.perpendicular((*mate)->pos());
            intersection = rcsc::Line2D::intersection(ball_line,
                                                      perpendicular_mate_ball_line);
            mate_to_intersection_dist =
                (*mate)->pos().dist(intersection) 
                - rcsc::ServerParam::i().defaultKickableArea();
            mate_step = static_cast<int>(mate_to_intersection_dist / player_dash_speed + 0.5);
            ball_step = 0;
            ball_dist = 0.0;
            while(ball_dist < ball_to_intersection_dist)
            {
                ball_step++;
                ball_dist = rcsc::calc_sum_geom_series(wm.ball().vel(),
                                                       rcsc::ServerParam::i().ballDecay(),
                                                       ball_step);
            }
            //std::cout<<"number = "<<
            if(mate_step < ball_step)
            {
                return false;
            }
        }
        return true;
    }

#if 0
        rcsc::dlog.addText( rcsc::Logger::PASS,
			    "______ opp%d(%.1f %.1f) reach cycle by util = %d.",
			    (*it)->unum(),
			    (*it)->pos().x, (*it)->pos().y, cycle );
#endif


        rcsc::Vector2D ball_to_opp = (*it)->pos();
        ball_to_opp -= wm.ball().pos();
        ball_to_opp -= first_vel;
        ball_to_opp.rotate( minus_target_angle );

        if ( 0.0 < ball_to_opp.x && ball_to_opp.x < target_dist )
        {
            double opp2line_dist = ball_to_opp.absY();
            opp2line_dist -= virtual_dash;
            opp2line_dist -= rcsc::ServerParam::i().defaultKickableArea();
            opp2line_dist -= 0.1;

            if ( opp2line_dist < 0.0 )
            {
#ifdef DEBUG
                rcsc::dlog.addText( rcsc::Logger::PASS,
				    "______ opp%d(%.1f %.1f) can reach pass line. rejected. vdash=%.1f",
				    (*it)->unum(),
				    (*it)->pos().x, (*it)->pos().y,
				    virtual_dash );
#endif
                return false;
            }

            const double ball_steps_to_project
                = rcsc::calc_length_geom_series(next_speed,
                                                ball_to_opp.x,
                                                rcsc::ServerParam::i().ballDecay() );
            //= move_step_for_first(ball_to_opp.x,
            //next_speed,
            //ServerParam::i().ballDecay());
            if ( ball_steps_to_project < 0.0
                 || opp2line_dist / player_dash_speed < ball_steps_to_project )
            {
#ifdef DEBUG
                rcsc::dlog.addText( rcsc::Logger::PASS,
				    "______ opp%d(%.1f %.1f) can reach pass line."
				    " ball reach step to project= %.1f",
				    (*it)->unum(),
				    (*it)->pos().x, (*it)->pos().y,
				    ball_steps_to_project );
#endif
                return false;
            }
#ifdef DEBUG
            rcsc::dlog.addText( rcsc::Logger::PASS,
				"______ opp%d(%.1f %.1f) cannot intercept.",
				(*it)->unum(),
				(*it)->pos().x, (*it)->pos().y );
#endif
        }
    }
*/ 
/*  return false;
}
*/

bool
Obake_Analysis::checkExistOpponent(rcsc::PlayerAgent * agent,
                                   const rcsc::Vector2D &base_pos,
                                   const rcsc::Vector2D &target_pos,
                                   const double &angle,
                                   const double &r)
{
    const rcsc::WorldModel & wm = agent->world();
    const double half_angle = angle / 2.0;
    rcsc::Vector2D first_apex = base_pos;
    rcsc::Vector2D second_apex  = target_pos;
    rcsc::Vector2D third_apex = target_pos;
    rcsc::Vector2D v = target_pos - base_pos;
    double tmp = v.length() * std::abs(std::tan(M_PI *  half_angle / 180));
    v.setLength(tmp);
    v.rotate(90.0);
    second_apex += v;
    v.rotate(180.0);
    third_apex += v;

    const rcsc::Triangle2D triangle(first_apex, second_apex, third_apex);
    const rcsc::Circle2D circle(target_pos, r);
    if(wm.existOpponentIn(triangle, 10, true)
       || wm.existOpponentIn(circle, 10, true))
    {
        return true;
    }

    return false;
}


bool
Obake_Analysis::checkExistOpponent(rcsc::PlayerAgent * agent,
				   const rcsc::Vector2D &base_pos,
				   const rcsc::Vector2D &target_pos,
				   const double &angle)
{
    const rcsc::WorldModel & wm = agent->world();
    const double half_angle = angle / 2.0;
    rcsc::Vector2D first_apex = base_pos;
    rcsc::Vector2D second_apex  = target_pos;
    rcsc::Vector2D third_apex = target_pos;
    rcsc::Vector2D v = target_pos - base_pos;
    double tmp = v.length() * std::abs(std::tan(M_PI *  half_angle / 180));
    v.setLength(tmp);
    v.rotate(90.0);
    second_apex += v;
    v.rotate(180.0);
    third_apex += v;
    const rcsc::Triangle2D triangle(first_apex, second_apex, third_apex);
    if(wm.existOpponentIn(triangle, 10, true))
    {
        return true;
    }

    return false;
}

bool
Obake_Analysis::checkSafeDefenseSituation(rcsc::PlayerAgent * agent)
{
    const rcsc::WorldModel & wm = agent->world();
    const double defense_line = getMateDefenseLine(agent);
    int center_back_number;
    bool exist_near_center_back;
    center_back_number =(wm.self().unum() == 4) ? 2 : 3;
    exist_near_center_back = false;
    const rcsc::AbstractPlayerObject * center_back =
        wm.teammate(center_back_number);
    if(center_back)
    {
        if(std::abs((center_back)->pos().x - defense_line) <= 5.0)
        {
            exist_near_center_back = true;
        }
    }
    const rcsc::PlayerPtrCont::const_iterator
        mate_end = wm.teammatesFromSelf().end();
    for(rcsc::PlayerPtrCont::const_iterator
        mate = wm.teammatesFromSelf().begin();
        mate != mate_end;
        mate++)
    {
        if((*mate)->pos().x < wm.ball().pos().x
           && std::abs((*mate)->pos().x - defense_line) <= 5.0)
        {
            if(exist_near_center_back)
            {
                if(wm.self().unum() == 4)
                {
                    if((*mate)->pos().y < (center_back)->pos().y)
                    {
                        return true;
                    }
                }
                else if(wm.self().unum() == 5)
                {
                    if((*mate)->pos().y > (center_back)->pos().y)
                    {
                        return true;
                    }
                }
            }
            else
            {
                if(std::abs((*mate)->pos().y - wm.ball().pos().y) <= 7.5)
                {
                    return true;
                }
            }
        }
    }
    const rcsc::PlayerPtrCont::const_iterator
        opp_end = wm.opponentsFromSelf().end();
    for(rcsc::PlayerPtrCont::const_iterator 
        opp = wm.opponentsFromSelf().begin();
        opp != opp_end;
        opp++)
    {
        if((*opp)->pos().x < wm.ball().pos().x
           && (*opp)->pos().x > defense_line - 8.0
           && (*opp)->distFromBall() > rcsc::ServerParam::i().defaultKickableArea() * 2)
        {
            if(exist_near_center_back)
            {
                if(wm.self().unum() == 4)
                {
                    if((*opp)->pos().y < (center_back)->pos().y)
                    {
                        return false;
                    }
                }
                else if(wm.self().unum() == 5)
                {
                    if((*opp)->pos().y > (center_back)->pos().y)
                    {
                        return false;
                    }
                }
            }
            else
            {
                if(std::abs((*opp)->pos().y - wm.ball().pos().y) <= 7.5)
                {
                    return false;
                }
            }
        }
    }
    return true;
}


bool
Obake_Analysis::checkTeammateIn(rcsc::PlayerAgent * agent,
                                 const int number,
                                 const rcsc::Vector2D &left_top,
                                 const double &length,
                                 const double &width)
{
    const rcsc::WorldModel & wm = agent->world();
    const rcsc::AbstractPlayerObject * mate = wm.teammate(number);
    if(mate)
    {
        if((mate)->pos().x >= left_top.x 
           && (mate)->pos().x <= (left_top.x + length)
           && (mate)->pos().y >= left_top.y
           && (mate)->pos().y <= (left_top.y + width))
        {
            return true;
        }
    }
    return false;
}

bool
Obake_Analysis::checkBackOfTheSideForward(rcsc::PlayerAgent * agent)
{
    const rcsc::WorldModel & wm = agent->world();
    const rcsc::AbstractPlayerObject * top_side_forward = wm.teammate(7);
    const rcsc::AbstractPlayerObject * bottom_side_forward = wm.teammate(8);
    if(top_side_forward && bottom_side_forward)
    {
        if(wm.self().pos().x < (top_side_forward)->pos().x
           && wm.self().pos().x < (bottom_side_forward)->pos().x)
        {
            return true;
        }
    }
    return false;
}

bool
Obake_Analysis::checkDefenseLine(rcsc::PlayerAgent * agent)
{
    const rcsc::WorldModel & wm = agent->world();
    const rcsc::AbstractPlayerObject * top_center_back = wm.teammate(2);
    const rcsc::AbstractPlayerObject * bottom_center_back = wm.teammate(3);
    const rcsc::AbstractPlayerObject * top_side_back = wm.teammate(4);
    const rcsc::AbstractPlayerObject * bottom_side_back = wm.teammate(5);
    if(wm.self().unum() == 4)
    {
        if(bottom_center_back && top_center_back
           && bottom_side_back)
        {
            if(wm.self().pos().y < (top_center_back)->pos().y
               && (top_center_back)->pos().y < (bottom_center_back)->pos().y
               && (bottom_center_back)->pos().y < (bottom_side_back)->pos().y)
            {
                return true;
            }
        }
    }
    else if(wm.self().unum() == 2)
    {
        if(top_side_back && bottom_center_back 
           && bottom_side_back)
        {
            if((top_side_back)->pos().y < wm.self().pos().y
               && wm.self().pos().y < (bottom_center_back)->pos().y
               && (bottom_center_back)->pos().y < (bottom_side_back)->pos().y)
            {
                return true;
            }
        }
    }
    else if(wm.self().unum() == 3)
    {
        if(top_side_back && top_center_back
           && bottom_side_back)
        {
            if((top_side_back)->pos().y < (top_center_back)->pos().y
               && (top_center_back)->pos().y < wm.self().pos().y
               && wm.self().pos().y < (bottom_side_back)->pos().y)
            {
                return true;
            }

        }
    }
    else
    {
        if(top_side_back && top_center_back
           && bottom_center_back)
        {
            if((top_side_back)->pos().y < (top_center_back)->pos().y
               && (top_center_back)->pos().y < (bottom_center_back)->pos().y
               && (bottom_center_back)->pos().y < wm.self().pos().y)
            {
                return true;
            }
        }
    }

    return false;
}

bool
Obake_Analysis::checkExistShootCourse(rcsc::PlayerAgent * agent,
                                      const rcsc::Vector2D &receiver_pos,
                                      const double &angle,
                                      const double &r)
{
    std::vector<rcsc::Vector2D> target_vector;
    /*
    const rcsc::Vector2D goal_top(rcsc::ServerParam::i().pitchHalfLength(),
                                  -rcsc::ServerParam::i().goalHalfWidth() + 1.5);
    const rcsc::Vector2D goal_middle(rcsc::ServerParam::i().pitchHalfLength(),
                                     0.0);
    const rcsc::Vector2D goal_bottom(rcsc::ServerParam::i().pitchHalfLength(),
                                     rcsc::ServerParam::i().goalHalfWidth() - 1.5);
    target_vector.push_back(goal_top);
    target_vector.push_back(goal_middle);
    target_vector.push_back(goal_bottom);
    */
    
    if(receiver_pos.absY() < rcsc::ServerParam::i().goalHalfWidth() - 1.0)
    {
        target_vector.push_back(rcsc::Vector2D(rcsc::ServerParam::i().pitchHalfLength(),
                                               receiver_pos.y));
    }
    /****/
    rcsc::Vector2D shoot_point(rcsc::ServerParam::i().pitchHalfLength(),
                               rcsc::ServerParam::i().goalHalfWidth() - 1.5);
    while(shoot_point.y >= -rcsc::ServerParam::i().goalHalfWidth() + 1.5)
    {
        target_vector.push_back(shoot_point);
        shoot_point.y -= 3.0;
    }
    /****/
    const std::vector<rcsc::Vector2D>::const_iterator end = target_vector.end();
    for(std::vector<rcsc::Vector2D>::const_iterator target = target_vector.begin();
        target != end;
        target++)
    {
        if(checkShootCourse(agent,
                            receiver_pos,
                            (*target),
                            angle,
                            r))
        {
            return true;
        }
    }
    return false;
}


bool
Obake_Analysis::checkShootCourse(rcsc::PlayerAgent * agent,
                                 const rcsc::Vector2D &receiver_pos,
                                 const rcsc::Vector2D &target_pos,
                                 const double &angle,
                                 const double &r)
{
    const rcsc::WorldModel & wm = agent->world();
    const rcsc::Line2D shoot_line(receiver_pos, target_pos);
    const rcsc::PlayerObject * opp_goalie = agent->world().getOpponentGoalie();  
    if(opp_goalie)
    {
        const rcsc::Vector2D opp_goalie_next = (opp_goalie)->pos() + (opp_goalie)->vel();
        const rcsc::Line2D perpendicular_opp_goalie_shoot_line = shoot_line.perpendicular(opp_goalie_next);
        const rcsc::Vector2D intersection = rcsc::Line2D::intersection(shoot_line,
                                                                       perpendicular_opp_goalie_shoot_line);
        //if(intersection.dist(opp_goalie_next) <= rcsc::ServerParam::i().catchableArea())
                if(intersection.dist(opp_goalie_next) <= rcsc::ServerParam::i().catchableArea() + 1.5)
        {
            //std::cout<<"goalie dist = "<<intersection.dist(opp_goalie_next)
            //<<"goalie = "<<opp_goalie_next<<", "<<receiver_pos<<", "<<target_pos<<std::endl;
            return false;
        }
    }
    const double half_angle = angle / 2;
    rcsc::Vector2D vector_target = target_pos - receiver_pos;
    rcsc::Vector2D vertical_vector = vector_target;
    rcsc::Vector2D first_apex = receiver_pos;
    rcsc::Vector2D second_apex  = target_pos;
    rcsc::Vector2D third_apex = target_pos;
    rcsc::Vector2D v = target_pos - receiver_pos;
    double tmp = v.length() * std::abs(std::tan(half_angle * 3.14 / 180));
    v.setLength(tmp);
    v.rotate(90.0);
    second_apex += v;
    v.rotate(180.0);
    third_apex += v;
    const rcsc::Triangle2D shoot_course_triangle(first_apex, second_apex, third_apex);
    const rcsc::Circle2D shoot_course_circle(target_pos, r);
    if(!wm.existOpponentIn(shoot_course_triangle, 10, true)
       && !wm.existOpponentIn(shoot_course_circle, 10, true))
    {       
        if(opp_goalie)
        {
            rcsc::Vector2D receiver_to_opp_goalie_vector = (opp_goalie)->pos() - receiver_pos;
            const double difference_angle = std::abs(receiver_to_opp_goalie_vector.th().degree() - vector_target.th().degree());
            //std::cout<<"difference_angle"<<difference_angle<<std::endl;
            if(difference_angle <= half_angle + 3.0)
            {
                return false;
            }
        }
        return true;
    }

    return false;
}

bool
Obake_Analysis::checkPassCourse(rcsc::PlayerAgent * agent,
                                const rcsc::Vector2D &passer_pos,
                                const rcsc::Vector2D &receiver_pos,
                                const double &angle)
{
    const rcsc::WorldModel & wm = agent->world();
    const rcsc::Triangle2D pass_course = getTriangle(passer_pos,
                                                     receiver_pos,
                                                     angle);
    if(!wm.existOpponentIn(pass_course, 
                           10,
                           true))
    {
        return true;
    }
    return false;
}

int
Obake_Analysis::getAssistCourseNumber(rcsc::PlayerAgent * agent,
                                      const rcsc::PlayerObject * passer,
                                      const double &angle,
                                      const double &max_pass_dist)
{
    const rcsc::WorldModel & wm = agent->world();
    const double r = 2.5;
    const double shoot_angle = 25.0;
    int number;
    number = 0;
    if(passer)
    {
        //std::cout<<"passer = "<<(passer)->unum()<<std::endl;
        const rcsc::PlayerPtrCont::const_iterator end = wm.teammatesFromBall().end();
        for(rcsc::PlayerPtrCont::const_iterator mate = wm.teammatesFromBall().begin();
            mate != end;
            mate++)
        {
            if((*mate)->unum() != (passer)->unum()
               && /*Obake_Analysis().checkExistOppPenaltyAreaIn((*mate)->pos())*/
                ObakeStrategy::getArea((*mate)->pos()) == ObakeStrategy::ShootChance)
            {
                /*shoot_angle = 30.0;
                if(ObakeStrategy::getArea((*mate)->pos()) == ObakeStrategy::ShootChance)
                {
                    shoot_angle -= 5.0;
                    }*/
                if(Obake_Analysis().checkExistShootCourse(agent,
                                                          (*mate)->pos(),
                                                          shoot_angle,
                                                          r))
                {
                    if(checkPassCourse(agent,
                                       (passer)->pos(),
                                       (*mate)->pos(),
                                       angle)
                       && (*mate)->pos().dist((passer)->pos()) <= max_pass_dist)
                    {
                        //std::cout<<"mate = "<<(*mate)->unum()<<" ";
                        number++;
                    }
                }   
            }    
        }        
        //std::cout<<"number = "<<number<<std::endl;
    }
    return number;
}

int
Obake_Analysis::getAssistCourseNumber(rcsc::PlayerAgent * agent,
                                      const rcsc::Vector2D &base_pos,
                                      const double &angle,
                                      const double max_pass_dist,
                                      const bool except_nearest_mate)
{
    const rcsc::WorldModel & wm = agent->world();
    const double r = 2.5;
    const double shoot_angle = 25.0;
    int number;
    number = 0;
    bool check_front = false;
    const rcsc::PlayerPtrCont::const_iterator end = wm.teammatesFromBall().end();
    for(rcsc::PlayerPtrCont::const_iterator mate = wm.teammatesFromBall().begin();
        mate != end;
        mate++)
    {
        if(!check_front
           && except_nearest_mate)
        {
            check_front = true;
            continue;
        }
        if(ObakeStrategy::getArea((*mate)->pos()) == ObakeStrategy::ShootChance)
        {
            if(Obake_Analysis().checkExistShootCourse(agent,
                                                      (*mate)->pos(),
                                                      shoot_angle,
                                                      r))
            {
                if(checkPassCourse(agent,
                                   base_pos,
                                   (*mate)->pos(),
                                   angle)
                   && (*mate)->pos().dist(base_pos) <= max_pass_dist)
                {
                    number++;
                }
            }   
        }    
    }        
    return number;
}

/*!
front_x is the distance from the defenseline
*/
int
Obake_Analysis::getBackNumber(rcsc::PlayerAgent * agent,
                              const double &front_x)
{
    const rcsc::WorldModel & wm = agent->world();
    int count, i;
    const double front = getMateDefenseLine(agent) + front_x;
    count = 0;
    for(i=2; i<=5; i++)
    {
        if(wm.self().unum() == i)
        {
            count++;
        }
        else
        {
            const rcsc::AbstractPlayerObject * back = wm.teammate(i);
            if(back)
            {
                if((back)->pos().x <= front)
                {
                    count++;
                    if((back)->pos().x > wm.ball().pos().x)
                    {
                        count--;
                    }
                }
            }
        }
    }
    return count;
}

int
Obake_Analysis::getBallKeeperNumber(rcsc::PlayerAgent * agent,
                                    const int count_thr)
{
    const rcsc::WorldModel & wm = agent->world();
    int number = 0;
    const rcsc::PlayerObject * ball_nearest_mate = wm.getTeammateNearestToBall(count_thr);
    const rcsc::PlayerObject * ball_nearest_opp = wm.getOpponentNearestToBall(count_thr);
    if(wm.existKickableTeammate())
    {
        if(ball_nearest_mate)
        {
            number = (ball_nearest_mate)->unum();
        }
    }
    else if(wm.existKickableOpponent())
    {
        if(ball_nearest_opp)
        {
            number = -(ball_nearest_opp)->unum();
        }
    }
    return number;
}

std::vector<rcsc::Vector2D>
Obake_Analysis::getDribbleTargetPointVector(rcsc::PlayerAgent * agent,
                                            const rcsc::Vector2D &search_point,
                                            const double &dribble_dist)
{
    const rcsc::Vector2D front_opp_goal(rcsc::ServerParam::i().pitchHalfLength() - 9.5, 0.0);
    rcsc::Vector2D near_centering_point(rcsc::ServerParam::i().pitchHalfLength() - rcsc::ServerParam::i().goalAreaLength(),
                                        rcsc::ServerParam::i().goalAreaHalfWidth());
    rcsc::Vector2D far_centering_point(rcsc::ServerParam::i().pitchHalfLength(), 
                                       (rcsc::ServerParam::i().goalAreaHalfWidth()
                                        + rcsc::ServerParam::i().penaltyAreaHalfWidth()) / 2.0);
    if(search_point.y < 0.0)
    {
        near_centering_point.y *= -1.0;
        far_centering_point.y *= -1.0;
    }
    rcsc::Vector2D additional_vector, target_point;
    std::vector<rcsc::Vector2D> additional_vector_vector;

    additional_vector = rcsc::Vector2D(dribble_dist, 0.0);
    additional_vector_vector.push_back(additional_vector);
    if(search_point.x >= rcsc::ServerParam::i().theirPenaltyAreaLineX())
    {
        additional_vector = front_opp_goal - search_point;
        additional_vector.setLength(dribble_dist);
        additional_vector_vector.push_back(additional_vector);

        additional_vector = near_centering_point - search_point;
        additional_vector.setLength(dribble_dist);
        additional_vector_vector.push_back(additional_vector);
    
        additional_vector = far_centering_point - search_point;
        additional_vector.setLength(dribble_dist);
        additional_vector_vector.push_back(additional_vector);
    }
    std::vector<rcsc::Vector2D> dribble_target_point_vector;
    const std::vector<rcsc::Vector2D>::const_iterator
        v_end = additional_vector_vector.end();
    for(std::vector<rcsc::Vector2D>::const_iterator
        v = additional_vector_vector.begin();
        v != v_end;
        v++)
    {
        target_point = search_point + (*v);
        dribble_target_point_vector.push_back(target_point);
    }

    return dribble_target_point_vector;
}


/*
except goalie and self
*/
double
Obake_Analysis::getLastMateDefenseLine(rcsc::PlayerAgent * agent)
{
    const rcsc::WorldModel & wm = agent->world();
    double last_defense_line = 100.0;
    const rcsc::PlayerPtrCont::const_iterator 
        mate_end = wm.teammatesFromSelf().end();
    for(rcsc::PlayerPtrCont::const_iterator
        mate = wm.teammatesFromSelf().begin();
        mate != mate_end;
        mate++)
    {
        if((*mate)->pos().x < last_defense_line)
        {
            if(!(*mate)->goalie())
            {
                last_defense_line = (*mate)->pos().x;
            }
        }
    }
    return last_defense_line;
}

double
Obake_Analysis::getMateDefenseLine(rcsc::PlayerAgent * agent)
{
    const rcsc::WorldModel & wm = agent->world();
    const rcsc::AbstractPlayerObject * top_center_back = wm.teammate(2);
    const rcsc::AbstractPlayerObject * bottom_center_back = wm.teammate(3);
    const rcsc::AbstractPlayerObject * top_side_back = wm.teammate(4);
    const rcsc::AbstractPlayerObject * bottom_side_back = wm.teammate(5);    
    std::list<double> x_list;
    double sum = 0.0;
    if(top_center_back)
    {
        x_list.push_back((top_center_back)->pos().x);
        sum += (top_center_back)->pos().x;
    }
    if(bottom_center_back)
    {
        x_list.push_back((bottom_center_back)->pos().x);
        sum += (bottom_center_back)->pos().x;
    }
    if(top_side_back)
    {
        x_list.push_back((top_side_back)->pos().x);
        sum += (top_side_back)->pos().x;
    }
    if(bottom_side_back)
    {
        x_list.push_back((bottom_side_back)->pos().x);
        sum += (bottom_side_back)->pos().x;
    }
    x_list.sort();
    double defense_line = -100.0;
    if(x_list.size() >= 1)
    {
        const double average = sum / x_list.size();
        std::list<double>::const_iterator xp = x_list.begin();
        const std::list<double>::const_iterator end = x_list.end();
        while(xp != end)
        {
            if(std::abs((*xp) - average) <= 5.0)
            {
                defense_line = (*xp) ;
                break;
            }
            xp++;
        }
    }
    return defense_line;
}
/*
double
Obake_Analysis::getOppDefenseLine(rcsc::PlayerAgent * agent)
{

}
*/
/*!
This function counts the number of opponents
in the specific circlular area
*/
int
Obake_Analysis::getOppNumber(rcsc::PlayerAgent * agent,
                             const rcsc::Vector2D &center,
                             const double &r)
{
    const rcsc::WorldModel & wm = agent->world();
    int number = 0;
    double max_dist;
    rcsc::Vector2D v = center - wm.self().pos();
    max_dist = v.length() + r;
    const rcsc::PlayerPtrCont::const_iterator end = wm.opponentsFromSelf().end();
    for(rcsc::PlayerPtrCont::const_iterator opp = wm.opponentsFromSelf().begin();
        opp != end;
        ++opp)
    {
        if((*opp)->pos().dist(center) <= r)
        {
            number++;
        }
        else if((*opp)->pos().dist(wm.self().pos()) > max_dist)
        {
            break;
        }
    }
    return number;
}

/*!
This function counts the number of opponents 
in the specific area 
*/
int 
Obake_Analysis::getOppNumber(rcsc::PlayerAgent * agent,
                             const double &front_x,
                             const double &back_x)
{
    int number;
    double  max_dist, front_difference, back_difference, top_difference, bottom_difference;
    const rcsc::WorldModel & wm = agent->world();
    rcsc::Vector2D v(0.0, 0.0);
    front_difference = std::abs(front_x);
    back_difference = std::abs(back_x);
    top_difference = std::abs(rcsc::ServerParam::i().pitchHalfWidth() - wm.self().pos().y);
    bottom_difference = std::abs(-rcsc::ServerParam::i().pitchHalfWidth() - wm.self().pos().y);
    v.x = (front_difference > back_difference) ? front_difference : back_difference;
    v.y = (top_difference > bottom_difference) ? top_difference : bottom_difference;
    max_dist = v.length();
    number = 0;
    const rcsc::PlayerPtrCont::const_iterator end = wm.opponentsFromSelf().end();
    for(rcsc::PlayerPtrCont::const_iterator opp = wm.opponentsFromSelf().begin();
        opp != end;
        ++opp)
    {
        if((*opp)->pos().x <= wm.self().pos().x + front_x 
           && (*opp)->pos().x >= wm.self().pos().x - back_x)
        {
            number++;
        }  
        else if((*opp)->pos().dist(wm.self().pos()) > max_dist)
        {
            break;
        }
    }
    return number;
}

/*!
This function counts the number of opponents 
in the specific rectangular area 
*/
int
Obake_Analysis::getOppNumber(rcsc::PlayerAgent * agent,
                             const rcsc::Vector2D &left_top,
                             const double &length,
                             const double &width)
{
    const rcsc::WorldModel & wm = agent->world();
    int number = 0;
    double max_r;
    rcsc::Vector2D left_bottom(left_top.x, left_top.y + width);
    rcsc::Vector2D right_top(left_top.x + length, left_top.y);
    rcsc::Vector2D right_bottom(right_top.x, right_top.y + width);
    max_r = (left_top - wm.ball().pos()).length();
    if(max_r < (left_bottom - wm.ball().pos()).length())
    {
        max_r = (left_bottom - wm.ball().pos()).length();
    }
    if(max_r < (right_top  - wm.ball().pos()).length())
    {
        max_r = (right_top  - wm.ball().pos()).length();
    }
    if(max_r < ( right_bottom - wm.ball().pos()).length())
    {
        max_r = (right_bottom - wm.ball().pos()).length();
    }
    const rcsc::PlayerPtrCont::const_iterator end = wm.opponentsFromBall().end();
    for(rcsc::PlayerPtrCont::const_iterator opp = wm.opponentsFromBall().begin();
        opp != end;
        ++opp)
    {
        if(((*opp)->pos().x >= left_top.x && (*opp)->pos().x <= right_top.x)
           &&((*opp)->pos().y >= left_top.y && (*opp)->pos().y <= left_bottom.y))
        {
            number++;
        }
        else if((*opp)->pos().dist(wm.ball().pos()) > max_r)
        {
            break;
        }
    }    
    return number;
}

int
Obake_Analysis::getOppNumberInOurPenaltyArea(rcsc::PlayerAgent * agent)
{
    const rcsc::Vector2D left_top(-rcsc::ServerParam::i().pitchHalfLength(),
                                  -rcsc::ServerParam::i().penaltyAreaHalfWidth());
    const double length = rcsc::ServerParam::i().penaltyAreaLength();
    const double width = rcsc::ServerParam::i().penaltyAreaWidth();
    int opp_number = getOppNumber(agent,
                                  left_top,
                                  length,
                                  width);
    return opp_number;
}
/*!
This function counts the number of opponents 
in the specific four areas 

    |
  2 | 3
---------->
  1 | 0
   \|/
*/
std::list<int>
Obake_Analysis::getOppNumberAroundBall(rcsc::PlayerAgent * agent)
                                     
{
    const rcsc::WorldModel & wm = agent->world();
    const double max_pass_dist = 25.0;
    rcsc::Vector2D v;
    const int all = 4;
    int region[all] = {0, 0, 0, 0};
    double angle;
    const rcsc::PlayerPtrCont::const_iterator end = wm.opponentsFromBall().end();
    for(rcsc::PlayerPtrCont::const_iterator opp = wm.opponentsFromBall().begin();
        opp != end;
        ++opp)
    {
        if((*opp)->distFromBall() <= max_pass_dist)
        {

            v = (*opp)->pos() - wm.ball().pos();
            angle = rcsc::AngleDeg::normalize_angle(v.th().degree());
            if(angle >= 0.0 && angle <= 90.0)
            {
                region[0]++;
                
            }
            else if(angle > 90.0 && angle <= 180.0)
            {
                region[1]++;
            }
            else if(angle < 0.0 && angle > -90.0)
            {
                region[3]++;
            }
            else
            {
                region[2]++;
            }
        }  
        else 
        {
            break;
        }
    }
    int i;
    std::list<int> opp_list;
    for(i=0; i<all; i++)
    {
        opp_list.push_back(region[i]);
    }
    return opp_list;
}

int 
Obake_Analysis::getPassCourseNumber(rcsc::PlayerAgent * agent,
                                   const rcsc::PlayerObject * passer,
                                   const double &angle,
                                   const double &max_pass_dist)
{
    const rcsc::WorldModel & wm = agent->world();
    int number = 0;
    if(passer)
    {
        const rcsc::PlayerPtrCont::const_iterator end = wm.teammatesFromBall().end();
        for(rcsc::PlayerPtrCont::const_iterator mate = wm.teammatesFromBall().begin();
            mate != end;
            mate++)
        {
            if((*mate)->unum() != (passer)->unum() 
               && (*mate)->pos().dist((passer)->pos()) <= max_pass_dist)
            {
                if(checkPassCourse(agent,
                                   (passer)->pos(),
                                   (*mate)->pos(),
                                   angle))
                {
                    number++;
                }
            }
            
        }
    }
    return number;
}

std::list<int>
Obake_Analysis::getPassCourseNumberList(rcsc::PlayerAgent * agent,
                                        const rcsc::PlayerObject * passer,
                                        const double &angle,
                                        const double &max_pass_dist)
{
    const rcsc::WorldModel & wm = agent->world();
    std::list<int> number_list;
    if(passer)
    {
        const rcsc::PlayerPtrCont::const_iterator end = wm.teammatesFromBall().end();
        for(rcsc::PlayerPtrCont::const_iterator mate = wm.teammatesFromBall().begin();
            mate != end;
            mate++)
        {
            if((*mate)->unum() != (passer)->unum() 
               && (*mate)->pos().dist((passer)->pos()) <= max_pass_dist)
            {
                if(checkPassCourse(agent,
                                   (passer)->pos(),
                                   (*mate)->pos(),
                                   angle))
                {
                    number_list.push_back((*mate)->unum());
                }
            }
            
        }
    }
    return number_list;
}

rcsc::Triangle2D
Obake_Analysis::getTriangle(const rcsc::Vector2D &base_pos,
                           const rcsc::Vector2D &target_pos,
                           const double &angle)
{
    const double half_angle = angle / 2.0;
    const rcsc::Vector2D first_apex = base_pos;
    double length, tmp;
    rcsc::Vector2D v = target_pos - base_pos;
    length = (v).length();
    tmp = length * std::tan(half_angle * 3.14 / 180);
    v.rotate(90);
    v.setLength(tmp);
    const rcsc::Vector2D second_apex = target_pos + v;
    v.rotate(180);
    const rcsc::Vector2D third_apex = target_pos + v;
    const rcsc::Triangle2D triangle(first_apex, 
                                    second_apex,
                                    third_apex);
    return triangle;
}

int
Obake_Analysis::getMateBackNumber(rcsc::PlayerAgent * agent,
                                  const rcsc::Vector2D &left_top,
                                  const double &length,
                                  const double &width)
{
    const rcsc::WorldModel & wm = agent->world();
    int i, number;
    number = 0;
    /* if(wm.self().unum()==3)
    {
        std::cout<<"number:";
    }
    */
    for(i=2; i<=5; i++)
    {
        if(wm.self().unum() == i)
        {
            continue;

        }
        if(checkTeammateIn(agent, i, left_top, length, width))
        {
            number++;
            /*     if(wm.self().unum() == 3)
           {
               std::cout<<i<<",";
               }*/
        }
    }
    if(wm.self().unum() == 2
       || wm.self().unum() == 3
       || wm.self().unum() == 4
       || wm.self().unum() == 5)
    {
        if(wm.self().pos().x >= left_top.x
           && wm.self().pos().x <= (left_top.x + length)
           && wm.self().pos().y >= left_top.y 
           && wm.self().pos().y <= (left_top.y + width))
        {
            number++;
/*           if(wm.self().unum() == 3)
           {
               std::cout<<"self,";
           }
*/
        }
    }
    return number;               
}

/*!
This function counts the number of teammates
in the specific circlular area
*/
int
Obake_Analysis::getMateNumber(rcsc::PlayerAgent * agent,
                              const rcsc::Vector2D &center,
                              const double &r)
{
   const rcsc::WorldModel & wm = agent->world();
   int number = 0;
   double max_dist;
   rcsc::Vector2D v = center - wm.ball().pos();
   max_dist = v.length() + r;
   v = wm.self().pos() - center;
   const rcsc::PlayerPtrCont::const_iterator end = wm.teammatesFromBall().end();
   for(rcsc::PlayerPtrCont::const_iterator mate = wm.teammatesFromBall().begin();
       mate != end;
       ++mate)
   {
     if((*mate)->pos().dist(center) <= r)
     {
         number++;
     }
     else if((*mate)->distFromBall() > max_dist)
     {
         break;
     }
   }
   return number;
}

/*!
This function counts the number of teammates 
in the specific rectangular area 
*/
int
Obake_Analysis::getMateNumber(rcsc::PlayerAgent * agent,
                              const rcsc::Vector2D &left_top,
                              const double &length,
                              const double &width)
{
    const rcsc::WorldModel & wm = agent->world();
    int number = 0;
    double max_dist;
    const rcsc::Vector2D left_bottom(left_top.x, left_top.y + width);
    const rcsc::Vector2D right_top(left_top.x + length, left_top.y);
    const rcsc::Vector2D right_bottom(right_top.x, right_top.y + width);
    max_dist = (left_top - wm.ball().pos()).length();
    if(max_dist < (left_bottom - wm.ball().pos()).length())
    {
        max_dist = (left_bottom - wm.ball().pos()).length();
    }
    if(max_dist < (right_top  - wm.ball().pos()).length())
    {
        max_dist = (right_top  - wm.ball().pos()).length();
    }
    if(max_dist < ( right_bottom - wm.ball().pos()).length())
    {
        max_dist = (right_bottom - wm.ball().pos()).length();
    }
    const rcsc::PlayerPtrCont::const_iterator end = wm.teammatesFromBall().end();
    for(rcsc::PlayerPtrCont::const_iterator mate = wm.teammatesFromBall().begin();
        mate != end;
        ++mate)
    {
        if(((*mate)->pos().x >= left_top.x && (*mate)->pos().x <= right_top.x)
           &&((*mate)->pos().y >= left_top.y && (*mate)->pos().y <= left_bottom.y))
        {
            number++;
        }
        else if((*mate)->distFromBall() > max_dist)
        {
            break;
        }
    }    
    return number;
}

int
Obake_Analysis::getMateNumber(rcsc::PlayerAgent * agent,
                              const rcsc::Vector2D &pass_point)
{
    const rcsc::WorldModel & wm = agent->world();
//    const double max_difference = 15.0;
    double min_dist;
    int number;
    number = 0;
    min_dist = 100.0;
    const rcsc::PlayerPtrCont::const_iterator
        mate_end = wm.teammatesFromBall().end();
    for(rcsc::PlayerPtrCont::const_iterator
            mate = wm.teammatesFromBall().begin();
        mate != mate_end;
        mate++)
    {
/*        if((*mate)->pos().dist(pass_point) > max_difference)
        {
            break;
        }
*/
        if((*mate)->pos().dist(pass_point) < min_dist)
        {
            min_dist = (*mate)->distFromBall();
            number = (*mate)->unum();
        }
    }
    return number;
}

int
Obake_Analysis::getMateNumberInOurPenaltyArea(rcsc::PlayerAgent * agent,
                                              const bool except_goalie)
{
    const rcsc::WorldModel & wm = agent->world();
    const rcsc::Vector2D left_top(-rcsc::ServerParam::i().pitchHalfLength(),
                                  -rcsc::ServerParam::i().penaltyAreaHalfWidth());
    const double length = rcsc::ServerParam::i().penaltyAreaLength();
    const double width = rcsc::ServerParam::i().penaltyAreaWidth();
    int mate_number = getMateNumber(agent,
                                    left_top,
                                    length,
                                    width);
    if(except_goalie)
    {
        const rcsc::AbstractPlayerObject * our_goalie = wm.teammate(1);
        if(our_goalie)
        {
            if(Obake_Analysis().checkExistOurPenaltyAreaIn((our_goalie)->pos()))
            {
                mate_number--;
            }
        }
    }
    
    return mate_number;
}

std::list<int> 
Obake_Analysis::getShootChanceMateNumberList(rcsc::PlayerAgent * agent,
                                             const rcsc::Vector2D &left_top,
                                             const double &length,
                                             const double &width,
                                             const double &angle,
                                             const double &r)
{
    const rcsc::WorldModel & wm = agent->world();
    std::list<int> number_list;
    double max_dist;
    const rcsc::Vector2D left_bottom(left_top.x, left_top.y + width);
    const rcsc::Vector2D right_top(left_top.x + length, left_top.y);
    const rcsc::Vector2D right_bottom(right_top.x, right_top.y + width);
    max_dist = (left_top - wm.ball().pos()).length();
    if(max_dist < (left_bottom - wm.ball().pos()).length())
    {
        max_dist = (left_bottom - wm.ball().pos()).length();
    }
    if(max_dist < (right_top  - wm.ball().pos()).length())
    {
        max_dist = (right_top  - wm.ball().pos()).length();
    }
    if(max_dist < ( right_bottom - wm.ball().pos()).length())
    {
        max_dist = (right_bottom - wm.ball().pos()).length();
    }
    const rcsc::PlayerPtrCont::const_iterator end = wm.teammatesFromBall().end();
    for(rcsc::PlayerPtrCont::const_iterator mate = wm.teammatesFromBall().begin();
        mate != end;
        ++mate)
    {
        if(((*mate)->pos().x >= left_top.x && (*mate)->pos().x <= right_top.x)
           &&((*mate)->pos().y >= left_top.y && (*mate)->pos().y <= left_bottom.y))
        {

                if(checkExistShootCourse(agent,
                                         (*mate)->pos(),
                                         angle,
                                         r))
                {
                    number_list.push_back((*mate)->unum());
                }
        }
        else if((*mate)->distFromBall() > max_dist)
        {
            break;
        }
    }    
    return number_list;
}

/*!
This function searchs dist from the nearest mate
that is nearest from the point
*/
double
Obake_Analysis::getDistFromNearestMate(rcsc::PlayerAgent * agent,
                                       const rcsc::Vector2D &point,
                                       const bool except_near_opp_defender,
                                       const bool except_role_center_or_side_back,
                                       const bool except_role_goalie)
{
    const rcsc::WorldModel & wm = agent->world();
    const double r = 3.3;
    double dist = 100.0;
    bool exist_defender, is_first_player;
    exist_defender = false;
    is_first_player = false;
    const rcsc::PlayerPtrCont::const_iterator end = wm.teammatesFromBall().end();
    for(rcsc::PlayerPtrCont::const_iterator mate = wm.teammatesFromBall().begin();
        mate != end;
        mate++)
    {
        if(except_role_goalie)
        {
            if((*mate)->goalie())
            {
                continue;
            }
        }
        if(except_role_center_or_side_back)
        {
            if((wm.self().unum() >= 2
                 && wm.self().unum() <= 4))
            {
                continue;
            }
            
        }
        if(except_near_opp_defender)
        {
            if(!exist_defender)
            {
                if(wm.ball().pos().x >= rcsc::ServerParam::i().ourPenaltyAreaLineX()
                   || wm.ball().pos().y <= rcsc::ServerParam::i().goalAreaHalfWidth())
                {
                    if((*mate)->pos().x <= wm.ball().pos().x)
                    {
                        exist_defender = true;
                        is_first_player = true;
                        continue;
                    }
                }
                else
                {
                    if((*mate)->pos().y * wm.ball().pos().y > 0.0
                       && (*mate)->pos().absY() < wm.ball().pos().y)
                    {
                        exist_defender = true;
                        is_first_player = true;
                        continue;
                    }
                }
            }
            if(!is_first_player)
            {
                is_first_player = true;
                continue;
            }
            if(dist > (*mate)->pos().dist(point)
               && exist_defender)
            {
                const rcsc::Circle2D circle((*mate)->pos(), r);
                if(!wm.existOpponentIn(circle, 10, true))
                {                    
                    dist = (*mate)->pos().dist(point);
                }
            }
        }
        else
        {
            if(dist > (*mate)->pos().dist(point))
            {
                dist = (*mate)->pos().dist(point);
            }
        }
    }
    if(wm.teammatesFromSelf().begin() == wm.teammatesFromSelf().end())
    {
        dist = -100;
    }
    return dist;
}

/*!
This function searchs dist from the nearest opp
that is nearest from the point
*/
double
Obake_Analysis::getDistFromNearestOpp(rcsc::PlayerAgent * agent,
                                      const rcsc::Vector2D &point)
{
    const rcsc::WorldModel & wm = agent->world();
    double dist = 100.0;
    const rcsc::PlayerPtrCont::const_iterator end = wm.opponentsFromSelf().end();
    for(rcsc::PlayerPtrCont::const_iterator opp = wm.opponentsFromSelf().begin();
        opp != end;
        opp++)
    {
        if(dist > (*opp)->pos().dist(point))
        {
            dist = (*opp)->pos().dist(point);
//            std::cout<<"dist = "<<dist<<std::endl;
        }
    }
    if(wm.opponentsFromSelf().begin() == wm.opponentsFromSelf().end())
    {
        dist = -100;
    }
//    std::cout<<"last dist = "<<dist<<std::endl<<std::endl;
    return dist;
}

/*!
This function counts the number of teammates 
in the specific quadrangle area 
*/
std::list<double>
Obake_Analysis::getOppYList(rcsc::PlayerAgent * agent,
                            const double &length,
                            const double &width)
{
    const rcsc::WorldModel & wm = agent->world();
    std::list<double> l;
    double half_width, max_dist;
    half_width = width / 2;
    rcsc::Vector2D v(length, half_width);
    max_dist = v.length();
    const rcsc::PlayerPtrCont::const_iterator end = wm.opponentsFromSelf().end();
    for(rcsc::PlayerPtrCont::const_iterator opp = wm.opponentsFromSelf().begin();
        opp != end;
        opp++)
    {
        v = (*opp)->pos() - wm.self().pos();
        if((*opp)->pos().dist(wm.self().pos()) > max_dist)
        {
            break;
        }
        else if((*opp)->pos().x >= wm.self().pos().x
                && (*opp)->pos().x <= wm.self().pos().x + length
                && v.absY() <= half_width)
        {
            l.push_back((*opp)->pos().y);
        }
    }
    l.sort();
    return l;
}


std::vector<double>
Obake_Analysis::getOppYVector(rcsc::PlayerAgent * agent,
                              const rcsc::Vector2D &left_top,
                              const double &length,
                              const double &width)
{
    const rcsc::WorldModel & wm = agent->world();
    std::vector<double> v;
    double max_dist;
    const rcsc::Vector2D left_bottom(left_top.x, left_top.y + width);
    const rcsc::Vector2D right_top(left_top.x + length, left_top.y);
    const rcsc::Vector2D right_bottom(right_top.x, right_top.y + width);
    max_dist = (left_top - wm.ball().pos()).length();
    if(max_dist < (left_bottom - wm.ball().pos()).length())
    {
        max_dist = (left_bottom - wm.ball().pos()).length();
    }
    if(max_dist < (right_top  - wm.ball().pos()).length())
    {
        max_dist = (right_top  - wm.ball().pos()).length();
    }
    if(max_dist < ( right_bottom - wm.ball().pos()).length())
    {
        max_dist = (right_bottom - wm.ball().pos()).length();
    }
    const rcsc::PlayerPtrCont::const_iterator end = wm.opponentsFromBall().end();
    for(rcsc::PlayerPtrCont::const_iterator opp = wm.opponentsFromBall().begin();
        opp != end;
        opp++)
    {
        if((*opp)->pos().dist(wm.self().pos()) > max_dist)
        {
            break;
        }
        else if((*opp)->pos().x >= left_top.x
                && (*opp)->pos().x <= (left_top.x + length)
                && (*opp)->pos().y >= left_top.y
                && (*opp)->pos().y <= (left_top.y + width))
        {
            v.push_back((*opp)->pos().y);
        }
    }
    sort(v.begin(), v.end());
    return v;
}

/*!
This function returns the list which contains
opponents uniform number, which are sorted
by y coordinates
*/
std::list<int>
Obake_Analysis::getOppNumberYList(rcsc::PlayerAgent * agent,
                                  const rcsc::Vector2D &left_top,
                                  const double &length,
                                  const double &width,
                                  const bool check_goalie)
{
    const rcsc::WorldModel & wm = agent->world();
    std::vector<double> opp_y_vector = getOppYVector(agent, left_top, length, width);
    std::list<int> opp_number_y_list;
    if(opp_y_vector.size() >= 1)
    {
        std::vector<double>::const_iterator opp_y_end = opp_y_vector.end();
        rcsc::PlayerPtrCont::const_iterator opp_end = wm.opponentsFromSelf().end();
        const rcsc::Vector2D left_bottom(left_top.x, left_top.y + width);
        const rcsc::Vector2D right_top(left_top.x + length, left_top.y);
        const rcsc::Vector2D right_bottom(right_top.x, right_top.y + width);
        double max_dist = (left_top - wm.self().pos()).length();
        if(max_dist < (left_bottom - wm.self().pos()).length())
        {
            max_dist = (left_bottom - wm.self().pos()).length();
        }
        if(max_dist < (right_top  - wm.self().pos()).length())
        {
            max_dist = (right_top  - wm.self().pos()).length();
        }
        if(max_dist < ( right_bottom - wm.self().pos()).length())
        {
            max_dist = (right_bottom - wm.self().pos()).length();
        }
        for(std::vector<double>::const_iterator opp_y = opp_y_vector.begin();
            opp_y != opp_y_end;
            opp_y++)
        {
            for(rcsc::PlayerPtrCont::const_iterator opp = wm.opponentsFromSelf().begin();
                opp != opp_end;
                opp++)
            {
                if((*opp)->pos().x >= left_top.x
                   && (*opp)->pos().x <= left_top.x + length
                   && (*opp)->pos().y == (*opp_y)
                   && (*opp)->pos().y >= left_top.y
                   && (*opp)->pos().y <= (left_top.y + width)
                   && (check_goalie || (!check_goalie && !(*opp)->goalie())))
                {
                    opp_number_y_list.push_back((*opp)->unum());
                }
                else if((*opp)->distFromSelf() > max_dist)
                {
                    break;
                }
            }
        }
    }
    return opp_number_y_list;
}

void
Obake_Analysis::setRole(int number,
			bool &role_side_or_center_back,
			bool &role_deffensive_half,
			bool &role_offensive_half,
			bool &role_side_or_center_forward)
{
    switch (number){
        case 2:
            role_side_or_center_back = true;
            break;
        case 3:
            role_side_or_center_back = true;
            break;
        case 4:
            role_side_or_center_back = true;
            break;
        case 5:
            role_side_or_center_back = true;
            break;
        case 6:
            role_deffensive_half = true;
            break;
        case 7:
            role_deffensive_half = true;
            break;
        case 8:
            role_offensive_half = true;
            break;
        case 9:
            role_side_or_center_forward = true;
            break;
        case 10:
            role_side_or_center_forward = true;
            break;
        case 11:
            role_side_or_center_forward = true;
            break;
    }
}





















