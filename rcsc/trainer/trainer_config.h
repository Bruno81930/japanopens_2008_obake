// -*-c++-*-

/*!
  \file trainer_config.h
  \brief trainer configuration Header File
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

#ifndef RCSC_TRAINER_CONFIG_H
#define RCSC_TRAINER_CONFIG_H

#include <string>

namespace rcsc {

class ParamMap;

/*!
  \class TrainerConfig
  \brief trainer configuration parameters
 */
class TrainerConfig {
private:

    // basic setting

    std::string M_team_name; //!< our team name string
    double      M_version; //!< client version

    //! max server message wait time by second
    int M_server_wait_seconds;

    //! server host name
    std::string M_rcssserver_host;
    //! server port number
    int         M_rcssserver_port;

    //! zlib compression level for the compression command
    int M_compression;

    //! if true trainer will send (eye on) command
    bool M_use_eye;

    //! if true trainer will send (ear on) command
    bool M_use_ear;

public:

    /*!
      \brief init variables by default value. create parametermap
     */
    TrainerConfig();

    /*!
      \brief nothing to do
     */
    ~TrainerConfig();

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

    bool useEar() const
      {
          return M_use_eye;
      }

};

}

#endif
