// -*-c++-*-

/*!
  \file say_message_parser.cpp
  \brief player's say message parser Source File
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

#include "say_message_parser.h"

#include "audio_codec.h"
#include "audio_memory.h"

#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include <rcsc/game_time.h>

#include <cstring>

namespace rcsc {


/*-------------------------------------------------------------------*/
/*!

*/
BallMessageParser::BallMessageParser( boost::shared_ptr< AudioMemory > memory )
    : M_memory( memory )
{

}


/*-------------------------------------------------------------------*/
/*!

*/
int
BallMessageParser::parse( const int sender ,
                          const double & ,
                          const char * msg,
                          const GameTime & current )
{
    // format:
    //    "b<pos_vel:5>"
    // the length of message == 6

    if ( *msg != sheader() )
    {
        return 0;
    }

    if ( std::strlen( msg ) < slength() )
    {
        std::cerr << "***ERROR*** BallMessageParser::parse()"
                  << " Illegal ball message [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "BallMessageParser: Illegal ball info [%s]",
                      msg );
        return -1;
    }
    ++msg;

    Vector2D ball_pos;
    Vector2D ball_vel;

    if ( ! AudioCodec::i().decodeStr5ToPosVel( std::string( msg, slength() - 1 ),
                                               &ball_pos, &ball_vel ) )
    {
        std::cerr << "***ERROR*** BallMessageParser::parse()"
                  << " Failed to decode ball [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "BallMessageParser: Failed to decode Ball Info [%s]",
                      msg );
        return -1;
    }

    dlog.addText( Logger::SENSOR,
                  "BallMessageParser::parse() success! pos(%.1f %.1f) vel(%.1f %.1f)",
                  ball_pos.x, ball_pos.y,
                  ball_vel.x, ball_vel.y );

    M_memory->setBall( sender, ball_pos, ball_vel, current );

    return slength();
};

/*-------------------------------------------------------------------*/
/*!

*/
PassMessageParser::PassMessageParser( boost::shared_ptr< AudioMemory > memory )
    : M_memory( memory )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
int
PassMessageParser::parse( const int sender,
                          const double & ,
                          const char * msg,
                          const GameTime & current )
{
    // format:
    //    "p<unum_pos:4><pos_vel:5>"
    // the length of message == 10

    if ( *msg != sheader() )
    {
        return 0;
    }

    if ( std::strlen( msg ) < slength() )
    {
        std::cerr << "PassMessageParser::parse()"
                  << " Illegal pass pass message ["
                  << msg << "] len = " << std::strlen( msg )
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "PassMessageParser Failed to decode Pass Info [%s]",
                      msg );
        return -1;
    }

    ++msg;

    int receiver_number = 0;
    Vector2D receive_pos;

    if ( ! AudioCodec::i().decodeStr4ToUnumPos( std::string( msg, 4 ),
                                                &receiver_number,
                                                &receive_pos ) )
    {
        std::cerr << "PassMessageParser::parse()"
                  << " Failed to parse [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "PassMessageParser: Failed to decode Pass Info [%s]",
                      msg );
        return -1;
    }
    msg += 4;

    Vector2D ball_pos;
    Vector2D ball_vel;

    if ( ! AudioCodec::i().decodeStr5ToPosVel( std::string( msg, 5 ),
                                               &ball_pos, &ball_vel ) )
    {
        std::cerr << "***ERROR*** PassMessageParser::parse()"
                  << " Failed to decode ball [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "PassMessageParser: Failed to decode Ball Info [%s]",
                      msg );
        return -1;
    }
    msg += 5;

    dlog.addText( Logger::SENSOR,
                  "PassMessageParser::parse() success! receiver %d"
                  " recv_pos(%.1f %.1f)"
                  " bpos(%.1f %.1f) bvel(%.1f %.1f)",
                  receiver_number,
                  receive_pos.x, receive_pos.y,
                  ball_pos.x, ball_pos.y,
                  ball_vel.x, ball_vel.y );

    M_memory->setPass( sender, receiver_number, receive_pos, current );
    M_memory->setBall( sender, ball_pos, ball_vel, current );

    return slength();
}


/*-------------------------------------------------------------------*/
/*!

*/
InterceptMessageParser::InterceptMessageParser( boost::shared_ptr< AudioMemory > memory )
    : M_memory( memory )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
