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

#ifndef OBAKE_ANALYSIS_H
#define OBAKE_ANALYSIS_H
#include <rcsc/geom/vector_2d.h>
#include <rcsc/geom/circle_2d.h>

class Obake_Analysis{
private:
public:
    bool checkExistOurPenaltyAreaIn(const rcsc::Vector2D &check_pos);
    bool checkExistOppPenaltyAreaIn(const rcsc::Vector2D &check_pos);
    bool checkExistDefender(rcsc::PlayerAgent * agent);
    bool checkOppCanControlsBall(rcsc::PlayerAgent * agent);
    bool checkExistOpponent(rcsc::PlayerAgent * agent,
                            const rcsc::Vector2D &base_pos,
                            const rcsc::Vector2D &target_pos,
                            const double &angle,
                            const double &r);
    bool checkExistOpponent(rcsc::PlayerAgent * agent,
			    const rcsc::Vector2D &base_pos,
			    const rcsc::Vector2D &target_pos,
			    const double &angle);
    bool checkSafeDefenseSituation(rcsc::PlayerAgent * agent);
    bool checkTeammateIn(rcsc::PlayerAgent * agent,
                         const int number,
                         const rcsc::Vector2D &left_top,
                         const double &length,
                         const double &width);
    bool checkBackOfTheSideForward(rcsc::PlayerAgent * agent);
    bool checkDefenseLine(rcsc::PlayerAgent * agent);
    bool checkShootCourse(rcsc::PlayerAgent * agent,
                          const rcsc::Vector2D &receiver_pos,
                          const rcsc::Vector2D &target_pos,
                          const double &angle,
                          const double &r);
    bool checkPassCourse(rcsc::PlayerAgent * agent,
                         const rcsc::Vector2D &passer_pos,
                         const rcsc::Vector2D &receiver_pos,
                         const double &angle);
    bool checkExistShootCourse(rcsc::PlayerAgent * agent,
                               const rcsc::Vector2D &receiver_pos,
                               const double &angle,
                               const double &r);
    int getAssistCourseNumber(rcsc::PlayerAgent * agent,
                              const rcsc::PlayerObject * passer,
                              const double &angle,
                              const double &max_pass_dist);
    int getAssistCourseNumber(rcsc::PlayerAgent * agent,
                              const rcsc::Vector2D &base_pos,
                              const double &angle,
                              const double max_pass_dist,
                              const bool except_nearest_mate);
    int getBackNumber(rcsc::PlayerAgent * agent,
                      const double &front_x);
    int getBallKeeperNumber(rcsc::PlayerAgent * agent,
                            const int count_thr);
    std::vector<rcsc::Vector2D> getDribbleTargetPointVector(rcsc::PlayerAgent * agent,
                                                            const rcsc::Vector2D &search_point,
                                                            const double &dribble_dist);
    double getLastMateDefenseLine(rcsc::PlayerAgent * agent);
    double getMateDefenseLine(rcsc::PlayerAgent * agent);
    double getOppDefenseLine(rcsc::PlayerAgent * agent);
    int  getOppNumber(rcsc::PlayerAgent * agent, 
                      const rcsc::Vector2D &center,
                      const double &r);
    int getOppNumber(rcsc::PlayerAgent * agent,
                     const double &front_x,
                     const double &back_x);
    int getOppNumber(rcsc::PlayerAgent * agent,
                     const rcsc::Vector2D &left_top,
                     const double &length,
                     const double &width);
    int getOppNumberInOurPenaltyArea(rcsc::PlayerAgent * agent);
    std::list<int> getOppNumberAroundBall(rcsc::PlayerAgent * agent);
    int getPassCourseNumber(rcsc::PlayerAgent * agent,
                            const rcsc::PlayerObject * passer,
                            const double &angle,
                            const double &max_pass_dist);
    std::list<int> getPassCourseNumberList(rcsc::PlayerAgent * agent,
                                           const rcsc::PlayerObject * passer,
                                           const double &angle,
                                           const double &max_pass_dist);
    
    rcsc::Triangle2D getTriangle(const rcsc::Vector2D &base_pos,
                                 const rcsc::Vector2D &target_pos,
                                 const double & angle);
    int getMateBackNumber(rcsc::PlayerAgent * agent,
                          const rcsc::Vector2D &left_top,
                          const double &length,
                          const double &width);
    int getMateNumber(rcsc::PlayerAgent * agent,
                      const rcsc::Vector2D &center,
                      const double &r);
    int getMateNumber(rcsc::PlayerAgent * agent,
                      const rcsc::Vector2D &left_top,
                      const double &length,
                      const double &width);
    int getMateNumber(rcsc::PlayerAgent * agent,
                      const rcsc::Vector2D &pass_point);
    int getMateNumberInOurPenaltyArea(rcsc::PlayerAgent * agent,
                                      const bool except_goalie = false);
    std::list<int> getShootChanceMateNumberList(rcsc::PlayerAgent * agent,
                                                const rcsc::Vector2D &left_top,
                                                const double &length,
                                                const double &width,
                                                const double &angle,
                                                const double &r);
    double getDistFromNearestMate(rcsc::PlayerAgent * agent,
                                  const rcsc::Vector2D &point,
                                  const bool except_near_opp_defender = false,
                                  const bool except_role_center_or_side_back = false,
                                  const bool except_role_goalie = true);
    double getDistFromNearestOpp(rcsc::PlayerAgent * agent,
                                 const rcsc::Vector2D &point);
    std::list<double> getOppYList(rcsc::PlayerAgent * agent,
                                  const double &length,
                                  const double &width);
    std::vector<double> getOppYVector(rcsc::PlayerAgent * agent,
                                      const rcsc::Vector2D &left_top,
                                      const double &length,
                                      const double &width);
    std::list<int> getOppNumberYList(rcsc::PlayerAgent * agent,
                                     const rcsc::Vector2D &left_top,
                                     const double &length,
                                     const double &width,
                                     const bool check_goalie = false);
    void setRole(int number,
		 bool &role_side_or_center_back,
		 bool &role_deffensive_half,
		 bool &role_offensive_half,
		 bool &role_side_or_center_forward);

};

#endif
