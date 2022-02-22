// -*-c++-*-

/*!
  \file shoot_table.h
  \brief shoot plan search and holder class
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

#ifndef RCSC_ACTION_SHOOT_TABLE_H
#define RCSC_ACTION_SHOOT_TABLE_H

#include <rcsc/geom/line_2d.h>
#include <rcsc/geom/vector_2d.h>
#include <rcsc/game_time.h>

#include <functional>
#include <vector>

namespace rcsc {

class PlayerAgent;
class PlayerObject;
class WorldModel;

/*!
  \class ShootTable
  \brief shoot plan search and holder table
 */
class ShootTable {
public:
    /*!
      \struct Shot
      \brief shoot object
     */
    struct Shot {
        Vector2D point_; //!< target point on the goal line
        Vector2D vel_; //!< first ball velocity
        double speed_; //!< first ball speed
        AngleDeg angle_; //!< shoot angle
        bool goalie_never_reach_; //!< true if goalie never reach the ball
        int score_; //!< evaluated value of this shoot

        /*!
          \brief constructor
          \param point shoot target point on the goal line
          \param speed first ball speed
          \param angle shoot angle
         */
        Shot( const Vector2D & point,
              const double & speed,
              const AngleDeg & angle )
            : point_( point )
            , vel_( Vector2D::polar2vector( speed, angle ) )
            , speed_( speed )
            , angle_( angle )
            , goalie_never_reach_( true )
            , score_( 0 )
          { }
    };

    //! type of the Shot container
    typedef std::vector< Shot > ShotCont;

    /*!
      \struct ScoreCmp
      \brief function object to evaluate the shoot object
     */
    struct ScoreCmp
        : public std::binary_function< Shot,
                                       Shot,
                                       bool > {
        /*!
          \brief compare operator
          \param lhs left hand side argument
          \param rhs right hand side argument
         */
        result_type operator()( const first_argument_type & lhs,
                                const second_argument_type & rhs ) const
          {
              return lhs.score_ > rhs.score_;
          }
    };

private:
    //! last calculated time
    GameTime M_time;
    //! cached calculated shoot pathes
    ShotCont M_shots;

public:
    /*!
      \brief accessible from global.
     */
    ShootTable()
      { }

    /*!
      \brief calculate the shoot and return the container
      \param agent const pointer to the agent
      \return const reference to the shoot container
     */
    const
    ShotCont & getShots( const PlayerAgent * agent )
      {
          search( agent );
          return M_shots;
      }

private:

    /*!
      \brief search shoot route patterns. goal mouth is divided to several
      segment and several ball speed for each segment are calculated.
      \param agent const pointer to the agent itself
     */
    void search( const PlayerAgent * agent );

    void calculateShotPoint( const WorldModel & wm,
                             const Vector2D & shot_point,
                             const PlayerObject * goalie );
    bool canScore( const WorldModel & wm,
                   Shot * shot );
    bool maybeGoalieCatch( const WorldModel & wm,
                           const PlayerObject * goalie,
                           Shot * shot );

};

}

#endif
