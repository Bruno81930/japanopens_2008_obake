// -*-c++-*-

/*!
  \file trainer_agent.cpp
  \brief basic trainer agent Source File
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

#include "trainer_agent.h"

#include "trainer_command.h"

#include <rcsc/coach/global_visual_sensor.h>

#include <rcsc/common/basic_client.h>
#include <rcsc/common/server_param.h>
#include <rcsc/common/player_param.h>
#include <rcsc/common/player_type.h>
#include <rcsc/common/audio_memory.h>

#include <rcsc/param/param_map.h>
#include <rcsc/param/conf_file_parser.h>
#include <rcsc/param/cmd_line_parser.h>

#include <sstream>

namespace rcsc {

///////////////////////////////////////////////////////////////////////
/*!
  \struct TrainerAgentImpl
  \brief trainer agent implementation
*/
struct TrainerAgentImpl {

    //! flag to check if (think) message was received or not.
    bool think_received_;

    //! flag to check if server cycle is stopped or not.
    bool server_cycle_stopped_;

    //! last action decision game time
    GameTime last_decision_time_;

    //! current game time
    GameTime current_time_;

    //! referee info
    GameMode game_mode_;

    //! visual sensor data
    GlobalVisualSensor visual_;

    /*!
      \brief initialize all members
    */
    TrainerAgentImpl()
        : think_received_( false )
        , server_cycle_stopped_( true )
        , last_decision_time_( -1, 0 )
        , current_time_( 0, 0 )
      { }

    /*!
      \brief update current time using analyzed time value
      \param new_time analyzed time value
      \param by_see_global true if called after see_global message
    */
    void updateCurrentTime( const long & new_time,
                            const bool by_see_global );

