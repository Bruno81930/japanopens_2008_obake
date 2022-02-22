// -*-c++-*-

/*!
  \file coach_agent.h
  \brief basic coach agent Header File
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

#ifndef RCSC_COACH_COACH_AGENT_H
#define RCSC_COACH_COACH_AGENT_H

#include <rcsc/coach/global_world_model.h>
#include <rcsc/coach/coach_config.h>
#include <rcsc/common/soccer_agent.h>
#include <rcsc/common/team_graphic.h>

#include <boost/scoped_ptr.hpp>

#include <string>
#include <set>

namespace rcsc {

class CoachAudioSensor;
class CoachCommand;
class GlobalVisualSensor;
class SayMessageParser;

struct CoachAgentImpl;

/*!
  \class CoachAgent
  \brief abstract coach agent class
 */
class CoachAgent
    : public SoccerAgent {
private:

    //! internal implementation object
    boost::scoped_ptr< CoachAgentImpl > M_impl;

protected:

    //! configuration parameter set
    CoachConfig M_config;

    //! internal memory of field status
    GlobalWorldModel M_worldmodel;

private:

    //! the flags for team_graphic ok message
    std::set< TeamGraphic::Index > M_team_graphic_ok_set;

public:
    /*!
      \brief init member variables
     */
    CoachAgent();
    /*!
      \brief delete dynamic allocated memory
     */
    virtual
    ~CoachAgent();

    /*!
      \brief get configuration set
      \return const reference to the configuration class object
     */
    const
    CoachConfig & config() const
      {
          return M_config;
      }

    /*!
      \brief get field status
      \return const reference to the worldmodel instance
     */
    const
    GlobalWorldModel & world() const
      {
          return M_worldmodel;
      }

    /*!
      \brief get visual sensor.
      \return const reference to the visual sensor instance.
     */
    const
    GlobalVisualSensor & visualSensor() const;

    /*!
      \brief get audio sensor
      \return const reference to the audio sensor instance
     */
    const
    CoachAudioSensor & audioSensor() const;

    /*!
      \brief get team_graphic ok flags
      \return const reference to the flag container
    */
    const
    std::set< TeamGraphic::Index > & teamGraphicOKSet() const
      {
          return M_team_graphic_ok_set;
      }

protected:
    /*!
      \brief analyze command line options
      \param cmd_parser command lien parser object
      \return only if "help" option is given, false is returned.

      This method is called from SoccerAgent::init( argc, argv )
      SoccerAgent::init(argc,argv) should be called in main().
      Do NOT call this method by yourself!
     */
    virtual
    bool initImpl( CmdLineParser & cmd_parser );

    /*!
      \brief handle start event
      \return status of start procedure.

      This method is called on the top of BasicClient::run() method.
      The concrete agent must connect to the server and send init command.
      Do NOT call this method by yourself!
    */
    virtual
    bool handleStart();

    /*!
      \brief handle server message event

      Do NOT call this method by yourself!
     */
    virtual
    void handleMessage();

    /*!
      \brief handle timeout event
      \param timeout_count count of timeout without sensory message.
      \param waited_msec elapsed milli seconds sinc last sensory message.

      This method is called when select() timeout occurs
      Do NOT call this method by yourself!
     */
    virtual
    void handleTimeout( const int timeout_count,
                        const int waited_msec );

    /*!
      \brief handle exit event
     */
    virtual
    void handleExit();

    /*!
      \brief register new say message parser object
      \param parser pointer to the say mesage parser.
     */
    void addSayMessageParser( boost::shared_ptr< SayMessageParser > parser );

    /*!
      \brief remove registered parser object
      \param header say message header character
     */
    void removeSayMessageParser( const char header );

public:
    /*!
      \brief finalize program process
     */
    void finalize();

private:

    /*!
      \brief send init or reconnect command to server

      init commad is sent in BasicClient's run() method
      Do not call this method yourself!
     */
    void sendInitCommand();

    /*!
      \brief send disconnection command message to server
      and set the server status to end.
     */
    void sendByeCommand();

    ////////////////////////////////////

    /*!
      \brief launch each parsing method depending on the message type
      \param msg raw server message
     */
    void parse( const char * msg );

    /*!
      \brief analyze init replay message
      \param msg raw server message
    */
    void analyzeInit( const char * msg );

    /*!
      \brief analyze cycle info in server message
      \param msg raw server message
      \param by_see_global if message type is see_global, this value becomes true
      \return parsing result status
     */
    bool analyzeCycle( const char * msg,
                       const bool by_see_global );

    /*!
      \brief analyze see_global message
      \param msg raw server message
     */
    void analyzeSeeGlobal( const char * msg );

    /*!
      \brief analyze hear message
      \param msg raw server message
     */
    void analyzeHear( const char * msg );

    /*!
      \brief analyze referee message
      \param msg raw server message
     */
    void analyzeHearReferee( const char * msg );

    /*!
      \brief analyze audio message from player
      \param msg raw server message
    */
    void analyzeHearPlayer( const char * msg );

    /*!
      \brief analyze change_player_type message
      \param msg raw server message
    */
    void analyzeChangePlayerType( const char * msg );

    /*!
      \brief analyze player_type parameter message
      \param msg raw server message
    */
    void analyzePlayerType( const char * msg );

    /*!
      \brief analyze player_param parameter message
      \param msg raw server message
    */
    void analyzePlayerParam( const char * msg );

    /*!
      \brief analyze server_param parameter message
      \param msg raw server message
    */
    void analyzeServerParam( const char * msg );

    /*!
      \brief analyze clang version message
      \param msg raw server message
    */
    void analyzeCLangVer( const char * msg );

    /*!
      \brief analyze score message
      \param msg raw server message
    */
    void analyzeScore( const char * msg );

    /*!
      \brief analyze ok message
      \param msg raw server message
    */
    void analyzeOK( const char * msg );

    /*!
      \brief analyze ok team_graphic_? message
      \param msg raw server message
    */
    void analyzeOKTeamGraphic( const char * msg );

    /*!
      \brief analyze ok teamnames message
      \param msg raw server message
     */
    void analyzeTeamNames( const char * msg );

    /*!
      \brief analyze error message
      \param msg raw server message
    */
    void analyzeError( const char * msg );

    /*!
      \brief analyze warning message
      \param msg raw server message
    */
    void analyzeWarning( const char * msg );

    /*!
      \brief analyze include message
      \param msg raw server message
    */
    void analyzeInclude( const char * msg );

    /*!
      \brief main action decision. this method is called from handleMessage()
    */
    void action();

    /*!
      \brief send command string to the rcssserver
      \param com coach command object
      \return true if command is sent
     */
    bool sendCommand( const CoachCommand & com );

public:
    /*!
      \brief send check_ball command
      \return true if command is generated and sent
    */
    bool doCheckBall();

    /*!
      \brief send look command
      \return true if command is generated and sent
    */
    bool doLook();

    /*!
      \brief send team_name command
      \return true if command is generated and sent
    */
    bool doTeamNames();

    //bool doTeamGraphic();

    /*!
      \brief send eye command
      \brief on if true, send (eye on), else (eye off)
      \return true if command is generated and sent
    */
    bool doEye( bool on );

    /*!
      \brief send change_player_type command
      \param unum target player's uniform number
      \param type new player type Id
      \return true if command is generated and sent
    */
    bool doChangePlayerType( const int unum,
                             const int type );

    /*!
      \brief send change_player_types command
      \param types player change pair list
      \return true if command is generated and sent
    */
    bool doChangePlayerTypes( const std::vector< std::pair< int, int > > & types );

    /*!
      \brief send freeform message by say command
      \return true if command is generated and sent
    */
    bool doSayFreeform( const std::string & msg );


    //bool doSendCLang( const CLang & lang );

    /*!
      \brief send team_graphic command
      \return true if command is generated and sent
    */
    bool doTeamGraphic( const int x,
                        const int y,
                        const TeamGraphic & team_graphic );

protected:

    /*!
      \brief pure virtual method. register decision.

      This method is used to set coach's action.
      This method is called from action().
      So, do *NOT* call this method by yourself.
    */
    virtual
    void actionImpl() = 0;
};

}

#endif
