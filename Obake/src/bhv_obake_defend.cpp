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
#include <rcsc/action/basic_actions.h>
#include <rcsc/common/logger.h>
#include <rcsc/player/debug_client.h>
#include <rcsc/player/player_agent.h>
#include <rcsc/game_time.h>
#include <fstream>
#include "bhv_basic_move.h"
#include "bhv_obake_defend.h"
#include "obake_analysis.h"

bool
Bhv_ObakeDefend::execute(rcsc::PlayerAgent * agent)
{
    rcsc::dlog.addText(rcsc::Logger::TEAM,
                       "%s:%d: Bhv_ObakeDefend"
                       ,__FILE__, __LINE__ );

    const rcsc::Vector2D defense_pos = getDefensePosition(agent);
    rcsc::dlog.addText(rcsc::Logger::TEAM,
                       "deoense_pos = (%.1f, %.1f)",
                       defense_pos.x, defense_pos.y);
    
    
    if(checkTurnIsNeeded(agent))
    {
        const rcsc::Vector2D face_point = getFacePoint(agent);
        rcsc::Body_TurnToPoint(face_point).execute(agent);
    }
    else
    {
        Bhv_BasicMove(defense_pos).execute( agent );
    }
    return true;
}

bool
Bhv_ObakeDefend::checkTurnIsNeeded(rcsc::PlayerAgent * agent)
{
    const rcsc::WorldModel & wm = agent->world();
    rcsc::Vector2D v;
    const rcsc::PlayerObject * target = wm.getOpponentNearestToBall(10);
    if(target)
    {
        rcsc::Vector2D target_next_pos = getTargetNextPos(agent,
                                                     target);
        rcsc::Vector2D target_polar_pos = target_next_pos - M_self_next_pos;
        const double angle_difference = std::abs(target_polar_pos.th().degree() - wm.self().body().degree());
        const double max_angle = 45.0;
        const rcsc::Vector2D intersectoin = getOppBodyIntersection(agent, 
                                                                   target);
        const rcsc::Vector2D target_to_intersection = intersectoin - target_next_pos;
        const rcsc::Vector2D self_to_intersection = intersectoin - M_self_next_pos;
        if(angle_difference > max_angle
           && self_to_intersection.length() <= 2.0 
           && target_to_intersection.length() >= self_to_intersection.length() * 2)
        {
            return true;
        }
        else if(angle_difference >= 90.0
                && M_self_next_pos.dist(target_next_pos) >= 8.5
                && self_to_intersection.length() < target_to_intersection.length() + 2.0 
                && intersectoin.x <= target_next_pos.x)
        {
            return true;
        }

    }
    return false;
}

rcsc::Vector2D
Bhv_ObakeDefend::getFacePoint(rcsc::PlayerAgent * agent)
{
    const rcsc::WorldModel & wm = agent->world();
    rcsc::Vector2D face_point, v;
    const rcsc::PlayerObject * target = wm.getOpponentNearestToBall(10);
    if(target)
    {
        face_point = getTargetNextPos(agent,
                                      target);
        rcsc::Body_TurnToPoint(face_point).execute(agent);
    }
    else
    {
        face_point = M_ball_next_pos;
    }
    return face_point;
}

