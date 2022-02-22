// -*-c++-*-

/*!
  \file say_message_builder.cpp
  \brief player's say message builder Source File
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

#include "say_message_builder.h"

#include <rcsc/common/audio_codec.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include <rcsc/math_util.h>

#include <cstring>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
bool
BallMessage::toStr( std::string & to ) const
{
    if ( static_cast< int >( to.length() + slength() )
         > ServerParam::i().playerSayMsgSize() )
    {
        dlog.addText( Logger::SENSOR,
                      "BallMessage. over the message size : buf = %d, this = %d",
                      to.length(), slength() );
        return false;
    }

    std::string msg;
    msg.reserve( slength() - 1 );

    if ( ! AudioCodec::i().encodePosVelToStr5( M_ball_pos,
                                               M_ball_vel,
                                               msg )
         || msg.length() != slength() - 1 )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** BallMessage. "
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "BallMessage. error!"
                      " pos=(%.1f %.1f) vel=(%.1f %.1f)",
                      M_ball_pos.x, M_ball_pos.y,
                      M_ball_vel.x, M_ball_vel.y );
        return false;
    }

    dlog.addText( Logger::SENSOR,
                  "BallMessage. success!"
                  " pos=(%.1f %.1f) vel=(%.1f %.1f) -> [%s]",
                  M_ball_pos.x, M_ball_pos.y,
                  M_ball_vel.x, M_ball_vel.y,
                  msg.c_str() );

    to += header();
    to += msg;

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
PassMessage::toStr( std::string & to ) const
{
    if ( static_cast< int >( to.length() + slength() )
         > ServerParam::i().playerSayMsgSize() )
    {
        dlog.addText( Logger::SENSOR,
                      "PassMessage. over the message size : buf = %d, this = %d",
                      to.length(), slength() );
        return false;
    }

    std::string msg;
    msg.reserve( slength() - 1 );

    if ( ! AudioCodec::i().encodeUnumPosToStr4( M_receiver_unum,
                                                M_receive_point,
                                                msg ) )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** PassMessage.  receiver"
                  << std::endl;

        dlog.addText( Logger::SENSOR,
                      "PassMessage. error! receiver=%d pos=(%.1f %.1f)",
                      M_receiver_unum,
                      M_receive_point.x, M_receive_point.y );
        return false;
    }

    if ( ! AudioCodec::i().encodePosVelToStr5( M_ball_pos,
                                               M_ball_vel,
                                               msg ) )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** PassMessage. ball info"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "PassMessage. error!"
                      " ball_pos=(%.1f %.1f) vel=(%.1f %.1f)",
                      M_ball_pos.x, M_ball_pos.y,
                      M_ball_vel.x, M_ball_vel.y );
        return false;
    }

    if ( msg.length() != slength() - 1 )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** PassMessage. length"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "PassMessage. error!"
                      " illegal message length = %d [%s] ",
                      msg.length(), msg.c_str() );
        return false;
    }

    dlog.addText( Logger::SENSOR,
                  "PassMessage. success!"
                  " receiver=%d recv_pos=(%.1f %.1f)"
                  " bpos(%.1f %.1f) bvel(%.1f %.1f) -> [%s]",
                  M_receiver_unum,
                  M_receive_point.x, M_receive_point.y,
                  M_ball_pos.x, M_ball_pos.y,
                  M_ball_vel.x, M_ball_vel.y,
                  msg.c_str() );

    to += header();
    to += msg;

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
GoalieMessage::toStr( std::string & to ) const
{
    if ( static_cast< int >( to.length() + slength() )
         > ServerParam::i().playerSayMsgSize() )
    {
        dlog.addText( Logger::SENSOR,
                      "GoalieMessage. over the message size : buf = %d, this = %d",
                      to.length(), slength() );
        return false;
    }

    // 74^4 = 29986576
    // 16 * 40 * 360 / 74^4 < prec^2, prec > 0.087655223

    if ( M_goalie_pos.x < 52.5 - 16.0
         || M_goalie_pos.absY() > 20.0 )
    {
//         std::cerr << __FILE__ << ":" << __LINE__
//                   << " ***ERROR*** GoalieMessage. over the position range. "
//                   << M_goalie_pos
//                   << std::endl;
        dlog.addText( Logger::SENSOR,
                      "GoalieMessage. over the position range : (%.1f %.1f)",
                      M_goalie_pos.x, M_goalie_pos.y );
        return false;
    }

    boost::int64_t ival = 0;

    double x = min_max( 52.5 - 16.0, M_goalie_pos.x, 52.5 ) - ( 52.5 - 16.0 );
    double y = min_max( -20.0, M_goalie_pos.y, 20.0 ) + 20.0;
    double body = M_goalie_body.degree() + 180.0;

    ival += static_cast< boost::int64_t >( rint( x / 0.1 ) );

    ival *= 400;
    ival += static_cast< boost::int64_t >( rint( y / 0.1 ) );

    ival *= 360;
    ival += static_cast< boost::int64_t >( rint( body ) );

    std::string msg;
    msg.reserve( slength() - 1 );

    if ( ! AudioCodec::i().encodeInt64ToStr( ival, slength() - 1, msg )
         || msg.length() != slength() - 1 )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** GoalieMessage. "
                  << std::endl;

        dlog.addText( Logger::SENSOR,
                      "GoalieMessage. error! unum=%d pos=(%.1f %.1f) body=%.1f",
                      M_goalie_unum, M_goalie_pos.x, M_goalie_pos.y,
                      M_goalie_body.degree() );
        return false;
    }

    dlog.addText( Logger::SENSOR,
                  "GoalieMessage. success! unum=%d pos=(%.1f %.1f) -> [%s]",
                  M_goalie_unum, M_goalie_pos.x, M_goalie_pos.y,
                  msg.c_str() );

    to += header();
    to += msg;

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
OffsideLineMessage::toStr( std::string & to ) const
{
    if ( M_offside_line_x < 10.0 )
    {
        return false;
    }

    if ( static_cast< int >( to.length() + slength() )
         > ServerParam::i().playerSayMsgSize() )
    {
        dlog.addText( Logger::SENSOR,
                      "OffsideLineMessage. over the message size : buf = %d, this = %d",
                      to.length(), slength() );
        return false;
    }

    double x = min_max( 10.0, M_offside_line_x, 52.0 ) - 10.0;
    double rate = x / ( 52.0 - 10.0 );

    char ch = AudioCodec::i().encodePercentageToChar( rate );

    if ( ch == '\0' )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** OffsideLineMessage. value = "
                  << M_offside_line_x
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "OffsideLineMessage. error! real_x=%.1f, rate=%.3f",
                      M_offside_line_x, rate );
        return false;
    }

    dlog.addText( Logger::SENSOR,
                  "OffsideLineMessage. success! x=%.1f rate=%.3f [%c]",
                  M_offside_line_x, rate, ch );

    to += header();
    to += ch;

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
DefenseLineMessage::toStr( std::string & to ) const
{
    if ( M_defense_line_x > -10.0 )
    {
        return false;
    }

    if ( static_cast< int >( to.length() + slength() )
         > ServerParam::i().playerSayMsgSize() )
    {
        dlog.addText( Logger::SENSOR,
                      "DefenseLineMessage. over the message size : buf = %d, this = %d",
                      to.length(), slength() );
        return false;
    }

    double x = min_max( -52.0, M_defense_line_x, -10.0 ) + 52.0;
    double rate = x / ( -10.0 + 52.0 );

    char ch = AudioCodec::i().encodePercentageToChar( rate );

    if ( ch == '\0' )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** DefenseLineMessage. value = " << M_defense_line_x
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "DefenseLineMessage. error! x=%.1f, rate=%.3f",
                      M_defense_line_x, rate );
        return false;
    }

    dlog.addText( Logger::SENSOR,
                  "DefenseLineMessage. success! x=%.1f rate=%.3f -> [%c]",
                  M_defense_line_x, rate, ch );

    to += header();
    to += ch;

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
WaitRequestMessage::toStr( std::string & to ) const
{
    if ( static_cast< int >( to.length() + slength() )
         > ServerParam::i().playerSayMsgSize() )
    {
        dlog.addText( Logger::SENSOR,
                      "WaitRequestMessage. over the message size : buf = %d, this = %d",
                      to.length(), slength() );
        return false;
    }

    dlog.addText( Logger::SENSOR,
                  "WaitRequestMessage. success! [w]" );

    to += header();

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
InterceptMessage::toStr( std::string & to ) const
{
    if ( static_cast< int >( to.length() + slength() )
         > ServerParam::i().playerSayMsgSize() )
    {
        dlog.addText( Logger::SENSOR,
                      "InterceptMessage. over the message size : buf = %d, this = %d",
                      to.length(), slength() );
        return false;
    }

    try
    {
        int unum = ( M_our ? M_unum : M_unum + MAX_PLAYER );

        char unum_ch = AudioCodec::i().intToCharMap().at( unum );
        char cycle_ch = AudioCodec::i().intToCharMap().at( M_cycle );

        to += header();
        to += unum_ch;
        to += cycle_ch;

        dlog.addText( Logger::SENSOR,
                      "InterceptMessage. success! %s unum = %d, cycle = %d -> [%c%c]",
                      M_our ? "our" : "opp",
                      M_unum, M_cycle, unum_ch, cycle_ch );

    }
    catch ( ... )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** InterceptMessage."
                  << " Exception caught! Failed to encode cycle = "
                  << M_cycle
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "InterceptMessage. error! unum = %d, cycle = %d",
                      M_unum, M_cycle );
        return false;
    }


    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
PassRequestMessage::toStr( std::string & to ) const
{
    if ( static_cast< int >( to.length() + slength() )
         > ServerParam::i().playerSayMsgSize() )
    {
        dlog.addText( Logger::SENSOR,
                      "PassRequestMessage. over the message size : buf = %d, this = %d",
                      to.length(), slength() );
        return false;
    }

    std::string msg;
    msg.reserve( slength() - 1 );

    if ( ! AudioCodec::i().encodePosToStr3( M_target_point, msg )
         || msg.length() != slength() - 1 )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** PassRequestMessage. "
                  << "Failed to encode a pass request message. dash_target="
                  << M_target_point
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "PassRequestMessage. error!. dash_target=(%.1f %.1f)",
                      M_target_point.x, M_target_point.y );
        return false;
    }

    dlog.addText( Logger::SENSOR,
                  "PassRequestMessage. success!. dash_target=(%.1f %.1f) -> [%s]",
                  M_target_point.x, M_target_point.y,
                  msg.c_str() );

    to += header();
    to += msg;

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
StaminaMessage::toStr( std::string & to ) const
{
    if ( static_cast< int >( to.length() + slength() )
         > ServerParam::i().playerSayMsgSize() )
    {
        dlog.addText( Logger::SENSOR,
                      "StaminaMessage. over the message size : buf = %d, this = %d",
                      to.length(), slength() );
        return false;
    }

    double rate = M_stamina / ServerParam::i().staminaMax();
    char ch = AudioCodec::i().encodePercentageToChar( rate );

    if ( ch == '\0' )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** Say_Stamina. value = " << M_stamina
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "StaminaMessage. error! value= %.1f",
                      M_stamina );
        return false;
    }

    dlog.addText( Logger::SENSOR,
                  "StaminaMessage. success! value= %.1f",
                  M_stamina );

    to += header();
    to += ch;

    return false;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
RecoveryMessage::toStr( std::string & to ) const
{
    if ( static_cast< int >( to.length() + slength() )
         > ServerParam::i().playerSayMsgSize() )
    {
        dlog.addText( Logger::SENSOR,
                      "RecoveryMessage. over the message size : buf = %d, this = %d",
                      to.length(), slength() );
        return false;
    }

    double rate
        = ( M_recovery - ServerParam::i().recoverMin() )
        / ( ServerParam::i().recoverInit() - ServerParam::i().recoverMin() );

    char ch = AudioCodec::i().encodePercentageToChar( rate );

    if ( ch == '\0' )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** RecoveryMessage. value = " << M_recovery
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "RecoveryMessage: error!. value = %.1f. rate = %.3f",
                      M_recovery, rate );
        return false;
    }

    dlog.addText( Logger::SENSOR,
                  "RecoveryMessage: success!. value = %.1f. rate = %.3f",
                  M_recovery, rate );

    to += header();
    to += ch;

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
DribbleMessage::toStr( std::string & to ) const
{
    if ( static_cast< int >( to.length() + slength() )
         > ServerParam::i().playerSayMsgSize() )
    {
        dlog.addText( Logger::SENSOR,
                      "DribbleMessage. over the message size : buf = %d, this = %d",
                      to.length(), slength() );
        return false;
    }

    // 74^3 = 405224
    // 10 * 105.0/prec * 68.0/prec < 74^3
    // 10 * 105.0 * 68.0 / 74^3 < prec^2
    // prec > 0.419760459

    boost::int64_t ival = 0;

    double x = min_max( -52.5, M_target_point.x, 52.5 ) + 52.5;
    double y = min_max( -34.0, M_target_point.y, 34.0 ) + 34.0;
    boost::int64_t count = min_max( 1, M_queue_count, 10 );

    ival += static_cast< boost::int64_t >( rint( x / 0.5 ) );

    ival *= static_cast< boost::int64_t >( std::ceil( 68.0 / 0.5 ) );
    ival += static_cast< boost::int64_t >( rint( y / 0.5 ) );

    ival *= 10;
    ival += count - 1;

    std::string msg;
    msg.reserve( slength() - 1 );

    if ( ! AudioCodec::i().encodeInt64ToStr( ival, slength() - 1, msg )
         || msg.length() != slength() - 1 )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** DribbleMessage. target=" << M_target_point
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "DribbleMessage. error!. pos=(%.1f %.1f) count=%d,"
                      " message_length=%d",
                      M_target_point.x, M_target_point.y,
                      M_queue_count, msg.length() );
        return false;
    }

    dlog.addText( Logger::SENSOR,
                  "DribbleMessage. success!. pos=(%.1f %.1f) count=%d -> [%s]",
                  M_target_point.x, M_target_point.y,
                  M_queue_count,
                  msg.c_str() );

    to += header();
    to += msg;

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
BallGoalieMessage::toStr( std::string & to ) const
{
    if ( static_cast< int >( to.length() + slength() )
         > ServerParam::i().playerSayMsgSize() )
    {
        dlog.addText( Logger::SENSOR,
                      "BallGoalieMessage. over the message size : buf = %d, this = %d",
                      to.length(), slength() );
        return false;
    }

    if ( M_goalie_pos.x < 52.5 - 16.0
         || M_goalie_pos.absY() > 20.0 )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** GoalieMessage. over the position range. "
                  << M_goalie_pos
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "GoalieMessage. over the position range : (%.1f %.1f)",
                      M_goalie_pos.x, M_goalie_pos.y );
        return false;
    }

    // 74^9                       = 66540410775079424
    // 1050*680*60*60*160*400*360 = 59222016000000000
    // 1050*680*63*63*160*400*360 = 65292272640000000

    const double max_speed = ServerParam::i().ballSpeedMax();
    const double prec = max_speed * 2.0 / 63.0;

    boost::int64_t ival = 0;
    double dval = 0.0;

    dval = min_max( -52.5, M_ball_pos.x, 52.5 ) + 52.5;
    // ival *= 1050
    ival += static_cast< boost::int64_t >( rint( dval / 0.1 ) );

    dval = min_max( -34.0, M_ball_pos.y, 34.0 ) + 34.0;
    ival *= 680;
    ival += static_cast< boost::int64_t >( rint( dval / 0.1 ) );

    //dval = min_max( -2.6, M_ball_vel.x, 2.6 ) + 2.6;
    //ival *= 52;
    dval = min_max( -max_speed, M_ball_vel.x, max_speed ) + max_speed;
    ival *= 63;
    ival += static_cast< boost::int64_t >( rint( dval / prec ) );

    //dval = min_max( -2.6, M_ball_vel.y, 2.6 ) + 2.6;
    //ival *= 52;
    dval = min_max( -max_speed, M_ball_vel.y, max_speed ) + max_speed;
    ival *= 63;
    ival += static_cast< boost::int64_t >( rint( dval / prec ) );

    dval = min_max( 52.5 - 16.0, M_goalie_pos.x, 52.5 ) - ( 52.5 - 16.0 );
    ival *= 160;
    ival += static_cast< boost::int64_t >( rint( dval / 0.1 ) );

    dval = min_max( -20.0, M_goalie_pos.y, 20.0 ) + 20.0;
    ival *= 400;
    ival += static_cast< boost::int64_t >( rint( dval / 0.1 ) );

    dval = M_goalie_body.degree() + 180.0;
    ival *= 360;
    ival += static_cast< boost::int64_t >( rint( dval ) );

    std::string msg;
    msg.reserve( slength() - 1 );

    if ( ! AudioCodec::i().encodeInt64ToStr( ival, slength() - 1, msg )
         || msg.length() != slength() - 1 )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** BallGoalieMessage. "
                  << std::endl;

        dlog.addText( Logger::SENSOR,
                      "BallGoalieMessage. error!"
                      " bpos(%.1f %.1f) bvel(%.1f %.1f)"
                      " gpos=(%.1f %.1f) gbody=%.1f",
                      M_ball_pos.x, M_ball_pos.y,
                      M_ball_vel.x, M_ball_vel.y,
                      M_goalie_pos.x, M_goalie_pos.y,
                      M_goalie_body.degree() );
        return false;
    }

    dlog.addText( Logger::SENSOR,
                  "BallGoalieMessage. success!. bpos=(%.1f %.1f) bvel(%.1f %.1f)"
                  " gpos(%.1f %.1f) gbody %.1f -> [%s]",
                  M_ball_pos.x, M_ball_pos.y,
                  M_ball_vel.x, M_ball_vel.y,
                  M_goalie_pos.x, M_goalie_pos.y,
                  M_goalie_body.degree(),
                  msg.c_str() );

    to += header();
    to += msg;

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
OnePlayerMessage::toStr( std::string & to ) const
{
    if ( static_cast< int >( to.length() + slength() )
         > ServerParam::i().playerSayMsgSize() )
    {
        dlog.addText( Logger::SENSOR,
                      "OnePlayerMessage. over the message size : buf = %d, this = %d",
                      to.length(), slength() );
        return false;
    }

    if ( M_unum < 1 || 22 < M_unum )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** OnePlayerMessage. illegal unum = "
                  << M_unum
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "OnePlayerMessage. illegal unum = %d",
                      M_unum );
        return false;
    }

    boost::int64_t ival = 0;
    double dval = 0.0;

    ival += M_unum - 1;

    dval = min_max( -52.5, M_player_pos.x, 52.5 ) + 52.5;
    ival *= 263;
    ival += static_cast< boost::int64_t >( rint( dval / 0.4 ) );

    dval = min_max( -34.0, M_player_pos.y, 34.0 ) + 34.0;
    ival *= 170;
    ival += static_cast< boost::int64_t >( rint( dval / 0.4 ) );

    dval = M_player_body.degree() + 180.0;
    ival *= 30; // = 360/12
    ival += static_cast< boost::int64_t >( rint( dval / 12.0 ) );

    std::string msg;
    msg.reserve( slength() - 1 );

    if ( ! AudioCodec::i().encodeInt64ToStr( ival, slength() - 1, msg )
         || msg.length() != slength() - 1 )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** OnePlayerMessage. "
                  << std::endl;

        dlog.addText( Logger::SENSOR,
                      "OnePlayerMessage. error!"
                      " unum=%d pos=(%.1f %.1f) body=%.1f",
                      M_unum,
                      M_player_pos.x, M_player_pos.y,
                      M_player_body.degree() );
        return false;
    }

    dlog.addText( Logger::SENSOR,
                  "OnePlayerMessage. success!. unum = %d pos=(%.1f %.1f)"
                  " body=%.1f -> [%s]",
                  M_unum,
                  M_player_pos.x, M_player_pos.y,
                  M_player_body.degree(),
                  msg.c_str() );

    to += header();
    to += msg;

    return true;
}


/*-------------------------------------------------------------------*/
/*!

*/
bool
BallPlayerMessage::toStr( std::string & to ) const
{
    if ( static_cast< int >( to.length() + slength() )
         > ServerParam::i().playerSayMsgSize() )
    {
        dlog.addText( Logger::SENSOR,
                      "OnePlayerMessage. over the message size : buf = %d, this = %d",
                      to.length(), slength() );
        return false;
    }

    if ( M_unum < 1 || 22 < M_unum )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** OnePlayerMessage. illegal unum = "
                  << M_unum
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "OnePlayerMessage. illegal unum = %d",
                      M_unum );
        return false;
    }

    std::string msg;
    msg.reserve( slength() - 1 );

    if ( ! AudioCodec::i().encodePosVelToStr5( M_ball_pos,
                                               M_ball_vel,
                                               msg )
         || msg.length() != 5 )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** BallPlayerMessage. "
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "BallPlayerMessage. ball encode error!"
                      " pos=(%.1f %.1f) vel=(%.1f %.1f)",
                      M_ball_pos.x, M_ball_pos.y,
                      M_ball_vel.x, M_ball_vel.y );
        return false;
    }

    boost::int64_t ival = 0;
    double dval = 0.0;

    ival += M_unum - 1;

    dval = min_max( -52.5, M_player_pos.x, 52.5 ) + 52.5;
    ival *= 263;
    ival += static_cast< boost::int64_t >( rint( dval / 0.4 ) );

    dval = min_max( -34.0, M_player_pos.y, 34.0 ) + 34.0;
    ival *= 170;
    ival += static_cast< boost::int64_t >( rint( dval / 0.4 ) );

    dval = M_player_body.degree() + 180.0;
    ival *= 30; // = 360/12
    ival += static_cast< boost::int64_t >( rint( dval / 12.0 ) );

    if ( ! AudioCodec::i().encodeInt64ToStr( ival, slength() - 1, msg )
         || msg.length() != slength() - 1 )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** BallPlayerMessage. "
                  << std::endl;

        dlog.addText( Logger::SENSOR,
                      "BallPlayerMessage. player encode error!"
                      " unum=%d pos=(%.1f %.1f) body=%.1f",
                      M_unum,
                      M_player_pos.x, M_player_pos.y,
                      M_player_body.degree() );
        return false;
    }

    dlog.addText( Logger::SENSOR,
                  "BallPlayerMessage. success!."
                  " bpos(%.1f %.1f) bvel(%.1f %.1f)"
                  " unum=%d ppos(%.1f %.1f) pbody=%.1f"
                  " -> [%s]",
                  M_ball_pos.x, M_ball_pos.y,
                  M_ball_vel.x, M_ball_vel.y,
                  M_unum,
                  M_player_pos.x, M_player_pos.y,
                  M_player_body.degree(),
                  msg.c_str() );

    to += header();
    to += msg;

    return true;
}

}
