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

#include <rcsc/player/player_agent.h>
#include <rcsc/common/server_param.h>

#include "obake_fuzzy_grade_model.h"
#include "obake_fuzzy_grade.h"

double
Obake_FuzzyGrade::degreeExistNearestOpp(const double &dist)
{
    const double top = 0.0;
    const double bottom = 10.0;
    const double grade = Obake_FuzzyGradeModel().straightDown(dist,
                                                              top,
                                                              bottom);
    return grade;
}

double
Obake_FuzzyGrade::degreeNearDestination(const rcsc::Vector2D &base,
					const rcsc::Vector2D &destination)
{
    const double dist = base.dist(destination);
    const double top = 0.0;
    const double bottom = 15.0;
    const double grade = Obake_FuzzyGradeModel().straightDown(dist,
							      top,
							      bottom);
    return grade;
}

double 
Obake_FuzzyGrade::degreeFarDestination(const rcsc::Vector2D &base,
				       const rcsc::Vector2D &destination)
{
    const double grade = 1 - degreeNearDestination(base,
						   destination);
    return grade;
}

double
Obake_FuzzyGrade::degreeNearDestinationX(const double &base_x,
                                         const double &destination_x)
{
    const double dist = std::abs(destination_x - base_x);
    const double top = 0.0;
    const double bottom = 15.0;
    const double grade = Obake_FuzzyGradeModel().straightDown(dist, 
                                                              top,
                                                              bottom);
    return grade;
}

double
Obake_FuzzyGrade::degreeFarDestinationX(const double &base_x,
                                        const double &destination_x)
{
    const double grade = 1 - degreeNearDestinationX(base_x,
                                                    destination_x);
    return grade;
}

double
Obake_FuzzyGrade::degreeNearDestinationY(const double &base_y,
                                         const double &destination_y)
{
    const double dist = destination_y - base_y;
    const double top = 0.0;
    const double bottom = 15.0;
    const double grade = Obake_FuzzyGradeModel().straightDown(dist,
                                                              top,
                                                              bottom);
    return grade;

}

double
Obake_FuzzyGrade::degreeFarDestinationY(const double &base_y,
                                        const double &destination_y)
{
    const double grade = 1 - degreeNearDestinationY(base_y,
                                                    destination_y);
    return grade;
}

double
Obake_FuzzyGrade::degreeNearOppGoal(const rcsc::Vector2D & target_point)
{
    const double dist = target_point.dist(rcsc::Vector2D(rcsc::ServerParam::i().pitchHalfLength(),
                                                         0.0));
    const double top = 0.0;
    const double bottom = 40.0;
    const double grade = Obake_FuzzyGradeModel().straightDown(dist,
                                                              top,
                                                              bottom);
    return grade;
}

double
Obake_FuzzyGrade::degreeNearMateGoalX(const double &base_x)
{
    const double dist = base_x - rcsc::ServerParam::i().pitchHalfLength();
    const double top = 0.0;
    const double bottom = rcsc::ServerParam::i().penaltyAreaLength();
    const double grade = Obake_FuzzyGradeModel().straightDown(dist,
                                                              top,
                                                              bottom);
    return grade;
}

double
Obake_FuzzyGrade::degreeNearOppGoalX(const double &base_x)
{
    const double dist = rcsc::ServerParam::i().pitchHalfLength() - base_x;
    const double top = 0.0;
    const double bottom = rcsc::ServerParam::i().penaltyAreaLength();
    const double grade = Obake_FuzzyGradeModel().straightDown(dist,
                                                              top,
                                                              bottom);
    return grade;
}

double
Obake_FuzzyGrade::degreeFarMateGoalX(const double &base_x)
{
    const double grade = 1 - degreeNearMateGoalX(base_x);
    return grade;
}

double
Obake_FuzzyGrade::degreeFarOppGoalX(const double &base_x)
{
    const double grade = 1 - degreeNearOppGoalX(base_x);
    return grade;
}

