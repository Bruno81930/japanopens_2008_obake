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

#include <rcsc/math_util.h>

#include <rcsc/action/basic_actions.h>
#include <rcsc/action/body_kick_multi_step.h>

#include "obake_analysis.h"
#include "obake_strategy.h"
#include "body_obake_clear.h"

bool
Body_ObakeClear::execute(rcsc::PlayerAgent * agent)
{
    const rcsc::WorldModel & wm = agent->world();
    //const double base_stamina = rcsc::ServerParam::i().staminaMax() * 0.325;
    const double base_r = 4.0;
    const rcsc::Vector2D ball_next_pos = wm.ball().pos() + wm.ball().vel();
    const rcsc::Circle2D circle(ball_next_pos, base_r);
    const double first_speed = rcsc::ServerParam::i().ballSpeedMax();
    const double max_angle_difference = 60.0;//70.0;
    const double angle_difference = 15.0;
    bool exist_clear_point = false;
    double angle, angle_difference_from_body, 
        max_angle, min_angle;
    if(ball_next_pos.y > 0.0)
    {
        max_angle = 150.0;
        if(ball_next_pos.x <= rcsc::ServerParam::i().ourPenaltyAreaLineX())
        {
            max_angle += 30.0;
        }
        min_angle = 30.0;//45.0;
        angle = min_angle;
        while(angle <= max_angle)
        {
            //std::cout<<"angle = "<<angle<<", ";
            angle_difference_from_body = std::abs(wm.self().body().degree() - angle);
            if(angle_difference_from_body <= max_angle_difference)
            {
                if(checkSafeClear(agent, first_speed, angle))
                {
                    //std::cout<<"true"<<std::endl;
                    exist_clear_point = true;
                    break;
                }
            }
            angle += angle_difference;
        }
    }
    else
    {
        min_angle = -150.0;
        if(ball_next_pos.x <= rcsc::ServerParam::i().ourPenaltyAreaLineX())
        {
            min_angle -= 30.0;
        }
        max_angle = -30.0;//-45.0;
        angle = max_angle;
        while(angle >= min_angle)
        {
            //std::cout<<"angle = "<<angle<<", ";
            angle_difference_from_body = std::abs(wm.self().body().degree() - angle);
            if(angle_difference_from_body <= max_angle_difference)
            {
                if(checkSafeClear(agent, first_speed, angle))
                {
                    //std::cout<<"true"<<std::endl;
                    exist_clear_point = true;
                    break;
                }
            }
            angle -= angle_difference;
        }
    }
    if(exist_clear_point)
    {
        const double length = 100;
        const rcsc::AngleDeg angle_deg(angle);
        const rcsc::Vector2D clear_point = ball_next_pos + rcsc::Vector2D::polar2vector(length, angle_deg);
        //std::cout<<"ball = "<<ball_next_pos<<", clear_point = "<<clear_point<<std::endl;
        rcsc::Body_KickMultiStep(clear_point,
                                 first_speed,
                                 false
            ).execute( agent );
        return true;
    }
    return false;
}

