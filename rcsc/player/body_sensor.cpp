// -*-c++-*-

/*!
  \file body_sensor.cpp
  \brief sense_body sensor Source File
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

#include "body_sensor.h"

#include <string>
#include <cstdio>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
BodySensor::BodySensor()
    : M_time( 0, 0 )
    , M_stamina( 4000.0 )
    , M_effort( 1.0 )
    , M_speed_mag( 0.0 )
    , M_speed_dir_relative( 0.0 )
    , M_neck_relative( 0.0 )
    , M_kick_count( 0 )
    , M_dash_count( 0 )
    , M_turn_count( 0 )
    , M_say_count( 0 )
    , M_turn_neck_count( 0 )
    , M_catch_count( 0 )
    , M_move_count( 0 )
    , M_change_view_count( 0 )
    , M_arm_movable( 0 )
    , M_arm_expires( 0 )
    , M_pointto_dist( 0.0 )
    , M_pointto_dir( 0.0 )
    , M_pointto_count( 0 )
    , M_attentionto_side( NEUTRAL )
    , M_attentionto_unum( 0 )
    , M_attentionto_count( 0 )
    , M_tackle_expires( 0 )
    , M_tackle_count( 0 )
    , M_none_collided( false )
    , M_ball_collided( false )
    , M_player_collided( false )
    , M_post_collided( false )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
void
BodySensor::parse1( const char * msg,
                    const double & version,
                    const GameTime & current )
{
    // ver. 4 & under
    // (sense_body 0 (view_mode high normal) (stamina 4000 1) (speed 0)
    //  (kick 0) (dash 0) (turn 0) (say 0))

    // ver. 5
    // (sense_body 0 (view_mode high normal) (stamina 4000 1) (speed 0) (head_angle 0)
    //  (kick 0) (dash 0) (turn 0) (say 0) (turn_neck 0))

    // ver. 6
    // (sense_body 0 (view_mode high normal) (stamina 4000 1) (speed 0 0) (head_angle 0)
    //  (kick 0) (dash 0) (turn 0) (say 0) (turn_neck 0))

    // ver. 7
    // (sense_body 0 (view_mode high normal) (stamina 4000 1) (speed 0 0) (head_angle 0)
    //  (kick 0) (dash 0) (turn 0) (say 0) (turn_neck 0) (catch 0) (move 1) (change_view 0))

    // ver. 8
    // (sense_body 66 (view_mode high normal) (stamina 3503.4 1) (speed 0.06 -79)
    //  (head_angle 89) (kick 4) (dash 20) (turn 24) (say 0) (turn_neck 28) (catch 0)
    //  (move 1) (change_view 16) (arm (movable 0) (expires 0) (target 0 0) (count 0))
    //  (focus (target none) (count 0)) (tackle (expires 0) (count 0)))

    // ver. 12
    // (sense_body 66 (view_mode high normal) (stamina 3503.4 1) (speed 0.06 -79)
    //  (head_angle 89) (kick 4) (dash 20) (turn 24) (say 0) (turn_neck 28) (catch 0)
    //  (move 1) (change_view 16) (arm (movable 0) (expires 0) (target 0 0) (count 0))
    //  (focus (target none) (count 0)) (tackle (expires 0) (count 0))
    //  (collision {none|[(ball)][player][post]}))

    //char ss[8];

    M_time = current;

    ++msg; // skip first paren
    while ( *msg != '(' ) ++msg; // skip "sense_body <time> "

    while ( *msg != ' ' ) ++msg; // skip "(view_mode"
    ++msg; // skip space
    // parse view quality
    switch ( *msg ) {
    case 'h':  // high
        M_view_quality = ViewQuality::HIGH;
        break;
    case 'l':  // low
        M_view_quality = ViewQuality::LOW;
        break;
    default:
        std::cerr << "sense_body: Unknown View Quality" << std::endl;
        break;
    }

    while ( *msg != ' ' ) ++msg; // skip view_quality string
    ++msg; // skip space

    // parse view width
    switch ( *( msg + 1 ) ) {
    case 'o':  // "normal"
        M_view_width = ViewWidth::NORMAL;
        break;
    case 'a':  // "narrow"
        M_view_width = ViewWidth::NARROW;
        break;
    case 'i':  // "wide"
        M_view_width = ViewWidth::WIDE;
        break;
    default:
        std::cerr << "sense_body: Unknown View Width" << std::endl;
        break;
    }

    char *next;

    // read stamina values
    while ( *msg != '(' ) ++msg;
    while ( *msg != ' ' ) ++msg; // skip "(stamina"
    M_stamina = std::strtod( msg, &next );
    msg = next;
    M_effort = std::strtod( msg, &next );
    msg = next;

    // read speed values
    while ( *msg != '(' ) ++msg;
    while ( *msg != ' ' ) ++msg; // skip "(speed"
    M_speed_mag = std::strtod( msg, &next );
    msg = next; // this value is quantized by 0.01
    if ( version >= 6.0 )
    {
        // Sensed speed_dir is the velocity dir relative to player's face angle
        // global_vel_dir = (sensed_speed_dir + my_global_neck_angle)
        M_speed_dir_relative = std::strtod( msg, &next );
        msg = next;
    }

    if ( version >= 5.0 )
    {
        while ( *msg != '(' ) ++msg;
        while ( *msg != ' ' ) ++msg; // skip "(head_angle"
        M_neck_relative = std::strtod( msg, &next );
        msg = next;
    }

    while ( *msg != '(' ) ++msg;
    while ( *msg != ' ' ) ++msg; // skip "(kick"
    M_kick_count = static_cast< int >( std::strtol( msg, &next, 10 ) );
    msg = next;

    while ( *msg != '(' ) ++msg;
    while ( *msg != ' ' ) ++msg; // skip "(dash"
    M_dash_count = static_cast< int >( std::strtol( msg, &next, 10 ) );
    msg = next;

    while ( *msg != '(' ) ++msg;
    while ( *msg != ' ' ) ++msg; // skip "(turn"
    M_turn_count = static_cast< int >( std::strtol( msg, &next, 10 ) );
    msg = next;

    while ( *msg != '(' ) ++msg;
    while ( *msg != ' ' ) ++msg; // skip "(say"
    M_say_count = static_cast< int >( std::strtol( msg, &next, 10 ) );
    msg = next;

    if ( version < 5.0 )
    {
        return;
    }

    while ( *msg != '(' ) ++msg;
    while ( *msg != ' ' ) ++msg; // skip "(turn_neck"
    M_turn_neck_count = static_cast< int >( std::strtol( msg, &next, 10 ) );
    msg = next;

    if ( version < 7.0 )
    {
        return;
    }

    while ( *msg != '(' ) ++msg;
    while ( *msg != ' ' ) ++msg; // skip "(catch"
    M_catch_count = static_cast< int >( std::strtol( msg, &next, 10 ) );
    msg = next;

    while ( *msg != '(' ) ++msg;
    while ( *msg != ' ' ) ++msg; // skip "(move"
    M_move_count  = static_cast< int >( std::strtol( msg, &next, 10 ) );
    msg = next;

    while ( *msg != '(' ) ++msg;
    while ( *msg != ' ' ) ++msg; // skip "(chage_view"
    M_change_view_count = static_cast< int >( std::strtol( msg, &next, 10 ) );
    msg = next;

    if ( version < 8.0 )
    {
        return;
    }

    // `(arm (movable <MOVABLE>) (expires <EXPIRES>)
    //   (target <DIST> <DIR>) (count <COUNT>))'
    while ( *msg != '(' ) ++msg;
    while ( *msg != ' ' ) ++msg; // skip "(arm"
    while ( *msg != '(' ) ++msg;
    while ( *msg != ' ' ) ++msg; // skip "(movable"
    M_arm_movable = static_cast< int >( std::strtol( msg, &next, 10 ) );
    msg = next;

    while ( *msg != '(' ) ++msg;
    while ( *msg != ' ' ) ++msg; // skip "(expires"
    M_arm_expires = static_cast< int >( std::strtol( msg, &next, 10 ) );
    msg = next;

    while ( *msg != '(' ) ++msg;
    while ( *msg != ' ' ) ++msg; // skip "(target"
    ++msg;
    M_pointto_dist = std::strtod( msg, &next );
    msg = next;
    M_pointto_dir = std::strtod( msg, &next );
    msg = next;

    while ( *msg != '(' ) ++msg;
    while ( *msg != ' ' ) ++msg; // skip "(count"
    M_pointto_count = static_cast< int >( std::strtol( msg, &next, 10 ) );
    msg = next;

    // `(focus (target <SIDE> [<UNUM>]) (count <COUNT>)'
    // <SIDE> := "none" | "l" | "r"
    while ( *msg != '(' ) ++msg;
    while ( *msg != ' ' ) ++msg; // skip "(focus"
    while ( *msg != '(' ) ++msg;
    while ( *msg != ' ' ) ++msg; // skip "(target"
    ++msg; // skip space
    if ( *msg == 'n' ) // "none"
    {
        M_attentionto_side = NEUTRAL;
        M_attentionto_unum = Unum_Unknown;
    }
    else if ( *msg == 'l' )
    {
        M_attentionto_side = LEFT;
        ++msg;
        M_attentionto_unum = static_cast< int >( std::strtol( msg, &next, 10 ) );
        msg = next;
    }
    else if ( *msg == 'r' )
    {
        M_attentionto_side = RIGHT;
        ++msg;
        M_attentionto_unum = static_cast< int >( std::strtol( msg, &next, 10 ) );
        msg = next;
    }
    else
    {
        std::cerr << "sense_body: focus ?? [" << msg << std::endl;
    }

    while ( *msg != '(' ) ++msg;
    while ( *msg != ' ' ) ++msg; // skip "(count"
    M_attentionto_count = static_cast< int >( std::strtol( msg, &next, 10 ) );
    msg = next;

    // `(tackle (expires <EXPIRES>) (count <COUNT>))'
    while ( *msg != '(' ) ++msg;
    while ( *msg != ' ' ) ++msg; // skip "(tackle"
    while ( *msg != '(' ) ++msg;
    while ( *msg != ' ' ) ++msg; // skip "(expires"
    M_tackle_expires = static_cast< int >( std::strtol( msg, &next, 10 ) );
    msg = next;

    while ( *msg != '(' ) ++msg;
    while ( *msg != ' ' ) ++msg; // skip "(count"
    M_tackle_count = static_cast< int >( std::strtol( msg, &next, 10 ) );
    msg = next;

    if ( version < 12.0 )
    {
        return;
    }

    while ( *msg != '\0' && *msg != '(' ) ++msg;

    parseCollision( msg );

}

/*-------------------------------------------------------------------*/
/*!

*/
void
BodySensor::parse2( const char * msg,
                    const double & version,
                    const GameTime & current )
{
    long sense_time;
    char vqual[8], vwidth[8];
    char attentionto_target[8];

    M_time = current;

    int n_read = 0;

    if ( version >= 8.0
         && std::sscanf( msg,
                         " (sense_body %ld (view_mode %7[^ ] %7[^)])"
                         " (stamina %lf %lf)"
                         " (speed %lf %lf) (head_angle %lf)"
                         " (kick %d) (dash %d) (turn %d) (say %d) (turn_neck %d)"
                         " (catch %d) (move %d) (change_view %d)"
                         " (arm (movable %d) (expires %d)"
                         " (target %lf %lf) (count %d))"
                         " (focus (target %[^)]) (count %d))"
                         " (tackle (expires %d) (count %d)) %n ",
                         &sense_time, vqual, vwidth,
                         &M_stamina, &M_effort,
                         &M_speed_mag, &M_speed_dir_relative,
                         &M_neck_relative,
                         &M_kick_count, &M_dash_count,
                         &M_turn_count, &M_say_count,
                         &M_turn_neck_count, &M_catch_count,
                         &M_move_count, &M_change_view_count,
                         &M_arm_movable , &M_arm_expires,
                         &M_pointto_dist, &M_pointto_dir,
                         &M_pointto_count,
                         attentionto_target , &M_attentionto_count,
                         &M_tackle_expires, &M_tackle_count,
                         &n_read ) == 25 )
    {
        if ( attentionto_target[0] == 'n' )
        {
            M_attentionto_side = NEUTRAL;
            M_attentionto_unum = Unum_Unknown;
        }
        else if ( attentionto_target[0] == 'l' )
        {
            M_attentionto_side = LEFT;
            M_attentionto_unum
                = static_cast< int >( std::strtol( attentionto_target + 2, NULL, 10 ) );
        }
        else if ( attentionto_target[0] == 'r' )
        {
            M_attentionto_side = RIGHT;
            M_attentionto_unum
                = static_cast< int >( std::strtol( attentionto_target + 2, NULL, 10 ) );
        }
        else
        {
            std::cerr << "Received unknown attentionto target ["
                      << attentionto_target << "]" << std::endl;
            M_attentionto_side = NEUTRAL;
            M_attentionto_unum = Unum_Unknown;
        }

        if ( version >= 12.0 )
        {
            parseCollision( msg + n_read );
        }
    }
    else if ( version >= 7.0
              && std::sscanf( msg,
                              "(sense_body %ld (view_mode %7[^ ] %7[^)])"
                              " (stamina %lf %lf)"
                              " (speed %lf %lf) (head_angle %lf)"
                              " (kick %d) (dash %d) (turn %d) (say %d) (turn_neck %d)"
                              " (catch %d) (move %d) (change_view %d))",
                              &sense_time, vqual, vwidth,
                              &M_stamina, &M_effort,
                              &M_speed_mag, &M_speed_dir_relative,
                              &M_neck_relative,
                              &M_kick_count, &M_dash_count,
                              &M_turn_count, &M_say_count,
                              &M_turn_neck_count, &M_catch_count,
                              &M_move_count, &M_change_view_count) == 16 )
    {

    }
    else if ( version >= 6.0
              && std::sscanf( msg,
                              "(sense_body %ld (view_mode %7[^ ] %7[^)])"
                              " (stamina %lf %lf)"
                              " (speed %lf %lf) (head_angle %lf)"
                              " (kick %d) (dash %d) (turn %d) (say %d) (turn_neck %d)",
                              &sense_time, vqual, vwidth,
                              &M_stamina, &M_effort,
                              &M_speed_mag, &M_speed_dir_relative,
                              &M_neck_relative,
                              &M_kick_count, &M_dash_count,
                              &M_turn_count, &M_say_count,
                              &M_turn_neck_count) == 13 )
    {

    }
    else if ( version >= 5.0
              && std::sscanf( msg,
                              "(sense_body %ld (view_mode %7[^ ] %7[^)])"
                              " (stamina %lf %lf)"
                              " (speed %lf) (head_angle %lf)"
                              " (kick %d) (dash %d) (turn %d) (say %d) (turn_neck %d)",
                              &sense_time, vqual, vwidth,
                              &M_stamina, &M_effort,
                              &M_speed_mag,
                              &M_neck_relative,
                              &M_kick_count, &M_dash_count,
                              &M_turn_count, &M_say_count,
                              &M_turn_neck_count) == 12 )
    {

    }
    else if ( std::sscanf( msg,
                           "(sense_body %ld (view_mode %7[^ ] %7[^)])"
                           " (stamina %lf %lf)"
                           " (speed %lf)"
                           " (kick %d) (dash %d) (turn %d) (say %d)",
                           &sense_time, vqual, vwidth,
                           &M_stamina, &M_effort,
                           &M_speed_mag,
                           &M_kick_count, &M_dash_count,
                           &M_turn_count, &M_say_count) == 10 )
    {

    }
    else
    {
        std::cerr << "parse2 error" << std::endl;
        return;
    }



    switch ( vqual[0] ) {
    case 'h':
        M_view_quality = ViewQuality::HIGH;
        break;
    case 'l':
        M_view_quality = ViewQuality::LOW;
        //std::cerr << "CAUTION!! sense_body: view quality is LOW" << std::endl;
        break;
    default:
        std::cerr << "sense_body: Unknown View Quality" << std::endl;
        break;
    }

    switch ( vwidth[1] ) {
    case 'o':  // "normal"
        M_view_width = ViewWidth::NORMAL;
        break;
    case 'a':  // "narrow"
        M_view_width = ViewWidth::NARROW;
        break;
    case 'i':  // "wide"
        M_view_width = ViewWidth::WIDE;
        break;
    default:
        std::cerr << "sense_body: Unknown View Width" << std::endl;
        break;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
BodySensor::parseCollision( const char * msg )
{
    // (collision {none|[(ball)][player][post]})

    if ( std::strncmp( "(collision ", msg, 11 ) )
    {
        std::cerr << M_time << " sense_body. illegal collision tag ["
                  << msg << "]" << std::endl;
        return;
    }

    M_none_collided = false;
    M_ball_collided = false;
    M_player_collided = false;
    M_post_collided = false;

    msg += 11;
    //while ( *msg != '\0' && *msg != '(' ) ++msg;
    //while ( *msg != '\0' && *msg != ' ' ) ++msg; // skip "(collision"
    while ( *msg == ' ' ) ++msg; // skip space

    if ( ! std::strncmp( msg, "none", 4 ) )
    {
        M_none_collided = true;
        return;
    }

    int n_read = 0;
    char name[16];
    while ( *msg != '\0' && *msg != ')' )
    {
        if ( std::sscanf( msg, " ( %15[^()] ) %n ",
                          name, &n_read ) != 1 )
        {
            break;
        }
        msg += n_read;

        if ( ! std::strcmp( "ball", name ) )
        {
            M_ball_collided = true;
        }
        else if ( ! std::strcmp( "player", name ) )
        {
            M_player_collided = true;
        }
        else if ( ! std::strcmp( "post", name ) )
        {
            M_post_collided = true;
        }
        else
        {
            std::cerr << M_time << " sense_body. Unknown collision type ["
                      << name << "]" << std::endl;
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
BodySensor::print( std::ostream & os ) const
{
    std::string side_str;
    switch ( M_attentionto_side ) {
    case LEFT:
        side_str = "left";
        break;
    case RIGHT:
        side_str = "right";
        break;
    default:
        side_str = "neutral";
        break;
    }

    os <<  "sense_body" << M_time
       << "\n view_quality: " << M_view_quality.str()
       << "\n view_width: " << M_view_width.str()
       << "\n stamina: " << M_stamina
       << "\n effort: " << M_effort
       << "\n speed-mag: " << M_speed_mag
       << "\n speed-dir: " << M_speed_dir_relative
       << "\n neck_angle: " << M_neck_relative
       << "\n"
       << "\n kick:  " << M_kick_count
       << "\n dash:  " << M_dash_count
       << "\n turn:  " << M_turn_count
       << "\n say:   " << M_say_count
       << "\n turn_neck: " << M_turn_neck_count
       << "\n catch: " << M_catch_count
       << "\n move:  " << M_move_count
       << "\n change_view: " << M_change_view_count
       << "\n attentionto: " << M_attentionto_count
       << "\n pointto: " << M_pointto_count
       << "\n tackle: " << M_tackle_count
       << "\n"
       << "\n arm-movable: " << M_arm_movable
       << "\n arm-expire:  " << M_arm_expires
       << "\n pointto-dist: " << M_pointto_dist
       << "\n pointto-dir:  " << M_pointto_dir
       << "\n"
       << "\n attentionto-side: " << side_str
       << "\n attentionto-num: " << M_attentionto_unum
       << "\n tackle-expires: " << M_tackle_expires

       << std::endl;

    return os;
}

}