rcsc::Vector2D
Bhv_ObakeDefend::getDefensePosition(rcsc::PlayerAgent * agent)
{

    const rcsc::WorldModel & wm = agent->world();
    rcsc::Vector2D defense_pos, target_next_pos, v;
    double length;
    const rcsc::PlayerObject * target = wm.getOpponentNearestToBall(10);
    /*output*/
/*
    std::ofstream fout;
    fout.open("defend.txt", std::ios::out | std::ios::app);
*/
    /****************************/
//    std::cout<<"ball"<<wm.ball().pos()<<"next"<<M_ball_next_pos<<std::endl;
    defense_pos = M_ball_next_pos;
    if(target)
    {
        target_next_pos = getTargetNextPos(agent,
                                           target);
        target_next_pos.x += rcsc::ServerParam::i().defaultKickableArea();
 //        std::cout<<"add"<<v<<std::endl;
//        std::cout<<"target"<<(target)->pos()<<"next"<<target_next_pos<<std::endl;
    }
    if(M_ball_next_pos.x < M_self_next_pos.x)
    {
        length = 3.8;
        if(M_ball_next_pos.dist(M_self_next_pos) <= 3.0)
        {
            length = 2.0;
        }
//        std::cout<<"kickableArea = "<<rcsc::ServerParam::i().defaultKickableArea();
//        std::cout<<",opp = "<<target_next_pos.dist(M_ball_next_pos)<<std::endl;
        if(M_exist_intercept_target
            /*
            target_next_pos.dist(M_ball_next_pos) <= rcsc::ServerParam::i().defaultKickableArea() * 2.5
            && M_self_next_pos.dist(target_next_pos) <= 5.0*/)
        {
            if(target)
            {
                if(((target)->body().degree() < -80.0
                       && (target)->body().degree() >= -180.0)
                   ||((target)->body().degree() > 80.0
                       && (target)->body().degree() <= 180.0))
                {
                    defense_pos = target_next_pos + rcsc::Vector2D::polar2vector(length, (target)->body());
                }
                else
                {
                    defense_pos.x -= length;
                }                
            }
            if(M_ball_next_pos.x <= rcsc::ServerParam::i().ourPenaltyAreaLineX()
               && M_ball_next_pos.absY() > rcsc::ServerParam::i().goalAreaWidth()
               && target)
            {
                defense_pos.x = std::min(target_next_pos.x, M_ball_next_pos.x);
            }
        }
    }
    else if(target)//
    {
        double min_angel = 90.0;
        if(M_self_next_pos.dist(target_next_pos) <= 2.5)
        {
            min_angel = 70.0;
        }

        const rcsc::Vector2D intersectoin = getOppBodyIntersection(agent,
                                                                   target);
        if(target
           && M_exist_intercept_target//M_self_next_pos.dist(M_ball_next_pos) <= 6.5
           && (((target)->body().degree() < -min_angel
                && (target)->body().degree() >= -180.0)
               ||((target)->body().degree() > min_angel
                  && (target)->body().degree() <= 180.0))
               && intersectoin.x <= target_next_pos.x
/*           && M_self_next_pos.dist(target_next_pos) <= 6.5
*/)
        {
//            std::cout<<"number:"<<wm.self().unum()<<"defend"<<std::endl;
            const rcsc::Vector2D target_to_intersection = intersectoin - target_next_pos;
            const rcsc::Vector2D self_to_intersection = intersectoin - M_self_next_pos;
//            fout<<"intersectoin = "<<intersectoin<<std::endl;
            

            if(self_to_intersection.length() > target_to_intersection.length())//+ 2.0/*-2.0*/)
            {
                defense_pos = intersectoin;
                if(M_self_next_pos.dist(M_ball_next_pos) <= 4.5)
                {
                    const double difference = std::abs(self_to_intersection.length()
                                                       - target_to_intersection.length());
                    rcsc::Vector2D block_vector = target_to_intersection;
                    block_vector.setLength(difference);
                    defense_pos += block_vector;
                }
                else
                {
                    defense_pos.x -= 6.5;
                }
            }
            else
            {
                if(self_to_intersection.length() >= 1.5)
                {
                    defense_pos = intersectoin;
//                    fout<<"second"<<std::endl;
                }
                else
                {
                    const rcsc::Vector2D middle_point((intersectoin.x+target_next_pos.x)/2, 
                                                      (intersectoin.y+target_next_pos.y)/2);
                    defense_pos = middle_point;
//                    fout<<"third"<<std::endl;
                }
            }            
        }
        else
        {
            const double difference_x = std::abs(M_self_next_pos.x - M_ball_next_pos.x);
            const double difference_y = std::abs(M_self_next_pos.y - M_ball_next_pos.y);
            if(difference_x < difference_y + 2.0)
            {
                defense_pos.x = M_ball_next_pos.x - (difference_y + 3.5);
            }
            else
            {
                if(std::abs(M_ball_next_pos.y - M_self_next_pos.y) > 1.5)
                {
                    defense_pos = M_self_next_pos;
                    defense_pos.y = M_ball_next_pos.y;
                }
                else
                {
                    if(std::abs(M_ball_next_pos.x - M_self_next_pos.x) <= 5.5)
                    {
                        defense_pos.x = M_self_next_pos.x + 1.5;
                    }
                }
            }
        }
    }
    if(target
       && M_ball_next_pos.x <= -rcsc::ServerParam::i().pitchHalfLength()
       + rcsc::ServerParam::i().goalAreaLength() + 3.5)
    {
        defense_pos.x = std::min(target_next_pos.x, M_ball_next_pos.x);
    }
    if(Obake_Analysis().checkExistOurPenaltyAreaIn(M_ball_next_pos)
       && M_ball_next_pos.x <= M_self_next_pos.x
       && M_ball_next_pos.absY() >= M_self_next_pos.absY()
       && target)
    {
        defense_pos.x = std::min(target_next_pos.x, M_ball_next_pos.x);
    }
    if(defense_pos.x <= -rcsc::ServerParam::i().pitchHalfLength() + 1.0)
    {
        defense_pos.x = -rcsc::ServerParam::i().pitchHalfLength() + 2.0;
    }
    else if(defense_pos.y >= rcsc::ServerParam::i().pitchHalfWidth() - 1.0)
    {
        defense_pos.y = rcsc::ServerParam::i().pitchHalfWidth() - 2.0;
    }
    else if(defense_pos.y <= -rcsc::ServerParam::i().pitchHalfWidth() + 1.0)
    {
        defense_pos.y = -rcsc::ServerParam::i().pitchHalfWidth() + 2.0;
    }
    /*output*/
/*    
    fout<<"number = "<<wm.self().unum()<<", ball = "<<M_ball_next_pos;
    fout<<", target_next_pos = "<<target_next_pos<<", ";
    if(wm.existKickableOpponent())
    {
        fout<<"kickable"<<std::endl;
    }
    else
    {
        fout<<"unkickable"<<std::endl;
    }
    fout<<"M_self_next_pos = "<<M_self_next_pos<<", defense_pos = "<<defense_pos<<std::endl;
    fout<<"M_self_next_pos_to_target = "<<M_self_next_pos.dist(target_next_pos);
    if(target)
    {
        fout<<", target_body_angle"<<(target)->body();
    }
    fout<<std::endl<<std::endl;
    fout.close();
*/
    /************/
    return defense_pos;
} 

