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

/*
In this file, the code in librcsc-1.3.2 is used 
between "begin of the code of librcsc-1.3.2" 
and "end  of the code of librcsc-1.3.2" 
in the functions, which are checkPassCourseCycle
and checkCombinationPassCourseCycle. 
*/


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

/////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <rcsc/common/server_param.h>
#include <rcsc/common/logger.h>
#include <rcsc/player/player_agent.h>
#include <rcsc/player/interception.h>
#include <rcsc/action/basic_actions.h>
#include <rcsc/action/intention_kick.h>
#include <rcsc/geom/circle_2d.h>
#include <rcsc/geom/rect_2d.h>


#include <rcsc/math_util.h>

#include "bhv_basic_move.h"
#include "obake_analysis.h"
#include "obake_fuzzy_grade.h"
#include "obake_strategy.h"
#include "bhv_obake_receive.h"

bool
Bhv_ObakeReceive::execute(rcsc::PlayerAgent * agent)
{
    rcsc::dlog.addText( rcsc::Logger::TEAM,
                        "%s:%d: Bhv_ObakeReceive"
                        ,__FILE__, __LINE__ );
    const rcsc::WorldModel & wm = agent->world();
    Obake_Analysis().setRole(wm.self().unum(),
			     M_role_side_or_center_back,
			     M_role_deffensive_half,
			     M_role_offensive_half,
			     M_role_side_or_center_forward);
    const rcsc::Vector2D target_pos = getReceiverPosition(agent);
    Bhv_BasicMove(target_pos).execute(agent);
    return true;
}