int
InterceptMessageParser::parse( const int sender,
                               const double & ,
                               const char * msg,
                               const GameTime & current )
{
    // format:
    //    "i<unum:1><cycle:1>"
    // the length of message == 3

    if ( *msg != sheader() )
    {
        return 0;
    }

    if ( std::strlen( msg ) < slength() )
    {
        std::cerr << "InterceptMessageParser::parse()"
                  << " Illegal message = [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "AudioSensor: Failed to decode intercept info [%s]",
                      msg );
        return -1;
    }
    ++msg;

    AudioCodec::CharToIntCont::const_iterator unum_it
        = AudioCodec::i().charToIntMap().find( *msg );
    if ( unum_it == AudioCodec::i().charToIntMap().end()
         || unum_it->second <= 0
         || MAX_PLAYER*2 < unum_it->second )
    {
        std::cerr << "InterceptMessageParser::parse() "
                  << " Illegal player number. message = [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "InterceptMessageParser: Failed to decode intercept info [%s]",
                      msg );
        return -1;
    }
    ++msg;

    AudioCodec::CharToIntCont::const_iterator cycle
        = AudioCodec::i().charToIntMap().find( *msg );
    if ( cycle == AudioCodec::i().charToIntMap().end() )
    {
        std::cerr << "InterceptMessageParser::parse() "
                  << " Illegal cycle. message = [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "InterceptMessageParser: Failed to decode intercept info [%s]",
                      msg );
        return -1;
    }

    dlog.addText( Logger::SENSOR,
                  "InterceptMessageParser: success! number=%d cycle=%d",
                  unum_it->second, cycle->second );

    M_memory->setIntercept( sender, unum_it->second, cycle->second, current );

    return slength();
}

/*-------------------------------------------------------------------*/
/*!

*/
GoalieMessageParser::GoalieMessageParser( boost::shared_ptr< AudioMemory > memory )
    : M_memory( memory )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
int
GoalieMessageParser::parse( const int sender,
                            const double & ,
                            const char * msg,
                            const GameTime & current )
{
    // format:
    //    "g<unum_pos:4>"
    // the length of message == 5

    if ( *msg != sheader() )
    {
        return 0;
    }

    if ( std::strlen( msg ) < slength() )
    {
        std::cerr << "GoalieMessageParser::parse()."
                  << " Illegal message [" << msg
                  << "] len = " << std::strlen( msg )
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "GoalieMessageParser: Failed to decode Goalie Info [%s]",
                      msg );
        return -1;
    }
    ++msg;

    boost::int64_t ival = 0;
    if ( ! AudioCodec::i().decodeStrToInt64( std::string( msg, slength() - 1 ),
                                             &ival ) )
    {
        std::cerr << "GoalieMessageParser::parse()"
                  << " Failed to parse [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "GoalieMessageParser: Failed to decode Goalie Info [%s]",
                      msg );
        return -1;
    }

    Vector2D goalie_pos;
    AngleDeg goalie_body;

    goalie_body = static_cast< double >( ival % 360 - 180 );
    ival /= 360;

    goalie_pos.y = ( ival % 400 ) * 0.1 - 20.0;
    ival /= 400;

    goalie_pos.x = ( ival % 160 ) * 0.1 + ( 52.5 - 16.0 );

    dlog.addText( Logger::SENSOR,
                  "GoalieMessageParser: success! goalie pos = (%.2f %.2f) body = %.1f",
                  goalie_pos.x, goalie_pos.y, goalie_body.degree() );

    M_memory->setOpponentGoalie( sender, goalie_pos, goalie_body, current );

    return slength();
}


/*-------------------------------------------------------------------*/
/*!

*/
OffsideLineMessageParser::OffsideLineMessageParser( boost::shared_ptr< AudioMemory > memory )
    : M_memory( memory )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