rcsc::Vector2D
Bhv_ObakeDefend::getOppBodyIntersection(rcsc::PlayerAgent * agent,
                                        const rcsc::PlayerObject * target)
{
    rcsc::Vector2D intersectoin;
    if(target)
    {
        const rcsc::Vector2D target_next_pos = getTargetNextPos(agent,
                                                                target);
        const rcsc::Line2D target_body_line(target_next_pos, (target)->body());
        const rcsc::Line2D perpendicular_target_body_line = target_body_line.perpendicular(M_self_next_pos);
        intersectoin = rcsc::Line2D::intersection(target_body_line,
                                                  perpendicular_target_body_line);
    }
    return intersectoin;
}

rcsc::Vector2D
Bhv_ObakeDefend::getTargetNextPos(rcsc::PlayerAgent * agent,
                                  const rcsc::PlayerObject * target)
{
    rcsc::Vector2D target_next_pos;
    if(target)
    {
        const double decay = rcsc::ServerParam::i().defaultPlayerDecay();
        rcsc::Vector2D v;
     
        target_next_pos = (target)->pos();
        if((target)->velCount() < 3)
        {
            v = (target)->vel();
            const double speed = (target)->vel().length();
            v.setLength(speed * (1-std::pow(decay, ((target)->velCount()+1))) / (1 - decay));
            target_next_pos += v;
        }
    }
    return target_next_pos;
}
