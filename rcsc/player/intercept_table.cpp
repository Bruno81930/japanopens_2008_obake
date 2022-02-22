// -*-c++-*-

/*!
  \file intercept_table.cpp
  \brief interception info holder Source File
*/

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA

 This code is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 *EndCopyright:
 */

/////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "intercept_table.h"
#include "self_intercept.h"
#include "player_intercept.h"
#include "world_model.h"
#include "player_object.h"

#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include <rcsc/game_time.h>

#include <algorithm>

namespace rcsc {

const std::size_t InterceptTable::MAX_CYCLE = 24;

/*-------------------------------------------------------------------*/
/*!

*/
InterceptTable::InterceptTable( const WorldModel & world )
    : M_world( world )
{
    M_ball_pos_cache.reserve( MAX_CYCLE + 2 );
    //M_ball_vel_cache.reserve( MAX_CYCLE + 2 );

    M_self_cache.reserve( MAX_CYCLE + 2 );

    clear();
}

/*-------------------------------------------------------------------*/
/*!

*/
InterceptTable::~InterceptTable()
{

}

/*-------------------------------------------------------------------*/
/*!

*/
void
InterceptTable::clear()
{
    M_ball_pos_cache.clear();
    //M_ball_vel_cache.clear();

    M_self_reach_cycle = 1000;
    M_self_exhaust_reach_cycle = 1000;
    M_teammate_reach_cycle = 1000;
    M_opponent_reach_cycle = 1000;

    M_fastest_teammate = static_cast< PlayerObject * >( 0 );
    M_fastest_opponent = static_cast< PlayerObject * >( 0 );

    M_self_cache.clear();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
InterceptTable::update()
{
    static GameTime s_update_time( 0, 0 );

    if ( M_world.time() == s_update_time )
    {
        return;
    }
    s_update_time = M_world.time();

    dlog.addText( Logger::INTERCEPT,
                  "InterceptTable update" );

    // clear all data
    this->clear();

    // playmode check
    // if ball is always stopped in the current playmode,
    // not needed to predict ball chaser.
    if ( M_world.gameMode().type() != GameMode::PlayOn
         && M_world.gameMode().type() != GameMode::GoalKick_ )
    {
        return;
    }

    if ( ! M_world.self().posValid()
         || ! M_world.ball().posValid() )
    {
        dlog.addText( Logger::INTERCEPT,
                      "%s:%d: Invalid self or ball pos"
                      ,__FILE__, __LINE__ );
        return;
    }

    if ( M_world.self().isKickable()
         || M_world.existKickableTeammate()
         || M_world.existKickableOpponent() )
    {
        dlog.addText( Logger::INTERCEPT,
                      "%s:%d: Already exist kickable player"
                      ,__FILE__, __LINE__ );
    }

    createBallCache();

#ifdef DEBUG
    dlog.addText( Logger::INTERCEPT,
                  "==========Intercept Predict Self==========" );
#endif

    predictSelf();

#ifdef DEBUG
    dlog.addText( Logger::INTERCEPT,
                  "==========Intercept Predict Opponent==========" );
#endif

    predictOpponent();

#ifdef DEBUG
    dlog.addText( Logger::INTERCEPT,
                  "==========Intercept Predict Teammate==========" );
#endif

    predictTeammate();


    dlog.addText( Logger::INTERCEPT,
                  "<-----Intercet Self reach cycle = %d. exhaust reach step = %d ",
                  M_self_reach_cycle,
                  M_self_exhaust_reach_cycle );
    if ( M_fastest_teammate )
    {
        dlog.addText( Logger::INTERCEPT,
                      "<-----Intercept Teammate  fastest reach step = %d."
                      " teammate %d (%.1f %.1f)",
                      M_teammate_reach_cycle,
                      M_fastest_teammate->unum(),
                      M_fastest_teammate->pos().x,
                      M_fastest_teammate->pos().y );

    }
    if ( M_fastest_opponent )
    {
        dlog.addText( Logger::INTERCEPT,
                      "<-----Intercept Opponent  fastest reach step = %d."
                      " player %d (%.1f %.1f)",
                      M_opponent_reach_cycle,
                      M_fastest_opponent->unum(),
                      M_fastest_opponent->pos().x,
                      M_fastest_opponent->pos().y );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
InterceptTable::hearTeammate( const int unum,
                              const int cycle )
{
    if ( M_fastest_teammate
         && cycle >= M_teammate_reach_cycle )
    {
        return;
    }

    const PlayerObject * p = static_cast< PlayerObject * >( 0 );

    const PlayerPtrCont::const_iterator end = M_world.teammatesFromSelf().end();
    for ( PlayerPtrCont::const_iterator it = M_world.teammatesFromSelf().begin();
          it != end;
          ++it )
    {
        if ( (*it)->unum() == unum )
        {
            p = *it;
            break;
        }
    }

    if ( p )
    {
        M_fastest_teammate = p;
        M_teammate_reach_cycle = cycle;

        dlog.addText( Logger::INTERCEPT,
                      "<----- Hear Intercept Teammate  fastest reach step = %d."
                      " teammate %d (%.1f %.1f)",
                      M_teammate_reach_cycle,
                      M_fastest_teammate->unum(),
                      M_fastest_teammate->pos().x,
                      M_fastest_teammate->pos().y );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
InterceptTable::hearOpponent( const int unum,
                              const int cycle )
{
    if ( M_fastest_opponent )
    {
        if ( cycle >= M_opponent_reach_cycle )
        {
            dlog.addText( Logger::INTERCEPT,
                          "<----- Hear Intercept Opponent. no update."
                          " exist faster reach cycle %d >= %d",
                          cycle, M_opponent_reach_cycle );
            return;
        }

        if ( M_fastest_opponent->unum() == unum
             && M_fastest_opponent->posCount() == 0 )
        {
            dlog.addText( Logger::INTERCEPT,
                          "<----- Hear Intercept Opponent . no update."
                          " opponent %d (%.1f %.1f) is seen",
                          M_fastest_opponent->unum(),
                          M_fastest_opponent->pos().x,
                          M_fastest_opponent->pos().y );
            return;
        }
    }

    const PlayerObject * p = static_cast< PlayerObject * >( 0 );

    const PlayerPtrCont::const_iterator end = M_world.opponentsFromSelf().end();
    for ( PlayerPtrCont::const_iterator it = M_world.opponentsFromSelf().begin();
          it != end;
          ++it )
    {
        if ( (*it)->unum() == unum )
        {
            p = *it;
            break;
        }
    }

    if ( p )
    {
        M_fastest_opponent = p;
        M_opponent_reach_cycle = cycle;

        dlog.addText( Logger::INTERCEPT,
                      "<----- Hear Intercept Opponent  fastest reach step = %d."
                      " opponent %d (%.1f %.1f)",
                      M_opponent_reach_cycle,
                      M_fastest_opponent->unum(),
                      M_fastest_opponent->pos().x,
                      M_fastest_opponent->pos().y );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
InterceptTable::createBallCache()
{
    const double pitch_x_max = ServerParam::i().pitchHalfLength() + 5.0;
    const double pitch_y_max = ServerParam::i().pitchHalfWidth() + 5.0;
    const double bdecay = ServerParam::i().ballDecay();

    Vector2D bpos = M_world.ball().pos();
    Vector2D bvel = M_world.ball().vel();

    M_ball_pos_cache.push_back( bpos );
    //M_ball_vel_cache.push_back( bvel );

    if ( M_world.self().isKickable() )
    {
        return;
    }

    for ( std::size_t i = 1; i <= MAX_CYCLE; ++i )
    {
        bpos += bvel;
        bvel *= bdecay;
        if ( bpos.absX() > pitch_x_max
             || bpos.absY() > pitch_y_max )
        {
            // out of pitch
            break;
        }
        if ( bvel.r2() < 0.01 )
        {
            // ball stopped
            M_ball_pos_cache.push_back( bpos );
            bpos += bvel;
            M_ball_pos_cache.push_back( bpos );
            break;
        }

        M_ball_pos_cache.push_back( bpos );
        //M_ball_vel_cache.push_back( bvel );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
InterceptTable::predictSelf()
{
    if ( M_world.self().isKickable() )
    {
        dlog.addText( Logger::INTERCEPT,
                      "Intercept Teammate. already kickable. no estimation loop!" );
        M_self_reach_cycle = 0;
        return;
    }

    std::size_t max_cycle = std::min( MAX_CYCLE, M_ball_pos_cache.size() );

    SelfIntercept predictor( M_world );
    predictor.predict( max_cycle,
                       M_self_cache );
    if ( M_self_cache.empty() )
    {
        std::cerr << "Interecet Self cache is empty!"
                  << std::endl;
        // if  M_self_cache.empty() is true
        // reach point should be the inertia final point of the ball
        return;
    }


    int min_cycle = M_self_reach_cycle;
    int exhaust_min_cycle = M_self_exhaust_reach_cycle;
    const std::vector< InterceptInfo >::iterator end = M_self_cache.end();
    for ( std::vector< InterceptInfo >::iterator it = M_self_cache.begin();
          it != end;
          ++it )
    {
        if ( it->mode() == InterceptInfo::NORMAL )
        {
            if ( it->reachCycle() < min_cycle )
            {
                min_cycle = it->reachCycle();
            }
        }
        else
        {
            if ( it->reachCycle() < exhaust_min_cycle )
            {
                exhaust_min_cycle = it->reachCycle();
            }
        }
    }
    M_self_reach_cycle = min_cycle;
    M_self_exhaust_reach_cycle = exhaust_min_cycle;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
InterceptTable::predictTeammate()
{
    const PlayerPtrCont & teammates = M_world.teammatesFromBall();
    const PlayerPtrCont::const_iterator t_end = teammates.end();

    if ( M_world.existKickableTeammate() )
    {
        dlog.addText( Logger::INTERCEPT,
                      "Intercept Teammate. exist kickable teammate. no estimation loop!" );
        M_teammate_reach_cycle = 0;

        for ( PlayerPtrCont::const_iterator t = teammates.begin();
              t != t_end;
              ++t )
        {
            if ( (*t)->isGhost()
                   || (*t)->posCount() > M_world.ball().posCount() + 1 )
              {
                  continue;
              }

              if ( (*t)->isKickable() )
              {
                  M_fastest_teammate = *t;
              }
        }
        return;
    }

    int max_cycle = 1000;

    PlayerIntercept predictor( M_world, M_ball_pos_cache );

    for ( PlayerPtrCont::const_iterator it = teammates.begin();
          it != t_end;
          ++it )
    {
        if ( (*it)->posCount() >= 10 )
        {
            dlog.addText( Logger::INTERCEPT,
                          "Intercept  %s Teammate %d.(%.1f %.1f) Low accuracy %d. skip...",
                          ( (*it)->unum() < 0 ? "Unknown" : "" ),
                          (*it)->unum(),
                          (*it)->pos().x, (*it)->pos().y,
                          (*it)->posCount() );
            continue;
        }

        const PlayerType * player_type = M_world.teammatePlayerType( (*it)->unum() );
        if ( ! player_type )
        {
            std::cerr << M_world.time()
                      << " " << __FILE__ << ":" << __LINE__
                      << "  Failed to get teammate player type. unum = "
                      << (*it)->unum()
                      << std::endl;
            dlog.addText( Logger::INTERCEPT,
                          "ERROR. Intercept. Failed to get teammate player type.",
                          " unum = %d",
                          (*it)->unum() );
            continue;
        }

        int cycle = predictor.predict( *(*it), *player_type,
                                       max_cycle );
        dlog.addText( Logger::INTERCEPT,
                      "---> %s Teammate %d.(%.1f %.1f) cycle = %d",
                      ( (*it)->unum() < 0 ? "Unknown" : "" ),
                      (*it)->unum(),
                      (*it)->pos().x, (*it)->pos().y,
                      cycle );
        if ( cycle < max_cycle )
        {
            max_cycle = cycle;
            M_fastest_teammate = *it;
        }
    }


    if ( M_fastest_teammate && max_cycle < 1000 )
    {
        M_teammate_reach_cycle = max_cycle;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
InterceptTable::predictOpponent()
{
    const PlayerPtrCont & opponents = M_world.opponentsFromBall();
    const PlayerPtrCont::const_iterator o_end = opponents.end();

    if ( M_world.existKickableOpponent() )
    {
        dlog.addText( Logger::INTERCEPT,
                      "Intercept Opponent. exist kickable opponent. no estimation loop!" );
        M_opponent_reach_cycle = 0;

        for ( PlayerPtrCont::const_iterator o = opponents.begin();
              o != o_end;
              ++o )
        {
            if ( (*o)->isGhost()
                   || (*o)->posCount() > M_world.ball().posCount() + 1 )
              {
                  continue;
              }

              if ( (*o)->isKickable() )
              {
                  M_fastest_opponent = *o;
              }
        }

        return;
    }

    int max_cycle = 1000;

    PlayerIntercept predictor( M_world, M_ball_pos_cache );

    for ( PlayerPtrCont::const_iterator it = opponents.begin();
          it != o_end;
          ++it )
    {
        if ( (*it)->posCount() >= 10 )
        {
            dlog.addText( Logger::INTERCEPT,
                          "Intercept  %s Opponent %d. .(%.1f %.1f) Low accuracy %d. skip...",
                          ( (*it)->unum() < 0 ? "Unknown" : "" ),
                          (*it)->unum(),
                          (*it)->pos().x, (*it)->pos().y,
                          (*it)->posCount() );
            continue;
        }

        const PlayerType * player_type = M_world.opponentPlayerType( (*it)->unum() );
        if ( ! player_type )
        {
            std::cerr << M_world.time()
                      << " " << __FILE__ << ":" << __LINE__
                      << "  Failed to get opponent player type. unum = "
                      << (*it)->unum()
                      << std::endl;
            dlog.addText( Logger::INTERCEPT,
                          "ERROR. Intercept Failed to get opponent player type."
                          " unum = %d",
                          (*it)->unum());
            continue;
        }
        int cycle = predictor.predict( *(*it), *player_type,
                                       max_cycle );
        dlog.addText( Logger::INTERCEPT,
                      "---> %s Opponent.%d (%.1f %.1f) reach cycle = %d",
                      ( (*it)->unum() < 0 ? "Unknown" : "" ),
                      (*it)->unum(),
                      (*it)->pos().x, (*it)->pos().y,
                      cycle );
        if ( cycle < max_cycle )
        {
            max_cycle = cycle;
            M_fastest_opponent = *it;
        }
    }

    if ( M_fastest_opponent && max_cycle < 1000 )
    {
        M_opponent_reach_cycle = max_cycle;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
Vector2D
InterceptTable::selfInterceptPoint() const
{
    return M_world.ball().inertiaPoint( selfReachCycle() );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
InterceptTable::isSelfFastestPlayer() const
{
    if ( M_world.existKickableOpponent()
         || M_world.existKickableTeammate() )
    {
        dlog.addText( Logger::TEAM,
                      "%s: isSelfFastestPlayer. exist other kickable player"
                      ,__FILE__ );
        return false;
    }
    if ( M_world.self().isKickable() )
    {
        dlog.addText( Logger::TEAM,
                      "%s: isSelfFastestPlayer. I am already kickable"
                      ,__FILE__ );
        return true;
    }

    if ( selfReachCycle() == 1 )
    {
        dlog.addText( Logger::TEAM,
                      "%s: isSelfFastestPlayer. self min = 1"
                      ,__FILE__ );
        return true;
    }

    Vector2D reach_point = M_world.ball().inertiaPoint( selfReachCycle() );
    double self_dash = M_world.self().pos().dist( reach_point );

    double teammate_dash = 500.0, opponent_dash = 300.0;
    M_world.getTeammateNearestTo( reach_point, 30, &teammate_dash );
    M_world.getOpponentNearestTo( reach_point, 30, &opponent_dash );

    if ( teammateReachCycle() > 2
         && self_dash < teammate_dash - 1.0
         && opponentReachCycle() > 2
         && self_dash < opponent_dash )
    {
        dlog.addText( Logger::TEAM,
                      "%s: isSelfFastestPlayer. Maybe I am fastest. cycle = %d"
                      " move_dist=%.2f  teammate_move=%.2f  opponent_move=%.2f"
                      ,__FILE__,
                      teammateReachCycle(),
                      self_dash, teammate_dash, opponent_dash );
        return true;
    }

    dlog.addText( Logger::TEAM,
                  "%s: isSelfFastestPlayer. I am *NOT* a fastest player"
                  ,__FILE__ );
    return false;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
InterceptTable::isOurTeamBallPossessor() const
{
    if ( M_world.existKickableTeammate() )
    {
        dlog.addText( Logger::TEAM,
                      "%s: isOurTeamBallPossessor. exist kickable teammate"
                      ,__FILE__ );
        return true;
    }

    if ( teammateReachCycle() < opponentReachCycle() - 1 )
    {
        dlog.addText( Logger::TEAM,
                      "%s: isOurTeamBallPossessor. teammate can reach faster than opps"
                      ,__FILE__ );
        return true;
    }

    if ( isSelfFastestPlayer() )
    {
        dlog.addText( Logger::TEAM,
                      "%s: isOurTeamBallPossessor. I am fastest"
                      ,__FILE__ );
        return true;
    }

    dlog.addText( Logger::TEAM,
                  "%s: isOurTeamBallPossessor. Ball is out of our posession"
                  ,__FILE__ );

    return false;
}

}
