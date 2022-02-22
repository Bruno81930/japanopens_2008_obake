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

#include "obake_fuzzy_grade.h"
#include "body_obake_pass.h"
#include "obake_analysis.h"
#include <fstream>
#include "bhv_obake_action_strategy.h"
#include "bhv_obake_action_strategy_test.h"

bool
Bhv_ObakeActionStrategyTest::execute(rcsc::PlayerAgent * agent)
{
    rcsc::dlog.addText( rcsc::Logger::TEAM,
                        "%s:%d: Bhv_ObakeActionStrategy"
                        ,__FILE__, __LINE__ );
    const rcsc::WorldModel & wm = agent->world();
}

bool
Bhv_ObakeActionStrategyTest::checkAvoidanceFromNearestOpp(rcsc::PlayerAgent * agent)
{
    const rcsc::WorldModel &wm =agent->world();
    const rcsc::PlayerObject * nearest_opp_from_ball = wm.getOpponentNearestToBall(10);
    if(nearest_opp_from_ball)
    {
        std::ofstream fout;
        fout.open("action_test.txt", std::ios::out | std::ios::app);
        const double avoidance_max_multiplier = 1.1;
        const rcsc::Vector2D nearest_opp_next_pos = (nearest_opp_from_ball)->pos() + (nearest_opp_from_ball)->vel();
	const rcsc::Vector2D ball_next_pos = wm.ball().pos() + wm.ball().vel();
        double dist, multiplier;
        rcsc::Vector2D additional_vector, target_point;
        additional_vector = nearest_opp_next_pos - ball_next_pos;
        target_point = ball_next_pos + additional_vector;
        if(Bhv_ObakeActionStrategy().checkAvoidanceFromNearestOpp(agent,
                                                                  target_point,
                                                                  avoidance_max_multiplier,
                                                                  multiplier))
        {
            fout<<"error"<<std::endl;
            fout.close();
            return false;
        }
        dist = 5.0;
        additional_vector = ball_next_pos - nearest_opp_next_pos;
        additional_vector.setLength(dist);
        target_point = ball_next_pos + additional_vector;
        fout<<"dist = "<<dist;
        if(Bhv_ObakeActionStrategy().checkAvoidanceFromNearestOpp(agent,
                                                                  target_point,
                                                                  avoidance_max_multiplier,
                                                                  multiplier))
        {
            const double criterion = 1.05;
            if(multiplier != criterion)//independent on membership fuction
            {
                fout<<"error";
                fout<<", multiplier = "<<multiplier<<std::endl;
                fout.close();
                return false;
            }
            else
            {
                fout<<"success";
            }
            fout<<std::endl;
        }

        dist = 10.0;
        additional_vector = ball_next_pos - nearest_opp_next_pos;
        additional_vector.setLength(dist);
        target_point = ball_next_pos + additional_vector;
        fout<<"dist = "<<dist;
        if(Bhv_ObakeActionStrategy().checkAvoidanceFromNearestOpp(agent,
                                                                  target_point,
                                                                  avoidance_max_multiplier,
                                                                  multiplier))
        {
            if(multiplier != avoidance_max_multiplier)//independent on membership fuction
            {
                fout<<"error";
                fout.close();
                return false;
            }
            else
            {
                fout<<"success";
            }
            fout<<std::endl;
        }
        fout.close();
    }
    return true;
}

bool
Bhv_ObakeActionStrategyTest::checkVeryLackStaminaTest(rcsc::PlayerAgent * agent)
{
    const rcsc::WorldModel & wm = agent->world();
    const double lack_stamina_bottom_rate = 0.65;//independent on degreeLackStamina
    const double lack_stamina_top_rate = 0.3;//independent on degreeLackStamina
    std::ofstream fout;
    fout.open("action_test.txt", std::ios::out | std::ios::app);
    const double stamina_max_multiplier = 1.2;
    double multiplier;
    if(Bhv_ObakeActionStrategy().checkVeryLackStamina(agent,
                                                      stamina_max_multiplier,
                                                      multiplier))
    {
        if(wm.self().stamina() >= rcsc::ServerParam::i().staminaMax() * lack_stamina_bottom_rate)
        {
            fout<<"error"<<std::endl;
            fout<<"stamina = "<<wm.self().stamina();
            fout<<", multiplier = "<<multiplier<<std::endl;
            fout.close();
            return false;
        }
        
        if(wm.self().stamina() <= rcsc::ServerParam::i().staminaMax() * lack_stamina_top_rate
           && multiplier != 1.0)
        {
            fout<<"error"<<std::endl;
            fout<<"stamina = "<<wm.self().stamina();
            fout<<", multiplier = "<<multiplier<<std::endl;
            fout.close();
            return false;
        }
    }
    fout<<"stamina = "<<wm.self().stamina();
    fout<<", multiplier = "<<multiplier<<std::endl;
    fout.close();
    return true;
}