rcsc::Vector2D
Bhv_ObakeReceive::getReceiverPosition(rcsc::PlayerAgent * agent)
{
    const rcsc::WorldModel & wm = agent->world();
    /*
    */
    rcsc::Vector2D spare_target_pos = M_home_pos;
    bool spare = false;
    const double max_pass_dist = 35.0;//25.0;
    const double length = 10.0;
    rcsc::Vector2D front(M_home_pos.x + 15.0, M_home_pos.y);
    rcsc::Vector2D back = rcsc::Vector2D(wm.self().pos().x - 3.5, wm.self().pos().y);//M_home_pos;
/*
    if(wm.self().unum() == 11)
    {
        std::cout<<"back.x = "<<back.x;
    }
*/
    setSearchRange(agent,
                   front,
                   back);
/*    if(wm.self().unum() == 11)
    {
        std::cout<<", back.x = "<<back.x<<std::endl;
    }
*/
    double best_difference, short_rate, long_rate;
    rcsc::Vector2D receiver_pos_middle = front;
    rcsc::Vector2D receiver_pos = M_home_pos;
    const rcsc::Vector2D self_next_pos = wm.self().pos() + wm.self().vel();
    if(wm.self().unum() == 9 || wm.self().unum() == 10)
    {
        front.x = std::min(front.x + 10.0, wm.offsideLineX() - 0.7);
    }
    if(wm.ball().pos().x >= rcsc::ServerParam::i().theirPenaltyAreaLineX() - 10.0
       && wm.offsideLineX() >= rcsc::ServerParam::i().theirPenaltyAreaLineX()
       && (wm.self().unum() == 9 || wm.self().unum() == 10)
       && ((M_home_pos.y * wm.ball().pos().y < 0.0
            && receiver_pos_middle.absY() > rcsc::ServerParam::i().goalAreaHalfWidth())
           || wm.ball().pos().absY() <= rcsc::ServerParam::i().goalAreaHalfWidth()))
    {
        receiver_pos_middle.y =(receiver_pos_middle.y > 0.0) 
            ? rcsc::ServerParam::i().goalAreaHalfWidth() : -rcsc::ServerParam::i().goalAreaHalfWidth();
    }
    std::vector<rcsc::Vector2D> vector2d_vector;
/*
    const double base_short_difference = 3.25;//5.5;//5.0;
    const double base_long_difference = 13.5;//12.5;
    const double far_penalty_area_base_opp_r = 4.5;
    const double near_goal_base_opp_r = 2.0;//3.55; //3.5;//2.5;
    const double near_penalty_area_base_opp_r = 4.0;
    const double far_penalty_area_base_mate_r = 7.5;
    const double near_goal_base_mate_r = 4.0;
    const double far_penalty_area_base_ball_r = 12.5;
    const double near_penalty_area_base_ball_r = 8.0;
    const double near_penalty_area_base_mate_r = 6.0;//5.5;//4.5
    const double base_long_width = 20.0;//30.0;
    const double base_short_width = 15.0;//10.0;

    std::ofstream fout;
    fout.open("receive_test.txt", std::ios::out | std::ios::app);
*/
/*ga data*/

const double base_short_difference = 4.03116;
const double base_long_difference = 13.6885;
const double near_penalty_area_base_mate_r = 5.77044;
const double far_penalty_area_base_mate_r = 10.6305;
const double near_goal_base_mate_r = 2.79459;
const double near_penalty_area_base_opp_r = 4.29773;
const double far_penalty_area_base_opp_r = 6.91236;
const double near_goal_base_opp_r = 2.68574;
const double near_penalty_area_base_ball_r =  7.55913;
const double far_penalty_area_base_ball_r =  10.364;
const double base_short_width = 7.38459;
const double base_long_width = 16.8007;

    double  best_opp_r, best_mate_r, best_ball_r, best_width, degree_near_goal_x, 
        degree_near_goal_y, degree_far_penalty_area_x, degree_near_penalty_area_x,
        degree_near_offside_line, sum;
    const double safe_dist = 2.5;
    front.x = std::min(front.x, rcsc::ServerParam::i().pitchHalfLength() - safe_dist);
    //evaluate through pass
    if(wm.ball().pos().x < rcsc::ServerParam::i().theirPenaltyAreaLineX()
       && wm.self().pos().x < rcsc::ServerParam::i().theirPenaltyAreaLineX()
       && std::abs(wm.self().pos().x - wm.offsideLineX()) < 7.0)
    {
/*        const double base_short_through_pass_difference = 6.5;//5.5;
        const double base_long_through_pass_difference = 10.0;//17.5;//13.5;
        receiver_pos_middle.y = wm.self().pos().y;
        short_rate = Obake_FuzzyGrade().degreeNearOppPenaltyAreaX(receiver_pos_middle.x);
        long_rate =  Obake_FuzzyGrade().degreeFarOppPenaltyAreaX(receiver_pos_middle.x);
        best_difference = (base_short_through_pass_difference * short_rate 
                           + base_long_through_pass_difference * long_rate)
                          / sum;
*/
        best_difference = 5.0;
        //receiver_pos_middle = getBestMiddle(agent, receiver_pos_middle, best_difference, length, width);  
        const int search_count = 3;
        int i;
        receiver_pos = receiver_pos_middle; 
        for(i=0; i<= search_count; i++)
        {
            if(checkThroughPassCourseCycle(agent,
                                           receiver_pos))
            {
                rcsc::Circle2D circle(receiver_pos, 6.5);
                if(wm.existTeammateIn(circle, 10, false))
                {
                    break;
                }
//                std::cout<<"number = "<< wm.self().unum()<<","<<receiver_pos<<std::endl;
                return receiver_pos;
            }
            if(i == 0)
            {
                receiver_pos.y -=  receiver_pos.y / receiver_pos.absY() * best_difference;
                if(receiver_pos.y == 0.0)
                {
                    receiver_pos.y += best_difference;
                }
            }
            else
            {
                receiver_pos = receiver_pos_middle;
                receiver_pos.y =  receiver_pos.y / receiver_pos.absY() * best_difference;
                if(receiver_pos.y == 0.0)
                {
                    receiver_pos.y -= best_difference;
                }
                else if(receiver_pos.y > rcsc::ServerParam::i().pitchHalfWidth() - 3.5)
                {
                    receiver_pos.y = rcsc::ServerParam::i().pitchHalfWidth() - 3.5;
                }
                else if(receiver_pos.y < -rcsc::ServerParam::i().pitchHalfWidth() + 3.5)
                {
                    receiver_pos.y = -rcsc::ServerParam::i().pitchHalfWidth() + 3.5;
                }
            }
        }
    }
    //evaluate direct or combination pass
    receiver_pos_middle = front;
    while(receiver_pos_middle.x >= back.x)
    {
        degree_near_penalty_area_x = Obake_FuzzyGrade().degreeNearOppPenaltyAreaX(receiver_pos_middle.x);
        degree_far_penalty_area_x =  Obake_FuzzyGrade().degreeFarOppPenaltyAreaX(receiver_pos_middle.x);
        degree_near_goal_x = Obake_FuzzyGrade().degreeNearOppGoalX(receiver_pos_middle.x);
        degree_near_goal_y = Obake_FuzzyGrade().degreeNearOppGoalY(receiver_pos_middle.y);
        sum = degree_far_penalty_area_x + degree_near_penalty_area_x
            + std::min(degree_near_goal_x, degree_near_goal_y);
        best_opp_r = (far_penalty_area_base_opp_r * degree_far_penalty_area_x
                      + near_penalty_area_base_opp_r * degree_near_penalty_area_x
                      + near_goal_base_opp_r * std::min(degree_near_goal_x,
                                                        degree_near_goal_y))
                     / sum;
        best_mate_r = (far_penalty_area_base_mate_r * degree_far_penalty_area_x
                       + near_penalty_area_base_mate_r * degree_near_penalty_area_x
                       + near_goal_base_mate_r * std::min(degree_near_goal_x,
                                                          degree_near_goal_y))
                      / sum;
        best_ball_r = (far_penalty_area_base_ball_r * degree_far_penalty_area_x
                       + near_penalty_area_base_ball_r * degree_near_penalty_area_x)
                      / (degree_far_penalty_area_x + degree_near_penalty_area_x);
        degree_near_offside_line = Obake_FuzzyGrade().degreeNearOffsideLine(agent,
                                                                            receiver_pos_middle.x);
        short_rate = degree_near_penalty_area_x;
        long_rate = degree_far_penalty_area_x;
        sum = short_rate + long_rate;
        best_width = (base_short_width * degree_near_penalty_area_x
                      + base_long_width * degree_far_penalty_area_x) / sum;//30.0;
        /*if(wm.ball().pos().x >= rcsc::ServerParam::i().theirPenaltyAreaLineX())
        {
            //width = 10.0;
            if(wm.ball().pos().y * M_home_pos.y > 0.0
               && M_home_pos.absY() > wm.ball().pos().absY()
               && wm.ball().pos().absY() > rcsc::ServerParam::i().goalAreaHalfWidth())
            {
                width += 5.0;
            }
            else if(wm.self().unum() != 8
                    && Obake_Analysis().checkExistOppPenaltyAreaIn(wm.ball().pos()))
            {
                width -= 4.0;
            }
        }
        */
        /*else if(std::abs(M_home_pos.y - wm.ball().pos().y) > 15.0
          && width > 15.0)
          {
        width = 15.0;
        }*/
        rcsc::dlog.addText( rcsc::Logger::TEAM,
                            "%s:%d: sum = %.1f (short_rate = %.1f,  long_rate = %.1f)"
                            ,__FILE__, __LINE__,
                            sum,  short_rate, long_rate);
        best_difference = (base_short_difference * short_rate 
                           + base_long_difference * long_rate)
                          / sum;
        /*if(receiver_pos_middle.absY() >= rcsc::ServerParam::i().goalHalfWidth())
        {
            double dist = 4.5;
            const double top_range = (rcsc::ServerParam::i().goalAreaHalfWidth() 
                                          + rcsc::ServerParam::i().penaltyAreaHalfWidth()) / 2;            
            if(wm.ball().pos().y > 0.0
               && wm.self().unum() == 9)
            {
                dist += 3.5;

            }
            else if(wm.ball().pos().y < 0.0
                    && wm.self().unum() == 10)
            {
                dist += 3.5;
            }            
            double difference = dist * std::max(degree_near_penalty_area_x, degree_near_offside_line);
            rcsc::Vector2D v(0.0, -front.y);
            v.setLength(difference);
            receiver_pos_middle += v;
            const rcsc::Vector2D check_area_left_top(wm.ball().pos().x, -rcsc::ServerParam::i().goalAreaHalfWidth());
            const double check_area_length = std::abs(rcsc::ServerParam::i().pitchHalfLength()-wm.ball().pos().x);
            const double check_area_width = rcsc::ServerParam::i().goalAreaWidth();
            const int opp_number_in_check_area = Obake_Analysis().getOppNumber(agent,
                                                                               check_area_left_top,
                                                                               check_area_length,
                                                                               check_area_width);
            if(opp_number_in_check_area >= 4)
            {
                v = rcsc::Vector2D(0.0, front.y);
                difference = 4.5;
                v.setLength(difference);
            }
        }
        */
//        fout<<"receiver_pos_middle = "<<receiver_pos_middle;
        receiver_pos_middle = getBestMiddle(agent, receiver_pos_middle, best_difference, length, best_width);
        vector2d_vector = getSearchOrder(agent, receiver_pos_middle,
                                         best_difference, length, best_mate_r, best_opp_r);
//        fout<<", "<<receiver_pos_middle<<std::endl;
        /**********************************/
/*
        if(!vector2d_vector.empty())
        {
            fout<<"first"<<std::endl;
            std::vector<rcsc::Vector2D>::const_iterator end = vector2d_vector.end();
            for(std::vector<rcsc::Vector2D>::const_iterator p = vector2d_vector.begin();
                p != end;
                p++)
            {
                if((*p).absY() > rcsc::ServerParam::i().pitchHalfWidth())
                {
                    fout<<"bad "<<(*p)<<std::endl;
                }
            }
            fout<<std::endl<<std::endl;
        }
*/
        /***********************************/
        bool is_comleted = false;
        int count = 0;
        while(!is_comleted)
        {
            if(vector2d_vector.size() > 0)
            {
                std::vector<rcsc::Vector2D>::const_iterator end = vector2d_vector.end();
                for(std::vector<rcsc::Vector2D>::const_iterator p = vector2d_vector.begin();
                    p != end;
                    p++)
                {
                    if(wm.ball().pos().dist((*p)) <= best_ball_r)
                    {
                        continue;
                    }

                    if(//wm.ball().pos().x > rcsc::ServerParam::i().theirPenaltyAreaLineX()
                        wm.offsideLineX() > rcsc::ServerParam::i().theirPenaltyAreaLineX()
                       /*||(wm.ball().pos().dist(wm.self().pos()) <= max_pass_dist
                         && std::abs(wm.self().pos().x - wm.offsideLineX()) <= 10.0)*/)
                    {
                        const double difference_y = 
                            (wm.ball().pos().absY() > rcsc::ServerParam::i().goalAreaWidth())
                            ? 5.0 : 0.0;
                        if(Obake_Analysis().checkExistOppPenaltyAreaIn(wm.ball().pos())
                           //&& wm.self().pos().absY() <= rcsc::ServerParam::i().penaltyAreaHalfWidth()
                           && (*p).absY() > rcsc::ServerParam::i().penaltyAreaHalfWidth()
                           && M_role_side_or_center_forward)
                        {
                            continue;
                        }

                        if((wm.ball().pos().y > 0.0
                            && wm.self().pos().y < wm.ball().pos().y
                            && (*p).y >= wm.ball().pos().y - difference_y)
                           ||(wm.ball().pos().y < 0.0
                              && wm.self().pos().y > wm.ball().pos().y
                              && (*p).y <= wm.ball().pos().y + difference_y)
                           || (wm.ball().pos().y * (*p).y <= 0.0
                               && (*p).absY() > rcsc::ServerParam::i().goalAreaWidth())
                           || (wm.ball().pos().y * (*p).y <= 0.0
                               && wm.ball().pos().absY() <= rcsc::ServerParam::i().penaltyAreaHalfWidth() + 3.5
                               && (*p).absY() > (rcsc::ServerParam::i().penaltyAreaHalfWidth()
                                                 +  rcsc::ServerParam::i().goalAreaHalfWidth()) / 2.0))
                        {
                            continue;
                        }
                    }

                    if(wm.ball().pos().dist((*p)) <= max_pass_dist
                       /* && !(wm.ball().pos().x > rcsc::ServerParam::i().theirPenaltyAreaLineX()
                          && (*p).y * wm.ball().pos().y < 0.0)*/)
                    {
                        if(checkPassCourseCycle(agent, (*p)))
                        {
                            receiver_pos = getAdvancedPostion(agent,
                                                              (*p));
                            rcsc::dlog.addText(rcsc::Logger::TEAM,
                                               "%s:%d cycle: receiver_pos(%.1f %.1f)"
                                               ,__FILE__, __LINE__,
                                               receiver_pos.x, receiver_pos.y);
//                            fout.close();
                            return receiver_pos;
                        }
                        /*
                        const double predict_dist = 5.0;
                        if(!M_role_offensive_half
                           && Obake_Analysis().checkExistOppPenaltyAreaIn(wm.ball().pos())
                           && wm.ball().pos().x <= rcsc::ServerParam::i().pitchHalfLength() 
                           - rcsc::ServerParam::i().goalAreaLength() - 2.5
                           && (*p).absY() <= rcsc::ServerParam::i().goalAreaHalfWidth()*/
                           /*wm.offsideLineX() - wm.ball().pos().x < 5.0*///)
/*                        {
                                if(checkPassCourseCyclePrediction(agent, (*p), predict_dist))
                                {
                                    receiver_pos = getAdvancedPostion(agent,
                                                                      (*p));
                                    rcsc::dlog.addText(rcsc::Logger::TEAM,
                                                       "%s:%d cycle: receiver_pos(%.1f %.1f)"
                                                       ,__FILE__, __LINE__,
                                                       receiver_pos.x, receiver_pos.y);
//                                    fout.close();
                                    return receiver_pos;
                                }
                        }*/
                        if(//Obake_Analysis().checkExistOppPenaltyAreaIn(wm.ball().pos())
                            wm.ball().pos().x >= rcsc::ServerParam::i().theirPenaltyAreaLineX()
                            && !(M_role_offensive_half
                                 && wm.ball().pos().x < rcsc::ServerParam::i().pitchHalfLength()
                                 - rcsc::ServerParam::i().goalAreaLength()))
                        {
                            rcsc::Vector2D prediction_vector(-3.5, 0.0);
                            //rcsc::Vector2D prediction_vector(-1.5, 0.0);
                            if(wm.ball().pos().x >= rcsc::ServerParam::i().pitchHalfLength()
                               - rcsc::ServerParam::i().goalAreaLength())
                            {
                                prediction_vector.x = std::min(-3.5, rcsc::ServerParam::i().pitchHalfLength()
                                                               - rcsc::ServerParam::i().goalAreaLength() - wm.ball().pos().x);
                            }
                            if(checkPassCourseCyclePrediction(agent, (*p), prediction_vector))
                            {
                                receiver_pos = getAdvancedPostion(agent,
                                                                  (*p));
                                rcsc::dlog.addText(rcsc::Logger::TEAM,
                                                   "%s:%d cycle: receiver_pos(%.1f %.1f)"
                                                   ,__FILE__, __LINE__,
                                                   receiver_pos.x, receiver_pos.y);
//                                fout.close();
                                return receiver_pos;
                            }
                        }
                    }
                    else if(wm.ball().pos().dist(M_home_pos) > max_pass_dist) 
                    {
                        /*if(!(wm.self().pos().y * wm.ball().pos().y >= 0.0
                            && wm.ball().pos().x >= rcsc::ServerParam::i().theirPenaltyAreaLineX()))
                            {*/
                        if(checkCombinationPassCourseCycle(agent, (*p)))
                        {
                            receiver_pos = getAdvancedPostion(agent,
                                                              (*p));
                            rcsc::dlog.addText(rcsc::Logger::TEAM,
                                               "%s:%d combination receiver_pos(%.1f %.1f)"
                                               ,__FILE__, __LINE__,
                                               receiver_pos.x, receiver_pos.y);
//                            fout.close();
                            return receiver_pos;
                        }
//                        }
                    }
                
                }
            }
            if(!spare)
            {
                receiver_pos_middle = getBestMiddle(agent, receiver_pos_middle, best_difference, length, best_width);
                vector2d_vector = getSearchOrder(agent, receiver_pos_middle,
                                                 best_difference, length, best_mate_r, best_opp_r);
                /**********************************/
/*
                if(!vector2d_vector.empty())
                {
                    fout<<"second"<<std::endl;
                    std::vector<rcsc::Vector2D>::const_iterator end = vector2d_vector.end();
                    for(std::vector<rcsc::Vector2D>::const_iterator p = vector2d_vector.begin();
                        p != end;
                        p++)
                    {
                        if((*p).absY() > rcsc::ServerParam::i().pitchHalfWidth())
                        {
                            fout<<(*p)<<std::endl;
                        }
                    }
                    fout<<std::endl<<std::endl;
                }
*/
                /***********************************/
                if(vector2d_vector.size() > 0)
                {
                    //if(wm.ball().pos().dist((*vector2d_vector.begin())) > best_ball_r)
                    double opp_r, mate_r;
                    //r = far_penalty_area_base_ball_r;
                    opp_r = far_penalty_area_base_opp_r;
                    mate_r = near_penalty_area_base_mate_r;
                    if(Obake_Analysis().checkExistOppPenaltyAreaIn(wm.ball().pos()))
                    {
                        //r = best_ball_r;
                        //opp_r = 1.5;
                        opp_r = 4.0;//3.5;//2.5;//1.0;
                        //mate_r = near_goal_base_mate_r;
                        mate_r = 3.0;
                    }
                    const rcsc::Circle2D opp_circle((*vector2d_vector.begin()), opp_r);
                    const rcsc::Circle2D mate_circle((*vector2d_vector.begin()), mate_r);
                    if(!wm.existTeammateIn(opp_circle, 10, false))
                    {
                        if(!(wm.ball().pos().x > rcsc::ServerParam::i().theirPenaltyAreaLineX()
                             &&(wm.ball().pos().y > 0.0
                                && wm.self().pos().y < wm.ball().pos().y
                                && (*vector2d_vector.begin()).y >= wm.ball().pos().y)
                             ||(wm.ball().pos().y < 0.0
                                && wm.self().pos().y > wm.ball().pos().y
                                && (*vector2d_vector.begin()).y <= wm.ball().pos().y)
                               || (wm.ball().pos().y * (*vector2d_vector.begin()).y <= 0.0
                                   && (*vector2d_vector.begin()).absY() > rcsc::ServerParam::i().goalAreaWidth())))
                        {
                            if(wm.self().pos().y * wm.ball().pos().y > 0.0
                               || wm.ball().pos().absY() <= rcsc::ServerParam::i().goalAreaHalfWidth())
                            {
                                const double pass_angle = 50.0;//45.0;//40.0;//30.0;
                                if(Obake_Analysis().checkPassCourse(agent,
                                                                    wm.ball().pos(),
                                                                    (*vector2d_vector.begin()),
                                                                    pass_angle))
                                {
                                    spare_target_pos = (*vector2d_vector.begin());
                                    spare = true;
                                }
/*                                if(Obake_Analysis().checkExistOppPenaltyAreaIn((*vector2d_vector.begin()))
                                   && wm.self().unum() == 11)
                                {
                                    std::cout<<"spare = "<<spare_target_pos<<std::endl;
                                    }*/
                            }
                            else
                            {
                                if(checkCombinationPassCourseCycle(agent, (*vector2d_vector.begin())))
                                {
                                    spare_target_pos = (*vector2d_vector.begin());
                                    spare = true;
                                }
                            }
                         
                        }
                    }
                }  
            }
            if(receiver_pos_middle.x >= rcsc::ServerParam::i().theirPenaltyAreaLineX()
               && receiver_pos_middle.absY() < rcsc::ServerParam::i().goalAreaWidth()
               && count == 0)
            {
                rcsc::Vector2D ball_to_receiver_pos = receiver_pos_middle - wm.ball().pos();
                if(ball_to_receiver_pos.y > 0.0)
                {
                    receiver_pos_middle.y -= best_difference / 2.0;
                }
                else
                {
                    receiver_pos_middle.y += best_difference / 2.0;
                }
                count++;
            }
            else 
            {
                break;
            }
        }
        receiver_pos_middle.x -= 2.0;//3.0;//4.0;
        if(receiver_pos_middle.x < rcsc::ServerParam::i().theirPenaltyAreaLineX()
            && std::abs(receiver_pos_middle.x - wm.offsideLineX()) > 10.0)
        {
            receiver_pos_middle.x -= 1.5;//1.0;
        }
    }

    receiver_pos = spare_target_pos;
    rcsc::dlog.addText(rcsc::Logger::TEAM,
                       "%s:%d receiver_pos(spare)(%.1f %.1f)"
                       ,__FILE__, __LINE__,
                       receiver_pos.x, receiver_pos.y);
//    fout.close();
    return receiver_pos;
}

