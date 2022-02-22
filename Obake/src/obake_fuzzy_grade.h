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

#ifndef OBAKE_FUZZY_GRADE_H
#define OBAKE_FUZZY_GRADE_H

class Obake_FuzzyGrade{
public:
    double degreeExistNearestOpp(const double &dist);
    double degreeNearDestination(const rcsc::Vector2D &base,
				 const rcsc::Vector2D &destination);
    double degreeFarDestination(const rcsc::Vector2D &base,
				const rcsc::Vector2D &destination);
    double degreeNearDestinationX(const double &base_x,
                                  const double &destination_x);
    double degreeFarDestinationX(const double &base_x,
                                 const double &destination_x);
    double degreeNearDestinationY(const double &base_y,
                                  const double &destination_y);
    double degreeFarDestinationY(const double &base_y,
                                 const double &destination_y);
    double degreeNearOppGoal(const rcsc::Vector2D & target_point);
    double degreeNearMateGoalX(const double &base_x);
    double degreeNearOppGoalX(const double &base_x);
    double degreeFarMateGoalX(const double &base_x);
    double degreeFarOppGoalX(const double &base_x);
    double degreeNearOppGoalAreaY(const double &base_y);
    double degreeFarOppGoalAreaY(const double &base_y);
    double degreeNearMateGoalY(const double &base_y);
    double degreeNearOppGoalY(const double &base_y);
    double degreeFarMateGoalY(const double &base_y);
    double degreeFarOppGoalY(const double &base_y);
    double degreeNearOffsideLine(rcsc::PlayerAgent * agent,
                                 const double &base_x);
    double degreeFarOffsideLine(rcsc::PlayerAgent * agent,
                                const double &base_x);
    double degreeNearMatePenaltyAreaX(const double &base_x);
    double degreeNearOppPenaltyAreaX(const double &base_x);
    double degreeFarMatePenaltyAreaX(const double &base_x);
    double degreeFarOppPenaltyAreaX(const double &base_x);
    double degreeFullStamina(const double &stamina);
    double degreeModerateStamina(const double &stamina);
    double degreeLackStamina(const double &stamina);
    double degreeLongAvoidanceDist(const double &dist);
    double degreeShortAvoidanceDist(const double &dist);
    double degreeManyMateNumber(const int mate_number);
    double degreeManyDirConf(const int confidence);
    double degreeManyPosConf(const int confidence);
    double degreeNearFromOpp(const double &dist);
    double degreeModerateFromOpp(const double &dist);
    double degreeFarFromOpp(const double &dist);
};

#endif
