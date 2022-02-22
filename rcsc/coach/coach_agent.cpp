// -*-c++-*-

/*!
  \file coach_agent.cpp
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "coach_agent.h"

#include "coach_audio_sensor.h"
#include "coach_config.h"
#include "coach_command.h"
#include "global_visual_sensor.h"
#include "global_world_model.h"

#include <rcsc/common/basic_client.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include <rcsc/common/player_param.h>
#include <rcsc/common/player_type.h>
#include <rcsc/common/team_graphic.h>
#include <rcsc/common/audio_memory.h>

#include <rcsc/param/param_map.h>
#include <rcsc/param/conf_file_parser.h>
#include <rcsc/param/cmd_line_parser.h>

#include <sstream>

namespace rcsc {

///////////////////////////////////////////////////////////////////////
/*!
  \struct CoachAgentImpl
  \brief coach agent implementation
*/
struct CoachAgentImpl {

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

    //! audio sensor
    CoachAudioSensor audio_;

    /*!
      \brief initialize all members
    */
    CoachAgentImpl()
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
CoachAgentImpl::updateCurrentTime( const long & new_time,
                                   const bool by_see_global )
{
    if ( server_cycle_stopped_ )
    {
        if ( new_time != current_time_.cycle() )
        {
            dlog.addText( Logger::LEVEL_ANY,
                          "CYCLE %ld-0 --------------------"
                          " return from cycle stop",
                          new_time );
            if ( new_time - 1 != current_time_.cycle() )
            {
                std::cerr << "coach: server cycle stopped mode:"
                          << " previous server time is incorrect?? "
                          << current_time_ << " -> " << new_time
                          << std::endl;
                dlog.addText( Logger::SYSTEM,
                              "server cycle stopped mode:"
                              " previous server time is incorrect?? "
                              " (%ld, %ld) -> %ld",
                              current_time_.cycle(), current_time_.stopped(),
                              new_time );
            }

            current_time_.assign( new_time, 0 );
        }
        else
        {
            if ( by_see_global )
            {
                dlog.addText( Logger::LEVEL_ANY,
                              "CYCLE %ld-%ld --------------------"
                              " stopped time was updated by see_global",
                              current_time_.cycle(), current_time_.stopped() + 1 );
                current_time_.assign( current_time_.cycle(),
                                      current_time_.stopped() + 1 );
            }
        }
    }
    // normal case
    else
    {
        if ( current_time_.cycle() != new_time )
        {
            dlog.addText( Logger::LEVEL_ANY,
                          "CYCLE %ld-0  -------------------------------------------------",
                          new_time );
        }
        current_time_.assign( new_time, 0 );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
CoachAgentImpl::updateServerStatus()
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
CoachAgent::CoachAgent()
    : SoccerAgent()
    , M_impl( new CoachAgentImpl )
{
    boost::shared_ptr< AudioMemory > audio_memory( new AudioMemory );

    M_worldmodel.setAudioMemory( audio_memory );
}

/*-------------------------------------------------------------------*/
/*!

*/
CoachAgent::~CoachAgent()
{

}

/*-------------------------------------------------------------------*/
/*!

*/
const
GlobalVisualSensor &
CoachAgent::visualSensor() const
{
    return M_impl->visual_;
}

/*-------------------------------------------------------------------*/
/*!

*/
const
CoachAudioSensor &
CoachAgent::audioSensor() const
{
    return M_impl->audio_;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
CoachAgent::initImpl( CmdLineParser & cmd_parser )
{
    bool help = false;
    std::string coach_config_file;

    ParamMap system_param_map( "System options" );
    system_param_map.add()
        ( "help" , "", BoolSwitch( &help ), "print help message.")
        ( "coach-config", "", &coach_config_file, "specifies coach config file." );

    ParamMap coach_param_map( "Coach options" );
    M_config.createParamMap( coach_param_map );

    // analyze command line for system options
    cmd_parser.parse( system_param_map );
    if ( help )
    {
        system_param_map.printHelp( std::cout );
        coach_param_map.printHelp( std::cout );
        M_client->setServerAlive( false );
        return false;
    }

    // analyze config file for coach config options
    if ( ! coach_config_file.empty() )
    {
        ConfFileParser conf_parser( coach_config_file.c_str() );
        conf_parser.parse( coach_param_map );
    }

    // analyze command line for coach options
    cmd_parser.parse( coach_param_map );

    //if ( ! cmd_parser.invalidOptions().empty() )
    //{
    //    std::cerr << "coach: ***WARNING*** detected invalid options: ";
    //    cmd_parser.printError( std::cerr );
    //    std::cerr << std::endl;
    //}

    if ( config().debug() )
    {
        dlog.setLogFlag( &world().time(), Logger::SYSTEM, config().debugSystem() );
        dlog.setLogFlag( &world().time(), Logger::SENSOR, config().debugSensor() );
        dlog.setLogFlag( &world().time(), Logger::WORLD, config().debugWorld() );
        dlog.setLogFlag( &world().time(), Logger::ACTION, config().debugAction() );
        dlog.setLogFlag( &world().time(), Logger::INTERCEPT, config().debugIntercept() );
        dlog.setLogFlag( &world().time(), Logger::KICK, config().debugKick() );
        dlog.setLogFlag( &world().time(), Logger::DRIBBLE, config().debugDribble() );
        dlog.setLogFlag( &world().time(), Logger::PASS, config().debugPass() );
        dlog.setLogFlag( &world().time(), Logger::CROSS, config().debugCross() );
        dlog.setLogFlag( &world().time(), Logger::SHOOT, config().debugShoot() );
        dlog.setLogFlag( &world().time(), Logger::CLEAR, config().debugClear() );
        dlog.setLogFlag( &world().time(), Logger::TEAM, config().debugTeam() );
        dlog.setLogFlag( &world().time(), Logger::ROLE, config().debugRole() );
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
CoachAgent::handleStart()
{
    if ( ! M_client )
    {
        return false;
    }

    if ( config().host().empty() )
    {
        std::cerr << config().teamName()
                  << " coach: ***ERROR*** coach: server host name is empty"
                  << std::endl;
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
                  << " coach: ***ERROR*** Failed to connect." << std::endl;
        M_client->setServerAlive( false );
    }

    sendInitCommand();
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
CoachAgent::handleMessage()
{
    if ( ! M_client )
    {
        std::cerr << "CoachAgent::handleMessage(). Client is not registered."
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

    if ( M_impl->current_time_.cycle() > start_time.cycle() + 1
         && start_time.stopped() == 0
         && M_impl->current_time_.stopped() == 0 )
    {
        std::cerr << config().teamName()
                  << " coach: parser used several steps -- missed an action!  received"
                  << counter << " messages     start time=" << start_time
                  << " end time=" << M_impl->current_time_
                  << std::endl;
    }


    if ( M_impl->think_received_ )
    {
        action();
    }
#if 0
    else if ( ! ServerParam::i().synchMode() )
    {
        if ( M_impl->last_decision_time_ != M_impl->current_time_
             && M_impl->visual_.time() == M_impl->current_time_ )
        {
            action();
        }
    }
#endif
}

/*-------------------------------------------------------------------*/
/*!

*/
void
CoachAgent::handleTimeout( const int timeout_count,
                           const int waited_msec )
{
    if ( ! M_client )
    {
        std::cerr << "CoachAgent::handleTimeout(). Client is not registered."
                  << std::endl;
        return;
    }

    if ( waited_msec > config().serverWaitSeconds() * 1000 )
    {
        if ( config().useEye() )
        {
            std::cout << config().teamName()
                      << " coach: waited "
                      << waited_msec / 1000
                      << " seconds. server down??" << std::endl;
            M_client->setServerAlive( false );
            return;
        }

        if ( waited_msec > config().serverWaitSeconds() * 2 * 1000 )
        {
            std::cout << config().teamName()
                      << " coach: waited "
                      << waited_msec / 1000
                      << " seconds. server down??" << std::endl;
            M_client->setServerAlive( false );
            return;
        }

        //std::cout << config().teamName() << ": coach requests server response..."
        //<< std::endl;
        doCheckBall();
    }


    if ( M_impl->last_decision_time_ != M_impl->current_time_ )
    {
        if ( M_impl->visual_.time() == M_impl->current_time_
             || waited_msec >= 20 * ServerParam::i().slowDownFactor() )
        {
            dlog.addText( Logger::SYSTEM,
                          "----- TIMEOUT DECISION !! [%d]ms from last sensory",
                          waited_msec );
            action();
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
CoachAgent::handleExit()
{
    finalize();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
CoachAgent::addSayMessageParser( boost::shared_ptr< SayMessageParser > parser )
{
    M_impl->audio_.addParser( parser );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
CoachAgent::removeSayMessageParser( const char header )
{
    M_impl->audio_.removeParser( header );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
CoachAgent::finalize()
{
    if ( M_client->isServerAlive() )
    {
        sendByeCommand();
    }
    std::cout << config().teamName() << " coach: finished."
              << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
CoachAgent::action()
{
    if ( M_impl->last_decision_time_ != M_impl->current_time_ )
    {
        actionImpl();
        M_impl->last_decision_time_ = M_impl->current_time_;
    }

    if ( M_impl->think_received_ )
    {
        CoachDoneCommand com;
        sendCommand( com );
        M_impl->think_received_ = false;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
CoachAgent::parse( const char * msg )
{
    if ( ! std::strncmp( msg, "(see_global ", 12 ) )
    {
        analyzeSeeGlobal( msg );
    }
#if 0
    else if ( ! std::strncmp( msg, "(see ", 5 ) )
    {
        analyzeSeeGlobal( msg );
    }
#endif
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
    else if ( ! std::strncmp( msg, "(clang ", 7 ) )
    {
        analyzeCLangVer( msg );
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
        analyzeScore( msg );
    }
    else if ( ! std::strncmp( msg, "(init ", 6 ) )
    {
        analyzeInit( msg );
    }
    else if ( ! std::strncmp( msg, "(include ", 9 ) )
    {
        analyzeInclude( msg );
    }
    else
    {
        std::cerr << config().teamName()
                  << " coach: "
                  << world().time()
                  << " received unsupported Message : ["
                  << msg << "]" << std::endl;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
CoachAgent::analyzeInit( const char * msg )
{
    // "(init l ok)" | "(init r ok)"

    char side;

    if ( std::sscanf( msg, "(init %c ok)", &side ) != 1 )
    {
        M_client->setServerAlive( false );
        return;
    }

    if ( side != 'l' && side != 'r' )
    {
        std::cerr << config().teamName()
                  << " coach: "
                  << world().time()
                  << " received unexpected init message. " << msg
                  << std::endl;
        M_client->setServerAlive( false );
        return;
    }

    // inititalize member variables
    SideID side_id = ( side == 'l'
                       ? LEFT
                       : RIGHT );
    M_worldmodel.init( side_id );

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
        ostrm << config().teamName() << "-coach"
              << config().logExt();
        dlog.open( ostrm.str().c_str() );
        if ( ! dlog.isOpen() )
        {
            std::cerr << config().teamName() << " coach: "
                      << " Failed to open file [" << ostrm.str() << "]"
                      << std::endl;
        }
    }

    // send specific settings
    // if we intend to advise to the team,
    // set visual sensory
    if ( config().useEye() )
    {
        doEye( true );
    }

    if ( config().hearSay() )
    {
        M_impl->audio_.setTeamName( config().teamName() );
    }

    // set compression level
    if ( 0 < config().compression()
         && config().compression() <= 9 )
    {
        CoachCompressionCommand com( config().compression() );
        sendCommand( com );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
CoachAgent::analyzeCycle( const char * msg,
                          const bool by_see_global )
{
    char id[16];
    long cycle = 0;
    if ( std::sscanf( msg, "(%15s %ld ",
                      id, &cycle ) != 2 )
    {
        std::cerr << config().teamName()
                  << " coach:"
                  << world().time()
                  << " ***ERROR*** failed to parse time msg=["
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
CoachAgent::analyzeSeeGlobal( const char * msg )
{
    if ( ! analyzeCycle( msg, true ) )
    {
        return;
    }

    // analyze message
    M_impl->visual_.parse( msg,
                           config().version(),
                           M_impl->current_time_ );

    // update world model
    if ( M_impl->visual_.time() == M_impl->current_time_ )
    {
        M_worldmodel.updateAfterSeeGlobal( M_impl->visual_,
                                           M_impl->current_time_ );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
CoachAgent::analyzeHear( const char * msg )
{
    if ( ! analyzeCycle( msg, false ) )
    {
        return;
    }

    long cycle;
    char sender[128];

    if ( std::sscanf( msg, "(hear %ld %127s ",
                      &cycle, sender ) != 2 )
    {
        std::cerr << config().teamName()
                  << " coach: "
                  << world().time()
                  << " Error. failed to parse audio sender. ["
                  << msg
                  << std::endl;
        return;
    }

    // check sender name
    if ( ! std::strcmp( sender, "referee" ) )
    {
        analyzeHearReferee( msg );
    }
    else if ( ! std::strncmp( sender, "(p", 2 ) )
    {
        // (hear <time> (player "<teamname>" <unum>) "<message>")
        // (hear <time> (p "<teamname>" <unum>) "<message>")
        analyzeHearPlayer( msg );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
CoachAgent::analyzeHearReferee( const char * msg )
{
    long cycle;
    char mode[512];

    if ( std::sscanf( msg, "(hear %ld referee %511[^)]", &cycle, mode ) != 2 )
    {
        std::cerr << config().teamName()
                  << " coach: "
                  << world().time()
                  << " ***ERROR*** Failed to scan playmode." << msg
                  << std::endl;
        return;
    }

    if ( ! M_impl->game_mode_.update( mode, M_impl->current_time_ ) )
    {
        //std::cerr << config().teamName()
        //          << " coach: Unknown playmode string." << mode
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
CoachAgent::analyzeHearPlayer( const char * msg )
{
    // (hear <time> (player "<teamname>" <unum>) "<message>")
    // (hear <time> (p "<teamname>" <unum>) "<message>")

    if ( config().hearSay() )
    {
        M_impl->audio_.parsePlayerMessage( msg, M_impl->current_time_ );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
CoachAgent::analyzeChangePlayerType( const char * msg )
{
    // teammate: "(change_player_type <unum> <type>)\n"
    //           "(ok change_player_type <unum> <type>)\n"
    // opponent: "(change_player_type <unum>)\n"

    int unum = -1, type = -1;

    if ( std::sscanf( msg, " ( ok change_player_type %d %d ) ",
                      &unum, &type ) == 2 )
    {
        // do nothing
    }
    else if ( std::sscanf( msg, " ( change_player_type %d %d ) ",
                           &unum, &type ) == 2 )
    {
        // teammate
        M_worldmodel.setPlayerType( world().ourSide(),
                                    unum,
                                    type );
    }
    else if ( std::sscanf( msg, " ( change_player_type %d ) ",
                           &unum ) == 1 )
    {
        // opponent
        M_worldmodel.setPlayerType( world().theirSide(),
                                    unum,
                                    Hetero_Unknown );
    }
    else
    {
        std::cerr << " ***ERROR*** parse error. "
                  << msg
                  << std::endl;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
CoachAgent::analyzePlayerType( const char * msg )
{
    PlayerType player_type( ServerParam::i(),
                            msg, config().version() );
    PlayerTypeSet::instance().insert( player_type );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
CoachAgent::analyzePlayerParam( const char * msg )
{
    PlayerParam::instance().parse( msg, config().version() );
    //PlayerParam::i().print( std::cout );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
CoachAgent::analyzeServerParam( const char * msg )
{
    ServerParam::instance().parse( msg, config().version() );
    PlayerTypeSet::instance().resetDefaultType( ServerParam::i() );

    M_worldmodel.initFreeformCount();

    if ( ! ServerParam::i().synchMode()
         && ServerParam::i().slowDownFactor() > 1 )
    {
        long interval = ( config().intervalMSec()
                          * ServerParam::i().slowDownFactor() );
        M_client->setIntervalMSec( interval );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
CoachAgent::analyzeCLangVer( const char * msg )
{
    //     std::cerr << config().teamName()
    //               << " coach: "
    //               << world().time()
    //               << " recv: " << msg << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
CoachAgent::analyzeOK( const char * msg )
{
    //std::cout << M_impl->current_time_
    //<< " Coach received ok " << std::endl;

    if ( ! std::strncmp( msg, "(ok say)",
                         std::strlen( "(ok say)" ) ) )
    {
        // nothing to do
    }
    else if ( ! std::strncmp( msg, "(ok team_graphic ",
                              std::strlen( "(ok team_graphic " ) ) )
    {
        analyzeOKTeamGraphic( msg );
    }
    else if ( ! std::strncmp( msg, "(ok look ",
                              std::strlen( "(ok look " ) ) )
    {
        std::cout << config().teamName()
                  << " coach: "
                  << world().time()
                  << "recv (ok look ..." << std::endl;
    }
    else if ( ! std::strncmp( msg, "(ok check_ball ",
                              std::strlen( "(ok check_ball " ) ) )
    {
        std::cout << config().teamName()
                  << " coach: "
                  << world().time()
                  << " recv (ok check_ball ..." << std::endl;;
    }
    else if ( ! std::strncmp( msg, "(ok change_player_type ",
                              std::strlen( "(ok change_player_type " ) ) )
    {
        analyzeChangePlayerType( msg );
    }
    else if ( ! std::strncmp( msg, "(ok compression ",
                              std::strlen( "(ok compression " ) ) )
    {
        int level = 0;
        if ( std::sscanf( msg, "(ok compression %d)", &level ) == 1 )
        {
            std::cout << config().teamName()
                      << " coach: "
                      << world().time()
                      << " set compression level " << level<< std::endl;
            M_client->setCompressionLevel( level );
        }
    }
    else if ( ! std::strncmp( msg, "(ok eye ", std::strlen( "(ok eye " ) ) )
    {
        std::cout << config().teamName()
                  << " coach: "
                  << world().time()
                  << " recv " << msg << std::endl;
    }
    else if ( ! std::strncmp( msg, "(ok team_names ",
                              std::strlen( "(ok team_names " ) ) )
    {
        std::cout << config().teamName()
                  << " coach: "
                  << world().time()
                  << " recv " << msg << std::endl;
        analyzeTeamNames( msg );
    }
    else
    {
        std::cout << config().teamName()
                  << " coach: "
                  << world().time()
                  << " recv " << msg << std::endl;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
CoachAgent::analyzeOKTeamGraphic( const char * msg )
{
    // "(ok team_graphic <x> <y>)"
    // "(ok team_graphic <x> <y>)"

    int x = -1, y = -1;

    if ( std::sscanf( msg,
                      "(ok team_graphic %d %d)",
                      &x, &y ) != 2
         || x < 0
         || y < 0 )
    {
        std::cout << config().teamName()
                  << " coach: "
                  << world().time()
                  << " recv illegal message. " << msg
                  << std::endl;
        return;
    }

    M_team_graphic_ok_set.insert( TeamGraphic::Index( x, y ) );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
CoachAgent::analyzeTeamNames( const char * msg )
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
CoachAgent::analyzeScore( const char * msg )
{
    std::cerr << config().teamName()
              << " coach: "
              << world().time()
              << " recv " << msg << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
CoachAgent::analyzeError( const char * msg )
{
    std::cerr << config().teamName()
              << " coach: "
              << world().time()
              << " recv " << msg << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
CoachAgent::analyzeWarning( const char * msg )
{
    std::cerr << config().teamName()
              << " coach: "
              << world().time()
              << " recv " << msg
              << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
CoachAgent::analyzeInclude( const char * msg )
{
    std::cerr << config().teamName()
              << " coach: "
              << world().time()
              << " recv " << msg << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
CoachAgent::sendCommand( const CoachCommand & com )
{
    std::ostringstream os;
    com.toStr( os );

    std::string str = os.str();
    if ( str.length() == 0 )
    {
        return false;
    }

    return ( M_client->sendMessage( str.c_str() ) > 0 );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
CoachAgent::sendInitCommand()
{
    if ( ! M_client->isServerAlive() )
    {
        std::cerr << config().teamName()
                  << " coach: server is not alive" << std::endl;
        return;
    }

    // make command string
    bool success = true;
    if ( config().useCoachName()
         && ! config().coachName().empty() )
    {
        CoachInitCommand com( config().teamName(),
                              config().version(),
                              config().coachName() );
        success = sendCommand( com );
    }
    else
    {
        CoachInitCommand com( config().teamName(),
                              config().version() );
        success = sendCommand( com );
    }

    if ( ! success )
    {
        std::cerr << config().teamName()
                  << " coach: Failed to init coach...\nExit ..."
                  << std::endl;
        M_client->setServerAlive( false );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
CoachAgent::sendByeCommand()
{
    CoachByeCommand com;
    sendCommand( com );
    M_client->setServerAlive( false );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
CoachAgent::doCheckBall()
{
    CoachCheckBallCommand com;
    return sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
CoachAgent::doLook()
{
    CoachLookCommand com;
    return sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
CoachAgent::doTeamNames()
{
    CoachTeamNamesCommand com;
    return sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
CoachAgent::doEye( bool on )
{
    CoachEyeCommand com( on );
    return sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
CoachAgent::doChangePlayerType( const int unum,
                                const int type )
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << config().teamName()
                  << "coach: "
                  << world().time()
                  << " doChangePlayerType. Illegal player number = "
                  << unum
                  << std::endl;
        return false;
    }

    if ( type < Hetero_Default
         || PlayerParam::i().playerTypes() <= type )
    {
        std::cerr << config().teamName()
                  << " coach: "
                  << world().time()
                  << "doChangePlayerType. Illegal player type = "
                  << type
                  << std::endl;
        return false;
    }

    CoachChangePlayerTypeCommand com( unum, type );
    return sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
CoachAgent::doChangePlayerTypes( const std::vector< std::pair< int, int > > & types )
{
    if ( types.empty() )
    {
        return false;
    }

    //CoachChangePlayerTypesCommand com( types );
    //return sendCommand( com );

    bool result = true;
    for ( std::vector< std::pair< int, int > >::const_iterator it = types.begin();
          it != types.end();
          ++it )
    {
        result = doChangePlayerType( it->first, it->second );
    }

    return result;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
CoachAgent::doSayFreeform( const std::string & msg )
{
    if ( msg.empty()
         || static_cast< int >( msg.length() ) > ServerParam::i().coachSayMsgSize() )
    {
        // over the message size limitation
        std::cerr << config().teamName()
                  << " coach: "
                  << world().time()
                  << " ***WARNING** invalid free form message length = "
                  << msg.length()
                  << std::endl;
        return false;
    }

    if ( config().version() < 7.0 )
    {
        if ( world().gameMode().type() == GameMode::PlayOn )
        {
            // coach cannot send freeform while playon
            std::cerr << config().teamName()
                      << " coach: "
                      << world().time()
                      << " ***WARNING*** cannot send message while playon. "
                      << std::endl;
            return false;
        }

        // raw message string is sent.
        // (say <message>)

        M_worldmodel.incFreeformSendCount();

        CoachSayCommand com( msg );
        return sendCommand( com );
    }

    // check if coach is allowd to send the freeform message or not.
    if ( ! world().canSendFreeform() )
    {
        std::cerr << config().teamName()
                  << " coach: "
                  << world().time()
                  << " ***WARNING*** cannot send freeform now. "
                  << std::endl;
        return false;
    }

    // send clang format message
    // (say (freeform "<message>"))

    M_worldmodel.incFreeformSendCount();

    //std::ostringstream ostr;
    //ostr << "(say (freeform \"" << msg << "\"))";
    //return ( M_client->sendMessage( ostr.str().c_str() ) > 0 );

    std::string freeform_msg;
    freeform_msg.reserve( msg.length() + 32 );

    freeform_msg = "(say (freeform \"";
    freeform_msg += msg;
    freeform_msg += "\"))";

    return ( M_client->sendMessage( freeform_msg.c_str() ) > 0 );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
CoachAgent::doTeamGraphic( const int x,
                           const int y,
                           const TeamGraphic & team_graphic )
{
    static int send_count = 0;
    static GameTime send_time( -1, 0 );

    if ( send_time != M_impl->current_time_ )
    {
        send_count = 0;
    }

    send_time = M_impl->current_time_;
    ++send_count;

    if ( send_count > config().maxTeamGraphicPerCycle() ) // XXX Magic Number
    {
        return false;
    }

    TeamGraphic::Index index( x, y );
    TeamGraphic::Map::const_iterator tile = team_graphic.tiles().find( index );

    if ( tile == team_graphic.tiles().end() )
    {
        std::cerr << config().teamName()
                  << " coach: "
                  << world().time()
                  << " ***WARNING*** The xpm tile ("
                  << x << ',' << y << ") does not found in the team graphic."
                  << std::endl;
        return false;
    }

    std::ostringstream ostr;

    ostr << "(team_graphic (" << x << ' ' << y << ' ';
    tile->second->print( ostr );
    ostr << "))";

    return ( M_client->sendMessage( ostr.str().c_str() ) > 0 );
}

}