/*
bool
Bhv_ObakeActionStrategyTest::checkSideAttackPositionTest(rcsc::PlayerAgent * agent)
{
    std::ofstream fout;
    fout.open("action_test.txt", std::ios::out | std::ios::app);
    const double side_attack_max_multier = 1.05;
    double multiplier;
    rcsc::Vector2D target_point(10.0, -30.0);
    while(target_point.absY() < rcsc::ServerParam::i().pitchHalfWidth())
    {
        fout<<"target_point = "<<target_point;
        if(!Bhv_ObakeActionStrategy().checkSideAttackPosition(agent,
                                                              target_point,
                                                              side_attack_max_multier,
                                                              multiplier))
        {
            if(target_point.absY() >= rcsc::ServerParam::i().pitchHalfWidth() - 8.0
               && target_point.x <= rcsc::ServerParam::i().theirPenaltyAreaLine())
            {
                fout<<"error";
                fout.close();
                return false;
            }
            
            if(multiplier == 1.0)
            {
                fout<<", true"<<std::endl;
            }
        }
        else if(multiplier == side_attack_max_multier)
        {
            fout<<", true"<<std::endl;
        }

        target_point.y += 10.0;
    }
    fout.close();
    return true;
}
*/
bool
Bhv_ObakeActionStrategyTest::getPositionMultiplierTest(rcsc::PlayerAgent * agent)
{
    std::ofstream fout;
    fout.open("action_test.txt", std::ios::out | std::ios::app);
    const rcsc::WorldModel & wm = agent->world();
    const double position_max_multiplier = 1.2;
    const rcsc::Vector2D ball_next_pos = wm.ball().pos() + wm.ball().vel();
    double multiplier;
    rcsc::Vector2D target_point(0.0, 0.0);
    if(ball_next_pos.x < rcsc::ServerParam::i().theirPenaltyAreaLineX())
    {
        fout<<"offside line = "<<wm.offsideLineX()<<std::endl;
        target_point.x = wm.offsideLineX();
        multiplier = Bhv_ObakeActionStrategy().getPositionMultiplier(agent,
                                                                     target_point,
                                                                     position_max_multiplier);
        fout<<"target_point = "<<target_point<<std::endl;
        if(multiplier != position_max_multiplier)
        {
            fout<<"error, multiplier = "<<multiplier<<std::endl;
            fout.close();
            return false;
        }
        target_point.x -= 12.5;
        fout<<"target_point = "<<target_point<<std::endl;
        multiplier = Bhv_ObakeActionStrategy().getPositionMultiplier(agent,
                                                                     target_point,
                                                                     position_max_multiplier);
        if(multiplier != 1.10) //independent on member ship function
        {
            fout<<"error, multiplier = "<<multiplier<<std::endl;
            fout.close();
            return false;
        }
    }
    else
    {
        rcsc::Vector2D target_point(rcsc::ServerParam::i().pitchHalfLength(), 0.0);
        multiplier = Bhv_ObakeActionStrategy().getPositionMultiplier(agent,
                                                                     target_point,
                                                                     position_max_multiplier);
        fout<<"target_point = "<<target_point;
        if(multiplier == position_max_multiplier)
        {
            fout<<", success";
        }
        else 
        {
            fout<<", error";
            fout.close();
            return false;
        }
    
        target_point.y = rcsc::ServerParam::i().pitchHalfWidth() / 2.0;
        double criterion = 1.1;
        multiplier = Bhv_ObakeActionStrategy().getPositionMultiplier(agent,
                                                                     target_point,
                                                                     position_max_multiplier);
        fout<<"target_point = "<<target_point;
        if(multiplier == criterion)
        {
            fout<<", success"<<std::endl;
        }
        else 
        {
            fout<<", error";
            fout<<", multiplier = "<<multiplier<<std::endl;
            fout.close();
            return false;
        }
        target_point.x -= rcsc::ServerParam::i().penaltyAreaLength() / 2.0;
        multiplier = Bhv_ObakeActionStrategy().getPositionMultiplier(agent,
                                                                     target_point,
                                                                     position_max_multiplier);
        fout<<"target_point = "<<target_point;
        if(multiplier == criterion)
        {
            fout<<", success";
        }
        else 
        {
            fout<<", error";
            fout<<", multiplier = "<<multiplier<<std::endl;
            fout.close();
            return false;
        }
        fout<<std::endl;
        return true;

    }
    fout.close();
    return true;
}


bool
Bhv_ObakeActionStrategyTest::getShootMultiplierTest(rcsc::PlayerAgent * agent)
{
    std::ofstream fout;
    fout.open("action_test.txt", std::ios::out | std::ios::app);
    const double shoot_max_multiplier = 1.4;
    double multiplier;
    rcsc::Vector2D target_point(rcsc::ServerParam::i().pitchHalfLength(), 0.0);
    multiplier = Bhv_ObakeActionStrategy().getShootMultiplier(agent,
                                                              target_point,
                                                              shoot_max_multiplier);
    fout<<"target_point = "<<target_point;
    if(multiplier == shoot_max_multiplier)
    {
        fout<<", success";
    }
    else 
    {
        fout<<", error";
        fout.close();
        return false;
    }
    
    target_point.y = rcsc::ServerParam::i().pitchHalfWidth() / 2.0;
    double criterion = 1.2;
    multiplier = Bhv_ObakeActionStrategy().getShootMultiplier(agent,
                                                              target_point,
                                                              shoot_max_multiplier);
    fout<<"target_point = "<<target_point;
    if(multiplier == criterion)
    {
        fout<<", success"<<std::endl;
    }
    else 
    {
        fout<<", error";
        fout<<", multiplier = "<<multiplier<<std::endl;
        fout.close();
        return false;
    }
    target_point.x -= rcsc::ServerParam::i().penaltyAreaLength() / 2.0;
    multiplier = Bhv_ObakeActionStrategy().getShootMultiplier(agent,
                                                              target_point,
                                                              shoot_max_multiplier);
    fout<<"target_point = "<<target_point;
    if(multiplier == criterion)
    {
        fout<<", success";
    }
    else 
    {
        fout<<", error";
        fout<<", multiplier = "<<multiplier<<std::endl;
        fout.close();
        return false;
    }
    fout<<std::endl;
    return true;
}