int
OffsideLineMessageParser::parse( const int sender,
                                 const double & ,
                                 const char * msg,
                                 const GameTime & current )
{
    // format:
    //    "o<x_rate:1>"
    // the length of message == 2

    if ( *msg != sheader() )
    {
        return 0;
    }

    if ( std::strlen( msg ) < slength() )
    {
        std::cerr << "OffsideLineMessageParser::parse()"
                  << " Illegal message [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "OffsideLineMessageParser: Failed to decode Offside Line Info [%s]",
                      msg );
        return -1;
    }
    ++msg;

    double rate = AudioCodec::i().decodeCharToPercentage( *msg );

    if ( rate == AudioCodec::ERROR_VALUE )
    {
        std::cerr << "OffsideLineMessageParser::parse()"
                  << " Failed to read offside line"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "OffsideLineMessageParser: Failed to decode Offside Line Info [%s]",
                      msg );
        return -1;
    }

    double offside_line_x = 10.0 + ( 52.0 - 10.0 ) * rate;

    dlog.addText( Logger::SENSOR,
                  "OffsideLineMessageParser: success! x=%.1f rate=%.3f",
                  offside_line_x, rate );

    M_memory->setOffsideLine( sender, offside_line_x, current );

    return slength();
}


/*-------------------------------------------------------------------*/
/*!

*/
DefenseLineMessageParser::DefenseLineMessageParser( boost::shared_ptr< AudioMemory > memory )
    : M_memory( memory )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
int
DefenseLineMessageParser::parse( const int sender,
                                 const double & ,
                                 const char * msg,
                                 const GameTime & current )
{
    // format:
    //    "d<x_rate:1>"
    // the length of message == 2

    if ( *msg != sheader() )
    {
        return 0;
    }

    if ( std::strlen( msg ) < slength() )
    {
        std::cerr << "DefenseLineMessageParser::parse()"
                  << " Illegal message [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "DefenseLineMessageParser: Failed to decode Defense Line Info [%s]",
                      msg );
        return -1;
    }
    ++msg;

    double rate = AudioCodec::i().decodeCharToPercentage( *msg );

    if ( rate == AudioCodec::ERROR_VALUE )
    {
        std::cerr << "DefenseLineMessageParser::parser()"
                  << " Failed to read offside line [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "DefenseLineMessageParser: Failed to decode Defense Line Info [%s]",
                      msg );
        return -1;
    }

    double defense_line_x = 52.0 + ( -10.0 + 52.0 ) * rate;

    dlog.addText( Logger::SENSOR,
                  "DefenseLineMessageParser::parse() success! x=%.1f rate=%.3f",
                  defense_line_x, rate );

    M_memory->setDefenseLine( sender, defense_line_x, current );

    return slength();
}


/*-------------------------------------------------------------------*/
/*!

*/
WaitRequestMessageParser::WaitRequestMessageParser( boost::shared_ptr< AudioMemory > memory )
    : M_memory( memory )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
int
WaitRequestMessageParser::parse( const int sender,
                                 const double & ,
                                 const char * msg,
                                 const GameTime & current )
{
    if ( *msg != sheader() )
    {
        return 0;
    }

    M_memory->setWaitRequest( sender, current );
    return slength();
}

/*-------------------------------------------------------------------*/
/*!

*/
PassRequestMessageParser::PassRequestMessageParser( boost::shared_ptr< AudioMemory > memory )
    : M_memory( memory )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
int
PassRequestMessageParser::parse( const int sender,
                                 const double & ,
                                 const char * msg,
                                 const GameTime & current )
{
    // format:
    //    "h<pos:3>"
    // the length of message == 4

    if ( *msg != sheader() )
    {
        return 0;
    }

    if ( std::strlen( msg ) < slength() )
    {
        std::cerr << "PassRequestMessageParser::parse()"
                  << " Illegal pass request message ["
                  << msg << "] len = " << std::strlen( msg )
                  << std::endl;
        return -1;
    }
    ++msg;

    Vector2D pos;

    if ( ! AudioCodec::i().decodeStr3ToPos( std::string( msg, slength() - 1 ),
                                            &pos ) )
    {
        std::cerr << "PassRequestMessage::parse()"
                  << " Failed to decode pass request potiiton. ["
                  << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "AudioSensor: Failed to decode hey pass potiiton" );
        return -1;
    }

    dlog.addText( Logger::SENSOR,
                  "PassRequestMessageParser: success! "
                  "sender = %d  request pos = (%.2f %.2f)",
                  sender, pos.x, pos.y );

    M_memory->setPassRequest( sender, pos, current );

    return slength();
}

