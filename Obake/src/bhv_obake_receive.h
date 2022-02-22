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

#ifndef BHV_OBAKE_RECEIVE_H
#define BHV_OBAKE_RECEIVE_H
#include <rcsc/player/soccer_action.h>
#include <rcsc/geom/vector_2d.h>

class Bhv_ObakeReceive : public rcsc::SoccerBehavior{
private:
    rcsc::Vector2D M_home_pos;
    bool M_role_side_or_center_back;
    bool M_role_deffensive_half;
    bool M_role_offensive_half;
    bool M_role_side_or_center_forward;
public:
    explicit Bhv_ObakeReceive(const rcsc::Vector2D &home_pos)
        : M_home_pos(home_pos)
        , M_role_side_or_center_back(false)
        , M_role_deffensive_half(false)
        , M_role_offensive_half(false)
        , M_role_side_or_center_forward(false)
        {}
    bool execute(rcsc::PlayerAgent * agent);
//private:
    rcsc::Vector2D getReceiverPosition(rcsc::PlayerAgent * agent);
    rcsc::Vector2D getAdvancedPostion(rcsc::PlayerAgent * agent,
                                      const rcsc::Vector2D &receiver_pos);
/*    bool checkExistOpponentInRegion(rcsc::PlayerAgent * agent,
                                    const double &length,
                                    const double &width,
                                    const rcsc::Vector2D &search_point);
*/
    bool checkPassCourseCycle(rcsc::PlayerAgent * agent,
                              const rcsc::Vector2D &receiver_pos);
    bool checkPassCourseCyclePrediction(rcsc::PlayerAgent * agent,
                                        const rcsc::Vector2D &receiver_pos,
                                        const double &dist);
    bool checkPassCourseCyclePrediction(rcsc::PlayerAgent * agent,
                                        const rcsc::Vector2D &receiver_pos,
                                        const rcsc::Vector2D &prediction_vector);
    bool checkLeasdPassCourseCycle(rcsc::PlayerAgent * agent,
                                   const rcsc::Vector2D &receiver_pos);
    bool checkThroughPassCourseCycle(rcsc::PlayerAgent * agent,
                                     const rcsc::Vector2D &receiver_pos);
    bool checkCombinationPassCourseCycle(rcsc::PlayerAgent * agent,
                                         const rcsc::Vector2D &receiver_pos);
    bool checkCombinationThroughPassCourseCycle(rcsc::PlayerAgent * agent,
                                                const rcsc::Vector2D &receiver_pos);
/*
    bool checkOpponentInRegion(rcsc::PlayerAgent * agent,
                               bool * region,
                               const rcsc::Vector2D &left_middle,
                               const double &length,
                               const double &width,
                               const int number_per_length,
                               const int number_per_width);
*/
    bool checkExistShootCourse(rcsc::PlayerAgent * agent,
                               const rcsc::Vector2D &receiver_pos,
                               const double &angle,
                               const double &r);
    int getDribbleCourseNumber(rcsc::PlayerAgent * agent,
                               const rcsc::Vector2D &serach_point,
                               const double &angle);
    rcsc::Vector2D getBestMiddle(rcsc::PlayerAgent * agent,
                                 const rcsc::Vector2D &middle,
                                 const double &difference,
                                 const double &length,
                                 const double &width);
/*
    std::list<rcsc::Vector2D> getSearchOrder(rcsc::PlayerAgent * agent,
                                             const rcsc::Vector2D &middle,
                                             const double &difference,
                                             const double &length,
                                             const double &mate_r,
                                             const double &opp_r);
*/
    std::vector<rcsc::Vector2D> getSearchOrder(rcsc::PlayerAgent * agent,
                                               const rcsc::Vector2D &middle,
                                               const double &difference,
                                               const double &length,
                                               const double &mate_r,
                                               const double &opp_r);
    template<class T>
    void putNewVectorIntoSearchOrder(std::vector<bool> &check_vector,
                                     std::vector<rcsc::Vector2D> &best_order_vector,
                                     const std::vector<int> &number_vector,
                                     const std::vector<rcsc::Vector2D> &search_point_vector,
                                     const std::vector<T> &base_vector);
    void setBoolVectorAtTrue(std::vector<bool> &v,
                             const int index);
    void setSearchRange(rcsc::PlayerAgent * agent,
                        rcsc::Vector2D &front,
                        rcsc::Vector2D &back);
    bool verify_through_pass(rcsc::PlayerAgent * agent,
                             const rcsc::Vector2D & receiver_pos,
                             const rcsc::Vector2D & target_point,
                             const double & target_dist,
                             const rcsc::AngleDeg & target_angle,
                             const double & first_speed,
                             const double & reach_step);
};

#endif
