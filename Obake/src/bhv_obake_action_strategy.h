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

#ifndef BHV_OBAKE_ACTION_STRATEGY_H
#define BHV_OBAKE_ACTION_STRATEGY_H
#include <rcsc/player/soccer_action.h>

class Bhv_ObakeActionStrategy : public rcsc::SoccerBehavior{
    bool M_role_side_or_center_back;
    bool M_role_defensive_half;
    bool M_role_offensive_half;
    bool M_role_side_or_center_forward;

public:
    Bhv_ObakeActionStrategy()
        : M_role_side_or_center_back(false)
        , M_role_defensive_half(false)
        , M_role_offensive_half(false)
        , M_role_side_or_center_forward(false)
        {}
    bool execute(rcsc::PlayerAgent * agent);
    double getDashPower(rcsc::PlayerAgent * agent,
                        const rcsc::Vector2D &target_point);
//private:
    bool checkStrategicDribble(rcsc::PlayerAgent * agent);
    bool checkAvailableTargetPoint(rcsc::PlayerAgent * agent,
                                   const rcsc::Vector2D &target_point);
    bool checkAvoidanceFromNearestOpp(rcsc::PlayerAgent *agent,
                                      const rcsc::Vector2D &target_point,
                                      const double &max_multiplier,
                                      double &multiplier);
    bool checkBetterAction(rcsc::PlayerAgent * agent,
                           const rcsc::Vector2D &target_point,
                           const double &max_score,
                           double &score);
    bool checkDistFromNearestOpp(rcsc::PlayerAgent * agent,
                                 const rcsc::Vector2D &target_point,
                                 const double &max_multiplier,
                                 double &multiplier);
    bool checkDribbleToBodyDir(rcsc::PlayerAgent * agent,
                               const rcsc::Vector2D &target_point);
    bool checkDribbleToNearGoalX(rcsc::PlayerAgent * agent, 
				 const double &target_point_x);
    bool checkDribbleToNearGoalY(rcsc::PlayerAgent * agent,
				 const double &target_point_y);
    bool checkExistAssistCourse(rcsc::PlayerAgent * agent,
                                const rcsc::Vector2D &base_pos,
				const double &pass_angle,
				const double &max_pass_dist,
				int &front_of_the_ball_mate,
				int &sum_of_the_ball_mate,
                                int &sum_of_the_mate_in_penalty_area);
    bool checkExistShootCourse(rcsc::PlayerAgent * agent,
                               const rcsc::Vector2D &target_point,
                               const double &max_multiplier,
                               double &multiplier);
    bool checkExistPassCourse(rcsc::PlayerAgent * agent,
                              const rcsc::Vector2D &target_point,
                              const double &max_multiplier,
                              double &multiplier);
    bool checkVeryLackStamina(rcsc::PlayerAgent * agent,
                              const double &max_multiplier,
                              double &multiplier);

    double getPassCourseMultiplier(rcsc::PlayerAgent * agent,
				   const rcsc::Vector2D &target_point,
				   const double &max_muliplier);
    double getPositionMultiplier(rcsc::PlayerAgent * agent,
                                 const rcsc::Vector2D &target_point,
                                 const double &max_multiplier);
    double getShootMultiplier(rcsc::PlayerAgent * agent,
                              const rcsc::Vector2D target_point,
                              const double &max_multiplier);
    double getSideAttackPositionMultiplier(rcsc::PlayerAgent * agent,
                                           const rcsc::Vector2D &target_point,
                                           const double &max_multiplier);
    double getMiddleAttackPositionMultiplier(rcsc::PlayerAgent * agent,
                                             const rcsc::Vector2D &target_point,
                                             const double &max_multiplier);
    double getNearOppGoalMultiplier(rcsc::PlayerAgent * agent,
                                    const rcsc::Vector2D &target_point,
                                    const double &max_multiplier);
    double getDangerousPositionMultiplier(rcsc::PlayerAgent * agent,
                                          const rcsc::Vector2D &target_point,
                                          const double &max_multiplier);
    
/* 
   std::vector<rcsc::Vector2D> getTargetPoint(rcsc::PlayerAgent * agent,
   std::vector<double> &score_target_vector);
*/
    std::vector<rcsc::Vector2D> getTargetPointVector(rcsc::PlayerAgent * agent);
    /*
      std::vector<rcsc::Vector2D> getAvailableTargetPoint(rcsc::PlayerAgent * agent,
                                                      const std::vector<rcsc::Vector2D> &target_point_vector);
    */
    /*
      std::vector<double> getTargetPointScore(rcsc::PlayerAgent * agent,
					    const std::vector<rcsc::Vector2D> &target_point_vector);
    */
/*
    rcsc::Vector2D getBestTargetPoint(rcsc::PlayerAgent * agent,
                                      const std::vector<rcsc::Vector2D> &target_point_vector,
                                      const std::vector<double> &target_point_score_vector);
*/
    bool getBestAction(rcsc::PlayerAgent * agent);
    double getPenetrationMultiplier(rcsc::PlayerAgent * agent,
                                    const rcsc::Vector2D &target_point,
                                    const double &max_multiplier);
    bool checkDribbleArea(rcsc::PlayerAgent * agent,
                          const rcsc::Vector2D &dribble_target,
                          const double &dist,
                          const double &front,
                          const double &angle);
    int getDashCount(rcsc::PlayerAgent * agent,
                     const rcsc::Vector2D &dribble_target);
};

#endif
