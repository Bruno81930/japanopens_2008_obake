// -*-c++-*-

/*!
  \file fullstate_sensor.h
  \brief fullstate info sensor Header File
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

#ifndef RCSC_PLAYER_FULLSTATE_SENSOR_H
#define RCSC_PLAYER_FULLSTATE_SENSOR_H

#include <rcsc/geom/vector_2d.h>
#include <rcsc/game_time.h>
#include <rcsc/types.h>

#include <vector>
#include <iostream>

namespace rcsc {

/*!
  \class FullstateSensor
  \brief fullstate info holder
*/
class FullstateSensor {
public:

    /*!
      \brief fullstate ball info
    */
    struct BallT {
        Vector2D pos_;
        Vector2D vel_;
    };

    /*!
      \brief fullstate player info
    */
    struct PlayerT {
        SideID side_; //!< left or right
        int unum_; //!< uniform number
        bool goalie_; //!< v8+.

        int type_; //!< player type id

        Vector2D pos_; //!< global position
        Vector2D vel_; //!< velocity
        double body_; // body angle
        double neck_; // neck angle relative to body angle

        double stamina_;
        double effort_;
        double recovery_;

        //! v8+. distance from pos_ to pointing point
        double pointto_dist_;
        //! v8+. pointing direction direction relative to face(=body+neck)
        double pointto_dir_;

        /*!
          \brief initialize member variables
        */
        PlayerT()
            : side_( NEUTRAL )
            , unum_( Unum_Unknown )
            , goalie_( false )
            , type_( 0 )
            , pos_( Vector2D::INVALIDATED )
            , vel_( 0.0, 0.0 )
            , body_( 0.0 )
            , neck_( 0.0 )
            , stamina_( 0.0 )
            , effort_( 0.0 )
            , recovery_( 0.0 )
            , pointto_dist_( -1.0 )
            , pointto_dir_( 0.0 )
          { }

        /*!
          \brief output to stream
          \param os reference to output stream
          \return reference to output stream
        */
        std::ostream & print( std::ostream & os ) const;
    };


    typedef std::vector< PlayerT > PlayerCont;

private:
    GameTime M_time; //!< last updated time

    /*
    // Following data are included in sense_body.
    // So, it is not necessary to analyze these data

    std::string M_playmode_str; //!< playmode string

    ViewQuality M_view_quality; //!< agent's view quality
    ViewWidth M_view_width; //!< agent's view width

    int M_kick_count; //!< executed command count
    int M_dash_count; //!< executed command count
    int M_turn_count; //!< executed command count
    int M_catch_count; //!< executed command count
    int M_move_count; //!< executed command count
    int M_turn_neck_count; //!< executed command count
    int M_change_view_count; //!< executed command count
    int M_say_count; //!< executed command count

    int M_arm_movable; //!< after this step, player can move the arm.
    int M_arm_expires; //!< while this step, player continue to point out.
    double M_pointto_dist; //!< pointing distance
    double M_pointto_dir; //!< pointing angle. this is relative to face
    int M_pointto_count; //!< sensed command count
    */

    // set the information of left-hand-side orientation

    //! fullstate ball info
    BallT M_ball;
    //! fullstate left team players
    PlayerCont M_left_team;
    //! fullstate right team players
    PlayerCont M_right_team;

    //! left team score
    int M_left_score;
    //! right team score
    int M_right_score;

    /*!
      \brief analyzer raw server message with protcol version 7
      \param msg server message
    */
    void parseV7( const char * msg );

    /*!
      \brief analyzer raw server message with protcol version 8 or later
      \param msg server message
    */
    void parseV8( const char * msg );
public:
    /*!
      \brief parse server message
      \param msg server message
      \param version client version
      \param current received game time
    */
    void parse( const char * msg,
                const double & version,
                const GameTime & current );

    /*!
      \brief reverse the coordinate system
     */
    void reverse();

    // accessor method

    /*!
      \brief get updated time
      \return cost reference to the game time
    */
    const
    GameTime & time() const
      {
          return M_time;
      }

    /*!
      \brief get analyzed ball data
      \return const reference to the internal ball object
    */
    const
    BallT & ball() const
      {
          return M_ball;
      }

    /*!
      \brief get analyzed left team data
      \return const reference to the player container
    */
    const
    PlayerCont & leftTeam() const
      {
          return M_left_team;
      }

    /*!
      \brief get analyzed right team data
      \return const reference to the player container
    */
    const
    PlayerCont & rightTeam() const
      {
          return M_right_team;
      }

    /*!
      \brief get left team score
      \return score value
    */
    int leftScore() const
      {
          return M_left_score;
      }

    /*!
      \brief get right team score
      \return score value
    */
    int rightScore() const
      {
          return M_right_score;
      }

    /*!
      \brief put all fullstate info to stream
      \param os reference to output stream
      \return reference to output stream
    */
    std::ostream & print( std::ostream & os ) const;

    /*!
      \brief put ball and one player's fullstate info to the debug stream
      \param world const reference to the WorldModel instance
    */
    //void printWithWorld( const WorldModel & world ) const;
};

}

#endif
