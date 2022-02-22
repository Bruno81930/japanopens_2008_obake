// -*-c++-*-

/*!
  \file coach_config.h
  \brief coach configuration Header File
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

#ifndef RCSC_COACH_CONFIG_H
#define RCSC_COACH_CONFIG_H

#include <string>

namespace rcsc {

class ParamMap;

/*!
  \class CoachConfig
  \brief coach configuration parameters
 */
class CoachConfig {
private:

    // basic setting

    std::string M_team_name; //!< our team name string
    std::string M_coach_name; //!< coach name string
    double      M_version; //!< client version

    //! if true coach should send its name by init command
    bool M_use_coach_name;

    //! select timeout interval
    int M_interval_msec;
    //! max server message wait time by second
    int M_server_wait_seconds;

    //! server host name
    std::string M_rcssserver_host;
    //! server port number
    int M_rcssserver_port;

    //! zlib compression level for the compression command
    int M_compression;

    //! if true, coach should send (eye on) command
    bool M_use_eye;

    //! if true, coach will analyze say message
    bool M_hear_say;

    //! if true, coach will try to analyze opponent team players' player type
    bool M_analyze_player_type;

    //! if true, coach send advise
    bool M_use_advise;
    //! if true, coach send freeform message
    bool M_use_freeform;

    //! use hetero player
    bool M_use_hetero;

    //! if true, coach may send the team logo by team_graphic command.
    bool M_use_team_graphic;

    //! maximum number of team_graphic command per cycle
    int M_max_team_graphic_per_cycle;

    //
    // debug
    //

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
    CoachConfig();

    /*!
      \brief nothing to do
     */
    ~CoachConfig();

    /*!
      \brief create parameter map
      \param param_map reference to the parameter map instance
     */
    void createParamMap( ParamMap & param_map );

private:
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
    const
    std::string & coachName() const
      {
          return M_coach_name;
      }

    bool useCoachName() const
      {
          return M_use_coach_name;
      }

    int intervalMSec() const
      {
          return M_interval_msec;
      }

    int serverWaitSeconds() const
      {
          return M_server_wait_seconds;
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

    bool useEye() const
      {
          return M_use_eye;
      }

    bool hearSay() const
      {
          return M_hear_say;
      }

    bool analyzePlayerType() const
      {
          return M_analyze_player_type;
      }

    bool useAdvise() const
      {
          return M_use_advise;
      }

    bool useFreeform() const
      {
          return M_use_freeform;
      }


    bool useHetero() const
      {
          return M_use_hetero;
      }

    bool useTeamGraphic() const
      {
          return M_use_team_graphic;
      }

    int maxTeamGraphicPerCycle() const
      {
          return M_max_team_graphic_per_cycle;
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