bool
Body_ObakeClear::checkSafeClear(rcsc::PlayerAgent * agent,
                               const double &first_speed,
                               const double &angle)
{    
    const rcsc::WorldModel & wm = agent->world();
    const rcsc::Vector2D ball_next_pos = wm.ball().pos() + wm.ball().vel();
    const double player_dash_speed = 1.0;
    const rcsc::AngleDeg angle_deg(angle);
    const rcsc::Line2D clear_line(ball_next_pos, angle);
    rcsc::Vector2D ball_to_opp_vector, intersection, opp_next_pos;
    double virtual_opp_to_intersection_dist, ball_to_intersection_dist,
        ball_dist, opp_to_intersection_dist;
    int opp_step;
    /*output*/
/*
    std::ofstream fout;
    fout.open("clear.txt", std::ios::out | std::ios::app);
    fout<<"ball_next_pos = "<<ball_next_pos<<", angle = "<<angle<<std::endl;
    fout<<", angle_difference = "<<std::abs(wm.self().body().degree() - angle)<<std::endl;
*/
    /*******************/
    const rcsc::PlayerPtrCont::const_iterator
        opp_end = wm.opponentsFromBall().end();
    for(rcsc::PlayerPtrCont::const_iterator
        opp = wm.opponentsFromBall().begin();
        opp != opp_end;
        opp++)
    {
        opp_next_pos = (*opp)->pos() + (*opp)->vel();
/*        fout<<"number = "<<(*opp)->unum()<<", count = "<<(*opp)->posCount()<<std::endl;
          fout<<"pos = "<<(*opp)->pos();
          fout<<"next = "<<opp_next_pos<<std::endl;
*/
        if(std::abs((*opp)->angleFromSelf().degree() - angle) >= 90.0)
        {
/*
           fout<<"anglefromself = "<<(*opp)->angleFromSelf().degree();
           fout<<"difference angle = "<<std::abs((*opp)->angleFromSelf().degree() - angle)<<std::endl;
*/
            continue;
        }
        const rcsc::Line2D perpendicular_opp_to_clear_line = clear_line.perpendicular(opp_next_pos);
        intersection = rcsc::Line2D::intersection(clear_line,
                                                  perpendicular_opp_to_clear_line);
        ball_to_intersection_dist = ball_next_pos.dist(intersection);
        opp_to_intersection_dist = opp_next_pos.dist(intersection)
            - rcsc::ServerParam::i().defaultKickableArea();
        virtual_opp_to_intersection_dist = opp_to_intersection_dist;
        const double virtual_dist = (*opp)->posCount() * player_dash_speed * 0.8; 
        if(virtual_dist > opp_to_intersection_dist)
        {
            return false;
        }
        else
        {
            virtual_opp_to_intersection_dist -= virtual_dist;
//            fout<<"virtual dist = "<<virtual_dist<<std::endl;
        }
        opp_step = std::ceil(virtual_opp_to_intersection_dist / player_dash_speed);
        ball_dist = rcsc::inertia_n_step_distance(first_speed,
                                                  opp_step,
                                                  rcsc::ServerParam::i().ballDecay());
/*
        fout<<"opp_step = "<<opp_step<<", ball_dist = "<<ball_dist;
        fout<<", ball_to_intersection_dist = "<<ball_to_intersection_dist<<std::endl;
*/
        if(ball_dist <= ball_to_intersection_dist)
        {
/*
            fout<<"false"<<std::endl<<std::endl;
            fout.close();
*/
            return false;
        }
    }

    double nearest_dist_from_intersection;
    const rcsc::Vector2D top_left_corner(-rcsc::ServerParam::i().pitchHalfLength(),
                                         -rcsc::ServerParam::i().pitchHalfWidth());
    const rcsc::Vector2D top_right_corner(rcsc::ServerParam::i().pitchHalfLength(),
                                          -rcsc::ServerParam::i().pitchHalfWidth());
    const rcsc::Vector2D bottom_left_corner(-rcsc::ServerParam::i().pitchHalfLength(),
                                            rcsc::ServerParam::i().pitchHalfWidth());
    const rcsc::Vector2D bottom_right_corner(rcsc::ServerParam::i().pitchHalfLength(),
                                             rcsc::ServerParam::i().pitchHalfWidth());
    const rcsc::Line2D top_side_line(top_left_corner,
                                     top_right_corner);
    const rcsc::Line2D bottom_side_line(bottom_left_corner,
                                   bottom_right_corner);
    const rcsc::Line2D side_line = (ball_next_pos.y > 0.0)
        ? bottom_side_line : top_side_line;
    intersection = rcsc::Line2D::intersection(clear_line,
                                              side_line);
    if(intersection.x < -rcsc::ServerParam::i().pitchHalfLength())
    {
        const rcsc::Line2D our_goal_line(top_left_corner,
                                         bottom_left_corner);
        intersection = rcsc::Line2D::intersection(clear_line,
                                                  our_goal_line);
        if(intersection.absY() <= rcsc::ServerParam::i().goalAreaHalfWidth())
        {
//            fout<<"goal_area false"<<std::endl<<std::endl;
//            fout.close();
            return false;
        }
    }
    nearest_dist_from_intersection = Obake_Analysis().getDistFromNearestOpp(agent,
                                                                                intersection);
    ball_to_intersection_dist = ball_next_pos.dist(intersection);
    opp_step = std::ceil(nearest_dist_from_intersection / player_dash_speed);
    ball_dist = rcsc::inertia_n_step_distance(first_speed,
                                              opp_step,
                                              rcsc::ServerParam::i().ballDecay());
/*
    fout<<"nearest_opp_dist = "<<nearest_dist_from_intersection<<", intersection = "<<intersection<<std::endl;
    fout<<"nearest_opp_step = "<<opp_step<<", ball_to_intersection_dist = "<<ball_to_intersection_dist<<std::endl;
    fout<<"ball_dist = "<<ball_dist<<std::endl;
*/
    if(ball_dist <= ball_to_intersection_dist)
    {
/*
        fout<<"false"<<std::endl<<std::endl;
        fout.close();
*/
        return false;
    }
/*
   fout<<"true"<<std::endl<<std::endl;
   fout.close();
*/
    return true;
}