rcsc::Vector2D
Bhv_ObakeReceive::getAdvancedPostion(rcsc::PlayerAgent * agent,
                                     const rcsc::Vector2D &receiver_pos)
{
    const rcsc::WorldModel & wm = agent->world();
    rcsc::Vector2D adanced_pos = receiver_pos;
    const double base_near_pos_y = receiver_pos.y;
    const double base_far_pos_y = wm.self().pos().y;
    const double degree_very_near_destination_x = std::pow(Obake_FuzzyGrade().degreeNearDestinationX(wm.self().pos().x,
                                                                                                     receiver_pos.x),
                                                           2);
    const double degree_near_offside_line = Obake_FuzzyGrade().degreeNearOffsideLine(agent,
                                                                                     receiver_pos.x);
    const double near_rate = degree_very_near_destination_x;
    /*
      Obake_FuzzyGrade().degreeNearDestinationX(wm.self().pos().x,
                               receiver_pos.x);
    */

    const double far_rate = Obake_FuzzyGrade().degreeFarDestinationX(wm.self().pos().x,
                                                                     receiver_pos.x);
    const double max_rate = std::max(far_rate, degree_near_offside_line);
    const double sum = near_rate + max_rate;
    adanced_pos.y = (base_near_pos_y * near_rate
                     + base_far_pos_y * max_rate)
                    / sum;
    return adanced_pos;
}

void
Bhv_ObakeReceive::setSearchRange(rcsc::PlayerAgent * agent,
                                 rcsc::Vector2D &front,
                                 rcsc::Vector2D &back)
{
    const rcsc::WorldModel & wm = agent->world();
    const double shoot_range_x = 38.0;
    const rcsc::Vector2D left_top(41.0, -rcsc::ServerParam::i().goalAreaHalfWidth());
    const int max_mate_number = (wm.ball().pos().absY() <= rcsc::ServerParam::i().goalAreaHalfWidth())
        ? 2 : 1;
    const double rect_length = rcsc::ServerParam::i().pitchHalfLength() - 41.0;
    const double rect_width = rcsc::ServerParam::i().goalAreaWidth();
        //const rcsc::Rect2D rect(left_top, rect_length, rect_width);
    const int mate_number = Obake_Analysis().getMateNumber(agent,
                                                           left_top,
                                                           rect_length,
                                                           rect_width);
    if(M_role_offensive_half)
    {
        
        if(!Obake_Analysis().checkExistOppPenaltyAreaIn(wm.ball().pos())
            )//&& opp_number >= 2/*!wm.existTeammateIn(rect, 10, true)*/))
        {
            if(wm.ball().pos().x >= rcsc::ServerParam::i().theirPenaltyAreaLineX())
            {
                //front.x = M_home_pos.x + 5.0;//3.0;
                front.x = std::max(front.x, rcsc::ServerParam::i().theirPenaltyAreaLineX()
                                   - rcsc::ServerParam::i().goalAreaLength());
            }
            else
            {
                front.x = M_home_pos.x;
            }
        }
        if(back.x < M_home_pos.x - 3.5)
        {
            back.x = M_home_pos.x - 3.5;
        }
        if(!(wm.self().pos().x >= rcsc::ServerParam::i().theirPenaltyAreaLineX()
             && wm.self().pos().absY() <= rcsc::ServerParam::i().goalAreaHalfWidth()))
        {
            back.x -= 5.0;
        }
        if(ObakeStrategy().getArea(wm.self().pos()) == ObakeStrategy::ShootChance
           && ObakeStrategy().getArea(front) != ObakeStrategy::ShootChance
           && wm.ball().pos().x >= rcsc::ServerParam::i().theirPenaltyAreaLineX())
        {
            if(mate_number <= max_mate_number)
            {
                front.y = wm.self().pos().y;
                back.y = wm.self().pos().y;
                front.x = rcsc::ServerParam::i().pitchHalfLength() - rcsc::ServerParam::i().goalAreaLength();
                back.x = shoot_range_x;
            }
        }
    }
    else if(M_role_deffensive_half)
    {
        if(Obake_Analysis().checkBackOfTheSideForward(agent))
        {
            front.x = M_home_pos.x - 5.0;
            back.x = M_home_pos.x - 5.0;
        }
    }
    else if(M_role_side_or_center_forward)
    {
        if(ObakeStrategy::getArea(M_home_pos) == ObakeStrategy::ShootChance)
        {
            if(back.x > shoot_range_x)
            {
                back.x = shoot_range_x;
            }
        }
        else if(wm.self().unum() == 11)
        {
            if(back.x < M_home_pos.x - 3.5)
            {
                back.x = M_home_pos.x - 3.5;
            }   
            if(wm.ball().pos().x >= rcsc::ServerParam::i().theirPenaltyAreaLineX()
               //&& wm.ball().pos().absY() > rcsc::ServerParam::i().penaltyAreaHalfWidth()
               /*               && mate_number >=max_mate_number*/) 
            {
                back.x = std::min(back.x, 38.0);
            }
        }
        else if(wm.self().unum() == 9 
                || wm.self().unum() == 10)
        {
            if(Obake_Analysis().checkExistOppPenaltyAreaIn(wm.ball().pos())
               && front.x > shoot_range_x
               && back.x < shoot_range_x)
            {
                back.x = shoot_range_x;
            }
            else if(wm.ball().pos().x >= wm.offsideLineX() - 10.0)
            {
                back.x = std::max(wm.offsideLineX() - 6.0,
                                  back.x);
            }
            else if(Obake_Analysis().checkExistOppPenaltyAreaIn(wm.ball().pos())
                    && M_home_pos.y * wm.ball().pos().y < 0.0
                    && back.x < M_home_pos.x - 6.0
                    && front.x > M_home_pos.x - 6.0)
            {
                back.x = M_home_pos.x - 6.0;
            }
        }
        const rcsc::Vector2D left_top(41.0, -rcsc::ServerParam::i().goalAreaHalfWidth());
        const double rect_length = rcsc::ServerParam::i().pitchHalfLength() - 41.0;
        const double rect_width = rcsc::ServerParam::i().goalAreaWidth();
        //const rcsc::Rect2D rect(left_top, rect_length, rect_width);
        const int mate_number = Obake_Analysis().getMateNumber(agent,
                                                               left_top,
                                                               rect_length,
                                                               rect_width);
        /*if(ObakeStrategy().getArea(front) == ObakeStrategy::ShootChance
           && mate_number >=1
           &&  M_home_pos.y * wm.ball().pos().y < 0.0)
        {
            back.x = 38.0;
            }*/
       if(ObakeStrategy().getArea(wm.self().pos()) == ObakeStrategy::ShootChance
          && ObakeStrategy().getArea(front) != ObakeStrategy::ShootChance
          && wm.ball().pos().x >= rcsc::ServerParam::i().theirPenaltyAreaLineX())
        {
            if(mate_number <= max_mate_number)
            {
                front.y = wm.self().pos().y;
                back.y = wm.self().pos().y;
                front.x = rcsc::ServerParam::i().pitchHalfLength() - rcsc::ServerParam::i().goalAreaLength();
                back.x = shoot_range_x;
            }
        }

    }

    const double max_range = (rcsc::ServerParam::i().goalHalfWidth() 
                              + rcsc::ServerParam::i().penaltyAreaHalfWidth()) / 2;

    if(front.absY() > max_range 
       || front.x >= rcsc::ServerParam::i().theirPenaltyAreaLineX())
    {
        front.x = std::min(front.x, wm.offsideLineX() - 1.0);
    }
    else
    {
        front.x = std::min(front.x, wm.offsideLineX() - 0.7);
    }

    back.x = std::min(back.x, front.x);
}


/*!
  This function is base on Body_Pass::create_direct_pass and
  Body_Pass::verify_direct_pass in librcsc-1.3.2
*/
bool
Bhv_ObakeReceive::checkPassCourseCycle(rcsc::PlayerAgent * agent,
                                       const rcsc::Vector2D &receiver_pos)
{
    //begin of the code of librcsc-1.3.2
    /********************************************/
    const rcsc::WorldModel & wm = agent->world();
    static const double player_dash_speed =(Obake_Analysis().checkExistOppPenaltyAreaIn(receiver_pos)) 
        ? 0.75 : 1.0;
    double end_speed, first_speed, receiver_dist, target_dist;
    const double near_penalty_area_base_opp_r = 1.5;
    const double not_near_penalty_area_base_opp_r = 3.0;
    const double very_near_penalty_area_base_virtual_dash_rate = 0.0;//0.3;
    const double not_very_near_penalty_area_base_virtual_dash_rate = 0.8;
    double degree_near_penalty_area_x, degree_not_near_penalty_area_x, 
        degree_very_near_penalty_area_x, degree_not_very_near_penalty_area_x,
        base_dist_from_opp, virtual_dash_rate;
    receiver_dist = 0.0;
    target_dist = wm.ball().pos().dist(receiver_pos);
    end_speed = 1.5;
    first_speed = 100.0;
    do
    {
        first_speed
            = rcsc::calc_first_term_geom_series_last
            (end_speed,
             receiver_dist,
             rcsc::ServerParam::i().ballDecay());
        if (first_speed < rcsc::ServerParam::i().ballSpeedMax())
        {
            break;
        }
        end_speed -= 0.1;
    }
    while (end_speed > 0.8);
    if (first_speed > rcsc::ServerParam::i().ballSpeedMax())
    {
        return false;
    }
    const rcsc::AngleDeg target_angle = (receiver_pos - wm.ball().pos()).th();
    const rcsc::Vector2D first_vel = rcsc::Vector2D::polar2vector(first_speed, target_angle);
    const rcsc::AngleDeg minus_target_angle = - target_angle;
    const double next_speed = first_speed * rcsc::ServerParam::i().ballDecay();
    const rcsc::PlayerPtrCont::const_iterator
        o_end = wm.opponentsFromSelf().end();
    for (rcsc::PlayerPtrCont::const_iterator
             it = wm.opponentsFromSelf().begin();
         it != o_end;
         ++it)
    {
        if ( (*it)->posCount() > 10 ) continue;
        if ( (*it)->isGhost() && (*it)->posCount() >= 4 ) continue;
/*
        const double virtual_dash
            = player_dash_speed * 0.8 * std::min( 5, (*it)->posCount() );

*/
        degree_near_penalty_area_x = Obake_FuzzyGrade().degreeNearOppPenaltyAreaX(receiver_pos.x);
        degree_not_near_penalty_area_x = 1 - degree_near_penalty_area_x;
        degree_very_near_penalty_area_x = pow(degree_very_near_penalty_area_x, 2.0);
        degree_not_very_near_penalty_area_x = 1 - degree_very_near_penalty_area_x;
        virtual_dash_rate = very_near_penalty_area_base_virtual_dash_rate * degree_very_near_penalty_area_x
            + not_very_near_penalty_area_base_virtual_dash_rate * degree_not_very_near_penalty_area_x;
        const double virtual_dash
            = player_dash_speed * virtual_dash_rate * std::min( 5, (*it)->posCount() );

        if ( ( (*it)->angleFromSelf() - target_angle ).abs() > 100.0 )
        {
            continue;
        }
        base_dist_from_opp = near_penalty_area_base_opp_r * degree_near_penalty_area_x
            + not_near_penalty_area_base_opp_r * degree_not_near_penalty_area_x;
        /*if ( (*it)->pos().dist2(receiver_pos) < 3.0 * 3.0 )
        {
            return false;
            }*/
        if((*it)->pos().dist2(receiver_pos) < base_dist_from_opp * base_dist_from_opp)
        {
            return false;
        }
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
                return false;
            }

            const double ball_steps_to_project
                = rcsc::calc_length_geom_series( next_speed,
                                                 ball_to_opp.x,
                                                 rcsc::ServerParam::i().ballDecay() );
            if ( ball_steps_to_project < 0.0
                 || opp2line_dist / player_dash_speed < ball_steps_to_project )
            {
                return false;
            }
        }
        /*else if(0.0 < ball_to_opp.x && (*it)->pos().dist(receiver_pos) < target_dist)
        {
	    const double dist_buf = 1.5;
            double opp_to_target_point_dist = (*it)->pos().dist(receiver_pos);
            opp_to_target_point_dist -= virtual_dash;
            opp_to_target_point_dist -= rcsc::ServerParam::i().defaultKickableArea();
            opp_to_target_point_dist -= 0.1;
            const double ball_steps_to_project
                = rcsc::calc_length_geom_series(next_speed,
                                                ball_to_opp.x,
                                                rcsc::ServerParam::i().ballDecay());
            if((opp_to_target_point_dist + dist_buf)/ player_dash_speed < ball_steps_to_project)
            {
                return false;
            }
            }*/
    }
    /********************************************/
    //end of the code of librcsc-1.3.2
    return true;
}