/*-------------------------------------------------------------------*/
/*!

*/
StaminaMessageParser::StaminaMessageParser( boost::shared_ptr< AudioMemory > memory )
    : M_memory( memory )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
int
StaminaMessageParser::parse( const int sender,
                             const double & ,
                             const char * msg,
                             const GameTime & current )
{
    // format:
    //    "s<rate:1>"
    // the length of message == 2

    if ( *msg != sheader() )
    {
        return 0;
    }

    if ( std::strlen( msg ) < slength() )
    {
        std::cerr << "StaminaMessageParser::parse()"
                  << " Illegal message [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "StaminaMessageParser: Failed to decode Stamina Rate [%s]",
                      msg );
        return -1;
    }
    ++msg;

    double rate = AudioCodec::i().decodeCharToPercentage( *msg );
    if ( rate < 0.0 || 1.00001 < rate )
    {
        std::cerr << "StaminaMessageParser::parser()"
                  << " Failed to read stamina rate [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "StaminaMessageParser: Failed to decode Stamina Rate [%s]",
                      msg );
        return -1;
    }

    double stamina = ServerParam::i().staminaMax() * rate;

    dlog.addText( Logger::SENSOR,
                  "StaminaMessageParser::parse() success! rate=%f stamina=%.1f",
                  rate, stamina );

    M_memory->setStamina( sender, rate, current );

    return slength();
}

/*-------------------------------------------------------------------*/
/*!

*/
RecoveryMessageParser::RecoveryMessageParser( boost::shared_ptr< AudioMemory > memory )
    : M_memory( memory )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
int
RecoveryMessageParser::parse( const int sender,
                              const double & ,
                              const char * msg,
                              const GameTime & current )
{
    // format:
    //    "r<rate:1>"
    // the length of message == 2

    if ( *msg != sheader() )
    {
        return 0;
    }

    if ( std::strlen( msg ) < slength() )
    {
        std::cerr << "RecoveryMessageParser::parse()"
                  << " Illegal message [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "RecoveryMessageParser: Failed to decode Recovery Rate [%s]",
                      msg );
        return -1;
    }
    ++msg;

    double rate = AudioCodec::i().decodeCharToPercentage( *msg );
    if ( rate == AudioCodec::ERROR_VALUE )
    {
        std::cerr << "RecoveryMessageParser::parser()"
                  << " Failed to read recovery rate [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "RecoveryMessageParser: Failed to decode Recovery Rate [%s]",
                      msg );
        return -1;
    }

    double recovery
        = rate * ( ServerParam::i().recoverInit() - ServerParam::i().recoverMin() )
        + ServerParam::i().recoverMin();

    dlog.addText( Logger::SENSOR,
                  "RecoverMessageParser::parse() success! rate=%f recovery=%.3f",
                  rate, recovery );

    M_memory->setRecovery( sender, rate, current );

    return slength();
}

/*-------------------------------------------------------------------*/
/*!

*/
DribbleMessageParser::DribbleMessageParser( boost::shared_ptr< AudioMemory > memory )
    : M_memory( memory )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
int
DribbleMessageParser::parse( const int sender,
                                   const double & ,
                                   const char * msg,
                                   const GameTime & current )
{
    // format:
    //    "D<count_pos:3>"
    // the length of message == 4

    if ( *msg != sheader() )
    {
        return 0;
    }

    if ( std::strlen( msg ) < slength() )
    {
        std::cerr << "DribbleMessageParser::parse()"
                  << " Illegal message ["
                  << msg << "] len = " << std::strlen( msg )
                  << std::endl;
        return -1;
    }
    ++msg;

    boost::int64_t ival = 0;

    if ( ! AudioCodec::i().decodeStrToInt64( std::string( msg, slength() - 1 ),
                                             &ival ) )
    {
        std::cerr << "DribbleMessageParser::parse()"
                  << " Failed to parse [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "DribbleMessageParser: Failed to decode Dribble Info [%s]",
                      msg );
        return -1;
    }

    Vector2D pos;
    int count;

    count = static_cast< int >( ival % 10 ) + 1;
    ival /= 10;

    boost::int64_t div = static_cast< boost::int64_t >( std::ceil( 68.0 / 0.5 ) );
    pos.y = ( ival % div ) * 0.5 - 34.0;
    ival /= div;

    // div = static_cast< boost::int64_t >( std::ceil( 105.0 / 0.5 ) );
    // pos.x = ( ival % div ) * 0.5 - 52.5;
    pos.x = ival * 0.5 - 52.5;


    dlog.addText( Logger::SENSOR,
                  "DribbleMessageParser: success! "
                  "sender = %d  target_pos=(%.2f %.2f) count=%d",
                  sender, pos.x, pos.y, count );

    M_memory->setDribbleTarget( sender, pos, count, current );

    return slength();
}

