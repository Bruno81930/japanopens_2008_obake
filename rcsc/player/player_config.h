// -*-c++-*-

/*!
  \file player_config.h
  \brief player configuration Header File
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

#ifndef RCSC_PLAYER_PLAYER_CONFIG_H
#define RCSC_PLAYER_PLAYER_CONFIG_H

#include <string>

namespace rcsc {

class ParamMap;

/*!
  \class PlayerConfig
  \brief player configuration variable set
*/
class PlayerConfig {
private:

    // basic setting

    std::string M_team_name; //!< team name string
    double      M_version; //!< client version
    int         M_reconnect_number; //!< uniform number for reconnection
    bool        M_goalie; //!< goalie flag

    int M_interval_msec; //!< timeout interval
    int M_server_wait_seconds; //!< time to wait server message

    //! msec threshold for action decision timing when see sync
    int M_wait_time_thr_synch_view;
    //! msec threshold for action decision timing when no see sync
    int M_wait_time_thr_nosynch_view;

    //! msec threshold for normal view width when manual see sync
    int M_normal_view_time_thr;

    std::string M_rcssserver_host; //!< host name that rcssserver is running
    int         M_rcssserver_port; //!< rcssserver connection port number

    int M_compression; //!< zlib compression level for the compression command

    int M_clang_min; //!< supported clang min version
    int M_clang_max; //!< supported clang max version

    bool M_use_communication; //!< if true, communiction is used
    bool M_hear_opponent_audio; //!< if true, opponent communication is heared

    bool M_use_fullstate; //!< if true, WorldModel is updated by fullstate info

    bool M_synch_see; //!< if true, synchronous see mode is used.

    // confidence value

    int M_self_pos_count_thr; //!< self position confidence threshold
    int M_self_vel_count_thr; //!< self velocity confidence threshold
    int M_self_face_count_thr; //!< self angle confidence threshold

    int M_ball_pos_count_thr; //!< ball position confidence threshold
    int M_ball_rpos_count_thr; //!< ball relative position confidence threshold
    int M_ball_vel_count_thr; //!< ball velocity confidence threshold

    int M_player_pos_count_thr; //!< player position confidence threshold
    int M_player_vel_count_thr; //!< player velocity confidence threshold
    int M_player_face_count_thr; //!< player angle confidence threshold


    //! specifies configuration file directory
    std::string M_config_dir;
    //! specifies player's number independent of uniform number
    int M_player_number;


    // debug

    bool M_debug_connect; //!< if true, connect to the debug server
    std::string M_debug_server_host; //!< host name that debug server is running
    int  M_debug_server_port; //!< debug server port number
    bool M_debug_write; //!< if true, debug info is written to file

    //! log written directory. this is common for all type log
    std::string M_log_dir;

    //! the extension string of debug log file
    std::string M_log_ext;

    // debug outut switches
    bool M_debug; //!< if false, log file or debug client are never opened
    bool M_debug_system;
    bool M_debug_sensor;
    bool M_debug_world;
    bool M_debug_action;
    bool M_debug_intercept;
    bool M_debug_kick;
    bool M_debug_dribble;
    bool M_debug_pass;
    bool M_debug_cross;
    bool M_debug_shoot;
    bool M_debug_clear;
    bool M_debug_team;
    bool M_debug_role;

public:

    /*!
      \brief init variables by default value. create parametermap
     */
    PlayerConfig();

    /*!
      \brief nothing to do
     */
    ~PlayerConfig();


    /*!
      \brief create parameter map
      \param param_map reference to the parameter map instance
     */
    void createParamMap( ParamMap & param_map );


protected:
    /*!
      \brief set default value
    */
    void setDefaultParam();

public:

    // basic settings
    const
    std::string & teamName() const
      {
          return M_team_name;
      }
    const
    double & version() const
      {
          return M_version;
      }

    int reconnectNumber() const
      {
          return M_reconnect_number;
      }

    bool goalie() const
      {
          return M_goalie;
      }

    int intervalMSec() const
      {
          return M_interval_msec;
      }

    int serverWaitSeconds() const
      {
          return M_server_wait_seconds;
      }

    int waitTimeThrSynchView() const
      {
          return M_wait_time_thr_synch_view;
      }

    int waitTimeThrNoSynchView() const
      {
          return M_wait_time_thr_nosynch_view;
      }

    int normalViewTimeThr() const
      {
          return M_normal_view_time_thr;
      }

    const
    std::string & host() const
      {
          return M_rcssserver_host;
      }

    int port() const
      {
          return M_rcssserver_port;
      }

    int compression() const
      {
          return M_compression;
      }

    int clangMin() const
      {
          return M_clang_min;
      }

    int clangMax() const
      {
          return M_clang_max;
      }


    bool useCommunication() const
      {
          return M_use_communication;
      }

    bool hearOpponentAudio() const
      {
          return M_hear_opponent_audio;
      }

    bool useFullstate() const
      {
          return M_use_fullstate;
      }

    bool synchSee() const
      {
          return M_synch_see;
      }

    // confidence value

    int selfPosCountThr() const
      {
          return M_self_pos_count_thr;
      }

    int selfVelCountThr() const
      {
          return M_self_vel_count_thr;
      }

    int selfFaceCountThr() const
      {
          return M_self_face_count_thr;
      }

    int ballPosCountThr() const
      {
          return M_ball_pos_count_thr;
      }

    int ballRPosCountThr() const
      {
          return M_ball_rpos_count_thr;
      }

    int ballVelCountThr() const
      {
          return M_ball_vel_count_thr;
      }

    int playerPosCountThr() const
      {
          return M_player_pos_count_thr;
      }

    int playerVelCountThr() const
      {
          return M_player_vel_count_thr;
      }

    int playerFaceCountThr() const
      {
          return M_player_face_count_thr;
      }

    const
    std::string & configDir() const
      {
          return M_config_dir;
      }

    int playerNumber() const
      {
          return M_player_number;
      }

    void setPlayerNumber( const int num )
      {
          M_player_number = num;
      }

    // debug

    bool debugConnect() const
      {
          return M_debug_connect;
      }

    const
    std::string & debugServerHost() const
      {
          return M_debug_server_host;
      }

    int debugServerPort() const
      {
          return M_debug_server_port;
      }

    bool debugWrite() const
      {
          return M_debug_write;
      }

    const
    std::string & logDir() const
      {
          return M_log_dir;
      }

    const
    std::string & logExt() const
      {
          return M_log_ext;
      }

    bool debug() const
      {
          return M_debug;
      }

    bool debugSystem() const
      {
          return M_debug_system;
      }

    bool debugSensor() const
      {
          return M_debug_sensor;
      }

    bool debugWorld() const
      {
          return M_debug_world;
      }

    bool debugAction() const
      {
          return M_debug_action;
      }

    bool debugIntercept() const
      {
          return M_debug_intercept;
      }

    bool debugKick() const
      {
          return M_debug_kick;
      }

    bool debugDribble() const
      {
          return M_debug_dribble;
      }

    bool debugPass() const
      {
          return M_debug_pass;
      }

    bool debugCross() const
      {
          return M_debug_cross;
      }

    bool debugShoot() const
      {
          return M_debug_shoot;
      }

    bool debugClear() const
      {
          return M_debug_clear;
      }

    bool debugTeam() const
      {
          return M_debug_team;
      }

    bool debugRole() const
      {
          return M_debug_role;
      }

};

}

#endif
