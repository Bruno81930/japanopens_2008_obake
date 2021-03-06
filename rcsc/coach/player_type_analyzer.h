// -*-c++-*-

/*!
  \file player_type_analyzer.h
  \brief player type analyzer class Header File
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

#ifndef RCSC_COACH_PLAYER_TYPE_ANALYZER_H
#define RCSC_COACH_PLAYER_TYPE_ANALYZER_H

#include <rcsc/coach/global_object.h>

#include <rcsc/geom/vector_2d.h>
#include <rcsc/game_time.h>
#include <rcsc/types.h>

namespace rcsc {

class GlobalPlayerObject;
class GlobalWorldModel;

/*!
  \class PlayerTypeAnalyzer
  \brief analyzer for opponent team players' player type
 */
class PlayerTypeAnalyzer {
private:

    struct Data {
        bool turned_; //!< player did turn
        bool maybe_referee_; //!< player may be moved by referee
        bool maybe_collide_; //!< player may be collided with others
        bool maybe_kick_;  //!< player may kick the ball
        Vector2D pos_; //!< last pos
        Vector2D vel_; //!< last vel
        double body_; //!< last body direction

        //! if invalid data is detected, positive value is set
        std::vector< int > invalid_flags_;

        int type_; //!< estimated type Id

        Data();
        void setDefaultType();
        void setUnknownType();
    };

    const GlobalWorldModel & M_world;

    GameTime M_updated_time; //!< last update time
    PlayMode M_playmode; //!< current game mode

    GlobalBallObject M_prev_ball; //!< last ball data
    Data M_teammate_data[11]; //!< data for analysis
    Data M_opponent_data[11]; //!< data for analysis

    //! not used
    PlayerTypeAnalyzer();
    //! not used
    PlayerTypeAnalyzer( const PlayerTypeAnalyzer & );
    //! not used
    PlayerTypeAnalyzer & operator=( const PlayerTypeAnalyzer & );
public:
    /*!
      \brief default constructor
      \param const reference to the world model instance
     */
    explicit
    PlayerTypeAnalyzer( const GlobalWorldModel & world );

    /*!
      \brief get the last updated time
      \return const reference to the variable
     */
    const
    GameTime & updatedTime() const
      {
          return M_updated_time;
      }

    /*!
      \brief analyze world model
      \param world const reference to the world model instance
     */
    void update();


    /*!
      \brief reset all data for the specified player.
      \param unum uniform number

      This method is called when coach receives the change_player_type message
     */
    void reset( const int unum );

   /*!
      \brief get opponent player's player type Id
      \param unum target teammate uniform number
      \return estimated player type Id. if unum is illegal, Unknown Id is returned.
    */
    int heteroID( const int unum ) const
      {
          if ( unum < 1 || 11 < unum )
          {
              std::cerr << "PlayerTypeAnalyzer::heteroID() Illegal unum "
                        << unum << std::endl;
              return Hetero_Unknown;
          }
          return M_opponent_data[ unum - 1 ].type_;
      }

private:

    /*!
      \brief reset last seen data
     */
    void updateLastData();

    /*!
      \brief analyzer player set
      \param players container of the opponent team players
     */
    void analyze();

    /*!
      \brief update referee flags
      \param world const reference to the world model
     */
    void checkTurn();

    /*!
      \brief update referee flags
      \param world const reference to the world model
     */
    void checkReferee();

    /*!
      \brief update collision flags
      \param world const reference to the world model
     */
    void checkCollisions();

    /*!
      \brief update kickable area weights
      \param world const reference to the world model
     */
    void checkKick();

    /*!
      \brief update player decay weights
      \param world const reference to the world model
     */
    void checkPlayerDecay();

    /*!
      \brief update player speed max weights
      \param world const reference to the world model
     */
    void checkPlayerSpeedMax();

};

}

#endif