/*-------------------------------------------------------------------*/
/*!

*/
BallGoalieMessageParser::BallGoalieMessageParser( boost::shared_ptr< AudioMemory > memory )
    : M_memory( memory )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
int
BallGoalieMessageParser::parse( const int sender,
                                const double & ,
                                const char * msg,
                                const GameTime & current )
{
    // format:
    //    "G<bpos_bvel_gpos_gbody:9>"
    // the length of message == 10

    if ( *msg != sheader() )
    {
        return 0;
    }

    if ( std::strlen( msg ) < slength() )
    {
        std::cerr << "BallGoalieMessageParser::parse()"
                  << " Illegal message ["
                  << msg << "] len = " << std::strlen( msg )
                  << std::endl;
        return -1;
    }
    ++msg;

    boost::int64_t ival = 0;
    if ( ! AudioCodec::i().decodeStrToInt64( std::string( msg, slength() - 1 ),
                                             &ival ) )
    {
        std::cerr << "BallGoalieMessageParser::parse()"
                  << " Failed to parse [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "BallGoalieMessageParser: Failed to decode Goalie Info [%s]",
                      msg );
        return -1;
    }

    // 74^9                       = 66540410775079424
    // 1050*680*60*60*160*400*360 = 59222016000000000
    // 1050*680*63*63*160*400*360 = 65292272640000000

    const double max_speed = ServerParam::i().ballSpeedMax();
    const double prec_speed = max_speed * 2.0 / 63.0;

    Vector2D ball_pos;
    Vector2D ball_vel;
    Vector2D goalie_pos;
    AngleDeg goalie_body;

    goalie_body = static_cast< double >( ival % 360 - 180 );
    ival /= 360;

    goalie_pos.y = ( ival % 400 ) * 0.1 - 20.0;
    ival /= 400;

    goalie_pos.x = ( ival % 160 ) * 0.1 + ( 52.5 - 16.0 );
    ival /= 160;

    ball_vel.y = ( ival % 63 ) * prec_speed - max_speed;
    ival /= 63;

    ball_vel.x = ( ival % 63 ) * prec_speed - max_speed;
    ival /= 63;

    ball_pos.y = ( ival % 680 ) * 0.1 - 34.0;
    ival /= 680;

    ball_pos.x = ( ival % 1050 ) * 0.1 - 52.5;
    //ival /= 1050;

    dlog.addText( Logger::SENSOR,
                  "BallGoalieMessageParser: success! "
                  "sender = %d  bpos(%.1f %.1f) bvel(%.1f %.1f)"
                  " gpos(%.1f %.1f) gbody %.1f",
                  sender,
                  ball_pos.x, ball_pos.y,
                  ball_vel.x, ball_vel.y,
                  goalie_pos.x, goalie_pos.y,
                  goalie_body.degree() );

    M_memory->setBall( sender, ball_pos, ball_vel, current );
    M_memory->setOpponentGoalie( sender, goalie_pos, goalie_body, current );

    return slength();
}

/*-------------------------------------------------------------------*/
/*!

*/
OnePlayerMessageParser::OnePlayerMessageParser( boost::shared_ptr< AudioMemory > memory )
    : M_memory( memory )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