bool
Bhv_ObakeReceive::checkPassCourseCyclePrediction(rcsc::PlayerAgent * agent,
                                                 const rcsc::Vector2D &receiver_pos,
                                                 const double &dist)
{
    //begin of the code of librcsc-1.3.2
    /********************************************/
    const rcsc::WorldModel & wm = agent->world();
    static const double player_dash_speed = 1.0;
    double end_speed, first_speed, receiver_dist, target_dist;
    rcsc::Vector2D prediction_ball_pos, prediction_vector;
    prediction_ball_pos = wm.ball().pos();
    prediction_vector = wm.ball().vel();
    const rcsc::PlayerObject * nearest_mate_from_ball =
        wm.getTeammateNearestToBall(10);
    if(nearest_mate_from_ball)
    {
        if((nearest_mate_from_ball)->distFromBall() <= rcsc::ServerParam::i().defaultKickableArea() * 2.0)
        {
            prediction_vector = (nearest_mate_from_ball)->vel();
        }
    }
    prediction_vector.setLength(dist);
    prediction_ball_pos += prediction_vector;
    receiver_dist = 0.0;
    target_dist = prediction_ball_pos.dist(receiver_pos);
    end_speed = 1.5;
    first_speed = 100.0;
    do
    {
        first_speed
            = rcsc::calc_first_term_geom_series_last
            (end_speed,
             receiver_dist,
             rcsc::ServerParam::i().ballDecay());
        if (first_speed < rcsc::ServerParam::i().ballSpeedMax())
        {
            break;
        }
        end_speed -= 0.1;
    }
    while (end_speed > 0.8);
    if (first_speed > rcsc::ServerParam::i().ballSpeedMax())
    {
        return false;
    }
    const rcsc::AngleDeg target_angle = (receiver_pos - prediction_ball_pos).th();
    const rcsc::Vector2D first_vel = rcsc::Vector2D::polar2vector(first_speed, target_angle);
    const rcsc::AngleDeg minus_target_angle = - target_angle;
    const double next_speed = first_speed * rcsc::ServerParam::i().ballDecay();
    const rcsc::PlayerPtrCont::const_iterator
        o_end = wm.opponentsFromSelf().end();
    for (rcsc::PlayerPtrCont::const_iterator
             it = wm.opponentsFromSelf().begin();
         it != o_end;
         ++it)
    {
        if ( (*it)->posCount() > 10 ) continue;
        if ( (*it)->isGhost() && (*it)->posCount() >= 4 ) continue;

        const double virtual_dash
            = player_dash_speed * 0.8 * std::min( 5, (*it)->posCount() );

        if ( ( (*it)->angleFromSelf() - target_angle ).abs() > 100.0 )
        {
            continue;
        }

        if ( (*it)->pos().dist2(receiver_pos) < 3.0 * 3.0 )
        {
            return false;
        }

        rcsc::Vector2D ball_to_opp = (*it)->pos();
        ball_to_opp -= prediction_ball_pos;
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
                return false;
            }

            const double ball_steps_to_project
                = rcsc::calc_length_geom_series( next_speed,
                                                 ball_to_opp.x,
                                                 rcsc::ServerParam::i().ballDecay() );
            if ( ball_steps_to_project < 0.0
                 || opp2line_dist / player_dash_speed < ball_steps_to_project )
            {
                return false;
            }
        }
    }
    /********************************************/
    //end of the code of librcsc-1.3.2
    return true;
}

bool
Bhv_ObakeReceive::checkPassCourseCyclePrediction(rcsc::PlayerAgent * agent,
                                                 const rcsc::Vector2D &receiver_pos,
                                                 const rcsc::Vector2D &prediction_vector)
{
    //begin of the code of librcsc-1.3.2
    /********************************************/
    const rcsc::WorldModel & wm = agent->world();
    static const double player_dash_speed = 1.0;
    double end_speed, first_speed, receiver_dist, target_dist;
    rcsc::Vector2D prediction_ball_pos;
    prediction_ball_pos = wm.ball().pos();
    prediction_ball_pos += prediction_vector;
    receiver_dist = 0.0;
    target_dist = prediction_ball_pos.dist(receiver_pos);
    end_speed = 1.5;
    first_speed = 100.0;
    do
    {
        first_speed
            = rcsc::calc_first_term_geom_series_last
            (end_speed,
             receiver_dist,
             rcsc::ServerParam::i().ballDecay());
        if (first_speed < rcsc::ServerParam::i().ballSpeedMax())
        {
            break;
        }
        end_speed -= 0.1;
    }
    while (end_speed > 0.8);
    if (first_speed > rcsc::ServerParam::i().ballSpeedMax())
    {
        return false;
    }
    const rcsc::AngleDeg target_angle = (receiver_pos - prediction_ball_pos).th();
    const rcsc::Vector2D first_vel = rcsc::Vector2D::polar2vector(first_speed, target_angle);
    const rcsc::AngleDeg minus_target_angle = - target_angle;
    const double next_speed = first_speed * rcsc::ServerParam::i().ballDecay();
    const rcsc::PlayerPtrCont::const_iterator
        o_end = wm.opponentsFromSelf().end();
    for (rcsc::PlayerPtrCont::const_iterator
             it = wm.opponentsFromSelf().begin();
          it != o_end;
          ++it)
    {
        if ( (*it)->posCount() > 10 ) continue;
        if ( (*it)->isGhost() && (*it)->posCount() >= 4 ) continue;

        const double virtual_dash
            = player_dash_speed * 0.8 * std::min( 5, (*it)->posCount() );

        if ( ( (*it)->angleFromSelf() - target_angle ).abs() > 100.0 )
        {
            continue;
        }

        if ( (*it)->pos().dist2(receiver_pos) < 3.0 * 3.0 )
        {
            return false;
        }

        rcsc::Vector2D ball_to_opp = (*it)->pos();
        ball_to_opp -= prediction_ball_pos;
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
                return false;
            }

            const double ball_steps_to_project
                = rcsc::calc_length_geom_series( next_speed,
                                                 ball_to_opp.x,
                                                 rcsc::ServerParam::i().ballDecay() );
            if ( ball_steps_to_project < 0.0
                 || opp2line_dist / player_dash_speed < ball_steps_to_project )
            {
                return false;
            }
        }
    }
    /********************************************/
    //end of the code of librcsc-1.3.2
    return true;
}

bool
Bhv_ObakeReceive::checkLeasdPassCourseCycle(rcsc::PlayerAgent * agent,
                                            const rcsc::Vector2D &receiver_pos)
{
    const rcsc::WorldModel & wm = agent->world();
    static const double MAX_LEAD_PASS_DIST
      = 35.0;
    static const
      rcsc::Rect2D shrinked_pitch( -rcsc::ServerParam::i().pitchHalfLength() + 3.0,
                                   -rcsc::ServerParam::i().pitchHalfWidth() + 3.0,
                                   rcsc::ServerParam::i().pitchLength() - 6.0,
                                   rcsc::ServerParam::i().pitchWidth() - 6.0 );

    //static const double receiver_dash_speed = 0.9;
    static const double receiver_dash_speed = 0.8;

    /////////////////////////////////////////////////////////////////
    // too far
    if (wm.ball().pos().dist(receiver_pos) > MAX_LEAD_PASS_DIST )
    {
        return false; 
    }
    // too close
    if (wm.ball().pos().dist(receiver_pos) < 2.0 )
    {
        return false;
    }
    if(receiver_pos.x < wm.ball().pos().x - 15.0
       && receiver_pos.x < 15.0)
    {
        return false;
    }
    //
    if ( receiver_pos.x < -10.0
         && std::fabs(receiver_pos.y - wm.ball().pos().y ) > 20.0 )
    {
      // receiver is in our field. or Y diff is big
        return false;
    }
    const rcsc::Vector2D receiver_rel = receiver_pos - wm.ball().pos();
    const double receiver_dist = receiver_rel.r();
    const rcsc::AngleDeg receiver_angle = receiver_rel.th();

    const double circum = 2.0 * receiver_dist * M_PI;
    const double angle_step_abs = std::max( 5.0, 360.0 * ( 2.0 / circum ));
    /// angle loop
    double total_add_angle_abs;
    int count;
    for ( total_add_angle_abs = angle_step_abs, count = 0;
          total_add_angle_abs < 31.0 && count < 5;
          total_add_angle_abs += angle_step_abs, ++count )
    {
        /////////////////////////////////////////////////////////////
        // estimate required first ball speed
        const double dash_step
            = ( (angle_step_abs / 360.0) * circum ) / receiver_dash_speed + 2.0;

        double end_speed = 1.2 - 0.11 * count; // Magic Number
        double first_speed = 100.0;
        double ball_steps_to_target = 100.0;
        do
        {
            first_speed
                = rcsc::calc_first_term_geom_series_last
                (end_speed,
                 receiver_dist,
                 rcsc::ServerParam::i().ballDecay());
            //= required_first_speed( receiver_dist,
            //ServerParam::i().ballDecay(),
            //end_speed );
            if ( first_speed < rcsc::ServerParam::i().ballSpeedMax() )
            {
                break;
            }
            ball_steps_to_target
                = rcsc::calc_length_geom_series(first_speed,
                                                receiver_dist,
                                                rcsc::ServerParam::i().ballDecay() );
            //= move_step_for_first( receiver_dist,
            //first_speed,
            //ServerParam::i().ballDecay() );
            if ( dash_step < ball_steps_to_target )
            {
                break;
            }
            end_speed -= 0.1;
        }
        while ( end_speed > 0.5 );


        if ( first_speed > rcsc::ServerParam::i().ballSpeedMax() )
        {
            continue;
        }

        /////////////////////////////////////////////////////////////
        // angle plus minus loop
        for ( int i = 0; i < 2; i++ )
        {
            rcsc::AngleDeg target_angle = receiver_angle;
            if ( i == 0 )
            {
                target_angle -= total_add_angle_abs;
            }
            else
            {
                target_angle += total_add_angle_abs;
            }

            // check dir confidence
            int max_count = 100, ave_count = 100;
            wm.dirRangeCount( target_angle, 20.0,
			      &max_count, NULL, &ave_count );
            if ( max_count > 9 || ave_count > 3 )
            {
                continue;
            }

            const rcsc::Vector2D target_point
                = wm.ball().pos()
                + rcsc::Vector2D::polar2vector(receiver_dist, target_angle);

            /////////////////////////////////////////////////////////////////
            // ignore back pass
            if ( target_point.x < 0.0
                 && target_point.x <  wm.ball().pos().x )
            {
                continue;
            }

            if ( target_point.x < 0.0
                 && target_point.x < receiver_pos.x - 3.0 )
            {
                continue;
            }
            if ( target_point.x < receiver_pos.x - 6.0 )
            {
                continue;
            }
            // out of pitch
            if ( ! shrinked_pitch.contains( target_point ) )
            {
                continue;
            }
            // not safety area
            if ( target_point.x < -10.0 )
            {
                if (target_point.x < wm.defenseLineX() + 10.0)
                {
                    continue;
                }
                else if (target_point.x > wm.ball().pos().x + 20.0
                          && fabs(target_point.y - wm.self().pos().y) < 20.0)
                {
                    // safety clear ??
                }
                else if ( target_point.x > wm.ball().pos().x + 5.0 // forward than ball
                          && std::fabs( target_point.y - wm.self().pos().y ) < 20.0
		    ) // out side of me
                {
                    // safety area
                }
                else if (target_point.x > wm.defenseLineX() + 20.0)
                {
                    // safety area
                }
                else
                {
                    // dangerous
                    continue;
                }
            }
            /////////////////////////////////////////////////////////////////

            // add lead pass route
            // this methid is same as through pass verification method.
            if(verify_through_pass(agent,
                                   receiver_pos,
                                   target_point,
                                   receiver_dist,
                                   target_angle,
                                   first_speed,
                                   ball_steps_to_target))
            {
                return true;
            }            
        }
    }
    return false;
}

