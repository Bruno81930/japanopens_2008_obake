// -*-c++-*-

/*!
  \file player_agent.h
  \brief basic player agent Header File
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

#ifndef RCSC_PLAYER_AGENT_H
#define RCSC_PLAYER_AGENT_H

#include <rcsc/player/world_model.h>
#include <rcsc/player/action_effector.h>
#include <rcsc/player/debug_client.h>
#include <rcsc/player/player_config.h>
#include <rcsc/player/see_state.h>
#include <rcsc/common/soccer_agent.h>
#include <rcsc/timer.h>
#include <rcsc/types.h>

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

namespace rcsc {

class DebugClient;
class BodySensor;
class VisualSensor;
class AudioSensor;
class FreeformParser;
class FullstateSensor;
class SeeState;
class ArmAction;
class NeckAction;
class ViewAction;
class SayMessage;
class SayMessageParser;
class SoccerIntention;

struct PlayerAgentImpl;

/*!
  \class PlayerAgent
  \brief basic player agent class
*/
class PlayerAgent
    : public SoccerAgent {
private:

    //! internal implementation object
    boost::scoped_ptr< PlayerAgentImpl > M_impl;

protected:

    //! configuration parameter set
    PlayerConfig M_config;

    //! debug client interface
    DebugClient M_debug_client;

    ///////////////////////////////

    //! mental memory of world status
    WorldModel M_worldmodel;

    //! action info manager
    ActionEffector M_effector;

public:
    /*!
      \brief create internal modules
    */
    PlayerAgent();

    /*!
      \brief virtual destructor
    */
    virtual
    ~PlayerAgent();

    /*!
      \brief get configuration set
      \return const reference to the configuration class object
    */
    const
    PlayerConfig & config() const
      {
          return M_config;
      }

    /*!
      \brief get debug client interface
      \return reference to the DebugClient object
    */
    DebugClient & debugClient()
      {
          return M_debug_client;
      }

    /*!
      \brief get worldmodel
      \return const reference to world model instance
    */
    const
    WorldModel & world() const
      {
          return M_worldmodel;
      }

    /*!
      \brief get action effector
      \return reference to action effector
    */
    const
    ActionEffector & effector() const
      {
          return M_effector;
      }

    /*!
      \brief get body sensor
      \return const reference to the body sensor instance
     */
    const
    BodySensor & bodySensor() const;

    /*!
      \brief get visual sensor
      \return const reference to the visual sensor instance
     */
    const
    VisualSensor & visualSensor() const;

    /*!
      \brief get audio sensor
      \return const reference to the audio sensor instance
     */
    const
    AudioSensor & audioSensor() const;

    /*!
      \brief get fullstate sensor
      \return const reference to the fullstate sensor instance
     */
    const
    FullstateSensor & fullstateSensor() const;

    /*!
      \brief get see state
      \return const reference to the see state instance
     */
    const
    SeeState & seeState() const;

    /*!
      \brief get time stamp when sense_body message is received
      \return const reference to the time stamp object
    */
    const
    TimeStamp & bodyTimeStamp() const;

    /*!
      \brief get time stamp of see message when see message is received
      \return const reference to the time stamp object
    */
    const
    TimeStamp & seeTimeStamp() const;

    /*!
      \brief register kick command
      \param power command argument kick power
      \param rel_dir command argument kick direction relative to body angle
      \return true if successfully registered.
    */
    bool doKick( const double & power,
                 const AngleDeg & rel_dir );

    /*!
      \brief register dash command
      \param power command argument dash power
      \return true if successfully registered.
    */
    bool doDash( const double & power );

    /*!
      \brief register turn command
      \param moment command argument moment
      \return true if successfully registered.
    */
    bool doTurn( const AngleDeg & moment );

    /*!
      \brief register catch command. catch direction is automatically calculated.
      \return true if successfully registered.
    */
    bool doCatch();

    /*!
      \brief register move command
      \param x move target x
      \param y move target y
      \return true if successfully registered.
    */
    bool doMove( const double & x,
                 const double & y );

    /*!
      \brief register tackle command
      \param power_or_dir command argument tackle power or direction
      \return true if successfully registered.
    */
    bool doTackle( const double & power_or_dir );

    /*!
      \brief register turn_neck command.
      \param moment command argument moment
      \return true if successfully registered.
    */
    bool doTurnNeck( const AngleDeg & moment );

    /*!
      \brief register change_view command.
      \param width new view width
      \return true if successfully registered.

      ViewQuality should not be changed by user
    */
    bool doChangeView( const ViewWidth & width );

    /*!
      \brief register say command.
      \param msg message string
      \return true if successfully registered.
    */
    //bool doSay( const std::string & msg );

    /*!
      \brief register pointto command.
      \param x target point x
      \param y target point y
      \return true if successfully registered.
    */
    bool doPointto( const double & x,
                    const double & y );

    /*!
      \brief register pointto command. turn off mode
      \return true if successfully registered.
    */
    bool doPointtoOff();

    /*!
      \brief register attentionto command by off mode
      \param side target player's side
      \param unum target player's uniform number
      \return true if successfully registered.
    */
    bool doAttentionto( SideID side,
                        const int unum );

    /*!
      \brief register attentionto command. turn off mode
      \return true if successfully registered.
    */
    bool doAttentiontoOff();


    /*!
      \brief reserve pointto action
      \param act pointer to the action. must be a dynamically allocated object.
    */
    void setArmAction( ArmAction * act );

    /*!
      \brief reserve turn neck action
      \param act pointer to the action. must be a dynamically allocated object.
    */
    void setNeckAction( NeckAction * act );

    /*!
      \brief reserve change view action
      \param act pointer to the action. must be a dynamically allocated object.
    */
    void setViewAction( ViewAction * act );

    /*!
      \brief add say message to the action effector
      \param message pointer to the say mesage builder. this must be a dynamically allocated object.
     */
    void addSayMessage( const SayMessage * message );

    /*!
      \brief remove the registered say message if exist
      \param header message header character
      \return true if removed
     */
    bool removeSayMessage( const char header );

    /*!
      \brief set intention object
      \param intention pointer to the intention. this must be a dynamically
      allocated object.
    */
    void setIntention( SoccerIntention * intention );

    /*!
      \brief execute queued intention if exist.
      \retval true if action is executed
      \retval false if queue is empty, or action is failed.
    */
    bool doIntention();

protected:

    /*!
      \brief analyze command line options
      \param cmd_parser command line parser object
      \return only if "help" option is given, false is returned.

      This method is called from SoccerAgent::init( argc, argv )
      SoccerAgent::init(argc,argv) should be called in main().
    */
    virtual
    bool initImpl( CmdLineParser & cmd_parser );

    /*!
      \brief handle start event
      \return status of start procedure.

      This method is called when client starts to run.
      The concrete agent must connect to the server and send init command.
      Do NOT call this method by yourself!
     */
    virtual
    bool handleStart();

    /*!
      \brief handle server message event

      This method is called from BasicClient::run() method.
      Do NOT call this method by yourself!
    */
    virtual
    void handleMessage();

    /*!
      \brief handle timeout event
      \param timeout_count count of timeout without sensory message.
      \param waited_msec elapsed milliseconds since last sensory message.
      This method is called from BasicClient::run() method.
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

    /*!
      \brief set new freeform message parser
      \param parser pointer to the freeform message parser.
     */
    void setFreeformParser( boost::shared_ptr< FreeformParser > parser );

public:
    /*!
      \brief finalize all things when program quits
    */
    void finalize();

private:

    /*!
      \brief send init or reconnect command to server

      init commad is sent in BasicClient's run() method
      Do NOT call this method by yourself!
    */
    void sendInitCommand();

    /*!
      \brief send disconnection command message to server
      and set the server status to end.
    */
    void sendByeCommand();

    /*!
      \brief set debug output flags to logger
     */
    void setDebugFlags();

    ////////////////////////////////////

    /*!
      \brief parser louncherinterface.
      \param msg raw server message
    */
    void parse( const char * msg );

    /*!
      \brief analyze cycle info in server message
      \param msg raw server message
      \param by_sense_body if message type is sense_body, this value becomes true
      \return parsing result status

      parse cycle data from several sensory message
      see, hear, sensebody and fullstate
    */
    bool analyzeCycle( const char * msg,
                       const bool by_sense_body );

    /*!
      \brief analyze see info
      \param msg raw server message
    */
    void analyzeSee( const char * msg );

    /*!
      \brief analyze sense_body info
      \param msg raw server message
    */
    void analyzeSenseBody( const char * msg );

    /*!
      \brief analyze hear info
      \param msg raw server message
    */
    void analyzeHear( const char * msg );

    /*!
      \brief analyze referee message inf hear info
      \param msg raw server message
    */
    void analyzeHearReferee( const char * msg );

    /*!
      \brief analyze player message in hear info
      \param msg raw server message
    */
    void analyzeHearPlayer( const char * msg );

    /*!
      \brief analyze our coach message in hear info
      \param msg raw server message
    */
    void analyzeHearOurCoach( const char * msg );

    /*!
      \brief analyze opponent coach message in hear info
      \param msg raw server message
    */
    void analyzeHearOpponentCoach( const char * msg );

    /*!
      \brief analyze trainer message in hear info
      \param msg raw server message
    */
    void analyzeHearTrainer( const char * msg );

    /*!
      \brief analyze fullstate message
      \param msg raw server message
    */
    void analyzeFullstate( const char * msg );

    /*!
      \brief analyze player_type message
      \param msg raw server message
    */
    void analyzePlayerType( const char * msg );

    /*!
      \brief analyze player_param message
      \param msg raw server message
    */
    void analyzePlayerParam( const char * msg );

    /*!
      \brief analyze server_param message
      \param msg raw server message
    */
    void analyzeServerParam( const char * msg );

    /*!
      \brief analyze init replay message
      \param msg raw server message
    */
    void analyzeInit( const char * msg );

    /*!
      \brief analyze change_player_type message
      \param msg raw server message
    */
    void analyzeChangePlayerType( const char * msg );


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
      \brief analyze error message
      \param msg raw server message
    */
    void analyzeError( const char * msg );

    /*!
      \brief analyze warningok message
      \param msg raw server message
    */
    void analyzeWarning( const char * msg );

    ////////////////////////////////////

    /*!
      \brief main action decision. this method calls doAction()
    */
    void action();

    /*!
      \brief perform reserved body action

      This method is called after doBodyAction()
    */
    void doArmAction();

    /*!
      \brief perform reserved change view action

      This method is called after doBodyAction()
      This method is called before doNeckAction()
    */
    void doViewAction();

    /*!
      \brief perform reserved turn neck action

      This method is called just after doViewAction()
    */
    void doNeckAction();

    ////////////////////////////////////

    /*!
      \brief adjust see message timing.

      This method is called when see info received
      This method is for normal server mode (not synch_mode).
    */
    void adjustSeeSynchNormalMode();

    /*!
      \brief adjust see message timing.

      This method is called only when 'think' message received(in decideAction())
      This method is for synch_mode.

      NOTE: this method depends on rcssserver setting.
      if synch_offset parametor is changed, this method must be modified.
    */
    void adjustSeeSynchSynchMode();

protected:

    /*!
      \brief pure virtual method. register body action to ActionEffector.

      This method is used to set player's action.
      This method is called from action().
      So, do *NOT* call this method by yourself.
    */
    virtual
    void actionImpl() = 0;

    /*!
      \brief virtual method. register say action to ActionEffector

      This method is called just after doTurnNeck() in decideAction();
      This method is used to set player's action.
      So, do *not* call this method by yourself.
    */
    virtual
    void communicationImpl()
      { }

    /*!
      \brief output debug messages to file or debug server

      This method is called in decideAction() and after send command.
      So, do *not* call this method by yourself.
    */
    virtual
    void outputDebug();
};

}

#endif
