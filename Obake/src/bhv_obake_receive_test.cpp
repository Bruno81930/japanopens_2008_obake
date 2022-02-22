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

#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include <rcsc/geom/vector_2d.h>
#include <rcsc/player/player_agent.h>
#include "bhv_obake_receive.h"
#include "bhv_obake_receive_test.h"
#include "obake_analysis.h"
#include <fstream>

bool
Bhv_ObakeReceiveTest::execute(rcsc::PlayerAgent * agent)
{
    return true;
}
/*
bool
Bhv_ObakeReceiveTest::checkExistOpponentTest(rcsc::PlayerAgent * agent)
{
    const rcsc::WorldModel & wm = agent->world();
    std::ofstream fout;
    fout.open("receive_test.txt", std::ios::out | std::ios::app);
    rcsc::Vector2D base_pos(0.0, 0.0);
    rcsc::Vector2D target_pos(10.0, 0.0);
    const double angle = 90.0;
    const int count_thr = 10;
    bool exist_oppnent = false;
    const rcsc::PlayerPtrCont::const_iterator 
        opp_end = wm.opponentsFromSelf().end();
    for(rcsc::PlayerPtrCont::const_iterator
        opp = wm.opponentsFromSelf().begin();
        opp != opp_end;
        opp++)
    {
        if((*opp)->posCount() > count_thr)
        {
            continue;
        }
        fout<<(*opp)->pos()<<", ";
        if((*opp)->pos().x >= 0.0
            && (*opp)->pos().x <= 10.0
           && (*opp)->pos().absY() <= (*opp)->pos().x)
        {
            fout<<"exist";
            exist_oppnent = true;
            break;
        }
    }
    fout<<std::endl<<std::endl;
    if(Bhv_ObakeReceive(rcsc::Vector2D(0.0, 0.0)).checkExistOpponent(agent,
                                                                     base_pos,
                                                                     target_pos,
                                                                     angle)
       == exist_oppnent)
    {
        fout<<"true";
    }
    else
    {
        fout<<"false";
        fout<<std::endl;
        fout.close();
        return false;
    }
    fout<<std::endl;
    fout.close();
    return true;
}
*/
void
Bhv_ObakeReceiveTest::getBestMiddleTest(rcsc::PlayerAgent * agent)
{
//    const rcsc::WorldModel & wm = agent->world();
    rcsc::Vector2D middle(0.0, 0.0);
    const double difference = 10.0;
    const double length = 15.0;
    const double width = 30.0;
    rcsc::Vector2D best_middle;
    std::ofstream fout;
    fout.open("receive_test.txt", std::ios::out | std::ios::app);
    /*const rcsc::PlayerPtrCont::const_iterator 
       opp_end = wm.opponentsFromSelf().end();
    for(rcsc::PlayerPtrCont::const_iterator
            opp = wm.opponentsFromSelf().begin();
        opp != opp_end;
        opp++)
    {
        if(middle.x <= (*opp)->pos().x 
           && (*opp)->pos().x <= middle.x + length)
        {
            fout<<(*opp)->pos()<<std::endl;
        }
    }
    */
    const rcsc::Vector2D left_top(0.0, -rcsc::ServerParam::i().pitchHalfWidth());
    std::vector<double> opp_y_vector = Obake_Analysis().getOppYVector(agent,
                                                                      left_top,
                                                                      length,
                                                                      rcsc::ServerParam::i().pitchWidth());  
    double previous_y, difference_y;
    previous_y = -rcsc::ServerParam::i().pitchHalfWidth();
    const std::vector<double>::const_iterator 
        opp_end = opp_y_vector.end();
    for(std::vector<double>::const_iterator
            opp = opp_y_vector.begin();
        opp != opp_end;
        opp++)
    {
        fout<<(*opp);
        difference_y = (*opp) - previous_y;
        fout<<", difference = "<<difference_y<<", half difference = "<<difference_y / 2.0;
        fout<<", middle point  = "<<((*opp) + previous_y)/2.0<<std::endl;
        previous_y = (*opp);
    }
    fout<<std::endl;
    fout<<"middle = "<<middle<<std::endl;
    best_middle = Bhv_ObakeReceive(rcsc::Vector2D(0.0, 0.0)).getBestMiddle(agent,
                                                                           middle,
                                                                           difference,
                                                                           length,
                                                                           width);
    fout<<"best middle = "<<best_middle<<std::endl<<std::endl;

    middle.y = -rcsc::ServerParam::i().pitchHalfWidth();
    fout<<"middle = "<<middle<<std::endl;
    best_middle = Bhv_ObakeReceive(rcsc::Vector2D(0.0, 0.0)).getBestMiddle(agent,
                                                                           middle,
                                                                           difference,
                                                                           length,
                                                                           width);
    fout<<"best middle = "<<best_middle<<std::endl<<std::endl;

    middle.y = -rcsc::ServerParam::i().pitchHalfWidth() + 10.0;
    fout<<"middle = "<<middle<<std::endl;
    best_middle = Bhv_ObakeReceive(rcsc::Vector2D(0.0, 0.0)).getBestMiddle(agent,
                                                                           middle,
                                                                           difference,
                                                                           length,
                                                                           width);
    fout<<"best middle = "<<best_middle<<std::endl<<std::endl;


    middle.y = rcsc::ServerParam::i().pitchHalfWidth();
    fout<<"middle = "<<middle<<std::endl;
    best_middle = Bhv_ObakeReceive(rcsc::Vector2D(0.0, 0.0)).getBestMiddle(agent,
                                                                           middle,
                                                                           difference,
                                                                           length,
                                                                           width);
    fout<<"best middle = "<<best_middle<<std::endl<<std::endl;
    fout.close();
}

bool
Bhv_ObakeReceiveTest::putNewVectorIntoSearchOrderTest(rcsc::PlayerAgent * agent)
{
    std::vector<rcsc::Vector2D> search_point_vector;
    std::vector<rcsc::Vector2D> best_order_vector;
    std::vector<int> number_vector;
    std::vector<double> base_vector;
    search_point_vector.push_back(rcsc::Vector2D(0.0, 0.0));
    search_point_vector.push_back(rcsc::Vector2D(0.0, 10.0));
    search_point_vector.push_back(rcsc::Vector2D(0.0, 20.0));
    std::vector<bool> check_vector(search_point_vector.size(), false);    
    number_vector.push_back(0);
    number_vector.push_back(2);
    base_vector.push_back(15.0);
    base_vector.push_back(30.0);
    Bhv_ObakeReceive(rcsc::Vector2D(0.0, 0.0)).putNewVectorIntoSearchOrder(check_vector,
                                                                           best_order_vector,
                                                                           number_vector,
                                                                           search_point_vector,
                                                                           base_vector);
    std::ofstream fout;
    fout.open("receive_test.txt", std::ios::out | std::ios::app);
    if(!(check_vector.at(0)
         && !check_vector.at(1)
         && check_vector.at(2)))
    {
        fout<<"check vector : error"<<std::endl;
        fout.close();
        return false;
    }
    if(!(best_order_vector.at(0).x == 0.0
       && best_order_vector.at(0).y == 20.0
       && best_order_vector.at(1).x == 0.0
       && best_order_vector.at(1).y == 0.0))
    {
        fout<<"best order vector : error"<<std::endl;
        fout.close();
        return false;
    }
    fout<<"true"<<std::endl;
    fout.close();
    return true;
}