bool
Bhv_ObakeReceive::checkThroughPassCourseCycle(rcsc::PlayerAgent * agent,
                                              const rcsc::Vector2D &receiver_pos)
{
    const rcsc::WorldModel & wm = agent->world();
    static const double MAX_THROUGH_PASS_DIST
        = //35.0;
        0.9 * rcsc::inertia_final_distance( rcsc::ServerParam::i().ballSpeedMax(),
                                            rcsc::ServerParam::i().ballDecay() );

    ////////////////////////////////////////////////////////////////
    static const
        rcsc::Rect2D shrinked_pitch( -rcsc::ServerParam::i().pitchHalfLength() + 3.0,
                                     -rcsc::ServerParam::i().pitchHalfWidth() + 3.0,
                                     rcsc::ServerParam::i().pitchLength() - 6.0,
                                     rcsc::ServerParam::i().pitchWidth() - 6.0 );

    static const double receiver_dash_speed = 1.0;
    //static const double receiver_dash_speed = 0.85;

    static const double S_min_dash = 5.0;
    static const double S_max_dash = 25.0;
    static const double S_dash_range = S_max_dash - S_min_dash;
    static const double S_dash_inc = 4.0;
    static const int S_dash_loop
        = static_cast< int >( std::ceil( S_dash_range / S_dash_inc ) ) + 1;

    static const rcsc::AngleDeg S_min_angle = -20.0;
    static const rcsc::AngleDeg S_max_angle = 20.0;
    static const double S_angle_range = ( S_min_angle - S_max_angle ).abs();
    static const double S_angle_inc = 10.0;
    static const int S_angle_loop
        = static_cast< int >( std::ceil( S_angle_range / S_angle_inc ) ) + 1;

    /////////////////////////////////////////////////////////////////
    // check receiver position
    if ( receiver_pos.x > wm.offsideLineX() - 0.5 )
    {
        //receiver is offside

        return false;
    }
    if(receiver_pos.x < wm.ball().pos().x - 10.0 )
    {
        //receiver is back
        return false;
    }
    if (std::fabs(receiver_pos.y - wm.ball().pos().y ) > 35.0 )
    {
        //receiver Y diff is big
        return false;
    }
    if (wm.defenseLineX() < 0.0
        && receiver_pos.x < wm.defenseLineX() - 15.0)
    {
        // receiver is near to defense line
        return false;
    }
    if (wm.offsideLineX() < 30.0
        && receiver_pos.x < wm.offsideLineX() - 15.0)
    {
        //receiver is far from offside line;
        return false;
    }


    // angle loop
    rcsc::AngleDeg dash_angle = S_min_angle;
    for ( int i = 0; i < S_angle_loop; ++i, dash_angle += S_angle_inc )
    {
        const rcsc::Vector2D base_dash = rcsc::Vector2D::polar2vector( 1.0, dash_angle );

        // dash dist loop
        double dash_dist = S_min_dash;
        for ( int j = 0; j < S_dash_loop; ++j, dash_dist += S_dash_inc )
        {
            rcsc::Vector2D target_point = base_dash;
            target_point *= dash_dist;
            target_point += receiver_pos;

            if ( ! shrinked_pitch.contains( target_point ) )
            {
                // out of pitch
                continue;
            }

            if ( target_point.x < wm.ball().pos().x + 3.0 )
            {
                continue;
            }

            const rcsc::Vector2D target_rel = target_point - wm.ball().pos();
            const double target_dist = target_rel.r();
            const rcsc::AngleDeg target_angle = target_rel.th();

            // check dir confidence
            int max_count = 100, ave_count = 100;
            wm.dirRangeCount( target_angle, 20.0,
			      &max_count, NULL, &ave_count );
            if(max_count > 9 || ave_count > 3)
            {
                continue;
            }

            if(target_dist > MAX_THROUGH_PASS_DIST) // dist range over
            {
                continue;
            }

            if (target_dist < dash_dist) // I am closer than receiver
            {
                continue;
            }

            const double dash_step = dash_dist / receiver_dash_speed + 7.0;//+2.0

            double end_speed = 0.81;//0.65
            double first_speed = 100.0;
            double ball_steps_to_target = 100.0;
            double min_speed = 0.5;
            if(target_point.x >rcsc::ServerParam::i().theirPenaltyAreaLineX()
               && target_point.absY() <= rcsc::ServerParam::i().goalAreaHalfWidth())
            {
                min_speed = 0.3;
            }
            else if(wm.ball().pos().x <= wm.offsideLineX() - 8.0 
                    &&target_point.x >= wm.offsideLineX() + 5.0
                    && std::abs(target_point.y - wm.ball().pos().y) <= 12.5)
            {
                min_speed = 0.3;
//                std::cout<<"min_speed"<<std::endl;
            }
            else if(target_point.x  > 20.0
                    && target_point.absY() <= rcsc::ServerParam::i().penaltyAreaHalfWidth())
            {
                min_speed = 0.4;
            }
            do
            {
                first_speed
                    = rcsc::calc_first_term_geom_series_last
                    (end_speed,
                     target_dist,
                     rcsc::ServerParam::i().ballDecay());
                //= required_first_speed(target_dist,
                //ServerParam::i().ballDecay(),
                //end_speed);
                if(first_speed > rcsc::ServerParam::i().ballSpeedMax())
                {
                    end_speed -= 0.1;
                    continue;
                }

                ball_steps_to_target
                    = rcsc::calc_length_geom_series(first_speed,
                                                    target_dist,
                                                    rcsc::ServerParam::i().ballDecay());
                //= move_step_for_first( target_dist,
                //first_speed,
                //ServerParam::i().ballDecay() );
                //if ( ball_steps_to_target < dash_step )
                if(dash_step < ball_steps_to_target)
                {
                    break;
                }

                end_speed -= 0.1;
            }
            while(end_speed > min_speed);

            if (first_speed > rcsc::ServerParam::i().ballSpeedMax()
                || dash_step > ball_steps_to_target)
            {
                continue;
            }

            if(verify_through_pass(agent,
                                   receiver_pos,
                                   target_point, target_dist, target_angle,
                                   first_speed,
                                   ball_steps_to_target))
            {
                return true;
            }
        }
    }

    return false;
}

template<class T> void
Bhv_ObakeReceive:: putNewVectorIntoSearchOrder(std::vector<bool> &check_vector,
                                               std::vector<rcsc::Vector2D> &best_order_vector,
                                               const std::vector<int> &number_vector,
                                               const std::vector<rcsc::Vector2D> &search_point_vector,
                                               const std::vector<T> &base_vector)
{
    std::vector<T> greater_vector;
    int i, index, j;
    rcsc::Vector2D search_point;
    if(number_vector.size() == 1)
    {
        index = number_vector.at(0);
        search_point = search_point_vector.at(index);
        best_order_vector.push_back(search_point);
        setBoolVectorAtTrue(check_vector, index);
    }
    else
    {
        T criterion;
        for(i=0; i<base_vector.size(); i++)
        {
            criterion = base_vector.at(i);
            greater_vector.push_back(criterion);
        }
        sort(greater_vector.begin(), greater_vector.end(), std::greater<T>());
        for(i=0; i<greater_vector.size(); i++)
        {
            criterion = greater_vector.at(i);
            for(j=0; j<base_vector.size(); j++)
            {
                if(base_vector.at(j) == criterion)
                {
                    index = number_vector.at(j);
                    if(!check_vector.at(index))
                    {
                        search_point = search_point_vector.at(index);
                        best_order_vector.push_back(search_point);
                        setBoolVectorAtTrue(check_vector, index);
                    }
                }
            }
        }
    }
}

void
Bhv_ObakeReceive::setBoolVectorAtTrue(std::vector<bool> &v,
                                      const int index)
{
    std::vector<bool>::iterator
        p = v.begin();
    p += index;
    (*p) = true;
}

bool 
Bhv_ObakeReceive::verify_through_pass(rcsc::PlayerAgent * agent,
                                      const rcsc::Vector2D & receiver_pos,
                                      const rcsc::Vector2D & target_point,
                                      const double & target_dist,
                                      const rcsc::AngleDeg & target_angle,
                                      const double & first_speed,
                                      const double & reach_step)
{
    const rcsc::WorldModel & wm = agent->world();
    
    static const double player_dash_speed = 1.0;

    const rcsc::Vector2D first_vel = rcsc::Vector2D::polar2vector( first_speed, target_angle );
    const rcsc::AngleDeg minus_target_angle = -target_angle;
    const double next_speed = first_speed * rcsc::ServerParam::i().ballDecay();

    const double receiver_to_target = receiver_pos.dist( target_point );



    const rcsc::Interception interception( wm.ball().pos() + first_vel,
                                           first_speed * rcsc::ServerParam::i().ballDecay(),
                                           target_angle );
    {
        /*
          int cycle = static_cast< int >
            ( std::ceil( interception.getReachCycle( receiver_pos,
                                                     &rcsc::Vector2D(0.0, 0.0),
                                                     NULL,
                                                     0,
                                                     0.9, // kickable area + buf
                                                     1.0 ))); // dash speed
        rcsc::dlog.addText( rcsc::Logger::PASS,
                            "______ receiver reach cycle by util = %d.",
                            cycle );
        */
    }

    bool very_aggressive = false;
    const rcsc::Vector2D receiver_to_target_vector = target_point - receiver_pos;
    if ( target_point.x > 28.0
         && target_point.x > wm.ball().pos().x + 20.0 
         && !(wm.ball().pos().absY() <= rcsc::ServerParam::i().goalAreaHalfWidth()
              && target_point.x < rcsc::ServerParam::i().theirPenaltyAreaLineX()))
    {
        very_aggressive = true;
    }
    else if(wm.ball().pos().x > wm.offsideLineX() - 10.0
            && target_point.x > wm.offsideLineX() + 5.0)
    {
        very_aggressive = true;
    }
    else if(wm.ball().pos().x >= wm.offsideLineX() - 15.5
            && receiver_pos.x - wm.ball().pos().x >= 5.0
            && target_point.x > wm.offsideLineX())
    {
        very_aggressive = true;
    }
    else if ( target_point.x > wm.offsideLineX() + 15.0
              && target_point.x > wm.ball().pos().x + 15.0 
              && !(wm.ball().pos().absY() <= rcsc::ServerParam::i().goalAreaHalfWidth()
                   && target_point.x < rcsc::ServerParam::i().theirPenaltyAreaLineX()))
    {
        very_aggressive = true;
    }
    else if(target_point.x > wm.offsideLineX() + 5.0 
            && target_point.absY() <= (rcsc::ServerParam::i().goalAreaHalfWidth() + 3.0)
            && target_point.x >= 20.0
            && target_point.x < rcsc::ServerParam::i().theirPenaltyAreaLineX())
    {
        very_aggressive = true;
    }

    bool agressivle = false;
    if(target_point.x > wm.offsideLineX()
       && target_point.absY() <= rcsc::ServerParam::i().penaltyAreaHalfWidth()
       && target_point.x >= 15.0
       && target_point.x < rcsc::ServerParam::i().theirPenaltyAreaLineX())
    {
        agressivle = true;
    }

    const rcsc::PlayerPtrCont::const_iterator
        o_end = wm.opponentsFromSelf().end();
    for ( rcsc::PlayerPtrCont::const_iterator
              it = wm.opponentsFromSelf().begin();
          it != o_end;
          ++it )
    {
        if ( (*it)->posCount() > 10 ) continue;

        if ( (*it)->goalie() )
        {
            if ( target_point.absY() > rcsc::ServerParam::i().penaltyAreaWidth() - 3.0
                 || target_point.x < rcsc::ServerParam::i().theirPenaltyAreaLineX() + 2.0 )
            {
                continue;
            }
        }

/*        int cycle = static_cast< int >
            ( std::ceil( interception
                         .getReachCycle( (*it)->pos(),
                                         ( (*it)->velCount() <= 1 ? &((*it)->vel()) : NULL ),
                                         ( (*it)->bodyCount() == 0 ? &((*it)->body()) : NULL ),
                                         (*it)->posCount(),
                                         1.2, // kickable area + buf
                                         1.1 ) ) ); // dash speed
*/
        const double virtual_dash
            = player_dash_speed * std::min( 2, (*it)->posCount() );
        double turn_advantage = 0.0;
        if((*it)->goalie())
        {
            turn_advantage = 8.5;//6.5;
        }
        const double opp_to_target = (*it)->pos().dist( target_point );
        double dist_rate = ( very_aggressive ? 0.8 : 1.0);
        double dist_buf = ( very_aggressive ? 0.5 : 1.5);
        if(!very_aggressive && agressivle)
        {
            dist_rate = 0.9;
            dist_buf = 1.0;
        }
        if ( opp_to_target - virtual_dash - turn_advantage < receiver_to_target * dist_rate + dist_buf )
        {
            //opp is closer than receiver.",
            return false;
        }

        rcsc::Vector2D ball_to_opp = (*it)->pos();
        ball_to_opp -= wm.ball().pos();
        ball_to_opp -= first_vel;
        ball_to_opp.rotate(minus_target_angle);

        if ( 0.0 < ball_to_opp.x && ball_to_opp.x < target_dist )
        {
            double opp2line_dist = ball_to_opp.absY();
            opp2line_dist -= virtual_dash;
            if((*it)->goalie())
            {
                opp2line_dist -= rcsc::ServerParam::i().catchableArea();
            }
            else
            {
                opp2line_dist -= rcsc::ServerParam::i().defaultKickableArea();
            }
            opp2line_dist -= 0.1;
            if ( opp2line_dist < 0.0 )
            {
               //opp is already on pass line.",

                return false;
            }

            const double ball_steps_to_project
                = rcsc::calc_length_geom_series( next_speed,
                                                 ball_to_opp.x,
                                                 rcsc::ServerParam::i().ballDecay() );
            //= move_step_for_first( ball_to_opp.x,
            //next_speed,
            //ServerParam::i().ballDecay() );
            if ( ball_steps_to_project < 0.0
                 || opp2line_dist / player_dash_speed < ball_steps_to_project )
            {
                //opp reach pass line
                return false;
            }
            else if(0.0 < ball_to_opp.x && (*it)->pos().dist(receiver_pos) < target_dist)
            {
		double opp_to_target_point_dist = (*it)->pos().dist(receiver_pos);
                opp_to_target_point_dist -= virtual_dash;
                if((*it)->goalie())
                {
                    opp_to_target_point_dist -= rcsc::ServerParam::i().catchableArea();
                }
                else
                {
                    opp_to_target_point_dist -= rcsc::ServerParam::i().defaultKickableArea();
                }
                opp_to_target_point_dist -= 0.1;
                const double ball_steps_to_project
                    = rcsc::calc_length_geom_series(next_speed,
                                                    ball_to_opp.x,
                                                    rcsc::ServerParam::i().ballDecay());
                if(opp_to_target_point_dist / player_dash_speed < ball_steps_to_project)
                {
                    return false;
                }
            }
        }
    }
    return true;
}

