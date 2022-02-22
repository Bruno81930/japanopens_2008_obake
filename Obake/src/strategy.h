// -*-c++-*-

/*!
  \file strategy.h
  \brief team strategh Header File
*/

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA

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

#ifndef AGENT2D_STRATEGY_H
#define AGENT2D_STRATEGY_H

#include <rcsc/geom/vector_2d.h>

#include <boost/shared_ptr.hpp>
#include <map>
#include <vector>
#include <string>

namespace rcsc {

class WorldModel;
class Formation;

}

class SoccerRole;


class Strategy {
public:
    static const std::string BEFORE_KICK_OFF_CONF;
    static const std::string GOAL_KICK_OPP_CONF;
    static const std::string GOAL_KICK_OUR_CONF;
    static const std::string GOALIE_CATCH_OPP_CONF;
    static const std::string GOALIE_CATCH_OUR_CONF;

    static const std::string DEFENSE_FORMATION_CONF;
    static const std::string OFFENSE_FORMATION_CONF;
    static const std::string KICKIN_OUR_FORMATION_CONF;
    static const std::string SETPLAY_OUR_FORMATION_CONF;
private:
    typedef SoccerRole * (*RoleCreator)( boost::shared_ptr< const rcsc::Formation > );
    typedef rcsc::Formation * (*FormationCreator)();

    typedef boost::shared_ptr< rcsc::Formation > FormationPtr;

    std::map< std::string, RoleCreator > M_role_factory;
    std::map< std::string, FormationCreator > M_formation_factory;

    FormationPtr M_before_kick_off_formation;
    std::vector< rcsc::Vector2D > M_goal_kick_opp_pos;
    std::vector< rcsc::Vector2D > M_goal_kick_our_pos;
    std::vector< rcsc::Vector2D > M_goalie_catch_opp_pos;
    std::vector< rcsc::Vector2D > M_goalie_catch_our_pos;

    FormationPtr M_offense_formation;
    FormationPtr M_defense_formation;

    FormationPtr M_kickin_our_formation;
    FormationPtr M_setplay_our_formation;

public:
    Strategy();

    bool read( const std::string & config_dir );


    rcsc::Vector2D getBeforeKickOffPos( const int number ) const;
    rcsc::Vector2D getPosWhenGoalKickOpp( const int number ) const;
    rcsc::Vector2D getPosWhenGoalKickOur( const int number ) const;
    rcsc::Vector2D getPosWhenGoalieCatchOpp( const int number ) const;
    rcsc::Vector2D getPosWhenGoalieCatchOur( const int number ) const;
    rcsc::Vector2D getSetPlayPosition( const int number,
                                       const rcsc::Formation & formation,
                                       const rcsc::WorldModel & world ) const;

    boost::shared_ptr<
        rcsc::Formation > createFormation( const std::string & type_name ) const;

    boost::shared_ptr<
        SoccerRole > createRole( const int number,
                                 const rcsc::WorldModel & world ) const;

private:
    boost:: shared_ptr<
        const rcsc::Formation > getFormation( const rcsc::WorldModel & world ) const;

    /////////////////////////////////////////////////////////
    bool readStaticPositions( const std::string & filepath,
                              std::vector< rcsc::Vector2D > & positions );

    /////////////////////////////////////////////////////////
    FormationPtr readFormation( const std::string & filepath );

public:
    enum BallArea {
        BA_CrossBlock, BA_DribbleBlock, BA_DribbleAttack, BA_Cross,
        BA_Stopper,    BA_DefMidField,  BA_OffMidField,   BA_ShootChance,
        BA_Danger,

        BA_None
    };

    static
    BallArea get_ball_area( const rcsc::Vector2D & ball_pos );
};

#endif