double
Obake_FuzzyGrade::degreeNearMateGoalY(const double &base_y)
{
    const double dist = (base_y > 0) ? base_y : -base_y;
    const double top = 0.0;
    const double bottom = rcsc::ServerParam::i().pitchHalfWidth();
    const double grade = Obake_FuzzyGradeModel().straightDown(dist,
                                                              top,
                                                              bottom);
    return grade;
}

double
Obake_FuzzyGrade::degreeNearOppGoalY(const double &base_y)
{
    const double dist = (base_y > 0) ? base_y : -base_y;
    const double top = 0.0;
    const double bottom = rcsc::ServerParam::i().pitchHalfWidth();
    const double grade = Obake_FuzzyGradeModel().straightDown(dist,
                                                              top,
                                                              bottom);
    return grade;
}

double  
Obake_FuzzyGrade::degreeFarMateGoalY(const double &base_y)
{
    const double grade = 1 - degreeNearMateGoalY(base_y);
    return grade;
}

double  
Obake_FuzzyGrade::degreeFarOppGoalY(const double &base_y)
{
    const double grade = 1 - degreeNearOppGoalY(base_y);
    return grade;
}

double 
Obake_FuzzyGrade::degreeNearOppGoalAreaY(const double &base_y)
{
    double grade = 1.0;
    if(std::abs(base_y) >= rcsc::ServerParam::i().goalAreaHalfWidth())
    {
        const double dist = std::abs(std::abs(base_y)-rcsc::ServerParam::i().goalAreaHalfWidth());
        const double top = 0.0;
        const double bottom = rcsc::ServerParam::i().pitchHalfWidth() - rcsc::ServerParam::i().goalAreaHalfWidth();
        grade = Obake_FuzzyGradeModel().straightDown(dist,
                                                     top,
                                                     bottom);
    }
    return grade;
}

double 
Obake_FuzzyGrade::degreeFarOppGoalAreaY(const double &base_y)
{
    const double grade = 1 - degreeNearOppGoalAreaY(base_y);
    return grade;
}

double
Obake_FuzzyGrade::degreeNearMatePenaltyAreaX(const double &base_x)
{
    const double dist = base_x - (-rcsc::ServerParam::i().pitchHalfLength() + rcsc::ServerParam::i().penaltyAreaLength());
    const double top = 0.0;
    const double bottom = 25.0;
    const double grade = Obake_FuzzyGradeModel().straightDown(dist,
                                                              top,
                                                              bottom);
    return grade;
}

double
Obake_FuzzyGrade::degreeNearOppPenaltyAreaX(const double &base_x)
{
    const double dist = (rcsc::ServerParam::i().theirPenaltyAreaLineX()) - base_x;
    const double top = 0.0;
    const double bottom = 25.0;
    const double grade = Obake_FuzzyGradeModel().straightDown(dist,
                                                              top,
                                                              bottom);
    return grade;
}

double
Obake_FuzzyGrade::degreeFarMatePenaltyAreaX(const double &base_x)
{
    const double grade = 1 - Obake_FuzzyGrade().degreeNearMatePenaltyAreaX(base_x);
    return grade;
}

double
Obake_FuzzyGrade::degreeFarOppPenaltyAreaX(const double &base_x)
{
    const double grade = 1 - Obake_FuzzyGrade().degreeNearOppPenaltyAreaX(base_x);
    return grade;
}

double
Obake_FuzzyGrade::degreeNearOffsideLine(rcsc::PlayerAgent * agent,
                                        const double &base_x)
                                        
{
    const rcsc::WorldModel & wm = agent->world();
    const double dist = wm.offsideLineX() - base_x;
    const double top = 0.0;
    const double bottom = 25.0;//15.0;
    const double grade = Obake_FuzzyGradeModel().straightDown(dist,
                                                              top,
                                                              bottom);
    return grade;
}