/*!
This function is base on Body_Pass::create_direct_pass and
Body_Pass::verify_direct_pass in librcsc-1.3.2
*/
bool
Bhv_ObakeReceive::checkCombinationPassCourseCycle(rcsc::PlayerAgent * agent,
                                                  const rcsc::Vector2D &receiver_pos)
{
    /*
    The code in librcsc1.3.2 is used 
    in this code 
    except the part of code that is 
    concerned with "safe".
    */
    //begin of the code of librcsc-1.3.2 
    /************************************/
    const rcsc::WorldModel & wm = agent->world();
    static const double player_dash_speed = 1.0;
    const double max_pass_dist = 30.0;
    const double combination_dist =
        (wm.ball().pos().absY() <= rcsc::ServerParam::i().goalHalfWidth()) ? 3.0 : 5.0;
    double end_speed, first_speed, receiver_dist, target_dist;
    bool safe;
    receiver_dist = 0.0;
    rcsc::PlayerPtrCont::const_iterator end = wm.teammatesFromSelf().end();
    for(rcsc::PlayerPtrCont::const_iterator mate = wm.teammatesFromSelf().begin();
        mate != end;
        mate++)
    {
        safe = true;
        rcsc::Vector2D vector_to_combination_passer = receiver_pos - (*mate)->pos();
        if(wm.ball().pos().x >= rcsc::ServerParam::i().theirPenaltyAreaLineX()
           && vector_to_combination_passer.length() < combination_dist)
        {
            continue;
        }
        rcsc::Vector2D vector_to_ball = wm.ball().pos() - (*mate)->pos();
        if(vector_to_ball.y * vector_to_combination_passer.y >= 0.0
           && vector_to_ball.length() <= max_pass_dist 
           && vector_to_combination_passer.length() <= max_pass_dist)
        {

            target_dist = (*mate)->pos().dist(receiver_pos);
            end_speed = 1.5;
            first_speed = 100.0;
            do
            {
                first_speed
                    = rcsc::calc_first_term_geom_series_last
                    (end_speed,
                     receiver_dist,
                     rcsc::ServerParam::i().ballDecay());
                if (first_speed < rcsc::ServerParam::i().ballSpeedMax())
                {
                    break;
                }
                end_speed -= 0.1;
            }
            while (end_speed > 0.8);
            if (first_speed > rcsc::ServerParam::i().ballSpeedMax())
            {
                continue;
            }
            const rcsc::AngleDeg target_angle = (receiver_pos - (*mate)->pos()).th();
            const rcsc::Vector2D first_vel = rcsc::Vector2D::polar2vector(first_speed, target_angle);
            const rcsc::AngleDeg minus_target_angle = - target_angle;
            const double next_speed = first_speed * rcsc::ServerParam::i().ballDecay();
            const rcsc::PlayerPtrCont::const_iterator
                o_end = wm.opponentsFromSelf().end();
            for (rcsc::PlayerPtrCont::const_iterator
                     it = wm.opponentsFromSelf().begin();
                 it != o_end;
                 ++it )
            {
                if ( (*it)->posCount() > 10 ) continue;
                if ( (*it)->isGhost() && (*it)->posCount() >= 4 ) continue;

                const double virtual_dash
                    = player_dash_speed * 0.8 * std::min( 5, (*it)->posCount() );

                if ( ( (*it)->angleFromSelf() - target_angle ).abs() > 100.0 )
                {
                    continue;
                }
                if ( (*it)->pos().dist2(receiver_pos) < 3.0 * 3.0 )
                {
                    safe = false;
                }
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
                        safe = false;
                    }

                    const double ball_steps_to_project
                        = rcsc::calc_length_geom_series( next_speed,
                                                         ball_to_opp.x,
                                                         rcsc::ServerParam::i().ballDecay() );
                    if ( ball_steps_to_project < 0.0
                         || opp2line_dist / player_dash_speed < ball_steps_to_project )
                    {
                        safe = false;
                    }
                }
                if(!safe)
                {
                    break;
                }
            }
        }
        if(safe)
        {
            return true;
        }
    }
    /************************************/
    //end of the code of librcsc-1.3.2
    return false;
}

bool
Bhv_ObakeReceive::checkCombinationThroughPassCourseCycle(rcsc::PlayerAgent * agent,
                                                                        const rcsc::Vector2D &receiver_pos)
{
    const rcsc::WorldModel & wm = agent->world();
    static const double MAX_THROUGH_PASS_DIST
        = //35.0;
        0.9 * rcsc::inertia_final_distance( rcsc::ServerParam::i().ballSpeedMax(),
                                            rcsc::ServerParam::i().ballDecay() );

    ////////////////////////////////////////////////////////////////
    static const
        rcsc::Rect2D shrinked_pitch( -rcsc::ServerParam::i().pitchHalfLength() + 3.0,
                                     -rcsc::ServerParam::i().pitchHalfWidth() + 3.0,
                                     rcsc::ServerParam::i().pitchLength() - 6.0,
                                     rcsc::ServerParam::i().pitchWidth() - 6.0 );

    static const double receiver_dash_speed = 1.0;
    //static const double receiver_dash_speed = 0.85;

    static const double S_min_dash = 5.0;
    static const double S_max_dash = 25.0;
    static const double S_dash_range = S_max_dash - S_min_dash;
    static const double S_dash_inc = 4.0;
    static const int S_dash_loop
        = static_cast< int >( std::ceil( S_dash_range / S_dash_inc ) ) + 1;

    static const rcsc::AngleDeg S_min_angle = -20.0;
    static const rcsc::AngleDeg S_max_angle = 20.0;
    static const double S_angle_range = ( S_min_angle - S_max_angle ).abs();
    static const double S_angle_inc = 10.0;
    static const int S_angle_loop
        = static_cast< int >( std::ceil( S_angle_range / S_angle_inc ) ) + 1;

    /////////////////////////////////////////////////////////////////
    // check receiver position
    if ( receiver_pos.x > wm.offsideLineX() - 0.5 )
    {
        //receiver is offside

        return false;
    }
    if(receiver_pos.x < wm.ball().pos().x - 10.0 )
    {
        //receiver is back
        return false;
    }
    if (std::fabs(receiver_pos.y - wm.ball().pos().y ) > 35.0 )
    {
        //receiver Y diff is big
        return false;
    }
    if (wm.defenseLineX() < 0.0
        && receiver_pos.x < wm.defenseLineX() - 15.0)
    {
        // receiver is near to defense line
        return false;
    }
    if (wm.offsideLineX() < 30.0
        && receiver_pos.x < wm.offsideLineX() - 15.0)
    {
        //receiver is far from offside line;
        return false;
    }


    // angle loop
    rcsc::AngleDeg dash_angle = S_min_angle;
    for ( int i = 0; i < S_angle_loop; ++i, dash_angle += S_angle_inc )
    {
        const rcsc::Vector2D base_dash = rcsc::Vector2D::polar2vector( 1.0, dash_angle );

        // dash dist loop
        double dash_dist = S_min_dash;
        for ( int j = 0; j < S_dash_loop; ++j, dash_dist += S_dash_inc )
        {
            rcsc::Vector2D target_point = base_dash;
            target_point *= dash_dist;
            target_point += receiver_pos;

            if ( ! shrinked_pitch.contains( target_point ) )
            {
                // out of pitch
                continue;
            }

            if ( target_point.x < wm.ball().pos().x + 3.0 )
            {
                continue;
            }

            const rcsc::Vector2D target_rel = target_point - wm.ball().pos();
            const double target_dist = target_rel.r();
            const rcsc::AngleDeg target_angle = target_rel.th();

            // check dir confidence
            int max_count = 100, ave_count = 100;
            wm.dirRangeCount( target_angle, 20.0,
			      &max_count, NULL, &ave_count );
            if(max_count > 9 || ave_count > 3)
            {
                continue;
            }

            if(target_dist > MAX_THROUGH_PASS_DIST) // dist range over
            {
                continue;
            }

            if (target_dist < dash_dist) // I am closer than receiver
            {
                continue;
            }

            const double dash_step = dash_dist / receiver_dash_speed + 7.0;//+2.0

            double end_speed = 0.81;//0.65
            double first_speed = 100.0;
            double ball_steps_to_target = 100.0;
            double min_speed = 0.5;
            if(target_point.x >rcsc::ServerParam::i().theirPenaltyAreaLineX()
               && target_point.absY() <= rcsc::ServerParam::i().goalAreaHalfWidth())
            {
                min_speed = 0.3;
            }
            else if(wm.ball().pos().x <= wm.offsideLineX() - 8.0 
                    &&target_point.x >= wm.offsideLineX() + 5.0
                    && std::abs(target_point.y - wm.ball().pos().y) <= 12.5)
            {
                min_speed = 0.3;
//                std::cout<<"min_speed"<<std::endl;
            }
            else if(target_point.x  > 20.0
                    && target_point.absY() <= rcsc::ServerParam::i().penaltyAreaHalfWidth())
            {
                min_speed = 0.4;
            }
            do
            {
                first_speed
                    = rcsc::calc_first_term_geom_series_last
                    (end_speed,
                     target_dist,
                     rcsc::ServerParam::i().ballDecay());
                //= required_first_speed(target_dist,
                //ServerParam::i().ballDecay(),
                //end_speed);
                if(first_speed > rcsc::ServerParam::i().ballSpeedMax())
                {
                    end_speed -= 0.1;
                    continue;
                }

                ball_steps_to_target
                    = rcsc::calc_length_geom_series(first_speed,
                                                    target_dist,
                                                    rcsc::ServerParam::i().ballDecay());
                //= move_step_for_first( target_dist,
                //first_speed,
                //ServerParam::i().ballDecay() );
                //if ( ball_steps_to_target < dash_step )
                if(dash_step < ball_steps_to_target)
                {
                    break;
                }

                end_speed -= 0.1;
            }
            while(end_speed > min_speed);

            if (first_speed > rcsc::ServerParam::i().ballSpeedMax()
                || dash_step > ball_steps_to_target)
            {
                continue;
            }

            if(verify_through_pass(agent,
                                   receiver_pos,
                                   target_point, target_dist, target_angle,
                                   first_speed,
                                   ball_steps_to_target))
            {
                return true;
            }
        }
    }

    return false;
}

