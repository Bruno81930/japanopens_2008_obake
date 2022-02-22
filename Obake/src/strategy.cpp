// -*-c++-*-

/*!
  \file strategy.cpp
  \brief team strategh Source File
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "strategy.h"

#include "soccer_role.h"

#include "role_sample.h"

#include "role_goalie.h"
#include "role_side_back.h"
#include "role_center_back.h"
#include "role_defensive_half.h"
#include "role_offensive_half.h"
#include "role_side_forward.h"
#include "role_center_forward.h"

#include <rcsc/formation/formation_static.h>
#include <rcsc/formation/formation_bpn.h>
#include <rcsc/formation/formation_dt.h>
#include <rcsc/formation/formation_ngnet.h>
#include <rcsc/formation/formation_uva.h>

#include <rcsc/player/intercept_table.h>
#include <rcsc/player/world_model.h>

#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include <rcsc/game_mode.h>

#include <set>
#include <fstream>
#include <iostream>
#include <cstdio>

const std::string Strategy::BEFORE_KICK_OFF_CONF = "before-kick-off.conf";
const std::string Strategy::GOAL_KICK_OPP_CONF = "goal-kick-opp.conf";
const std::string Strategy::GOAL_KICK_OUR_CONF = "goal-kick-our.conf";
const std::string Strategy::GOALIE_CATCH_OPP_CONF = "goalie-catch-opp.conf";
const std::string Strategy::GOALIE_CATCH_OUR_CONF = "goalie-catch-our.conf";

const std::string Strategy::DEFENSE_FORMATION_CONF = "defense-formation.conf";
const std::string Strategy::OFFENSE_FORMATION_CONF = "offense-formation.conf";
const std::string Strategy::KICKIN_OUR_FORMATION_CONF = "kickin-our-formation.conf";
const std::string Strategy::SETPLAY_OUR_FORMATION_CONF = "setplay-our-formation.conf";

/*-------------------------------------------------------------------*/
/*!

*/
Strategy::Strategy()
    : M_goal_kick_opp_pos( 11 )
    , M_goal_kick_our_pos( 11 )
    , M_goalie_catch_opp_pos( 11 )
    , M_goalie_catch_our_pos( 11 )
{
    M_role_factory[RoleSample::name()] = &RoleSample::create;

    M_role_factory[RoleGoalie::name()] = &RoleGoalie::create;
    M_role_factory["Sweeper"] = &RoleCenterBack::create;
    M_role_factory[RoleCenterBack::name()] = &RoleCenterBack::create;
    M_role_factory[RoleSideBack::name()] = &RoleSideBack::create;
    M_role_factory[RoleDefensiveHalf::name()] = &RoleDefensiveHalf::create;
    M_role_factory[RoleOffensiveHalf::name()] = &RoleOffensiveHalf::create;
    M_role_factory[RoleSideForward::name()] = &RoleSideForward::create;
    M_role_factory[RoleCenterForward::name()] = &RoleCenterForward::create;

    M_formation_factory[rcsc::FormationStatic::name()] = &rcsc::FormationStatic::create;
    M_formation_factory[rcsc::FormationBPN::name()] = &rcsc::FormationBPN::create;
    M_formation_factory[rcsc::FormationDT::name()] = &rcsc::FormationDT::create;
    M_formation_factory[rcsc::FormationNGNet::name()] = &rcsc::FormationNGNet::create;
    M_formation_factory[rcsc::FormationUvA::name()] = &rcsc::FormationUvA::create;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Strategy::read( const std::string & config_dir )
{
    std::string configpath = config_dir;
    if ( ! configpath.empty()
         && configpath[ configpath.length() - 1 ] != '/' )
    {
        configpath += '/';
    }

    // before kick off
    M_before_kick_off_formation
        = readFormation( configpath + BEFORE_KICK_OFF_CONF );
    if ( ! M_before_kick_off_formation )
    {
        std::cerr << "Failed to read before_kick_off formation" << std::endl;
        return false;
    }
    /*
    if ( ! readStaticPositions( configpath + BEFORE_KICK_OFF_CONF,
                                M_before_kick_off_pos ) )
    {
        return false;
    }
    */
    // opponent goal kick
    if ( ! readStaticPositions( configpath + GOAL_KICK_OPP_CONF,
                                M_goal_kick_opp_pos ) )
    {
        return false;
    }
    // our goal kick
    if ( ! readStaticPositions( configpath + GOAL_KICK_OUR_CONF,
                                M_goal_kick_our_pos ) )
    {
        return false;
    }
    // opponent goalie catch
    if ( ! readStaticPositions( configpath + GOALIE_CATCH_OPP_CONF,
                                M_goalie_catch_opp_pos ) )
    {
        return false;
    }
    // our goalie catch
    if ( ! readStaticPositions( configpath + GOALIE_CATCH_OUR_CONF,
                                M_goalie_catch_our_pos ) )
    {
        return false;
    }

    ///////////////////////////////////////////////////////////
    // offense area
    M_offense_formation = readFormation( configpath + OFFENSE_FORMATION_CONF );
    if ( ! M_offense_formation )
    {
        std::cerr << "Failed to read offense formation" << std::endl;
        return false;
    }
    // defense area
    M_defense_formation = readFormation( configpath + DEFENSE_FORMATION_CONF );
    if ( ! M_defense_formation )
    {
        std::cerr << "Failed to read defense formation" << std::endl;
        return false;
    }
    // kick_in our
    M_kickin_our_formation = readFormation( configpath + KICKIN_OUR_FORMATION_CONF );
    if ( ! M_kickin_our_formation )
    {
        std::cerr << "Failed to read kickin our formation" << std::endl;
        return false;
    }
    // setplay our
    M_setplay_our_formation = readFormation( configpath + SETPLAY_OUR_FORMATION_CONF );
    if ( ! M_setplay_our_formation )
    {
        std::cerr << "Failed to read setplay our formation" << std::endl;
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
boost::shared_ptr< rcsc::Formation >
Strategy::readFormation( const std::string & filepath )
{
    std::ifstream fin( filepath.c_str() );
    if ( ! fin.is_open() )
    {
        std::cerr << __FILE__ << ": " << __LINE__
                  << " ***ERROR*** failed to open file [" << filepath << "]"
                  << std::endl;
        return boost::shared_ptr< rcsc::Formation >
            ( static_cast< rcsc::Formation * >( 0 ) );
    }

    std::string temp, type;
    fin >> temp >> type; // read training method type name
    fin.seekg( 0 );

    boost::shared_ptr< rcsc::Formation > ptr = createFormation( type );

    if ( ! ptr )
    {
        std::cerr << __FILE__ << ": " << __LINE__
                  << " ***ERROR*** failed to create formation [" << filepath << "]"
                  << std::endl;
        return boost::shared_ptr< rcsc::Formation >();
    }

    if ( ! ptr->read( fin ) )
    {
        std::cerr << __FILE__ << ": " << __LINE__
                  << " ***ERROR*** failed to read formation [" << filepath << "]"
                  << std::endl;
        return boost::shared_ptr< rcsc::Formation >();
    }

    for ( int unum = 1; unum <= 11; ++unum )
    {
        if ( M_role_factory.find( ptr->getRoleName( unum ) ) == M_role_factory.end() )
        {
            std::cerr << __FILE__ << ": " << __LINE__
                      << " ***ERROR*** Unsupported role name ["
                      << ptr->getRoleName( unum ) << "] is appered in ["
                      << filepath << "]" << std::endl;
            return boost::shared_ptr< rcsc::Formation >();
        }
    }

    return ptr;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Strategy::readStaticPositions( const std::string & filepath,
                               std::vector< rcsc::Vector2D > & positions )
{
    std::ifstream fin( filepath.c_str() );
    if ( ! fin.is_open() )
    {
        std::cerr << __FILE__ << ": " << __LINE__
                  << " ***ERROR*** Failed to open configuration file ["
                  << filepath << "]"
                  << std::endl;
        return false;
    }

    positions.resize( 11 );

    std::string line_buf;
    int n_line = 0;
    std::set< int > read_number;
    while ( std::getline( fin, line_buf ) )
    {
        ++n_line;
        if ( line_buf.empty()
             || line_buf[0] == '#'
             || ( line_buf.length() > 1
                  && line_buf[0] == '/'
                  && line_buf[1] == '/' )
             )
        {
            continue;
        }

        int number;
        double x, y;
        if ( std::sscanf( line_buf.c_str(),
                          " %d %lf %lf ",
                          &number, &x, &y ) != 3 )
        {
            std::cerr << __FILE__ << ": " << __LINE__
                      << " ***ERROR*** Invalid line in ["
                      << filepath << "]:" << n_line
                      << std::endl;
            fin.close();
            return false;
        }

        if ( number < 0 || 11 < number )
        {
            std::cerr << __FILE__ << ": " << __LINE__
                      << " ***ERROR*** Invalid player number in ["
                      << filepath << ":" << n_line << "]"
                      << std::endl;
            fin.close();
            return false;
        }

        read_number.insert( number );
        positions[number - 1].assign( x, y );
    }

    fin.close();

    if ( read_number.size() != 11 )
    {
        std::cerr << __FILE__ << ": " << __LINE__
                  << " ***ERROR*** Invalid number of configuration in ["
                  << filepath << ":" << n_line << "]"
                  << std::endl;
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
boost::shared_ptr< rcsc::Formation >
Strategy::createFormation( const std::string & type_name ) const
{
    // get role factory of this role name
    std::map< std::string, FormationCreator >::const_iterator factory
        = M_formation_factory.find( type_name );
    if ( factory == M_formation_factory.end() )
    {
        std::cerr << __FILE__ << ": " << __LINE__
                  << " ***ERROR*** formation type ["
                  << type_name << "] is not supported"
                  << std::endl;
        return boost::shared_ptr< rcsc::Formation >
            ( static_cast< rcsc::Formation * >( 0 ) );
    }

    boost::shared_ptr< rcsc::Formation > f( factory->second() );
    return f;
}

/*-------------------------------------------------------------------*/
/*!
  create role instance
*/
boost::shared_ptr< SoccerRole >
Strategy::createRole( const int number,
                      const rcsc::WorldModel & world ) const
{
    if ( number < 1 || 11 < number )
    {
        std::cerr << __FILE__ << ": " << __LINE__
                  << " ***ERROR*** Invalid player number " << number
                  << std::endl;
        return boost::shared_ptr< SoccerRole >( static_cast< SoccerRole * >( 0 ) );
    }

    boost:: shared_ptr< const rcsc::Formation > formation = getFormation( world );
    if ( ! formation )
    {
        std::cerr << __FILE__ << ": " << __LINE__
                  << " ***ERROR*** faled to create role. Null formation" << std::endl;
        return boost::shared_ptr< SoccerRole >( static_cast< SoccerRole * >( 0 ) );
    }

    const std::string rolename = formation->getRoleName( number );

    // get role factory of this role name
    std::map< std::string, RoleCreator >::const_iterator factory
        = M_role_factory.find( rolename );
    if ( factory == M_role_factory.end() )
    {
        std::cerr << __FILE__ << ": " << __LINE__
                  << " ***ERROR*** No such a role name ["
                  << rolename << "]"
                  << std::endl;
        return boost::shared_ptr< SoccerRole >( static_cast< SoccerRole * >( 0 ) );
    }

    boost::shared_ptr< SoccerRole > role_ptr( factory->second( formation ) );
    return role_ptr;
}

/*-------------------------------------------------------------------*/
/*!

*/
rcsc::Vector2D
Strategy::getBeforeKickOffPos( const int number ) const
{
    return M_before_kick_off_formation->getPosition( number,
                                                     rcsc::Vector2D( 0.0, 0.0 ) );
}

/*-------------------------------------------------------------------*/
/*!

*/
rcsc::Vector2D
Strategy::getPosWhenGoalKickOpp( const int number ) const
{
    if ( number < 1 || 11 < number )
    {
        std::cerr << __FILE__ << ": " << __LINE__
                  << " ***ERROR*** Invalid player number " << number
                  << std::endl;
        return rcsc::Vector2D( -50.0, -30.0 );
    }

    return M_goal_kick_opp_pos[number - 1];
}

/*-------------------------------------------------------------------*/
/*!

*/
rcsc::Vector2D
Strategy::getPosWhenGoalKickOur( const int number ) const
{
    if ( number < 1 || 11 < number )
    {
        std::cerr << __FILE__ << ": " << __LINE__
                  << " ***ERROR*** Invalid player number " << number
                  << std::endl;
        return rcsc::Vector2D( -50.0, -30.0 );
    }

    return M_goal_kick_our_pos[number - 1];
}

/*-------------------------------------------------------------------*/
/*!

*/
rcsc::Vector2D
Strategy::getPosWhenGoalieCatchOpp( const int number ) const
{
    if ( number < 1 || 11 < number )
    {
        std::cerr << __FILE__ << ": " << __LINE__
                  << " ***ERROR*** Invalid player number " << number
                  << std::endl;
        return rcsc::Vector2D( -50.0, -30.0 );
    }

    return M_goalie_catch_opp_pos[number - 1];
}

/*-------------------------------------------------------------------*/
/*!

*/
rcsc::Vector2D
Strategy::getPosWhenGoalieCatchOur( const int number ) const
{
    if ( number < 1 || 11 < number )
    {
        std::cerr << __FILE__ << ": " << __LINE__
                  << " ***ERROR*** Invalid player number " << number
                  << std::endl;
        return rcsc::Vector2D( -50.0, -30.0 );
    }

    return M_goalie_catch_our_pos[number - 1];
}

/*-------------------------------------------------------------------*/
/*!

*/
rcsc::Vector2D
Strategy::getSetPlayPosition( const int number,
                              const rcsc::Formation & formation,
                              const rcsc::WorldModel & world ) const
{
    if ( world.gameMode().type() == rcsc::GameMode::GoalKick_ )
    {
        if ( world.gameMode().side() == world.ourSide() )
        {
            return getPosWhenGoalKickOur( number );
        }

        if ( world.gameMode().side() != world.ourSide() )
        {
            return getPosWhenGoalKickOpp( number );
        }
    }

    if ( world.gameMode().type() == rcsc::GameMode::GoalieCatch_ )
    {
        if ( world.gameMode().side() == world.ourSide() )
        {
            rcsc::dlog.addText( rcsc::Logger::TEAM,
                                "%s: Strategy. get our galie catch mode position"
                                ,__FILE__ );
            return getPosWhenGoalieCatchOur( number );
        }

        if ( world.gameMode().side() != world.ourSide() )
        {
            return getPosWhenGoalieCatchOpp( number );
        }
    }

    rcsc::Vector2D base_pos( 0.0, 0.0 );
    if ( ! world.ball().posValid() )
    {
        return formation.getPosition( number, base_pos );
    }


    base_pos = world.ball().pos();


    bool our_setplay = false;
    switch ( world.gameMode().type() ) {
    case rcsc::GameMode::KickOff_:
    case rcsc::GameMode::KickIn_:
    case rcsc::GameMode::CornerKick_:
    case rcsc::GameMode::GoalKick_:
    case rcsc::GameMode::FreeKick_:
    case rcsc::GameMode::GoalieCatch_:
    case rcsc::GameMode::IndFreeKick_:
        if ( world.gameMode().side() == world.ourSide() )
        {
            our_setplay = true;
        }
        break;
    case rcsc::GameMode::OffSide_:
    case rcsc::GameMode::FreeKickFault_:
    case rcsc::GameMode::BackPass_:
    case rcsc::GameMode::CatchFault_:
        if ( world.gameMode().side() != world.ourSide() )
        {
            our_setplay = true;
        }
        break;
    default:
        break;
    }

    if ( ! our_setplay )
    {
        base_pos.x = std::max( -50.0,
                               base_pos.x - 10.0 );
    }

    rcsc::Vector2D home_pos = formation.getPosition( number, base_pos );

    // to onside
    if ( rcsc::ServerParam::i().useOffside() )
    {
        if ( world.gameMode().type() != rcsc::GameMode::KickIn_
             || world.gameMode().side() != world.ourSide() )
        {
            home_pos.x = std::min( home_pos.x, world.offsideLineX() - 0.5 );
        }
    }

    return home_pos;
}

/*-------------------------------------------------------------------*/
/*!

*/
boost:: shared_ptr< const rcsc::Formation >
Strategy::getFormation( const rcsc::WorldModel & world ) const
{
    if ( world.gameMode().type() == rcsc::GameMode::KickIn_
         || world.gameMode().type() == rcsc::GameMode::CornerKick_ )
    {
        if ( world.ourSide() == world.gameMode().side() )
        {
            return M_kickin_our_formation;
        }
    }
    else if ( world.gameMode().isOurSetPlay( world.ourSide() ) )
    {
        // our set-play
        return M_setplay_our_formation;
    }

    if ( world.ball().pos().x > 0.0 )
    {
        return M_offense_formation;
    }
    else
    {
        return M_defense_formation;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
Strategy::BallArea
Strategy::get_ball_area( const rcsc::Vector2D & ball_pos )
{
    if ( ball_pos.x > 39.0 )
    {
        if ( ball_pos.absY() > 17.0 )
        {
            return BA_Cross;
        }
        else
        {
            return BA_ShootChance;
        }
    }
    else if ( ball_pos.x > 34.0 )
    {
        if ( ball_pos.absY() > 20.0 )
        {
            return BA_DribbleAttack;
        }
        else
        {
            return  BA_ShootChance;
        }
    }
    else if ( ball_pos.x > -1.0 )
    {
        if ( ball_pos.absY() > 18.0 )
        {
            return BA_DribbleAttack;
        }
        else
        {
            return BA_OffMidField;
        }
    }
    else if ( ball_pos.x > -25.0 )
    {
        if ( ball_pos.absY() > 18.0 )
        {
            return BA_DribbleBlock;
        }
        else
        {
            return BA_DefMidField;
        }
    }
    else if ( ball_pos.x > -34.0 )
    {
        if ( ball_pos.absY() > 18.0 )
        {
            return BA_DribbleBlock;
        }
        else
        {
            return BA_Stopper;
        }
    }
    else if ( ball_pos.x > -36.5 )
    {
        if ( ball_pos.absY() > 20.0 )
        {
            return BA_CrossBlock;
        }
        else
        {
            return BA_Stopper;
        }
    }
    else
    {
        if ( ball_pos.absY() > 20.0 )
        {
            return BA_CrossBlock;
        }
        else
        {
            return BA_Danger;
        }
    }

    return BA_None;
}