    /*!
      \brief update server game cycle status.

      This method must be called just after referee message
    */
    void updateServerStatus();
};

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgentImpl::updateCurrentTime( const long & new_time,
                                     const bool by_see_global )
{
    if ( server_cycle_stopped_ )
    {
        if ( new_time != current_time_.cycle() )
        {
            if ( new_time - 1 != current_time_.cycle() )
            {
                std::cerr << "trainer: server cycle stopped mode:"
                          << " previous server time is incorrect?? "
                          << current_time_ << " -> " << new_time
                          << std::endl;
            }

            current_time_.assign( new_time, 0 );
        }
        else
        {
            if ( by_see_global )
            {
                current_time_.assign( current_time_.cycle(),
                                      current_time_.stopped() + 1 );
            }
        }
    }
    // normal case
    else
    {
        current_time_.assign( new_time, 0 );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgentImpl::updateServerStatus()
{
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //  called just after referee.parse()

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

///////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------*/
/*!

*/
TrainerAgent::TrainerAgent()
    : SoccerAgent()
    , M_impl( new TrainerAgentImpl )
    , M_config()
{
    M_worldmodel.init( NEUTRAL );

    boost::shared_ptr< AudioMemory > audio_memory( new AudioMemory );

    M_worldmodel.setAudioMemory( audio_memory );
}

/*-------------------------------------------------------------------*/
/*!

*/
TrainerAgent::~TrainerAgent()
{

}

/*-------------------------------------------------------------------*/
/*!

*/
const
GlobalVisualSensor &
TrainerAgent::visualSensor() const
{
    return M_impl->visual_;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
TrainerAgent::initImpl( CmdLineParser & cmd_parser )
{
    bool help = false;
    std::string trainer_config_file;

    ParamMap system_param_map( "System options" );
    system_param_map.add()
        ( "help" , "", BoolSwitch( &help ), "print help message.")
        ( "trainer-config", "", &trainer_config_file, "specifies trainer config file." );

    ParamMap trainer_param_map( "Trainer options" );
    M_config.createParamMap( trainer_param_map );

    // analyze command line for system options.
    cmd_parser.parse( system_param_map );
    if ( help )
    {
        system_param_map.printHelp( std::cout );
        trainer_param_map.printHelp( std::cout );
        M_client->setServerAlive( false );
        return false;
    }

    // analyze config file for trainer config options
    if ( ! trainer_config_file.empty() )
    {
        ConfFileParser conf_parser( trainer_config_file.c_str() );
        conf_parser.parse( trainer_param_map );
    }

    // analyze command line for trainer options.
    cmd_parser.parse( trainer_param_map );

    //if ( ! cmd_parser.invalidOptions().empty() )
    //{
    //  std::cerr << "trainer: ***WARNING*** detected invalid options: ";
    //    cmd_parser.printError( std::cerr );
    //    std::cerr << std::endl;
    //}
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
TrainerAgent::handleStart()
{
    if ( ! M_client )
    {
        return false;
    }

    if ( config().host().empty() )
    {
        std::cerr << "trainer: ***ERROR*** server host name is empty"
                  << std::endl;
        M_client->setServerAlive( false );
        return false;
    }

    // just create a connection. init command is automaticcaly sent
    // by BasicClient's run() method.
    if ( ! M_client->connectTo( config().host().c_str(),
                                config().port(),
                                static_cast< long >( ServerParam::i().simulatorStep() ) ) )
    {
        std::cerr << "trainer: ***ERROR*** failed to connect." << std::endl;
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
TrainerAgent::handleMessage()
{
    if ( ! M_client )
    {
        std::cerr << "TrainerAgent::handleMessage(). Client is not registered."
                  << std::endl;
        return;
    }

    // start parse process

    while ( M_client->recvMessage() > 0 )
    {
        parse( M_client->message() );
    }

    if ( M_impl->think_received_ )
    {
        action();
    }
    else if ( ! ServerParam::i().synchMode() )
    {
        if ( M_impl->last_decision_time_ != M_impl->current_time_
             && M_impl->visual_.time() == M_impl->current_time_ )
        {
            action();
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::handleTimeout( const int timeout_count,
                             const int waited_msec )
{
    if ( ! M_client )
    {
        std::cerr << "TrainerAgent::handleTimeout(). Client is not registered."
                  << std::endl;
        return;
    }

    if ( waited_msec > config().serverWaitSeconds() * 1000 )
    {
        M_client->setServerAlive( false );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::handleExit()
{
    finalize();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::finalize()
{
    if ( M_client->isServerAlive() )
    {
        sendByeCommand();
    }
    std::cerr << "trainer: finished."<< std::endl;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::parse( const char * msg )
{

    if ( ! std::strncmp( msg, "(see_global ", strlen("(see_global ")) )
    {
        analyzeSeeGlobal( msg );
    }
    else if ( ! std::strncmp( msg, "(hear ", 6 ) )
    {
        analyzeHear( msg );
    }
    else if ( ! std::strncmp( msg, "(think)", 7 ) )
    {
        M_impl->think_received_ = true;
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
    else if ( ! std::strncmp( msg, "(server_param ", 14 ) )
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
        std::cerr << "trainer: "
                  << M_impl->current_time_
                  << " recv score " << msg << std::endl;
    }
    else if ( ! std::strncmp( msg, "(init ", 6 )
              || ! std::strncmp( msg, "(reconnect ", 11 ) )
    {
        analyzeInit( msg );
    }
    else
    {
        std::cerr << "trainer:"
                  << M_impl->current_time_
                  << " received unsupported Message : ["
                  << msg << "]" << std::endl;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::analyzeInit( const char * msg )
{
    if ( std::strncmp( msg, "(init ok)", std::strlen( "(init ok)" ) ) )
    {
        std::cerr << "trainer: Failed to init trainer.. init reply message=["
                  << msg << ']'
                  << std::endl;
        M_client->setServerAlive( false );
        return;
    }

    // initialization commands....
    if ( config().useEye() )
    {
        doEye( true );
    }

    if ( config().useEar() )
    {
        doEar( true );
    }

    // set compression level
    if ( 0 < config().compression()
         && config().compression() <= 9 )
    {
        TrainerCompressionCommand com( config().compression() );
        sendCommand( com );
    }

    M_client->setIntervalMSec( ServerParam::i().simulatorStep() );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
TrainerAgent::analyzeCycle( const char * msg,
                            const bool by_see_global )
{
    char buf[128];
    long cycle = 0;

    if ( std::sscanf( msg, "(%s %ld ",
                      buf, &cycle ) != 2
         && std::sscanf( msg, "(hear (%127[^()]) %ld ",
                         buf, &cycle ) != 2
         && std::sscanf( msg, "(hear %127s %ld ",
                         buf, &cycle ) != 2 )
    {
        std::cerr << "trainer: time parse error msg=["
                  << msg << "]"
                  << std::endl;
        return false;
    }

    M_impl->updateCurrentTime( cycle, by_see_global );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::analyzeSeeGlobal( const char * msg )
{
    if ( ! analyzeCycle( msg, true ) )
    {
        return;
    }

    M_impl->visual_.parse( msg,
                           config().version(),
                           M_impl->current_time_ );

    // update world model
    if ( M_impl->visual_.time() == M_impl->current_time_
         && world().time() != M_impl->current_time_ )
    {
        M_worldmodel.updateAfterSeeGlobal( M_impl->visual_,
                                           M_impl->current_time_ );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::analyzeHear( const char * msg )
{
    //std::cerr << "Trainer hear " << msg << std::endl;
    if ( ! analyzeCycle( msg, false ) )
    {
        return;
    }

    long cycle;
    char sender[128];

    if ( std::sscanf( msg, "(hear %ld (%127[^()]) ",
                      &cycle, sender ) != 2
         && std::sscanf( msg, "(hear %ld %s ",
                         &cycle, sender ) != 2
         && std::sscanf( msg, "(hear (%127[^()]) %ld ",
                         sender, &cycle ) != 2
         && std::sscanf( msg, "(hear %127s %ld ",
                         sender, &cycle ) != 2 )
    {
        std::cerr << "trainer: "
                  << M_impl->current_time_
                  << " *** ERROR *** failed to parse hear sender. ["
                  << msg << std::endl;
        return;
    }

    // check sender name
    if ( ! std::strcmp( sender, "referee" ) )
    {
        analyzeHearReferee( msg );
    }
    else
    {
        // player message
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::analyzeHearReferee( const char * msg )
{
    long cycle;
    char mode[512];

    if ( std::sscanf( msg, "(hear %ld referee %511[^)]", &cycle, mode ) != 2
         && std::sscanf( msg, "(hear referee %ld %511[^)]", &cycle, mode ) != 2 )
    {
        std::cerr << "trainer: "
                  << M_impl->current_time_
                  << " playmode scan error. "<< msg
                  << std::endl;
        return;
    }

    if ( ! M_impl->game_mode_.update( mode, M_impl->current_time_ ) )
    {
        //std::cerr << "trainer: "
        //          << M_impl->current_time_
        //          << " Unknown playmode string." << mode
        //          << std::endl;
        return;
    }


    M_impl->updateServerStatus();

    if ( M_impl->game_mode_.isGameEndMode() )
    {
        sendByeCommand();
        return;
    }

    M_worldmodel.updatePlayMode( M_impl->game_mode_, M_impl->current_time_ );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::analyzeHearPlayer( const char * msg )
{
    //std::cerr << "trainer: "
    //          << M_impl->current_time_
    //          << " recv player message [" << msg << "]"
    //          << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::analyzeChangePlayerType( const char * msg )
{
    // "(ok change_player_type <teamname> <unum> <type>)"

    char teamname[32];
    int unum = -1, type = -1;

    std::memset( teamname, 0, 32 );

    if ( std::sscanf( msg, " ( ok change_player_type %31[^ ] %d %d )",
                      teamname, &unum, &type ) != 3
         || unum < 0
         || type < 0 )
    {
        std::cerr << "trainer: "
                  << M_impl->current_time_
                  << " ***ERROR*** parse error. "
                  << msg
                  << std::endl;
        return;
    }

    if ( world().teamNameLeft() == teamname )
    {
        M_worldmodel.setPlayerType( LEFT, unum, type );
    }
    else if ( world().teamNameRight() == teamname )
    {
        M_worldmodel.setPlayerType( RIGHT, unum, type );
    }
    else if ( world().teamNameLeft().empty()
              && std::strlen( teamname ) != 0 )
    {
        M_worldmodel.setTeamName( LEFT, teamname );
        M_worldmodel.setPlayerType( LEFT, unum, type );
    }
    else if ( world().teamNameRight().empty()
              && std::strlen( teamname ) != 0 )
    {
        M_worldmodel.setTeamName( RIGHT, teamname );
        M_worldmodel.setPlayerType( RIGHT, unum, type );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::analyzePlayerType( const char * msg )
{
    //std::cerr << "trainer: parsed player_type " << std::endl;

    PlayerType player_type( ServerParam::i(),
                            msg, config().version() );
    PlayerTypeSet::instance().insert( player_type );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::analyzePlayerParam( const char * msg )
{
    PlayerParam::instance().parse( msg, config().version() );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::analyzeServerParam( const char * msg )
{
    ServerParam::instance().parse( msg, config().version() );
    PlayerTypeSet::instance().resetDefaultType( ServerParam::i() );

    // update alarm interval
    if ( ! ServerParam::i().synchMode()
         && ServerParam::i().slowDownFactor() > 1 )
    {
        long interval = ( ServerParam::i().simulatorStep()
                          * ServerParam::i().slowDownFactor() );
        M_client->setIntervalMSec( interval );
    }
}

/*-------------------------------------------------------------------*/
/*!
  (ok
*/
void
TrainerAgent::analyzeOK( const char * msg )
{
    if ( ! std::strncmp( msg, "(ok look ",
                         std::strlen( "(ok look " ) ) )
    {
        std::cout << "trainer: "
                  << M_impl->current_time_
                  << " recv (ok look ..." << std::endl;
    }
    else if ( ! std::strncmp( msg, "(ok check_ball ",
                              std::strlen( "(ok check_ball " ) ) )
    {
        std::cout << "trainer: "
                  << M_impl->current_time_
                  << " recv (ok check_ball ..." << std::endl;;
    }
    else if ( ! std::strncmp( msg, "(ok compression ",
                              std::strlen( "(ok compression " ) ) )
    {
        int level = 0;
        if ( std::sscanf( msg, "(ok compression %d)", &level ) == 1 )
        {
            std::cerr << "trainer: "
                      << M_impl->current_time_
                      << " set compression level " << level << std::endl;
            M_client->setCompressionLevel( level );
        }
    }
    else if ( ! std::strncmp( msg, "(ok eye ", std::strlen( "(ok eye " ) ) )
    {
        std::cout << "trainer: "
                  << M_impl->current_time_
                  << " recv " << msg << std::endl;
    }
    else if ( ! std::strncmp( msg, "(ok ear ", std::strlen( "(ok ear " ) ) )
    {
        std::cout << "trainer: "
                  << M_impl->current_time_
                  << " recv " << msg << std::endl;
    }
    else if ( ! std::strncmp( msg, "(ok team_names ",
                              std::strlen( "(ok team_names " ) ) )
    {
        std::cout << "trainer: "
                  << M_impl->current_time_
                  << " recv " << msg << std::endl;
        analyzeTeamNames( msg );
    }
    else
    {
        std::cout << "trainer: "
                  << M_impl->current_time_
                  << " recv " << msg << std::endl;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::analyzeTeamNames( const char * msg )
{
    // "(ok team_names (team l <name>)[ (team r <name>)])"

    char left[32], right[32];

    int n = std::sscanf( msg,
                         "(ok team_names (team l %31[^)]) (team r %31[^)]))",
                         left, right );
    if ( n == 2 )
    {
        M_worldmodel.setTeamName( LEFT, left );
        M_worldmodel.setTeamName( RIGHT, right );
    }
    else if ( n == 1 )
    {
        M_worldmodel.setTeamName( LEFT, left );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::analyzeError( const char * msg )
{
    std::cerr << "trainer: "
              << M_impl->current_time_
              << " recv " << msg  << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::analyzeWarning( const char * msg )
{
    std::cerr << "trainer: "
              << M_impl->current_time_
              << " recv " << msg << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
TrainerAgent::sendCommand( const TrainerCommand & com )
{
    std::ostringstream os;
    com.toStr( os );

    std::string str = os.str();
    if ( str.length() == 0 )
    {
        return false;
    }

    if ( M_client->sendMessage( str.c_str() ) > 0 )
    {
        if ( str != "(done)" )
        {
            std::cout << "OK send command [" << str << "]" << std::endl;
        }
        return true;
    }

    std::cout << "failed to send command [" << str << "]" << std::endl;
    return false;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::sendInitCommand()
{
    if ( ! M_client->isServerAlive() )
    {
        std::cerr << "trainer: server is not alive" << std::endl;
        return;
    }

    // make command string
    TrainerInitCommand com( config().version() );
    if ( ! sendCommand( com ) )
    {
        std::cerr << "trainer: Failed to init...\nExit." << std::endl;
        M_client->setServerAlive( false );
        return;
    }

    std::cerr << "trainer: send init" << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::sendByeCommand()
{
    // trainer has no bye type command
    M_client->setServerAlive( false );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
TrainerAgent::doCheckBall()
{
    TrainerCheckBallCommand com;
    return sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
TrainerAgent::doLook()
{
    TrainerLookCommand com;
    return sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
TrainerAgent::doTeamNames()
{
    TrainerTeamNamesCommand com;
    return sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
TrainerAgent::doEye( bool on )
{
    TrainerEyeCommand com( on );
    return sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
TrainerAgent::doEar( bool on )
{
    TrainerEarCommand com( on );
    return sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
TrainerAgent::doKickOff()
{
    TrainerKickOffCommand com;
    return sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
TrainerAgent::doMoveBall( const Vector2D & pos,
                          const Vector2D & vel )
{
    TrainerMoveBallCommand com( pos, vel );
    return sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
TrainerAgent::doMovePlayer( const std::string & teamname,
                            const int unum,
                            const Vector2D & pos )
{
    TrainerMovePlayerCommand com( teamname, unum, pos );
    return sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
TrainerAgent::doMovePlayer( const std::string & teamname,
                            const int unum,
                            const Vector2D & pos,
                            const AngleDeg & angle )
{
    TrainerMovePlayerCommand com( teamname, unum, pos, angle );
    return sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
TrainerAgent::doRecover()
{
    TrainerRecoverCommand com;
    return sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
TrainerAgent::doChangeMode( const PlayMode mode )
{
    TrainerChangeModeCommand com( mode );
    return sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
TrainerAgent::doChangePlayerType( const std::string & teamname,
                                  const int unum,
                                  const int type )
{
    TrainerChangePlayerTypeCommand com( teamname, unum, type );
    return sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
TrainerAgent::doSay( const std::string & msg )
{
    TrainerSayCommand com( msg );
    return sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::action()
{
    if ( M_impl->last_decision_time_ != M_impl->current_time_ )
    {
        actionImpl();
        M_impl->last_decision_time_ = M_impl->current_time_;
    }

    if ( M_impl->think_received_ )
    {
        TrainerDoneCommand com;
        sendCommand( com );
        M_impl->think_received_ = true;
    }
}

}
