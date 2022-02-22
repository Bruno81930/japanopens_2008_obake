// -*-c++-*-

/*!
  \file player_agent.cpp
  \brief basic player agent Source File
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

#include "player_agent.h"

#include "body_sensor.h"
#include "visual_sensor.h"
#include "audio_sensor.h"
#include "fullstate_sensor.h"

#include "debug_client.h"
#include "freeform_parser.h"
#include "player_command.h"
#include "say_message_builder.h"
#include "soccer_action.h"
#include "soccer_intention.h"

#include <rcsc/common/audio_memory.h>
#include <rcsc/common/basic_client.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/player_param.h>
#include <rcsc/common/player_type.h>
#include <rcsc/common/server_param.h>
#include <rcsc/param/param_map.h>
#include <rcsc/param/cmd_line_parser.h>
#include <rcsc/param/conf_file_parser.h>
#include <rcsc/math_util.h>
#include <rcsc/game_time.h>
#include <rcsc/game_mode.h>

#include <sstream>
#include <cstdio>
#include <cstring>

//#define PROFILE_SEE

namespace rcsc {

///////////////////////////////////////////////////////////////////////
/*!
  \struct PlayerAgentImpl
  \brief player agent implementation
*/
struct PlayerAgentImpl {

    //! reference to the PlayerAgent instance
    const PlayerAgent & agent_;

    //! flag to check if (think) message was received or not.
    bool think_received_;

    //! flag to check if server cycle is stopped or not.
    bool server_cycle_stopped_;

    //! last action decision game time
    GameTime last_decision_time_;

    //! current game time
    GameTime current_time_;


    int clang_min_; //!< supported minimal clang version
    int clang_max_; //!< supported maximal clang version

    //! referee info
    GameMode game_mode_;

    // it may be useful to make the abstract sensor class
    // to manage sensor container.

    //! sense_body info
    BodySensor body_;
    //! see info
    VisualSensor visual_;
    //! hear info
    AudioSensor audio_;
    //! fullstate info
    FullstateSensor fullstate_;

    //! time when sense_body is received
    TimeStamp body_time_stamp_;
    //!< time when see is received
    TimeStamp see_time_stamp_;

    //! status of the see messaege arrival timing
    SeeState see_state_;

    //! counter of see message arrival timing
    int see_timings_[11];

    //! pointer to reserved action
    boost::shared_ptr< ArmAction > arm_action_;

    //! pointer to reserved action
    boost::shared_ptr< NeckAction > neck_action_;

    //! pointer to reserved action
    boost::shared_ptr< ViewAction > view_action_;

    //! intention queue
    boost::shared_ptr< SoccerIntention > intention_;

    /*!
      \brief initialize all members
    */
    explicit
    PlayerAgentImpl( const PlayerAgent & agent )
        : agent_( agent )
        , think_received_( false )
        , server_cycle_stopped_( true )
        , last_decision_time_( -1, 0 )
        , current_time_( 0, 0 )
        , clang_min_( 0 )
        , clang_max_( 0 )
      {
          for ( int i = 0; i < 11; ++i ) see_timings_[i] = 0;
      }

    /*!
      \brief update current time using analyzed time value
      \param new_time analyzed time value
      \param by_sense_body true if called after sense_body message
    */
    void updateCurrentTime( const long & new_time,
                            const bool by_sense_body );

    /*!
      \brief update server game cycle status.

      This method must be called just after referee message
    */
    void updateServerStatus();

