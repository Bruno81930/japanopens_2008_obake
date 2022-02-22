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
#include "body_obake_shoot.h"

bool
Body_ObakeShoot::execute(rcsc::PlayerAgent * agent)
{
    const rcsc::WorldModel & wm = agent->world();
    const rcsc::PlayerObject * opp_goalie = agent->world().getOpponentGoalie();
    rcsc::Vector2D shoot_point(rcsc::ServerParam::i().pitchHalfLength() , 0.0);
    const double angle = 35.0;
    const double r = 1.5;
    const double post_difference = 1.05;
    if(opp_goalie)
    {
        const rcsc::Vector2D opp_goalie_next_pos = opp_goalie->pos() + opp_goalie->vel();
        std::vector<rcsc::Vector2D> shoot_point_vector;
        bool is_near;
        if(std::abs(opp_goalie_next_pos.y - wm.ball().pos().y) <= 2.5/*1.5*/)
        {
            shoot_point.y = rcsc::ServerParam::i().goalHalfWidth() - post_difference;
            while(shoot_point.y > 0.0/*-2.0*/)
            {
                if(Obake_Analysis().checkShootCourse(agent,wm.self().pos(),
                                                     shoot_point, angle,
                                                     r))
                {
                    shoot_point_vector.push_back(shoot_point);
                }
                shoot_point.y *= -1.0;
                if(Obake_Analysis().checkShootCourse(agent,wm.self().pos(),
                                                     shoot_point, angle,
                                                     r))
                {
                    shoot_point_vector.push_back(shoot_point);
                }
                shoot_point.y *= -1.0;
                shoot_point.y -= 2.0;
            }
            is_near = true;
        }
        else
        {
            if(wm.ball().pos().y > opp_goalie_next_pos.y)
            {
                shoot_point.y = rcsc::ServerParam::i().goalHalfWidth() - post_difference;
                if(std::abs(wm.ball().pos().y-rcsc::ServerParam::i().goalHalfWidth()) <= 2.5)
                {
                    shoot_point.y = rcsc::ServerParam::i().goalHalfWidth() - 0.8;
                }
                while(shoot_point.y > opp_goalie_next_pos.y)
                {
                    if(Obake_Analysis().checkShootCourse(agent,wm.self().pos(),
                                                         shoot_point, angle,
                                                         r))
                    {
                        shoot_point_vector.push_back(shoot_point);
                    }
                    shoot_point.y -= 2.0;
                }
            }
            else
            {
                shoot_point.y = shoot_point.y = -rcsc::ServerParam::i().goalHalfWidth() + post_difference;
                if(std::abs(wm.ball().pos().y-rcsc::ServerParam::i().goalHalfWidth()) <= 2.5)
                {
                    shoot_point.y = rcsc::ServerParam::i().goalHalfWidth() - 0.8;
                }
                while(shoot_point.y < opp_goalie_next_pos.y)
                {
                    if(Obake_Analysis().checkShootCourse(agent,wm.self().pos(),
                                                         shoot_point, 
                                                         angle,
                                                         r))
                    {
                        shoot_point_vector.push_back(shoot_point);
                    }
                    shoot_point.y += 2.0;
                }
            }
        }
        if(shoot_point_vector.size() > 0)
        {
            rcsc::Vector2D best_shoot_point = shoot_point_vector.at(0);
            std::vector<rcsc::Vector2D>::size_type i;
            if(shoot_point_vector.size() > 1)
            {
                double difference, maximul_difference;
                maximul_difference =  std::abs(opp_goalie_next_pos.y - shoot_point_vector.at(0).y);
                for(i=1; i<shoot_point_vector.size(); i++)
                {
                    difference = std::abs(opp_goalie_next_pos.y - shoot_point_vector.at(i).y);
                    if(difference > maximul_difference)
                    {
                        maximul_difference = difference;
                        best_shoot_point = shoot_point_vector.at(i);
                    }
                }
            }
            rcsc::Body_KickMultiStep( best_shoot_point,
                                      rcsc::ServerParam::i().ballSpeedMax(),
                                      false
                ).execute( agent );
            std::cout<<"obakeshoot"<<std::endl;
            return true;
        }
    }
    return false;
}