double
Obake_FuzzyGrade::degreeFarOffsideLine(rcsc::PlayerAgent * agent,
                                        const double &base_x)
                                        
{
    const rcsc::WorldModel & wm = agent->world();
    const double dist = wm.offsideLineX() - base_x;
    const double bottom = 12.5;
    const double top = 37.5;
    const double grade = Obake_FuzzyGradeModel().straightUp(dist,
                                                            bottom,
                                                            top);
    return grade;
}

double 
Obake_FuzzyGrade::degreeFullStamina(const double &stamina)
{
    const double stamina_max = rcsc::ServerParam::i().staminaMax();
    const double bottom = stamina_max * 0.7;
    const double top = stamina_max;
    const double grade = Obake_FuzzyGradeModel().straightUp(stamina, 
                                                            bottom,
                                                            top);
    return grade;


}

double
Obake_FuzzyGrade::degreeModerateStamina(const double &stamina)
{
    const double stamina_max = rcsc::ServerParam::i().staminaMax();
    const double first_bottom = stamina_max * 0.55;
    const double second_bottom = stamina_max * 0.8;
    const double grade = Obake_FuzzyGradeModel().triangle(stamina,
                                                          first_bottom,
                                                          second_bottom);
    return grade;
}

double
Obake_FuzzyGrade::degreeLackStamina(const double &stamina)
{
    const double stamina_max = rcsc::ServerParam::i().staminaMax();
    const double top = stamina_max * 0.3;
    const double bottom = stamina_max * 0.65;
    const double grade = Obake_FuzzyGradeModel().straightDown(stamina,
                                                              top,
                                                              bottom);
    return grade;
}

double
Obake_FuzzyGrade::degreeLongAvoidanceDist(const double &dist)
{
    const double top = 10.0;
    const double bottom = 0.0;
    const double grade = Obake_FuzzyGradeModel().straightUp(dist,
                                                            bottom,
                                                            top);
    return grade;
}

double
Obake_FuzzyGrade::degreeShortAvoidanceDist(const double &dist)
{
    const double grade = 1 - degreeLongAvoidanceDist(dist);
    return grade;
}


double
Obake_FuzzyGrade::degreeManyMateNumber(const int mate_number)
{
    const int top = 4;
    const int bottom = 0;
    const double grade = Obake_FuzzyGradeModel().straightUp(mate_number,
                                                            bottom,
                                                            top);
    return grade;
}

double
Obake_FuzzyGrade::degreeManyDirConf(const int confidence)
{
    const int top = 10;
    const int bottom = 0;
    const double grade = Obake_FuzzyGradeModel().straightUp(confidence,
                                                            bottom,
                                                            top);
    return grade;
}

double
Obake_FuzzyGrade::degreeManyPosConf(const int confidence)
{
    const int top = 5;
    const int bottom = 0;
    const double grade = Obake_FuzzyGradeModel().straightUp(confidence,
                                                            bottom,
                                                            top);
    return grade;
}

double
Obake_FuzzyGrade::degreeNearFromOpp(const double &dist)
{
    const double top = 0.0;
    const double bottom = 5.0;
    const double grade = Obake_FuzzyGradeModel().straightDown(dist,
                                                              top,
                                                              bottom);
    return grade;
}

double
Obake_FuzzyGrade::degreeModerateFromOpp(const double &dist)
{
    const double first_bottom = 3.0;
    const double second_bottom = 8.0;
    const double grade = Obake_FuzzyGradeModel().triangle(dist,
                                                          first_bottom,
                                                          second_bottom);
    return grade;
}

double 
Obake_FuzzyGrade::degreeFarFromOpp(const double &dist)
{
    const double top =  11.0;
    const double bottom = 6.0;
    const double grade = Obake_FuzzyGradeModel().straightUp(dist,
                                                            bottom,
                                                            top);
    return grade;
}