    /*!
      \brief check if now decision timing
      \param msec_from_sense elapsed milli seconds from last sense_body message arrival
      \return true if player should send action
    */
    bool isDecisionTiming( const long & msec_from_sense ) const;

};

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerAgentImpl::updateCurrentTime( const long & new_time,
                                    const bool by_sense_body )
{
    // called after sense (see, hear, sense_body, fullstate...)

    GameTime old_time = current_time_;

    // server cycle stopped (BeforeKickOff, Offside, BackPass, FreeKickFault)
    if ( server_cycle_stopped_ )
    {
        if ( new_time != current_time_.cycle() )
        {
            current_time_.assign( new_time, 0 ) ;

            dlog.addText( Logger::LEVEL_ANY,
                          "CYCLE %ld-0 --------------------"
                          " return from cycle stop",
                          new_time );
            if ( new_time - 1 != old_time.cycle() )
            {
                std::cout << agent_.config().teamName() << ' '
                          << agent_.world().self().unum() << ": "
                          << current_time_
                          << " Stop Mode: previous server time is incorrect?? "
                          << old_time << " -> " << new_time << std::endl;
                dlog.addText( Logger::SYSTEM,
                              "System: stop mode: previous server time is incorrect??  (%ld, %ld) -> %ld",
                              old_time.cycle(), old_time.stopped(),
                              new_time );
            }
        }
        else
        {
            // if sense type is sense_body, it can be updated very safety.
            if ( by_sense_body )
            {
                current_time_.assign( current_time_.cycle(),
                                      current_time_.stopped() + 1 );

                dlog.addText( Logger::LEVEL_ANY,
                              "CYCLE %ld-%ld --------------------"
                              " stopped time was updated by sense_body",
                              current_time_.cycle(), current_time_.stopped() + 1 );
                if ( last_decision_time_ != old_time )
                {
                    if ( old_time.stopped() == 0 )
                    {
                        // just after changed to stop mode.
                        //   e.g. after goal playmode
                        // no error message
                    }
                    else
                    {
                        dlog.addText( Logger::SYSTEM,
                                      "missed last action(1)..." );
                        std::cout << agent_.config().teamName() << ' '
                                  << agent_.world().self().unum() << ": "
                                  << current_time_
                                  << " missed last action?(1) last decision="
                                  << last_decision_time_
                                  << std::endl;
                    }
                }
            }
        }
    }
    // normal case
    else
    {
        current_time_.assign( new_time, 0 );

        if ( old_time.cycle() != new_time )
        {
            dlog.addText( Logger::LEVEL_ANY,
                          "CYCLE %ld-0  -------------------------------------------------",
                          new_time );
            if ( new_time - 1 != old_time.cycle() )
            {
                std::cout << agent_.config().teamName() << ' '
                          << agent_.world().self().unum() << ": "
                          << current_time_
                          << " skipped server time?? "
                          << old_time << " -> " << new_time << std::endl;
                dlog.addText( Logger::SYSTEM,
                              "skipped server time?? (%ld, %ld) -> %ld",
                              old_time.cycle(), old_time.stopped(),
                              new_time );
            }

            if ( last_decision_time_.stopped() == 0
                 && last_decision_time_.cycle() != new_time - 1 )
            {
                dlog.addText( Logger::SYSTEM,
                              "missed last action(2)..." );
                std::cout << agent_.config().teamName() << ' '
                          << agent_.world().self().unum() << ": "
                          << current_time_
                          << " missed last action?(2) last decision="
                          << last_decision_time_
                          << std::endl;
            }
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerAgentImpl::updateServerStatus()
{
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //  This method must be called just after referee message

    // if current mode is stopped mode,
    // stopped flag is cleared.
    if ( server_cycle_stopped_ )
    {
        server_cycle_stopped_ = false;
    }

    if ( game_mode_.isServerCycleStoppedMode() )
    {
        server_cycle_stopped_ = true;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
PlayerAgentImpl::isDecisionTiming( const long & msec_from_sense ) const
{
    // if server is synch mode, 'think' message is sent to all clients
    // so it is not necessary to check decision timing.
    if ( ServerParam::i().synchMode() )
    {
        return false;
    }

    // already done in current cycle
    if ( last_decision_time_ == current_time_ )
    {
        return false;
    }

    // not initialized
    if ( agent_.world().self().unum() == Unum_Unknown )
    {
        return false;
    }

    // got current see info
    if ( agent_.world().seeTime() == current_time_ )
    {
        return true;
    }

    if ( see_state_.isSynch()
         && see_state_.cyclesTillNextSee() > 0 )
    {
        dlog.addText( Logger::SYSTEM,
                      "estimated cycles till next see ----- %d",
                      see_state_.cyclesTillNextSee() );
        return true;
    }

    // sense_body is not received yet
    if ( msec_from_sense < 0 )
    {
        return false;
    }

    int wait_thr = ( see_state_.isSynch()
                     ? agent_.config().waitTimeThrSynchView()
                     : agent_.config().waitTimeThrNoSynchView() );
    if ( msec_from_sense >= wait_thr * ServerParam::i().slowDownFactor() )
    {
        if ( see_state_.isSynch() )
        {
            std::cout << agent_.config().teamName() << ' '
                      << agent_.world().self().unum() << ": "
                      << current_time_
                      << " over offset - " << msec_from_sense
                      << "   server response delayed"
                      << std::endl;
        }
        dlog.addText( Logger::SYSTEM,
                      "over offset - %ld",
                      msec_from_sense );
        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------*/
/*!

*/
PlayerAgent::PlayerAgent()
    : SoccerAgent()
    , M_impl( new PlayerAgentImpl( *this ) )
    , M_debug_client()
    , M_worldmodel()
    , M_effector( *this )
{
    // std::cerr << "construct player" << std::endl;

    boost::shared_ptr< AudioMemory > audio_memory( new AudioMemory );
    M_worldmodel.setAudioMemory( audio_memory );
}

/*-------------------------------------------------------------------*/
/*!

*/
PlayerAgent::~PlayerAgent()
{
    //cerr << "delete PlayerAgent" << endl;
}

/*-------------------------------------------------------------------*/
/*!

*/
const
BodySensor &
PlayerAgent::bodySensor() const
{
    return M_impl->body_;
}

/*-------------------------------------------------------------------*/
/*!

*/
const
VisualSensor &
PlayerAgent::visualSensor() const
{
    return M_impl->visual_;
}

/*-------------------------------------------------------------------*/
/*!

*/
const
AudioSensor &
PlayerAgent::audioSensor() const
{
    return M_impl->audio_;
}

/*-------------------------------------------------------------------*/
/*!

*/
const
FullstateSensor &
PlayerAgent::fullstateSensor() const
{
    return M_impl->fullstate_;
}

/*-------------------------------------------------------------------*/
/*!

*/
const
SeeState &
PlayerAgent::seeState() const
{
    return M_impl->see_state_;
}

/*-------------------------------------------------------------------*/
/*!

*/
const
TimeStamp &
PlayerAgent::bodyTimeStamp() const
{
    return M_impl->body_time_stamp_;
}

/*-------------------------------------------------------------------*/
/*!

*/
const
TimeStamp &
PlayerAgent::seeTimeStamp() const
{
    return M_impl->see_time_stamp_;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
PlayerAgent::initImpl( CmdLineParser & cmd_parser )
{
    bool help = false;
    std::string player_config_file;

    ParamMap system_param_map( "System options" );
    system_param_map.add()
        ( "help" , "", BoolSwitch( &help ), "print help message.")
        ( "player-config", "", &player_config_file, "specifies player config file." );

    ParamMap player_param_map( "Player options" );
    M_config.createParamMap( player_param_map );


    // analyze command line for system options.
    cmd_parser.parse( system_param_map );
    if ( help )
    {
        system_param_map.printHelp( std::cout );
        player_param_map.printHelp( std::cout );
        M_client->setServerAlive( false );
        return false;
    }

    // analyze config file for PlayerConfig
    if ( ! player_config_file.empty() )
    {
        ConfFileParser conf_parser( player_config_file.c_str() );
        conf_parser.parse( player_param_map );
    }

    // analyze command line for player options.
    cmd_parser.parse( player_param_map );

    if ( config().version() < 8.0
         || 13.0 <= config().version() )
    {
        std::cerr << "Unsupported client version: " << config().version()
                  << std::endl;
        return false;
    }

    setDebugFlags();

    SelfObject::set_count_thr( config().selfPosCountThr(),
                               config().selfVelCountThr(),
                               config().selfFaceCountThr() );

    BallObject::set_count_thr( config().ballPosCountThr(),
                               config().ballRPosCountThr(),
                               config().ballVelCountThr() );

    PlayerObject::set_count_thr( config().playerPosCountThr(),
                                 config().playerVelCountThr(),
                                 config().playerFaceCountThr() );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
PlayerAgent::handleStart()
{
    if ( ! M_client )
    {
        return false;
    }

    if ( config().host().empty() )
    {
        std::cerr << config().teamName()
                  << ": ***ERROR*** server host name is empty" << std::endl;
        M_client->setServerAlive( false );
        return false;
    }

    // just create a connection. init command is automaticcaly sent
    // by BasicClient's run() method.
    if ( ! M_client->connectTo( config().host().c_str(),
                                config().port(),
                                static_cast< long >( config().intervalMSec() ) ) )
    {
        std::cerr << config().teamName()
                  << ": ***ERROR*** Failed to connect to ["
                  << config().host()
                  << "]"
                  << std::endl;
        M_client->setServerAlive( false );
        return false;
    }

    sendInitCommand();
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerAgent::handleMessage()
{
    if ( ! M_client )
    {
        std::cerr << "PlayerAgent::handleMessage(). Client is not registered."
                  << std::endl;
        return;
    }

    int counter = 0;
    GameTime start_time = M_impl->current_time_;

    // receive and analyze message
    while ( M_client->recvMessage() > 0 )
    {
        ++counter;
        parse( M_client->message() );
    }

    // game cycle is change while several message parsing
    if ( M_impl->current_time_.cycle() > start_time.cycle() + 1
         && start_time.stopped() == 0
         && M_impl->current_time_.stopped() == 0 )
    {
        std::cout << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << "parser used several steps -- Missed an action?"
                  << "  sensory counts= " << counter
                  << "  start_time= " << start_time
                  << "  end_time= " << M_impl->current_time_
                  << std::endl;
        dlog.addText( Logger::SYSTEM,
                      "parser used several steps -- action missed! sensed %d"
                      " start=(%ld, %ld) end=(%ld, %ld)",
                      counter,
                      start_time.cycle(), start_time.stopped(),
                      M_impl->current_time_.cycle(),
                      M_impl->current_time_.stopped() );
    }

    if ( M_impl->think_received_ )
    {
        dlog.addText( Logger::SYSTEM,
                      "Got think message: decide action" );
        action();
        M_impl->think_received_ = false;
    }
    else if ( ! ServerParam::i().synchMode() )
    {
        if ( M_impl->last_decision_time_ != M_impl->current_time_
             && world().seeTime() == M_impl->current_time_
             )
        {
            // player got a current cycle visual information
            // decide action imeddiately
            dlog.addText( Logger::SYSTEM,
                          "Got see info: decide action" );
            action();
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerAgent::handleTimeout( const int timeout_count,
                            const int waited_msec )
{
    if ( ! M_client )
    {
        std::cerr << "PlayerAgent::handleTimeout(). Client is not registered."
                  << std::endl;
        return;
    }

    TimeStamp cur_time;
    cur_time.setCurrent();
    long msec_from_sense = -1;
    /*
      std::cerr << "cur_sec = " << cur_time.sec()
      << "  cur_usec = " << cur_time.usec()
      << "   sense_sec=" << M_impl->body_time_stamp_.sec()
      << "  sense_usec=" << M_impl->body_time_stamp_.usec()
      << std::endl;
    */
    if ( M_impl->body_time_stamp_.sec() > 0 )
    {
        msec_from_sense = cur_time.getMSecDiffFrom( M_impl->body_time_stamp_ );
    }

    dlog.addText( Logger::SYSTEM,
                  "----- Timeout. msec from sense_body = [%ld] ms."
                  " Timeout count = %d",
                  msec_from_sense / ServerParam::i().slowDownFactor(),
                  timeout_count );

    // estimate server down
    if ( waited_msec > config().serverWaitSeconds() * 1000 )
    {
        std::cout << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << "waited "
                  << waited_msec / 1000
                  << " seconds. server down??" << std::endl;
        M_client->setServerAlive( false );
        return;
    }

    // check alarm count etc...
    if ( ! M_impl->isDecisionTiming( msec_from_sense ) )
    {
        return;
    }

    // start decision
    dlog.addText( Logger::SYSTEM,
                  "----- TIMEOUT DECISION !! [%ld]ms from sense_body",
                  msec_from_sense / ServerParam::i().slowDownFactor() );

    action();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerAgent::handleExit()
{
    finalize();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerAgent::addSayMessageParser( boost::shared_ptr< SayMessageParser > parser )
{
    M_impl->audio_.addParser( parser );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerAgent::removeSayMessageParser( const char header )
{
    M_impl->audio_.removeParser( header );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerAgent::setFreeformParser( boost::shared_ptr< FreeformParser > parser )
{
    M_impl->audio_.setFreeformParser( parser );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerAgent::finalize()
{
    if ( M_client->isServerAlive() )
    {
        sendByeCommand();
    }
#ifdef PROFILE_SEE
    std::cout << config().teamName() << ' '
              << world().self().unum() << ": "
              << "profile see arrival timing\n";
    std::printf( "    10    20    30    40    50    60    70    80    90   100  over\n" );
    for ( int i = 0; i < 11; ++i )
    {
        std::printf( "%6d", M_impl->see_timings_[i] );
    }
    std::printf( "\n" );
#endif
    std::cout << config().teamName() << ' '
              << world().self().unum() << ": "
              << "finished."
              << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerAgent::sendInitCommand()
{
    std::ostringstream ostrm;

    // make command string

    if ( config().reconnectNumber() < 1
         || 11 < config().reconnectNumber() )
    {
        // normal case
        PlayerInitCommand com( config().teamName(),
                               config().version(),
                               config().goalie() );
        com.toStr( ostrm );
    }
    else
    {
        std::cout << config().teamName()
                  << ": reconnect. number = "
                  << config().reconnectNumber() << std::endl;
        // reconnect
        PlayerReconnectCommand com( config().teamName(),
                                    config().reconnectNumber() );
        com.toStr( ostrm );

    }

    // send to server
    if ( M_client->sendMessage( ostrm.str().c_str() ) <= 0 )
    {
        std::cout << config().teamName()
                  << ": Failed to init ...\nExit ..." << std::endl;
        M_client->setServerAlive( false );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerAgent::sendByeCommand()
{
    PlayerByeCommand com;
    std::ostringstream ostrm;

    com.toStr( ostrm );
    M_client->sendMessage( ostrm.str().c_str() );

    M_client->setServerAlive( false );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerAgent::setDebugFlags()
{
    if ( ! config().debug() )
    {
        return;
    }

    dlog.setLogFlag( &(M_impl->current_time_), Logger::SYSTEM, config().debugSystem() );
    dlog.setLogFlag( &(M_impl->current_time_), Logger::SENSOR, config().debugSensor() );
    dlog.setLogFlag( &(M_impl->current_time_), Logger::WORLD, config().debugWorld() );
    dlog.setLogFlag( &(M_impl->current_time_), Logger::ACTION, config().debugAction() );
    dlog.setLogFlag( &(M_impl->current_time_), Logger::INTERCEPT, config().debugIntercept() );
    dlog.setLogFlag( &(M_impl->current_time_), Logger::KICK, config().debugKick() );
    dlog.setLogFlag( &(M_impl->current_time_), Logger::DRIBBLE, config().debugDribble() );
    dlog.setLogFlag( &(M_impl->current_time_), Logger::PASS, config().debugPass() );
    dlog.setLogFlag( &(M_impl->current_time_), Logger::CROSS, config().debugCross() );
    dlog.setLogFlag( &(M_impl->current_time_), Logger::SHOOT, config().debugShoot() );
    dlog.setLogFlag( &(M_impl->current_time_), Logger::CLEAR, config().debugClear() );
    dlog.setLogFlag( &(M_impl->current_time_), Logger::TEAM, config().debugTeam() );
    dlog.setLogFlag( &(M_impl->current_time_), Logger::ROLE, config().debugRole() );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerAgent::parse( const char * msg )
{

    if ( ! std::strncmp( msg, "(see ", 5 ) )
    {
        analyzeSee( msg );
    }
    else if ( ! std::strncmp( msg, "(sense_body ", 12 ) )
    {
        analyzeSenseBody( msg );
    }
    else if ( ! std::strncmp( msg, "(hear ", 6 ) )
    {
        analyzeHear( msg );
    }
    else if ( ! std::strncmp( msg, "(think)", 7 ) )
    {
        M_impl->think_received_ = true;
    }
    else if ( ! std::strncmp( msg, "(fullstate ", 11 ) )
    {
        analyzeFullstate( msg );
    }
    else if ( ! std::strncmp( msg, "(change_player_type ", 20 ) )
    {
        analyzeChangePlayerType( msg );
    }
    else if ( ! std::strncmp( msg, "(player_type ", 13 ) )  // hetero param
    {
        analyzePlayerType( msg );
    }
    else if ( ! std::strncmp( msg, "(player_param ", 14 ) ) // tradeoff param
    {
        analyzePlayerParam( msg );
    }
    else if ( ! std::strncmp(msg, "(server_param ", 14) )
    {
        analyzeServerParam( msg );
    }
    else if ( ! std::strncmp( msg, "(ok ", 4 ) )
    {
        analyzeOK( msg );
    }
    else if ( ! std::strncmp( msg, "(error ", 7 ) )
    {
        analyzeError( msg );
    }
    else if ( ! std::strncmp( msg, "(warning ", 9 ) )
    {
        analyzeWarning( msg );
    }
    else if ( ! std::strncmp( msg, "(score ", 7 ) )
    {
        analyzeScore( msg );
    }
    else if ( ! std::strncmp( msg, "(init ", 6 )
              || ! std::strncmp( msg, "(reconnect ", 11 ) )
    {
        analyzeInit( msg );
    }
    else
    {
        std::cout << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << " Received unsupported message : ["
                  << msg << "]" << std::endl;
    };
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
PlayerAgent::analyzeCycle( const char * msg,
                           bool by_sense_body )
{
    char id[16];
    long cycle = 0;
    if ( std::sscanf( msg, "(%15s %ld ",
                      id, &cycle ) != 2 )
    {
        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << "time parse error in ["
                  << msg
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "Cycle parse error [%s]", msg );
        return false;
    }

    M_impl->updateCurrentTime( cycle,
                               by_sense_body );
    return true;
}

/*-------------------------------------------------------------------*/
/*!
  (see <TIME> <ObjInfo> <ObjInfo> <ObjInfo> ...
*/
void
PlayerAgent::analyzeSee( const char * msg )
{
    M_impl->see_time_stamp_.setCurrent();
    long msec_from_sense = -1;
    if ( M_impl->body_time_stamp_.sec() > 0 )
    {
        msec_from_sense = M_impl->see_time_stamp_.getMSecDiffFrom( M_impl->body_time_stamp_ );
#ifdef PROFILE_SEE
        if ( M_impl->see_state_.isSynch() )
        {
            int index = (int)(msec_from_sense / ServerParam::i().slowDownFactor()) / 10;
            if ( index > 10 )
            {
                index = 10;
            }
            M_impl->see_timings_[index] += 1;
        }
#endif
    }

    // parse cycle info
    if ( ! analyzeCycle( msg, false ) )
    {
        return;
    }

    dlog.addText( Logger::SENSOR,
                  "===receive see --- [%ld]ms from sense_body",
                  msec_from_sense );

    // parse see info
    M_impl->visual_.parse( msg,
                           config().teamName().c_str(),
                           config().version(),
                           M_impl->current_time_ );
    //M_impl->visual_.print( std::cout );

    // update see timing status
    M_impl->see_state_.updateBySee( M_impl->current_time_,
                                    world().self().viewWidth(),
                                    world().self().viewQuality() );
    // debug purpose
    if ( M_impl->visual_.time() != M_impl->body_.time() )
    {
        std::cout << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << " got see without sense_body." << std::endl;
        dlog.addText( Logger::SYSTEM,
                      "analyzeSee. (%ld, %ld) without sense_body",
                      M_impl->visual_.time().cycle(),
                      M_impl->visual_.time().stopped() );
    }

    // update world model
    if ( M_impl->visual_.time() == M_impl->current_time_
         && world().seeTime() != M_impl->current_time_ )
    {
        // update seen objects
        M_worldmodel.updateAfterSee( M_impl->visual_,
                                     M_impl->body_,
                                     M_effector,
                                     M_impl->current_time_ );
    }

    // adjust see synch
    if ( ! ServerParam::i().synchMode()
         && ! M_impl->see_state_.isSynch() )
    {
        dlog.addText( Logger::SYSTEM,
                      "SEE received. but see timing is not synched. try to adjust" );
        adjustSeeSynchNormalMode();
    }
}

/*-------------------------------------------------------------------*/
/*!
  (sense_body <Time> <BodyInfo> <BodyInfo> ...
*/
void
PlayerAgent::analyzeSenseBody( const char * msg )
{
    M_impl->body_time_stamp_.setCurrent();

    // parse cycle info
    if ( ! analyzeCycle( msg, true ) )
    {
        return;
    }

    // analyze process
    dlog.addText( Logger::SENSOR,
                  "===receive sense_body" );

    // parse body info
    M_impl->body_.parse( msg,
                         config().version(),
                         M_impl->current_time_ );
    // update see sync information
    //    M_impl->see_state_.setNewCycle( M_impl->current_time_ );
    M_impl->see_state_.updateBySenseBody( M_impl->current_time_,
                                          M_impl->body_.viewWidth(),
                                          M_impl->body_.viewQuality() );

    //----------------------------------------------
    // update process
    // check command counter
    M_effector.checkCommandCount( M_impl->body_ );
    // pure internal update
    M_worldmodel.updateAfterSense( M_impl->body_,
                                   M_effector,
                                   M_impl->current_time_ );
}

/*-------------------------------------------------------------------*/
/*!
  Referee:
  -> (hear <TIME> referee <PLAYMODE>)
  Trainer:
  -> v7-: (hear <TIME> referee <MSG>)
  -> v7+: (hear <TIME> coach <MSG>)
  Coach::
  -> v7-: (hear <TIME> online_coach_{left|right} <MSG>) // no double quatation
  Self or other Player:
  -> v7-:
  ---> self:  (hear <TIME> self <MSG>)
  ---> other: (hear <TIME> <DIR> <MSG>)
  -> v7:
  ---> self:   (hear <TIME> self "<MSG>")
  ---> other:  (hear <TIME> <DIR> "<MSG>")
  -> v8+:
  ---> self:   (hear <TIME> self "<MSG>")
  ---> teammate complete: (hear <TIME> <DIR> our <UNUM> "<MSG>")
  ---> teammate partial:  (hear <TIME> our <UNUM>)
  ---> opponent complete: (hear <TIME> <DIR> opp "<MSG>")
  ---> opponent partial:  (hear <TIME> opp)
*/
void
PlayerAgent::analyzeHear( const char * msg )
{
    // parse cycle info
    if ( ! analyzeCycle( msg, false ) )
    {
        return;
    }
    // parse sender info
    long cycle;
    char sender[128];

    if ( std::sscanf( msg, "(hear %ld %127s ",
                      &cycle, sender ) != 2 )
    {
        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << " ***ERRORR*** failed to parse audio sender. ["
                  << msg << ']'
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "Hear parse error [%s]", msg );
        return;
    }


    // check sender name

    if ( ! std::strncmp( sender, "self", 4 ) )
    {
        // nothing to do
    }
    else if ( sender[0] == '-' || std::isdigit( sender[0] ) )
    {
        // complete audio from other player
        // sender string means the direction to the sender player.
        analyzeHearPlayer( msg );
    }
    else if ( ! std::strncmp( sender, "our", 3 )
              || ! std::strncmp( sender, "opp", 3 ) )
    {
        // partial audio from other player
        // nothing to do
    }
    else if ( ! std::strncmp( sender, "referee", 7 ) )
    {
        analyzeHearReferee( msg );
    }
    else if ( ! std::strncmp( sender, "online_coach_left", 17 ) )
    {
        if ( world().isOurLeft() ) analyzeHearOurCoach( msg );
        if ( world().isOurRight() ) analyzeHearOpponentCoach( msg );
    }
    else if ( ! std::strncmp( sender, "online_coach_right", 18 ) )
    {
        if ( world().isOurRight() ) analyzeHearOurCoach( msg );
        if ( world().isOurLeft() ) analyzeHearOpponentCoach( msg );
    }
    else if ( ! std::strncmp( sender, "coach", 5 ) )
    {
        analyzeHearTrainer( msg );
    }
    else
    {
        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << " Received unsupported hear message ["
                  << msg << "]" << std::endl;
    }

}

/*-------------------------------------------------------------------*/
/*!
  TODO: In protocol version 6-, tarainer messages are delivered as an
  refereee message.
*/
void
PlayerAgent::analyzeHearReferee( const char * msg )
{
    dlog.addText( Logger::SENSOR,
                  "===receive referee [%s]",
                  msg );
    long cycle;
    char mode[512]; // playmode or trainer's message

    if ( std::sscanf( msg, "(hear %ld referee %511[^)]", &cycle, mode ) != 2 )
    {
        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << " playmode scan error. " << msg
                  << std::endl;
        return;
    }

    if ( ! M_impl->game_mode_.update( mode, M_impl->current_time_ ) )
    {
        if ( config().version() < 7.0 )
        {
            std::cerr << world().teamName() << ' '
                      << world().self().unum() << ": "
                      << world().time()
                      << " Referee messaga is analyzed as trainer message"
                      << std::endl;
            analyzeHearTrainer( msg );
        }
        else
        {
            std::cerr << world().teamName() << ' '
                      << world().self().unum() << ": "
                      << world().time()
                      << " Unknown playmode string. [" << mode << ']'
                      << std::endl;
            M_impl->game_mode_.update( "play_on", M_impl->current_time_ );
        }
        return;
    }

    M_impl->updateServerStatus();

    if ( M_impl->game_mode_.isGameEndMode() )
    {
        sendByeCommand();
        return;
    }

    M_worldmodel.updateGameMode( M_impl->game_mode_, M_impl->current_time_ );

    ////////////////////////////////////////////////////////
    // if playmode change to NOT play_on mode,
    // reset current intention queue
    if ( M_impl->game_mode_.type() != GameMode::PlayOn
         && M_impl->game_mode_.type() != GameMode::PenaltyTaken_ )
    {
        M_impl->intention_.reset();
    }
    ////////////////////////////////////////////////////////
    // if playmode is change to penalty shoot out mode,
    // role must be changed to PenaltyShooter or PenaltyGoalie

}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerAgent::analyzeHearPlayer(  const char * msg )
{
    //----------------------------------------------
    // analyze process
    dlog.addText( Logger::SENSOR,
                  "===receive hear [%s]",
                  msg );

    if ( ! config().useCommunication() )
    {
        return;
    }

    // parse message
    M_impl->audio_.parsePlayerMessage( msg, M_impl->current_time_ );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerAgent::analyzeHearOurCoach( const char * msg )
{
    dlog.addText( Logger::SENSOR,
                  "===receive say message from our coach" );

    M_impl->audio_.parseCoachMessage( msg, M_impl->current_time_ );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerAgent::analyzeHearOpponentCoach( const char * msg )
{
    dlog.addText( Logger::SENSOR,
                  "===receive say message from opponent coach" );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerAgent::analyzeHearTrainer( const char * msg )
{
    dlog.addText( Logger::SENSOR,
                  "===receive trainer audio" );

    M_impl->audio_.parseTrainerMessage( msg, M_impl->current_time_ );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerAgent::analyzeFullstate( const char * msg )
{
    if ( ! analyzeCycle( msg, false ) )
    {
        return;
    }

    dlog.addText( Logger::SENSOR,
                  "===receive fullstate" );

    M_impl->fullstate_.parse( msg,
                              config().version(),
                              M_impl->current_time_ );
    if ( world().isOurRight() )
    {
        M_impl->fullstate_.reverse();
    }

    if ( config().useFullstate() )
    {
        M_worldmodel.updateAfterFullstate( M_impl->fullstate_,
                                           effector(),
                                           M_impl->current_time_ );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerAgent::analyzePlayerType( const char * msg )
{
    dlog.addText( Logger::SENSOR,
                  "===receive player_type" );
    PlayerType player_type( ServerParam::i(),
                            msg,
                            config().version() );
    PlayerTypeSet::instance().insert( player_type );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerAgent::analyzePlayerParam( const char * msg )
{
    dlog.addText( Logger::SENSOR,
                  "===receive player_param" );
    PlayerParam::instance().parse( msg, config().version() );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerAgent::analyzeServerParam( const char * msg )
{
    dlog.addText( Logger::SENSOR,
                  "===receive server_param" );
    //std::cout << msg << std::endl;
    ServerParam::instance().parse( msg, config().version() );
    PlayerTypeSet::instance().resetDefaultType( ServerParam::i() );

    M_worldmodel.setTeammatePlayerType( world().self().unum(),
                                        Hetero_Default );

    // update alarm interval
    if ( ! ServerParam::i().synchMode()
         && ServerParam::i().slowDownFactor() > 1 )
    {
        long interval = ( config().intervalMSec()
                          * ServerParam::i().slowDownFactor() );
        /*
          std::cout << "slow_down_factor is changed. new simst="
          << ServerParam::i().simulatorStep()
          << "  new interval=" << interval
          << std::endl;
        */
        M_client->setIntervalMSec( interval );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerAgent::analyzeInit( const char * msg )
{
    char mode[128];
    char side_char;
    int  myunum;

    bool reconnect = false;

    // init message
    if ( ! std::strncmp( msg, "(init ", std::strlen( "(init " ) ) )
    {
        // get SIDE, UNUM and PLAYMODE
        std::sscanf( msg, "(init %c %d %127[^)]", &side_char, &myunum, mode );
        std::cerr << config().teamName() << ": "
                  << "init ok.  unum: " << myunum << " side: " << side_char
                  << std::endl;
    }
    // reconnect message
    else if ( ! std::strncmp( msg, "(reconnect ", std::strlen( "(reconnect " ) ) )
    {
        // get the SIDE and PLAYMODE
        reconnect = true;
        std::sscanf( msg, "(reconnect %c %127[^)]", &side_char, mode );
        myunum = config().reconnectNumber();
        std::cerr << config().teamName()
                  << ": reconnected as number:"
                  << config().reconnectNumber()
                  << "  side: " << side_char << std::endl;
    }
    else
    {
        std::cout << config().teamName()
                  << ": failed to get an init message: " << msg << std::endl;
        M_client->setServerAlive( false );
        return;
    }


    if ( reconnect
         && ( config().reconnectNumber() < 1
              || 11 < config().reconnectNumber() ) )
    {
        std::cerr << config().teamName()
                  << ": parsed reconnect, but reconect number is not specified??"
                  << std::endl;
        M_client->setServerAlive( false );
        return;
    }


    ////////////////////////////////////////////////////////
    SideID sidetype = ( side_char == 'l'
                        ? LEFT
                        : RIGHT );

    ////////////////////////////////////////////////////////
    // member initialization concerned with team side & uniform number

    if ( ! M_impl->game_mode_.update( mode, M_impl->current_time_ ) )
    {
        std::cerr << config().teamName() << ' '
                  << " Failed to parse init replay message."
                  << " Unknown playmode string. [" << mode << ']'
                  << std::endl;
        M_impl->game_mode_.update( "play_on", M_impl->current_time_ );
    }

    M_impl->updateServerStatus();

    if ( config().playerNumber() == 0 )
    {
        M_config.setPlayerNumber( myunum );
    }

    if ( ! M_worldmodel.initTeamInfo( config().teamName(),
                                      sidetype, myunum,
                                      config().goalie() ) )
    {
        M_client->setServerAlive( false );
        return;
    }

    ////////////////////////////////////////////////////////
    // debugger initialization

    if ( config().debugConnect() )
    {
        M_debug_client.connect( config().debugServerHost(),
                                config().debugServerPort() );
    }

    if ( config().debugWrite() )
    {
        M_debug_client.setWriteMode( true );
    }

    if ( config().debug() )
    {
        std::ostringstream ostrm;
        std::string file_dir = config().logDir();
        if ( file_dir.empty() )
        {
            ostrm << "./";
        }
        else
        {
            ostrm << file_dir;
            if ( file_dir[file_dir.length() - 1] != '/' )
            {
                ostrm << '/';
            }
        }
        ostrm << config().teamName() << "-" << myunum
              << config().logExt();
        dlog.open( ostrm.str().c_str() );
        if ( ! dlog.isOpen() )
        {
            std::cerr << config().teamName() << ' '
                      << world().self().unum() << ": "
                      << " Failed to open file [" << ostrm.str() << "]"
                      << std::endl;
        }
    }

    ////////////////////////////////////////////////////////
    // send special settings

    {
        std::ostringstream ostrm;
        //////////////////////////
        // these must be moved to before kick off action ??

        // set synch see mode
        if ( config().synchSee() )
        {
            ostrm << "(synch_see)";
        }

        // turn off opponent all audio message

        if ( ! config().hearOpponentAudio() )
        {
            // off, opp
            PlayerEarCommand opp_ear_com( PlayerEarCommand::OFF,
                                          PlayerEarCommand::OPP );
            opp_ear_com.toStr( ostrm );
        }

        if ( ! config().useCommunication() )
        {
            // off, our
            PlayerEarCommand our_ear_com( PlayerEarCommand::OFF,
                                          PlayerEarCommand::OUR );
            our_ear_com.toStr( ostrm );
        }

        // set clang version

        if ( config().clangMin() != M_impl->clang_min_
             || config().clangMax() != M_impl->clang_max_ )
        {
            PlayerCLangCommand com( config().clangMin(),
                                    config().clangMax() );
            com.toStr( ostrm );
        }

        // set compression level
        if ( 0 < config().compression()
             && config().compression() <= 9 )
        {
            PlayerCompressionCommand com( config().compression() );
            com.toStr( ostrm );
        }

        dlog.addText( Logger::SYSTEM,
                      "---- send[%s]",
                      ostrm.str().c_str() );
        M_client->sendMessage( ostrm.str().c_str() );
        //////////////////////////
    }

}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerAgent::analyzeChangePlayerType( const char * msg )
{
    // teammate: "(change_player_type <unum> <type>)\n"
    // opponent: "(change_player_type <unum>)\n"

    int unum = Unum_Unknown;
    int type = Hetero_Unknown;

    dlog.addText( Logger::SENSOR,
                  "parse change_player_type [%s]",
                  msg );

    if ( std::sscanf( msg, " ( change_player_type %d %d ) ",
                      &unum, &type ) == 2 )
    {
        M_worldmodel.setTeammatePlayerType( unum, type );
    }
    else if ( std::sscanf( msg, " ( change_player_type %d ) ",
                           &unum ) == 1 )
    {
        M_worldmodel.setOpponentPlayerType( unum, Hetero_Unknown );
    }
    else
    {
        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << " ***ERROR*** Failed to analyze change_player_type" << std::endl;
        dlog.addText( Logger::SENSOR,
                      "error: change_player_type" );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerAgent::analyzeOK( const char * msg )
{
    dlog.addText( Logger::SENSOR,
                  "===receive ok [%s]",
                  msg );

    if ( ! std::strncmp( msg,
                         "(ok synch_see)",
                         std::strlen( "(ok synch_see)" ) ) )
    {
        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << " set synch see mode."
                  << std::endl;
        M_impl->see_state_.setSynchSeeMode();
        M_impl->see_state_.setViewMode( world().self().viewWidth(),
                                        world().self().viewQuality() );
        return;
    }
    if ( ! std::strncmp( msg,
                         "(ok compression ",
                         std::strlen( "(ok compression " ) ) )
    {
        int level = 0;
        if ( std::sscanf( msg, "(ok compression %d)", &level ) == 1 )
        {
            std::cerr << world().teamName() << ' '
                      << world().self().unum() << ": "
                      << world().time()
                      << " set compression level " << level
                      << std::endl;
            M_client->setCompressionLevel( level );
            return;
        }
    }
    else if ( ! std::strncmp( msg,
                              "(ok clang ",
                              std::strlen( "(ok clang " ) ) )
    {
        // (ok clang (ver 7 8))
        int vermin, vermax;
        if ( std::sscanf( msg, "(ok clang (ver %d %d))", &vermin, &vermax ) == 2 )
        {
            //std::cerr << config().teamName() << ' '
            //          << world().self().unum() << ": "
            //          << M_impl->current_time_
            //          << "set clang version " << vermin << " " << vermax
            //          << std::endl;
            M_impl->clang_min_ = vermin;
            M_impl->clang_max_ = vermax;
            return;
        }
    }

    std::cerr << config().teamName() << ' '
              << world().self().unum() << ": "
              << M_impl->current_time_
              << " recv unsupported or illegal ok message [" << msg << "]" << std::endl;
    dlog.addText( Logger::SENSOR,
                  "unsupported ok" );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerAgent::analyzeScore( const char * msg )
{
    dlog.addText( Logger::SENSOR,
                  "===receive score [%s]",
                  msg );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerAgent::analyzeError( const char * msg )
{
    dlog.addText( Logger::SENSOR,
                  "===receive error [%s]",
                  msg );
    std::cerr << world().teamName() << ' '
              << world().self().unum() << ": "
              << world().time()
              << " recv error message [" << msg << "]" << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerAgent::analyzeWarning( const char * msg )
{
    dlog.addText( Logger::SENSOR,
                  "===receive warning [%s]",
                  msg );
    std::cerr << world().teamName() << ' '
              << world().self().unum() << ": "
              << world().time()
              << "recv warning message [" << msg << "]" << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerAgent::action()
{
    dlog.addText( Logger::SYSTEM,
                  "decide action" );

    // check see synchronization
    if ( M_impl->see_state_.isSynch()
         && M_impl->see_state_.cyclesTillNextSee() == 0
         && world().seeTime() != M_impl->current_time_ )
    {
        dlog.addText( Logger::SYSTEM,
                      "missed see synch. action without see" );
        std::cout << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << " missed see synch. action without see" << std::endl;

        // set synch timing to illegal.
        M_impl->see_state_.setLastSeeTiming( SeeState::TIME_NOSYNCH );
    }

    // ------------------------------------------------------------------------
    // last update
    // update positining matrix, offside line, defense line, etc.
    M_worldmodel.updateJustBeforeDecision( effector(),
                                           M_impl->current_time_ );
    // reset last action effect
    M_effector.reset();

    // ------------------------------------------------------------------------
    // decide action

    if ( ServerParam::i().synchMode()
         && ! M_impl->see_state_.isSynch() )
    {
        adjustSeeSynchSynchMode();
    }

    actionImpl(); // this is pure virtual method
    doArmAction();
    doViewAction();
    doNeckAction();
    communicationImpl();

    // ------------------------------------------------------------------------
    // set command effect. these must be called before command composing.
    // set self view mode, pointto and attentionto info.
    M_worldmodel.setCommandEffect( effector() );
    // set cycles till next see, update estimated next see arrival timing
    M_impl->see_state_.setViewMode( world().self().viewWidth(),
                                    world().self().viewQuality() );

    // ------------------------------------------------------------------------
    // compose command string, and send it to the rcssserver
    {
        std::ostringstream ostrm;
        M_effector.makeCommand( ostrm );
        const std::string str = ostrm.str();
        if ( str.length() > 0 )
        {
            dlog.addText( Logger::SYSTEM,
                          "---- send[%s]",
                          str.c_str() );
            M_client->sendMessage( str.c_str() );
        }
    }

    // ------------------------------------------------------------------------
    // update last decision time
    M_impl->last_decision_time_ = M_impl->current_time_;

    // ------------------------------------------------------------------------
    // debugger output
    outputDebug();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerAgent::doArmAction()
{
    if ( M_impl->arm_action_ )
    {
        M_impl->arm_action_->execute( this );
        M_impl->arm_action_.reset();
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerAgent::doViewAction()
{
    if ( ! M_impl->see_state_.isSynch()
         && world().gameMode().type() != GameMode::PlayOn )
    {
        dlog.addText( Logger::SYSTEM,
                      "%s:%d: not play_on. system must be adjust see synch..."
                      ,__FILE__, __LINE__ );
        return;
    }

    if ( M_impl->view_action_ )
    {
        M_impl->view_action_->execute( this );
        M_impl->view_action_.reset();
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerAgent::doNeckAction()
{
    if ( M_impl->neck_action_ )
    {
        M_impl->neck_action_->execute( this );
        M_impl->neck_action_.reset();
    }
    else
    {
        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << "  WARNING. no turn_neck." << std::endl;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerAgent::adjustSeeSynchNormalMode()
{
    if ( M_impl->see_state_.isSynch() )
    {
        return;
    }

    // now, see & sense_body must be received simultaneously
    // --> synch chance
    if ( M_impl->see_state_.isSynchedSeeCountNormalMode() )
    {
        dlog.addText( Logger::SYSTEM,
                      "adjustSeeSynchNormalMode. see count is synch case" );
        // set synchronized see timing
        M_impl->see_state_.setLastSeeTiming( SeeState::TIME_0_00 );

        PlayerChangeViewCommand com( ViewWidth::NORMAL, ViewQuality::HIGH );
        std::ostringstream ostrm;
        com.toStr( ostrm );
        M_client->sendMessage( ostrm.str().c_str() );

        dlog.addText( Logger::SYSTEM,
                      "---- send[%s] see sync",
                      ostrm.str().c_str() );

        M_effector.incCommandCount( PlayerCommand::CHANGE_VIEW );
        M_worldmodel.setViewMode( com.width(), com.quality() );
        M_impl->see_state_.setViewMode( com.width(), com.quality() );

        std::cout << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << "  see synch." << std::endl;
        return;
    }

    // Now, no synchronization

    // playmode is play_on
    // --> stop adjustment trial.
    if ( world().gameMode().type() == GameMode::PlayOn )
    {
        if ( world().self().viewQuality().type() == ViewQuality::LOW )
        {
            PlayerChangeViewCommand com( ViewWidth::NARROW, ViewQuality::HIGH );
            std::ostringstream ostrm;
            com.toStr( ostrm );
            M_client->sendMessage( ostrm.str().c_str() );

            dlog.addText( Logger::SYSTEM,
                          "---- send[%s] no sync. change to high",
                          ostrm.str().c_str() );

            M_effector.incCommandCount( PlayerCommand::CHANGE_VIEW );
            M_worldmodel.setViewMode( com.width(), com.quality() );
            M_impl->see_state_.setViewMode( com.width(), com.quality() );
        }
        return;
    }

    // playmode is not play_on
    // --> try adjustment. view mode is changed to (Narrow, Low)
    if ( world().self().viewWidth().type() != ViewWidth::NARROW
         || world().self().viewQuality().type() != ViewQuality::LOW )
    {
        PlayerChangeViewCommand com( ViewWidth::NARROW, ViewQuality::LOW );
        std::ostringstream ostrm;
        com.toStr( ostrm );
        M_client->sendMessage( ostrm.str().c_str() );

        dlog.addText( Logger::SYSTEM,
                      "---- send[%s] prepare see sync",
                      ostrm.str().c_str() );

        M_effector.incCommandCount( PlayerCommand::CHANGE_VIEW );
        M_worldmodel.setViewMode( com.width(), com.quality() );
        M_impl->see_state_.setViewMode( com.width(), com.quality() );

        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << "  prepare see synch" << std::endl;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerAgent::adjustSeeSynchSynchMode()
{
    if ( M_impl->see_state_.isSynch() )
    {
        return;
    }

    // last see timing is just 50ms from sense_body
    // --> synch chance
    if ( M_impl->see_state_.isSynchedSeeCountSynchMode() )
    {
        // set synchronized see timing
        M_impl->see_state_.setLastSeeTiming( SeeState::TIME_50_0 );

        PlayerChangeViewCommand com( ViewWidth::NARROW, ViewQuality::HIGH );
        std::ostringstream ostrm;
        com.toStr( ostrm );
        M_client->sendMessage( ostrm.str().c_str() );

        dlog.addText( Logger::SYSTEM,
                      "---- send[%s] synch_mode. see synch",
                      ostrm.str().c_str() );

        M_effector.incCommandCount( PlayerCommand::CHANGE_VIEW );
        M_worldmodel.setViewMode( com.width(), com.quality() );
        M_impl->see_state_.setViewMode( com.width(), com.quality() );

        std::cout << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << "  see synch." << std::endl;
        return;
    }

    // Now, no synchronization

    // playmode is play_on
    // --> stop adjustment trial.
    if ( world().gameMode().type() == GameMode::PlayOn )
    {
        if ( world().self().viewQuality().type() == ViewQuality::LOW )
        {
            PlayerChangeViewCommand com( ViewWidth::NARROW, ViewQuality::HIGH );
            std::ostringstream ostrm;
            com.toStr( ostrm );
            M_client->sendMessage( ostrm.str().c_str() );

            dlog.addText( Logger::SYSTEM,
                          "---- send[%s] synch_mode. no sync. change to high",
                          ostrm.str().c_str() );

            M_effector.incCommandCount( PlayerCommand::CHANGE_VIEW );
            M_worldmodel.setViewMode( com.width(), com.quality() );
            M_impl->see_state_.setViewMode( com.width(), com.quality() );
        }
        return;
    }

    // playmode is not play_on
    // --> try adjustment. view mode is changed to (Narrow, Low)
    if ( world().self().viewWidth() != ViewWidth::NARROW
         && world().self().viewQuality() != ViewQuality::LOW )
    {
        PlayerChangeViewCommand com( ViewWidth::NARROW, ViewQuality::LOW );
        std::ostringstream ostrm;
        com.toStr( ostrm );
        M_client->sendMessage( ostrm.str().c_str() );

        dlog.addText( Logger::SYSTEM,
                      "---- send[%s] synch_mode. prepare see sync",
                      ostrm.str().c_str() );

        M_effector.incCommandCount( PlayerCommand::CHANGE_VIEW );
        M_worldmodel.setViewMode( com.width(), com.quality() );
        M_impl->see_state_.setViewMode( com.width(), com.quality() );

        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << "  prepare see synch" << std::endl;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerAgent::outputDebug()
{
    dlog.flush();

    if ( config().debugConnect()
         || config().debugWrite() )
    {
        // set say message
        if ( ! effector().getSayMessage().empty() )
        {
            M_debug_client.addMessage( "Say[%s]",
                                       effector().getSayMessage().c_str() );
        }

        // set defense line
        M_debug_client.addLine( Vector2D( world().defenseLineX(),
                                          world().self().pos().y - 2.0 ),
                                Vector2D( world().defenseLineX(),
                                          world().self().pos().y + 2.0 ) );
        // set offside line
        M_debug_client.addLine( Vector2D( world().offsideLineX(),
                                          world().self().pos().y - 15.0 ),
                                Vector2D( world().offsideLineX(),
                                          world().self().pos().y + 15.0 ) );
        // compose worldmodel & some debug message
        // send to debug server or write to disc
        M_debug_client.writeAll( world() );
    }
    else
    {
        M_debug_client.clear();
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
PlayerAgent::doKick( const double & power,
                     const AngleDeg & rel_dir )
{
    if ( ! world().self().isKickable() )
    {
        dlog.addText( Logger::ACTION,
                      "agent->doKick. but not kickable" );
        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << " doKick(). but not kickable" << std::endl;
        return false;
    }
    if ( world().self().isFreezed() )
    {
        dlog.addText( Logger::ACTION,
                      "agent->doKick. but in tackle expire period  %d",
                      world().self().tackleExpires() );
        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << " Now Tackle expire period" << std::endl;
        return false;
    }

    M_effector.setKick( power, rel_dir );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
PlayerAgent::doTurn( const AngleDeg & moment )
{
    if ( world().self().isFreezed() )
    {
        dlog.addText( Logger::ACTION,
                      "agent->doTurn. but in tackle expire period  %d",
                      world().self().tackleExpires() );
        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << " Now Tackle expire period" << std::endl;
        return false;
    }

    M_effector.setTurn( moment );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
PlayerAgent::doDash( const double & power )
{
    if ( world().self().isFreezed() )
    {
        dlog.addText( Logger::ACTION,
                      "agent->doDash. but in tackle expire period  %d",
                      world().self().tackleExpires() );
        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << " Now Tackle expire period" << std::endl;
        return false;
    }

    M_effector.setDash( power );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
PlayerAgent::doMove( const double & x,
                     const double & y )
{
    if ( world().self().isFreezed() )
    {
        dlog.addText( Logger::ACTION,
                      "agent->doMove. but in tackle expire period  %d",
                      world().self().tackleExpires() );
        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << " Now Tackle expire period" << std::endl;
        return false;
    }

    // check if I am movable
    if ( ! ( world().gameMode().type() == GameMode::BeforeKickOff
             || world().gameMode().type() == GameMode::AfterGoal_
             || ( world().self().goalie()
                  && world().gameMode().type() == GameMode::GoalieCatch_
                  && world().gameMode().side() == world().ourSide() )
             )
         )
    {
        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << " Can move only in before kickoff mode (or after goalie catch)"
                  << std::endl;
        dlog.addText( Logger::ACTION,
                      "agent->doMove. cannot move to (%.1f %.1f)",
                      x, y );
        return false;
    }

    M_effector.setMove( x, y );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
PlayerAgent::doCatch()
{
    if ( world().self().isFreezed() )
    {
        dlog.addText( Logger::ACTION,
                      "agent->doCatch. refused. tackle expire period  %d",
                      world().self().tackleExpires() );
        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << " Now Tackle expire period" << std::endl;
        return false;
    }

    if ( ! world().self().goalie() )
    {
        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << " Only goalies can catch" << std::endl;
        dlog.addText( Logger::ACTION,
                      "agent->doCatch. only goalie can catch" );
        return false;
    }

    if ( world().gameMode().type() != GameMode::PlayOn
         && world().gameMode().type() != GameMode::PenaltyTaken_ )
    {
        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << " not play_on mode, cannot catch"
                  << std::endl;
        dlog.addText( Logger::ACTION,
                      "agent->doCatch. playmode is not play_on" );
        return false;
    }

    if ( ! world().ball().rposValid() )
    {
        std::cerr << world().teamName() << ": "
                  << world().self().unum() << ' '
                  << world().time()
                  << " doCatch: ball is unknown." << std::endl;
        dlog.addText( Logger::ACTION,
                      "Effector::setCatch. ball is unknown. rpos conf count = %d",
                      world().ball().rposCount() );
        return false;
    }

    M_effector.setCatch();
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
PlayerAgent::doTackle( const double & power_or_dir )
{
    if ( world().self().isFreezed() )
    {
        dlog.addText( Logger::ACTION,
                      "agent->doTackle. refused. tackle expire period  %d",
                      world().self().tackleExpires() );
        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << " Now Tackle expire period" << std::endl;
        return false;
    }

    M_effector.setTackle( power_or_dir );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
PlayerAgent::doTurnNeck( const AngleDeg & moment )
{
    M_effector.setTurnNeck( moment );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
PlayerAgent::doChangeView( const ViewWidth & width )
{
    if ( M_impl->see_state_.isSynch() )
    {
        if ( ! M_impl->see_state_.canChangeViewTo( width,
                                                   world().time() ) )
        {
            dlog.addText( Logger::ACTION,
                          "agent->doChangeView. width(%d) will break see synch... ",
                          width.type() );
            return false;
        }
    }
    else
    {
        if ( world().gameMode().type() != GameMode::PlayOn )
        {
            dlog.addText( Logger::ACTION,
                          "agent->doChangeView. no synch. not play on."
                          " should try to adjust. " );
            return false;
        }
    }


    if ( width == world().self().viewWidth() )
    {
        dlog.addText( Logger::ACTION,
                      "agent->doChangeView. already same view mode %d",
                      width.type() );
        return false;
    }

    M_effector.setChangeView( width );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
// bool
// PlayerAgent::doSay( const std::string & msg )
// {
//     if ( ! config().useCommunication() )
//     {
//         dlog.addText( Logger::ACTION,
//                       "agent->doSay. communication is not allowed" );
//         return false;
//     }

//     // check message length
//     if ( static_cast< int >( msg.length() )
//          > ServerParam::i().playerSayMsgSize() )
//     {
//         std::cerr << world().teamName() << ' '
//                   << world().self().unum() << ": "
//                   << world().time()
//                   << "  say message too long [" << msg << "]="
//                   << msg.length() << std::endl;
//         dlog.addText( Logger::ACTION,
//                       "agent->doSay. message too long. length= %d",
//                       msg.length() );
//         return false;
//     }

//     M_effector.setSay( msg, config().version() );
//     return true;
// }

/*-------------------------------------------------------------------*/
/*!

*/
bool
PlayerAgent::doPointto( const double & x,
                        const double & y )
{
    if ( world().self().armMovable() > 0 )
    {
        dlog.addText( Logger::ACTION,
                      "agent->doPointto. now pointing and cannot move arm." );
        return false;
    }

    if ( ! world().self().posValid() )
    {
        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << " doPointto : invalid localization" << std::endl;
        dlog.addText( Logger::ACTION,
                      "Effector::setPointto. invalid self localization..." );
        return false;
    }

    M_effector.setPointto( x, y );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
PlayerAgent::doPointtoOff()
{
    if ( world().self().armMovable() > 0 )
    {
        dlog.addText( Logger::ACTION,
                      "agent->doPointtoOff. now pointing and cannot move arm." );
        return false;
    }

    M_effector.setPointtoOff();
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
PlayerAgent::doAttentionto( SideID side,
                            const int unum )
{
    if ( side == NEUTRAL )
    {
        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << "  attentionto. invalid side " << side << std::endl;
        dlog.addText( Logger::ACTION,
                      "agent->doAttentionto. Invalid side %d",
                      side );
        return false;
    }

    if ( unum == Unum_Unknown )
    {
        return false;
    }

    if ( unum < 1 || 11 < unum )
    {
        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << "  attentionto. invalid unum " << unum << std::endl;
        dlog.addText( Logger::ACTION,
                      "agent->doAttentionto. Invalid unum %d",
                      unum );
        return false;
    }

    if ( world().ourSide() == side
         && world().self().unum() == unum )
    {
        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << "  attentionto. try to attention to itself " << std::endl;
        dlog.addText( Logger::ACTION,
                      "agent->doAttentionto. try to attention to self." );
        return false;
    }

    if ( world().self().attentiontoUnum() == unum
         && world().self().attentiontoSide() == side )
    {
        dlog.addText( Logger::ACTION,
                      "agent->doAttentionto. already attended." );
        return false;
    }

    M_effector.setAttentionto( side, unum );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
PlayerAgent::doAttentiontoOff()
{
    M_effector.setAttentiontoOff();
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerAgent::setArmAction( ArmAction * act )
{
    M_impl->arm_action_ = boost::shared_ptr< ArmAction >( act );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerAgent::setNeckAction( NeckAction * act )
{
    M_impl->neck_action_ = boost::shared_ptr< NeckAction >( act );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerAgent::setViewAction( ViewAction * act )
{
    M_impl->view_action_ = boost::shared_ptr< ViewAction >( act );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerAgent::addSayMessage( const SayMessage * message )
{
    if ( ! config().useCommunication() )
    {
        dlog.addText( Logger::ACTION,
                      "agent->addSayMessage. communication is not allowed" );
        return;
    }

    M_effector.addSayMessage( message );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
PlayerAgent::removeSayMessage( const char header )
{
    return M_effector.removeSayMessage( header );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerAgent::setIntention( SoccerIntention * intention )
{
    M_impl->intention_ = boost::shared_ptr< SoccerIntention >( intention );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
PlayerAgent::doIntention()
{
    if ( M_impl->intention_ )
    {
        if ( M_impl->intention_->finished( this ) )
        {
            M_impl->intention_.reset();
            return false;
        }

        return M_impl->intention_->execute( this );
    }

    return false;
}

}