/*!
This function check whether there are opponensts 
in near regions 
number_per_width is  the number of the points that are searched
except middle point.
ex.
the number of the search point 5
number_per_width 4(except middle point)
*/
/*
bool
Bhv_ObakeReceive::checkOpponentInRegion(rcsc::PlayerAgent * agent,
                                        bool * region,
                                        const rcsc::Vector2D &left_middle,
                                        const double &length,
                                        const double &width,
                                        const int number_per_length,
                                        const int number_per_width)
{
    if(length <= 0.0 || width < 0.0
       || number_per_length <= 0 || number_per_width <= 0)
    {      
        return false;
    }
    const rcsc::WorldModel & wm = agent->world();
    rcsc::Vector2D left_top(left_middle.x, left_middle.y - width * number_per_width / 2);
    int i, j;
    rcsc::Rect2D check_region(left_top, length, width);
    for(i=0; i<number_per_width; i++)
    {
        left_top.x = left_middle.x;
        left_top.y = left_middle.y + width * (i - number_per_width / 2);
        check_region.setTopLeft(left_top);
        for(j=0; j<number_per_length; j++)
        {
            (*region) = (wm.existOpponentIn(check_region, 10, true)) ? true : false;
            left_top.x += length;
            check_region.setTopLeft(left_top);
            region++;
        }
    }
    return true;
}
*/
/*
bool
Bhv_ObakeReceive::checkExistOpponentInRegion(rcsc::PlayerAgent * agent,
                                             const double &length,
                                             const double &width,
                                             const rcsc::Vector2D &serach_point)
{
    const rcsc::WorldModel & wm = agent->world();
    const rcsc::Rect2D region(serach_point, length, width);
    if(!wm.existOpponentIn(region, 10, true))
    {
        return false;
    }
    return true;
}
*/

