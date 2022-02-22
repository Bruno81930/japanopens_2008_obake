// -*-c++-*-

/*!
  \file intercept_table.h
  \brief interception info holder Header File
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

#ifndef RCSC_PLAYER_INTERCEPT_TABLE_H
#define RCSC_PLAYER_INTERCEPT_TABLE_H

#include <rcsc/geom/vector_2d.h>
#include <vector>

namespace rcsc {

class PlayerObject;
class WorldModel;

/*-------------------------------------------------------------------*/

/*!
  \class InterceptInfo
  \brief interception data that contains required action count
*/
class InterceptInfo {
public:
    enum Mode {
        NORMAL = 0, //!< ball gettable without stamina exhaust
        EXHAUST = 100, //!< fastest ball gettable, but recovery may be consumed.
    };
private:
    Mode M_mode; //!< interception mode, NORMAL or EXHAUST
    int M_turn_cycle; //!< estimated turn cycles
    int M_dash_cycle; //!< estimated dash cycles
    double M_dash_power; //!< dash power to be used

    //! not used
    InterceptInfo();
public:

    /*!
      \brief construct with all variables
    */
    InterceptInfo( const Mode mode,
                   const int turn_cycle,
                   const int dash_cycle,
                   const double & dash_power )
        : M_mode( mode )
        , M_turn_cycle( turn_cycle )
        , M_dash_cycle( dash_cycle )
        , M_dash_power( dash_power )
      { }

    /*!
      \brief get interception mode Id
      \return mode Id
    */
    Mode mode() const
      {
          return M_mode;
      }

    /*!
      \brief get estimated total turn cycles
      \return the number of turn cycles
    */
    int turnCycle() const
      {
          return M_turn_cycle;
      }

    /*!
      \brief get estimated total dash cycles
      \return the number of dash cycles
    */
    int dashCycle() const
      {
          return M_dash_cycle;
      }

    /*
      \brief get esitimated total cycle to reach
      \return the number of total cycles
    */
    int reachCycle() const
      {
          return turnCycle() + dashCycle();
      }

    /*!
      \brief get dash power to be used
      \return dash power to be used
    */
    const
    double & dashPower() const
      {
          return M_dash_power;
      }
};

/*-------------------------------------------------------------------*/

/*!
  \class InterceptTable
  \brief interception info holder for all players
*/
class InterceptTable {
private:

    //! maximal estimation cycle
    static const std::size_t MAX_CYCLE;

    //! const reference to the WorldModel instance
    const WorldModel & M_world;

    //! cache of predicted future ball positions
    std::vector< Vector2D > M_ball_pos_cache;
    //std::vector< Vector2D > M_ball_vel_cache;

    //! predicted min reach cycle for self without stamina exhaust
    int M_self_reach_cycle;
    //! predicted min reach cycle for self with stamina exhaust
    int M_self_exhaust_reach_cycle;
    //! predicted min reach cycle for teammate
    int M_teammate_reach_cycle;
    //! predicted min reach cycle for opponent
    int M_opponent_reach_cycle;

    //! const pointer to the fastest ball gettable teammate player object
    const PlayerObject * M_fastest_teammate;
    //! const pointer to the fastest ball gettable opponent player object
    const PlayerObject * M_fastest_opponent;

    //! interception info cache for smart interception
    std::vector< InterceptInfo > M_self_cache;


    //! not used
    InterceptTable();
    // noncopyable
    InterceptTable( const InterceptTable & );
    InterceptTable & operator=( const InterceptTable & );
public:
    /*!
      \brief init member variables, reserve cache vector memory
    */
    explicit
    InterceptTable( const WorldModel & world );

    /*!
      \brief destructor. nothing to do
    */
    ~InterceptTable();

    /*!
      \brief recreate all interception info
    */
    void update();

    /*!
      \brief set teammate intercept info mainly by heard info
      \param unum uniform number
      \param cycle interception cycle
     */
    void hearTeammate( const int unum,
                       const int cycle );

    /*!
      \brief set opponent intercept info mainly by heard info
      \param unum uniform number
      \param cycle interception cycle
     */
    void hearOpponent( const int unum,
                       const int cycle );

    /*!
      \brief get minimal ball gettable cycle for self without stamina exaust
      \return cycle value to get the ball
    */
    int selfReachCycle() const
      {
          return M_self_reach_cycle;
      }

    /*!
      \brief get minimal ball gettable cycle for self with stamina exaust
      \return cycle value to get the ball
    */
    int selfExhaustReachCycle() const
      {
          return M_self_exhaust_reach_cycle;
      }

    /*!
      \brief get minimal ball gettable cycle for teammate
      \return cycle value to get the ball
    */
    int teammateReachCycle() const
      {
          return M_teammate_reach_cycle;
      }

    /*!
      \brief get minimal ball gettable cycle for opponent
      \return cycle value to get the ball
    */
    int opponentReachCycle() const
      {
          return M_opponent_reach_cycle;
      }

    /*!
      \brief get fastest ball gettable teammate object
      \return const pointer to the PlayerObject.
      if not exist such a player, return NULL
    */
    const
    PlayerObject * fastestTeammate() const
      {
          return M_fastest_teammate;
      }

    /*!
      \brief get fastest ball gettable oppnent object
      \return const pointer to the PlayerObject.
      if not exist such a player, return NULL
    */
    const
    PlayerObject * fastestOpponent() const
      {
          return M_fastest_opponent;
      }

    /*!
      \brief get self interception cache container
      \return const reference to the interception info container
    */
    const
    std::vector< InterceptInfo > & selfCache() const
      {
          return M_self_cache;
      }

    /*!
      \brief get ball gettable point for self
      \return ball gettaable point
    */
    Vector2D selfInterceptPoint() const;

    /*!
      \brief check if self is the fastest ball bettable player
      \return true if estimated self is the fastest player
    */
    bool isSelfFastestPlayer() const;

    /*!
      \brief check if our team has the ball.
      \return true if estimated our team has the ball.
    */
    bool isOurTeamBallPossessor() const;

private:
    /*!
      \brief clear all cached data
    */
    void clear();

    /*!
      \brief create cache of future ball status
    */
    void createBallCache();

    /*!
      \brief predict self interception
    */
    void predictSelf();

    /*!
      \predict teammate interception
    */
    void predictTeammate();

    /*!
      \predict opponent interception
    */
    void predictOpponent();
};

}

#endif