int
OnePlayerMessageParser::parse( const int sender,
                               const double & ,
                               const char * msg,
                                const GameTime & current )
{
    // format:
    //    "1<unum_pos_body:4>"
    // the length of message == 5

    // 21 * 105/0.4 * 68/0.4 * 360/12
    // 28113750 < 4 characters(74^4=29986576)

    if ( *msg != sheader() )
    {
        return 0;
    }

    if ( std::strlen( msg ) < slength() )
    {
        std::cerr << "OnePlayerMessageParser::parse()"
                  << " Illegal message ["
                  << msg << "] len = " << std::strlen( msg )
                  << std::endl;
        return -1;
    }
    ++msg;

    boost::int64_t ival = 0;
    if ( ! AudioCodec::i().decodeStrToInt64( std::string( msg, slength() - 1 ),
                                             &ival ) )
    {
        std::cerr << "OnePlayerMessageParser::parse()"
                  << " Failed to parse [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "OnePlayerMessageParser: Failed to decode Player Info [%s]",
                      msg );
        return -1;
    }

    int player_unum = Unum_Unknown;
    Vector2D player_pos;
    AngleDeg player_body;

    // 30=360/12
    player_body = static_cast< double >( ( ival % 30 ) * 12 - 180 );
    ival /= 30;

    // 170=68/0.4
    player_pos.y = ( ival % 170 ) * 0.4 - 34.0;
    ival /= 170;

    // 263>105/0.4=262.5
    player_pos.x = ( ival % 263 ) * 0.4 - 52.5;
    ival /= 263;

    player_unum = ( ival % 21 ) + 1;
    // ival /= 21

    dlog.addText( Logger::SENSOR,
                  "OnePlayerMessageParser: success! "
                  "unum = %d  pos(%.1f %.1f) body %.1f",
                  player_unum,
                  player_pos.x, player_pos.y,
                  player_body.degree() );

    M_memory->setPlayer( sender, player_unum, player_pos, player_body, current );

    return slength();
}

/*-------------------------------------------------------------------*/
/*!

*/
BallPlayerMessageParser::BallPlayerMessageParser( boost::shared_ptr< AudioMemory > memory )
    : M_memory( memory )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
int
BallPlayerMessageParser::parse( const int sender,
                                const double & ,
                                const char * msg,
                                const GameTime & current )
{
    // format:
    //    "P<bpos_bvel_unum_pos_body:9>"
    // the length of message == 10


    if ( *msg != sheader() )
    {
        return 0;
    }

    if ( std::strlen( msg ) < slength() )
    {
        std::cerr << "OnePlayerMessageParser::parse()"
                  << " Illegal message ["
                  << msg << "] len = " << std::strlen( msg )
                  << std::endl;
        return -1;
    }
    ++msg;

    Vector2D ball_pos;
    Vector2D ball_vel;

    if ( ! AudioCodec::i().decodeStr5ToPosVel( std::string( msg, 5 ),
                                               &ball_pos, &ball_vel ) )
    {
        std::cerr << "***ERROR*** BallPlayerMessageParser::parse()"
                  << " Failed to decode ball [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "BallPlayerMessageParser: Failed to decode Ball Info [%s]",
                      msg );
        return -1;
    }
    msg += 5;

    boost::int64_t ival = 0;
    if ( ! AudioCodec::i().decodeStrToInt64( std::string( msg, 4 ),
                                             &ival ) )
    {
        std::cerr << "BallPlayerMessageParser::parse()"
                  << " Failed to parse [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "BallPlayerMessageParser: Failed to decode Player Info [%s]",
                      msg );
        return -1;
    }

    int player_unum = Unum_Unknown;
    Vector2D player_pos;
    AngleDeg player_body;

    // 30=360/12
    player_body = static_cast< double >( ( ival % 30 ) * 12 - 180 );
    ival /= 30;

    // 170=68/0.4
    player_pos.y = ( ival % 170 ) * 0.4 - 34.0;
    ival /= 170;

    // 263>105/0.4=262.5
    player_pos.x = ( ival % 263 ) * 0.4 - 52.5;
    ival /= 263;

    player_unum = ( ival % 21 ) + 1;
    // ival /= 21

    dlog.addText( Logger::SENSOR,
                  "BallPlayerMessageParser: success! "
                  " bpos(%.1f %.1f) bvel(%.1f %.1f)"
                  " unum = %d  pos(%.1f %.1f) body %.1f",
                  ball_pos.x, ball_pos.y,
                  ball_vel.x, ball_vel.y,
                  player_unum,
                  player_pos.x, player_pos.y,
                  player_body.degree() );

    M_memory->setBall( sender, ball_pos, ball_vel, current );
    M_memory->setPlayer( sender, player_unum, player_pos, player_body, current );

    return slength();
}

} // end namespace rcsc