bool
Bhv_ObakeReceive::checkExistShootCourse(rcsc::PlayerAgent * agent,
                                        const rcsc::Vector2D &receiver_pos,
                                        const double &angle,
                                        const double &r)
{
    const rcsc::Vector2D goal_top(rcsc::ServerParam::i().pitchHalfLength(),
                                  -rcsc::ServerParam::i().goalHalfWidth() + 1.5);
    const rcsc::Vector2D goal_middle(rcsc::ServerParam::i().pitchHalfLength(),
                                     0.0);
    const rcsc::Vector2D goal_bottom(rcsc::ServerParam::i().pitchHalfLength(),
                                     rcsc::ServerParam::i().goalHalfWidth() - 1.5);
    std::list<rcsc::Vector2D> target_list;
    target_list.push_back(goal_top);
    target_list.push_back(goal_middle);
    target_list.push_back(goal_bottom);
    if(receiver_pos.absY() < rcsc::ServerParam::i().goalHalfWidth() - 1.0/*1.5*/)
    {
        target_list.push_back(rcsc::Vector2D(rcsc::ServerParam::i().pitchHalfLength(),
                                             receiver_pos.y));
    }
    const std::list<rcsc::Vector2D>::const_iterator end = target_list.end();
    for(std::list<rcsc::Vector2D>::const_iterator target = target_list.begin();
        target != end;
        target++)
    {
        if(!Obake_Analysis().checkExistOpponent(agent,
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

int
Bhv_ObakeReceive::getDribbleCourseNumber(rcsc::PlayerAgent * agent,
                                         const rcsc::Vector2D &search_point,
                                         const double & angle)
{
    int dribble_course_number = 0;
    const double dribble_dist = 10.0;
    const std::vector<rcsc::Vector2D> dribble_target_point_vector = Obake_Analysis().getDribbleTargetPointVector(agent,
                                                                                                                 search_point,
                                                                                                                 dribble_dist);
    const std::vector<rcsc::Vector2D>::const_iterator 
        t_end = dribble_target_point_vector.end();
    for(std::vector<rcsc::Vector2D>::const_iterator
            t = dribble_target_point_vector.begin();
        t != t_end;
        t++)
    {
        if(!Obake_Analysis().checkExistOpponent(agent,
                                                search_point,
                                                (*t),
                                                angle))
        {
            dribble_course_number++;
        }
    }
    return dribble_course_number;
}

/*!
This function analyzes near opponensts and
returns the best middle to give getSearchOrder, 
which is often middle between two opponents
*/
/*
rcsc::Vector2D
Bhv_ObakeReceive::getBestMiddle(rcsc::PlayerAgent * agent,
                                const rcsc::Vector2D &middle,
                                const double &difference,
                                const double &length,
                                const double &width)

{
    std::list<double> opp_y_list = Obake_Analysis().getOppYList(agent, length, width);  
    rcsc::Vector2D best_middle = middle;
    if(opp_y_list.size() >= 1)
    {
        double base, half_width, max_dist;
        const std::list<double>::const_iterator end = opp_y_list.end();
        std::list<double>::const_iterator opp = opp_y_list.begin();
        half_width = width / 2;
        base = middle.y - half_width;
        if(base < -rcsc::ServerParam::i().pitchHalfWidth())
        {
            base = -rcsc::ServerParam::i().pitchHalfWidth();
        }
        max_dist= (*opp) - base;
        best_middle.y = ((*opp) + base) / 2;
        base = (*opp);
        if(base > rcsc::ServerParam::i().pitchHalfWidth())
        {
            base = rcsc::ServerParam::i().pitchHalfWidth();
        }
        opp_y_list.push_back(middle.y + half_width);
        while(opp != end)
        {
            if(max_dist < ((*opp) - base))
            {
                best_middle.y = ((*opp) + base) / 2;
            }
            base = (*opp);
            opp++;
        }
    }
    const rcsc::Vector2D v = best_middle - middle;
    if(v.absY() > difference)
    {
        best_middle.y += (v.y > 0.0) ?  -difference : difference;
    }
    
    return best_middle;
}
*/

rcsc::Vector2D
Bhv_ObakeReceive::getBestMiddle(rcsc::PlayerAgent * agent,
                                const rcsc::Vector2D &middle,
                                const double &difference,
                                const double &length,
                                const double &width)

{
    const rcsc::Vector2D left_top(middle.x, -rcsc::ServerParam::i().pitchHalfWidth());
    const double half_width = width / 2.0;
    const double safe_dist = 2.5;
    const double shoot_half_width = 11.0;
    std::vector<double> opp_y_vector = Obake_Analysis().getOppYVector(agent,
                                                                      left_top,
                                                                      length,
                                                                      rcsc::ServerParam::i().pitchWidth());  
    double adjusted_middle_y = middle.y;
    const rcsc::Vector2D mate_left_top(41.0, -rcsc::ServerParam::i().goalAreaHalfWidth());
    const double rect_length = rcsc::ServerParam::i().pitchHalfLength() - 41.0;
    const double rect_width = rcsc::ServerParam::i().goalAreaWidth();
    //const rcsc::Rect2D rect(left_top, rect_length, rect_width);
    const int mate_number = Obake_Analysis().getMateNumber(agent,
                                                           mate_left_top,
                                                           rect_length,
                                                           rect_width);
    if(adjusted_middle_y - half_width < -rcsc::ServerParam::i().pitchHalfWidth() + safe_dist)
    {
        adjusted_middle_y = -rcsc::ServerParam::i().pitchHalfWidth() + safe_dist + half_width;
    }
    else if(adjusted_middle_y + half_width > rcsc::ServerParam::i().pitchHalfWidth() - safe_dist)
    {
        adjusted_middle_y = rcsc::ServerParam::i().pitchHalfWidth() - safe_dist - half_width;
    }
    else if(ObakeStrategy().getArea(middle) == ObakeStrategy::ShootChance
            && mate_number == 0)
    {
        const rcsc::WorldModel & wm = agent->world();
        /*if(wm.ball().pos().y * middle.y < 0.0)
          {*/
        if((std::abs(adjusted_middle_y) + half_width > rcsc::ServerParam::i().goalAreaHalfWidth()
            && wm.ball().pos().y * adjusted_middle_y < 0.0)
           ||  std::abs(adjusted_middle_y) + half_width > shoot_half_width)
        {
            double base_dist = 11.0;
            if(wm.ball().pos().absY() > rcsc::ServerParam::i().goalAreaHalfWidth()
               && wm.ball().pos().y * adjusted_middle_y <= 0.0)
            {
                base_dist = rcsc::ServerParam::i().goalAreaHalfWidth();
            }
            adjusted_middle_y = (adjusted_middle_y > 0.0) 
                ? base_dist - half_width : -base_dist + half_width;
        }
//        }
    }
    const double top_y = adjusted_middle_y - half_width;
    const double bottom_y = adjusted_middle_y + half_width;
    rcsc::Vector2D best_middle = rcsc::Vector2D(middle.x, adjusted_middle_y);
    if(opp_y_vector.size() >= 1)
    {
        double dist, max_dist, temporary_middle_y;
        const std::vector<double>::const_iterator opp_end = opp_y_vector.end();
        std::vector<double>::const_iterator opp = opp_y_vector.begin();
        double previous_opp_y = (*opp);
        dist = std::abs((*opp) - adjusted_middle_y);
        if(dist <= half_width)
        {
            max_dist = ((*opp) - top_y) * 2.0;
            best_middle.y = top_y;
            opp++;
            while(opp != opp_end)
            {
                if((*opp) < adjusted_middle_y + half_width)
                {
                    dist = (*opp) - previous_opp_y;
                    if(dist > max_dist)
                    {
                        best_middle.y = ((*opp) + previous_opp_y) / 2;
                        max_dist = dist;
                    }
                    previous_opp_y = (*opp);
                    opp++;
                }
                else
                {
                    dist = (*opp) - previous_opp_y;
                    if(dist > max_dist)
                    {
                        if(((*opp) + previous_opp_y) / 2.0 <= adjusted_middle_y + half_width)
                        {
                            best_middle.y = ((*opp) + previous_opp_y) / 2;
                            max_dist = dist;
                        }
                        else if((bottom_y - previous_opp_y) * 2.0 > max_dist)
                        {
                            best_middle.y = bottom_y;
                            max_dist = dist;
                        }
                    }
                    previous_opp_y = (*opp);
                    break;
                }
            }
            if(previous_opp_y < adjusted_middle_y + half_width)
            {
                dist = bottom_y - previous_opp_y;
                if(dist * 2.0 > max_dist)
                {
                    best_middle.y = bottom_y;
                }
            }
        }
        else if(bottom_y < (*opp))
        {
            best_middle.y = top_y;
        }
        else
        {
            if(opp_y_vector.size() == 1)
            {
                best_middle.y = bottom_y;
            }
            else
            {
                opp++;
                max_dist = 0.0;
                while(opp != opp_end)
                {
                    if(top_y < (*opp))
                    {
                        temporary_middle_y = ((*opp) + previous_opp_y) / 2.0;
                        if(temporary_middle_y >= top_y)
                        {
                            if(temporary_middle_y > bottom_y)
                            {
                                dist = (bottom_y - previous_opp_y) * 2.0;
                                if(dist > max_dist)
                                {
                                    best_middle.y = bottom_y;
                                    break;
                                }
                            }
                            else
                            {
                                dist = (*opp) - previous_opp_y;
                                if(dist > max_dist)
                                {
                                    best_middle.y = ((*opp) + previous_opp_y) / 2.0;
                                    max_dist = dist;
                                }
                            }
                        }
                        else
                        {
                            dist = ((*opp) - top_y) * 2.0;
                            if(dist > max_dist)
                            {
                                best_middle.y = top_y;
                                max_dist = dist;
                            }
                        }
                    }
                    previous_opp_y = (*opp);
                    opp++;
                }                
                if(previous_opp_y < adjusted_middle_y + half_width)
                {
                    dist = bottom_y - previous_opp_y;
                    if(dist * 2.0 > max_dist)
                    {
                        best_middle.y = bottom_y;
                    }
                }
            }
        }
    }
    const double one_third_width = width / 3.0;
    if(ObakeStrategy().getArea(middle) == ObakeStrategy::ShootChance
       && best_middle.absY() + difference > shoot_half_width
       && mate_number <= 1)
    {
       best_middle.y += (best_middle.y > 0.0)
           ? -difference : difference;

    }     
    else if(best_middle.absY() + difference > rcsc::ServerParam::i().pitchHalfWidth() - safe_dist)
    {
        best_middle.y +=(best_middle.y > 0.0)
            ? -difference : difference;
        
    }
    else
    {
        if(best_middle.y <= adjusted_middle_y - width + one_third_width)
        {
            best_middle.y += difference;
        }
        else if(best_middle.y >= adjusted_middle_y + width - one_third_width)
        {
            best_middle.y -= difference;
        }
    }
    return best_middle;
}

/*!
This function returns the best order for searching
receiver positions where a player can receive the ball
*/
/*
std::list<rcsc::Vector2D>
Bhv_ObakeReceive::getSearchOrder(rcsc::PlayerAgent * agent,
                                 const rcsc::Vector2D &middle,
                                 const double &difference,
                                 const double &length,
                                 const double &mate_r,
                                 const double &opp_r)
{
    int i;
    const rcsc::WorldModel & wm = agent->world();
    const int number_per_length = 1;
    const int number_per_width = 4;
    const int all = number_per_length * number_per_width;
    const int vector_number = 3;
    double width = 4.0;
    if(wm.ball().pos().x >= (wm.offsideLineX() - 6.5))
    {
        width -= 0.15;
    }
    const double search_length = 10.0;
    const double search_long_width = 25.0;
    const double search_short_width = 15.0;
    const double half_search_long_width = search_long_width / 2;
    const double half_search_short_width = search_short_width / 2;
    const rcsc::Vector2D left_long_top(wm.self().pos().x, wm.self().pos().y - half_search_long_width);
    const rcsc::Vector2D left_short_top(wm.self().pos().x, wm.self().pos().y - half_search_short_width);
    rcsc::Vector2D top(middle.x, middle.y - difference);
    rcsc::Vector2D bottom(middle.x, middle.y + difference);
    if(top.y < -rcsc::ServerParam::i().pitchHalfWidth() + 3.0)
    {
        top.y = -rcsc::ServerParam::i().pitchHalfWidth() + 4.0;
    }
    if(bottom.y > rcsc::ServerParam::i().pitchHalfWidth() - 3.0)
    {
        bottom.y = rcsc::ServerParam::i().pitchHalfWidth() - 4.0;
    }
    if(Obake_Analysis().getOppNumber(agent, left_short_top, search_length, search_short_width) >= 3)
    {
        width -= 2.5;
    }
    else if(Obake_Analysis().getOppNumber(agent, left_long_top, search_length, search_long_width) >= 3)
    {
        width -= 1.5;
    }
    double mate_dist, opp_dist; 
    bool * region;
    region = new bool[all];
    std::list<rcsc::Vector2D> vector_list;    
    std::list<double> dist_list;
    std::list<double> rest_dist_list;
    const double angle = 30.0;
    const double r = 3.5;
    if(checkOpponentInRegion(agent, region, middle, length, width,
                             number_per_length, number_per_width))
    {
        for(i=0; i<vector_number; i++)
        {
            switch(i){
            case 0:
                mate_dist = Obake_Analysis().getDistFromNearestMate(agent, top);
                opp_dist = Obake_Analysis().getDistFromNearestOpp(agent, top);
                break;
            case 1:
                mate_dist = Obake_Analysis().getDistFromNearestMate(agent, middle);
                opp_dist = Obake_Analysis().getDistFromNearestOpp(agent, middle);
                break;
            default:
                mate_dist = Obake_Analysis().getDistFromNearestMate(agent, bottom);
                opp_dist = Obake_Analysis().getDistFromNearestOpp(agent, bottom);
            }
            if(((region[i] || region[i+1])
                 || (i == 0 && ObakeStrategy().getArea(top) == ObakeStrategy::ShootChance
                     && checkExistShootCourse(agent,
                                              top,
                                              angle,
                                              r))
                 || (i == 1 && ObakeStrategy().getArea(middle) == ObakeStrategy::ShootChance
                     && checkExistShootCourse(agent,
                                              middle,
                                              angle,
                                              r))
                 || (i == 2 && ObakeStrategy().getArea(bottom) == ObakeStrategy::ShootChance
                     && checkExistShootCourse(agent,
                                              bottom,
                                              angle,
                                              r)))
               && mate_dist >= mate_r && opp_dist >= opp_r) 
            {
                rest_dist_list.push_back(opp_dist);
            }
            else if(mate_dist >= mate_r && opp_dist >= opp_r)
            {
                switch(i){
                case 0:
                    dist_list.push_back(Obake_Analysis().getDistFromNearestOpp(agent, top));
                    break;
                case 1:
                    dist_list.push_back(Obake_Analysis().getDistFromNearestOpp(agent, middle));
                    break;
                case 2:
                    dist_list.push_back(Obake_Analysis().getDistFromNearestOpp(agent, bottom));
                }   
            }
        }
        dist_list.sort(std::greater<double>());
        std::list<double>::const_iterator end = dist_list.end();
        double dist_top, dist_middle;
        dist_top = Obake_Analysis().getDistFromNearestOpp(agent, top);
        dist_middle = Obake_Analysis().getDistFromNearestOpp(agent, middle);
        for(std::list<double>::const_iterator p = dist_list.begin();
            p != end;
            p++)
        {
            if((*p) == dist_top)
            {
                vector_list.push_back(top);
            }
            else if((*p) == dist_middle)
            {
                vector_list.push_back(middle);
            }
            else
            {
                vector_list.push_back(bottom);
            }        
        }
        rest_dist_list.sort(std::greater<double>());
        for(std::list<double>::const_iterator p = rest_dist_list.begin();
            p != rest_dist_list.end();
            p++)
        {
            if((*p) == dist_top)
            {
                vector_list.push_back(top);
            }
            else if((*p) == dist_middle)
            {
                vector_list.push_back(middle);
            }
            else
            {
                vector_list.push_back(bottom);
            }        
        }
    }
    return vector_list;
}
*/

std::vector<rcsc::Vector2D>
Bhv_ObakeReceive::getSearchOrder(rcsc::PlayerAgent * agent,
                                 const rcsc::Vector2D &middle,
                                 const double &difference,
                                 const double &length,
                                 const double &mate_r,
                                 const double &opp_r)
{
    const rcsc::WorldModel & wm = agent->world();
    int i;
    const int search_point_size = 3;
    const double shoot_angle = 30.0;
    const double dribble_angle = 30.0;
    const double r = 2.5;
    std::vector<rcsc::Vector2D> best_order_vector, search_point_vector;
    std::vector<double> mate_dist_vector, opp_dist_vector;
    std::vector<bool> check_vector(search_point_size, false);
    
    const rcsc::Vector2D top(middle.x, middle.y - difference);
    rcsc::Vector2D search_point = top;
    for(i=0; i<search_point_size; i++)
    {
        mate_dist_vector.push_back(Obake_Analysis().getDistFromNearestMate(agent, search_point));
        opp_dist_vector.push_back(Obake_Analysis().getDistFromNearestOpp(agent, search_point));
        search_point_vector.push_back(search_point);
        search_point.y += difference;
    }

    //put into search point where an agent can shoot best_order_vector
    for(std::vector<bool>::size_type i=0; i<check_vector.size(); i++)
    {
        if(!(opp_dist_vector.at(i) > opp_r
             && mate_dist_vector.at(i) > mate_r))
        {
            setBoolVectorAtTrue(check_vector, i);
            continue;
        }
        search_point = search_point_vector.at(i);
        if(Obake_Analysis().checkExistOppPenaltyAreaIn(wm.ball().pos())
           && wm.ball().pos().y * search_point.y < 0.0
           && search_point.absY() > rcsc::ServerParam::i().goalAreaHalfWidth()
           && search_point.x >= rcsc::ServerParam::i().theirPenaltyAreaLineX())
        {
            setBoolVectorAtTrue(check_vector, i);
        }
    }
    opp_dist_vector.clear();
    mate_dist_vector.clear();
    std::vector<int> number_vector;
    
    for(std::vector<int>::size_type i=0; i<search_point_vector.size(); i++)
    {
        search_point = search_point_vector.at(i);
        if(ObakeStrategy().getArea(search_point) == ObakeStrategy::ShootChance
           && !check_vector.at(i))
        {
            if(checkExistShootCourse(agent,
                                     search_point_vector.at(i),
                                     shoot_angle,
                                     r))
            {
                opp_dist_vector.push_back(Obake_Analysis().getDistFromNearestOpp(agent, search_point));
                number_vector.push_back(i);
            }
        }
    }
    if(number_vector.size() >= 1)
    {
        putNewVectorIntoSearchOrder(check_vector,
                                    best_order_vector,
                                    number_vector,
                                    search_point_vector,
                                    opp_dist_vector);
    }
//    std::cout<<"l3"<<std::endl;

    number_vector.clear();

    int assist_course_number, count, dribble_course_number;
    std::vector<int> dribble_course_number_vector;
    count = 0;
    std::vector<double> degree_near_goal_y_vector;
    double degree_near_goal_y;

    // consider assist
    if(wm.ball().pos().x >= rcsc::ServerParam::i().theirPenaltyAreaLineX())
    {
        const double pass_angle = 30.0;
        const double max_pass_dist = 35.0;
        const bool except_nearest_mate = true;
        std::vector<int> assist_course_number_vector;
        //std::cout<<"unum = "<<wm.self().unum()<<std::endl;
        for(std::vector<bool>::size_type i=0; i<check_vector.size(); i++)
        {
            if(!check_vector.at(i))
            {
                search_point = search_point_vector.at(i);
                assist_course_number = Obake_Analysis().getAssistCourseNumber(agent,
                                                                              search_point,
                                                                              pass_angle,
                                                                              max_pass_dist,
                                                                              except_nearest_mate);
                if(assist_course_number >= 1)
                {
                    assist_course_number_vector.push_back(assist_course_number);
                    number_vector.push_back(i);
                    //std::cout<<assist_course_number<<", ";
                }
            }
            //std::cout<<std::endl;
        }

        putNewVectorIntoSearchOrder(check_vector,
                                    best_order_vector,
                                    number_vector,
                                    search_point_vector,
                                    assist_course_number_vector);
    }

    //consider dribble
    number_vector.clear();
    for(std::vector<bool>::size_type i=0; i<check_vector.size(); i++)
    {
        if(!check_vector.at(i))
        {
            search_point = search_point_vector.at(i);
            dribble_course_number = getDribbleCourseNumber(agent,
                                                           search_point,
                                                           dribble_angle);
            if(dribble_course_number >= 1)
            {            
                if(std::abs(middle.y - wm.ball().pos().y) > 30.0)
                {
                    degree_near_goal_y = 1.0 /search_point_vector.at(i).absY();
                    degree_near_goal_y_vector.push_back(degree_near_goal_y);
                }
                else
                {
                    dribble_course_number_vector.push_back(dribble_course_number);
                }

                number_vector.push_back(i);
            }
        }
    }
    
    if(std::abs(middle.y - wm.ball().pos().y) > 30.0)
    {
        putNewVectorIntoSearchOrder(check_vector,
                                    best_order_vector,
                                    number_vector,
                                    search_point_vector,
                                    degree_near_goal_y_vector);
    }
    else
    {
        putNewVectorIntoSearchOrder(check_vector,
                                    best_order_vector,
                                    number_vector,
                                    search_point_vector,
                                    dribble_course_number_vector);
    }

    //consider rest search_point_vector
    number_vector.clear();
    opp_dist_vector.clear();
    for(std::vector<bool>::size_type i=0; i<check_vector.size(); i++)
    {
        if(!check_vector.at(i))
        {
            number_vector.push_back(i);
            search_point = search_point_vector.at(i);
            if(std::abs(middle.y - wm.ball().pos().y) > 15.0
               && std::abs(middle.y - wm.ball().pos().y) <= 30.0)
            {
                degree_near_goal_y = 1.0 /search_point.absY();
                degree_near_goal_y_vector.push_back(degree_near_goal_y);
            }
            else
            {
                opp_dist_vector.push_back(Obake_Analysis().getDistFromNearestOpp(agent, search_point));
            }
        }
    }
    if(std::abs(middle.y - wm.ball().pos().y) > 15.0
       && std::abs(middle.y - wm.ball().pos().y) <= 30.0)
    {
        putNewVectorIntoSearchOrder(check_vector,
                                    best_order_vector,
                                    number_vector,
                                    search_point_vector,
                                    degree_near_goal_y_vector);
    }
    else
    {
        putNewVectorIntoSearchOrder(check_vector,
                                    best_order_vector,
                                    number_vector,
                                    search_point_vector,
                                    opp_dist_vector);
    }

    return best_order_vector;
}

