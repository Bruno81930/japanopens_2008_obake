// -*-c++-*-

/*!
  \file parser_v4.cpp
  \brief rcg v4 parser Source File.
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

#include "parser_v4.h"

#include "handler.h"
#include "types.h"

#include <iostream>

namespace rcsc {
namespace rcg {

/*-------------------------------------------------------------------*/
/*!

*/
bool
ParserV4::parse( std::istream & is,
                 Handler & handler ) const
{
    // streampos must be the first point!!!
    is.seekg( 0 );

    if ( ! is.good() )
    {
        return false;
    }

    std::string line;
    line.reserve( 8192 );

    // skip header line
    if ( ! std::getline( is, line )
         || line != "ULG4" )
    {
        return false;
    }

    if ( ! handler.handleLogVersion( REC_VERSION_4 ) )
    {
        return false;
    }

    int n_line = 1;
    while ( std::getline( is, line ) )
    {
        ++n_line;
        if ( ! parseLine( n_line, line, handler ) )
        {
            return false;
        }
    }

    if ( is.eof() )
    {
        return handler.handleEOF();
    }

    return false;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
ParserV4::parseLine( const int n_line,
                     const std::string & line,
                     Handler & handler ) const
{
    char name[32];

    if ( std::sscanf( line.c_str(), " ( %s ", name ) != 1 )
    {
        std::cerr << n_line << ": Illegal line: [" << line << ']'
                  << std::endl;;
        return false;
    }

    if ( ! std::strcmp( name, "show" ) )
    {
        parseShow( n_line, line, handler );
    }
    else if ( ! std::strcmp( name, "playmode" ) )
    {
        parsePlayMode( n_line, line, handler );
    }
    else if ( ! std::strcmp( name, "team" ) )
    {
        parseTeam( n_line, line, handler );
    }
    else if ( ! std::strcmp( name, "msg" ) )
    {
        parseMsg( n_line, line, handler );
    }
    else if ( ! std::strcmp( name, "player_type" ) )
    {
        parsePlayerType( n_line, line, handler );
    }
    else if ( ! std::strcmp( name, "player_param" ) )
    {
        parsePlayerParam( n_line, line, handler );
    }
    else if ( ! std::strcmp( name, "server_param" ) )
    {
        parseServerParam( n_line, line, handler );
    }
    else
    {
        std::cerr << n_line << ": error:"
                  << " Unknown mode [" << line << ']'
                  << std::endl;;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
ParserV4::parseShow( const int n_line,
                     const std::string & line,
                     Handler & handler ) const
{
    /*
      (show <Time> <Ball> <Players>)
    */

    const char * buf = line.c_str();

    int n_read = 0;

    // time
    int time = 0;

    if ( std::sscanf( buf, " ( show %d %n ",
                      &time, &n_read ) != 1 )
    {
        std::cerr << n_line << ": error: "
                  << "Illegal time info \"" << line << "\"" << std::endl;
        return false;
    }
    buf += n_read;

    //
    // playmode
    //
    {
        int pm = 0;
        if ( std::sscanf( buf,
                          " ( pm %d ) %n ",
                          &pm, &n_read ) == 1 )
        {
            buf += n_read;
            handler.handlePlayMode( time, static_cast< PlayMode >( pm ) );
        }
    }

    //
    // team
    //
    if ( ! std::strncmp( buf, "(tm", 3 ) )
    {
        char name_l[32], name_r[32];
        int score_l = 0, score_r = 0;
        int pen_score_l = 0, pen_miss_l = 0, pen_score_r = 0, pen_miss_r = 0;

        int n = std::sscanf( buf,
                             " ( tm %31s %31s %d %d %d %d %d %d ",
                             name_l, name_r,
                             &score_l, &score_r,
                             &pen_score_l, &pen_miss_l,
                             &pen_score_r, &pen_miss_r );

        if ( n != 4 && n != 8 )
        {
            std::cerr << n_line << ": error: n=" << n << ' '
                      << "Illegal team info. \"" << line << "\"" << std::endl;;
            return false;
        }
        while ( *buf != ')' && *buf != '\0' ) ++buf;
        while ( *buf == ')' && *buf != '\0' ) ++buf;

        if ( ! std::strcmp( name_l, "null" ) ) std::memset( name_l, 0, 4 );
        if ( ! std::strcmp( name_r, "null" ) ) std::memset( name_r, 0, 4 );

        TeamT team_l( name_l, score_l, pen_score_l, pen_miss_l );
        TeamT team_r( name_r, score_r, pen_score_r, pen_miss_r );

        handler.handleTeam( time, team_l, team_r );
    }

    //
    // show main
    //

    ShowInfoT show;

    show.time_ = time;

    //
    // ball
    //

    if ( std::sscanf( buf, " ( ( b ) %f %f %f %f ) %n ",
                      &show.ball_.x_, &show.ball_.y_,
                      &show.ball_.vx_, &show.ball_.vy_,
                      &n_read ) != 4 )
    {
        std::cerr << n_line << ": error: "
                  << "Illegal ball info \"" << line << "\"" << std::endl;
        return false;
    }
    buf += n_read;

    //
    // players
    //

    for ( int i = 0; i < MAX_PLAYER * 2; ++i )
    {
        if ( *buf == ')' ) break;

        char side;
        Int16 unum = 0;

        if ( std::sscanf( buf,
                          " ( ( %c %hd ) %n ",
                          &side, &unum,
                          &n_read ) != 2 )
        {
            std::cerr << n_line << ": error: "
                      << " Illegal player " << i << " \"" << line << "\""
                      << std::endl;;
            return false;
        }
        buf += n_read;

        int idx = unum - 1;
        if ( side == 'r' ) idx += MAX_PLAYER;
        if ( idx < 0 || MAX_PLAYER*2 <= idx )
        {
            std::cerr << n_line << ": error: "
                      << " Illegal player " << i << " \"" << line << "\""
                      << std::endl;;
            return false;
        }

        PlayerT & p = show.player_[idx];

        p.side_ = side;
        p.unum_ = unum;

        if ( std::sscanf( buf,
                          " %hd %x %f %f %f %f %f %f %n ",
                          &p.type_, &p.state_,
                          &p.x_, &p.y_, &p.vx_, &p.vy_, &p.body_, &p.neck_,
                          &n_read ) != 8 )
        {
            std::cerr << n_line << ": error: "
                      << " Illegal player " << i << " \"" << line << "\""
                      << std::endl;;
            return false;
        }
        buf += n_read;

        if ( std::sscanf( buf,
                          " %f %f %n ",
                          &p.point_x_, &p.point_y_,
                          &n_read ) == 2 )
        {
            buf += n_read;
        }

        if ( std::sscanf( buf,
                          " ( v %c %f ) %n ",
                          &p.view_quality_, &p.view_width_,
                          &n_read ) != 2 )
        {
            std::cerr << n_line << ": error: "
                      << "Illegal player " << i << " view \"" << line << "\"" << std::endl;;
            return false;
        }
        buf += n_read;

        if ( std::sscanf( buf,
                          " ( s %f %f %f ) %n ",
                          &p.stamina_, &p.effort_, &p.recovery_,
                          &n_read ) != 3 )
        {
            std::cerr << n_line << ": error: "
                      << "Illegal player " << i << " stamina \"" << line << "\"" << std::endl;;
            return false;
        }
        buf += n_read;

        if ( std::sscanf( buf,
                          " ( f %c %hd ) %n ",
                          &p.focus_side_, &p.focus_unum_,
                          &n_read ) == 2 )
        {
            buf += n_read;
        }

        if ( std::sscanf( buf,
                          " ( c %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd ) ) %n ",
                          &p.kick_count_,
                          &p.dash_count_,
                          &p.turn_count_,
                          &p.catch_count_,
                          &p.move_count_,
                          &p.turn_neck_count_,
                          &p.change_view_count_,
                          &p.say_count_,
                          &p.tackle_count_,
                          &p.pointto_count_,
                          &p.attentionto_count_,
                          &n_read ) != 11 )
        {
            std::cerr << n_line << ": error: "
                      << "Illegal player " << i << " count \"" << line << "\"" << std::endl;;
            return false;
        }
        buf += n_read;

    }

    handler.handleShow( time, show );

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
ParserV4::parseMsg( const int n_line,
                    const std::string & line,
                    Handler & handler ) const
{
    /*
      (msg <Time> <Board> "<Message>")
    */
    int time = 0;
    int board = 0;
    char msg[8192];

    if ( std::sscanf( line.c_str(),
                      " ( msg %d %d \"%8191[^\"]\" ) ",
                      &time, &board, msg ) != 3 )
    {
        std::cerr << n_line << ": error: "
                  << "Illegal msg line. \"" << line << "\"" << std::endl;;
        return false;
    }

    handler.handleMsg( time, board, msg );

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
ParserV4::parsePlayMode( const int n_line,
                         const std::string & line,
                         Handler & handler ) const
{
    /*
      (playmode <Time> <Playmode>)
    */

    static const char * playmode_strings[] = PLAYMODE_STRINGS;

    int time = 0;
    char pm_string[32];

    if ( std::sscanf( line.c_str(),
                      " ( playmode %d %31[^)] ) ",
                      &time, pm_string ) != 2 )
    {
        std::cerr << n_line << ": error: "
                  << "Illegal playmode line. \"" << line << "\"" << std::endl;;
        return false;
    }

    PlayMode pm = PM_Null;
    for ( int n = 0; n < PM_MAX; ++n )
    {
        if ( ! std::strcmp( playmode_strings[n], pm_string ) )
        {
            pm = static_cast< PlayMode >( n );
            break;
        }
    }

    handler.handlePlayMode( time, pm );

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
ParserV4::parseTeam( const int n_line,
                     const std::string & line,
                     Handler & handler ) const
{
    int time = 0;
    char name_l[32], name_r[32];
    int score_l = 0, score_r = 0;
    int pen_score_l = 0, pen_miss_l = 0, pen_score_r = 0, pen_miss_r = 0;

    int n = std::sscanf( line.c_str(),
                         " ( team %d %31s %31s %d %d %d %d %d %d ",
                         &time,
                         name_l, name_r,
                         &score_l, &score_r,
                         &pen_score_l, &pen_miss_l,
                         &pen_score_r, &pen_miss_r );

    if ( n != 5 && n != 9 )
    {
        std::cerr << n_line << ": error: "
                  << "Illegal team line. \"" << line << "\"" << std::endl;;
        return false;
    }

    TeamT team_l( name_l, score_l, pen_score_l, pen_miss_l );
    TeamT team_r( name_r, score_r, pen_score_r, pen_miss_r );

    handler.handleTeam( time, team_l, team_r );

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
ParserV4::parsePlayerType( const int n_line,
                           const std::string & line,
                           Handler & handler ) const
{
    if ( ! handler.handlePlayerType( line ) )
    {
        std::cerr << n_line << ": error: "
                  << "Illegal player_type line. \"" << line << "\"" << std::endl;;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
ParserV4::parseServerParam( const int n_line,
                            const std::string & line,
                            Handler & handler ) const
{
    if ( ! handler.handleServerParam( line ) )
    {
        std::cerr << n_line << ": error: "
                  << "Illegal server_param line. \"" << line << "\"" << std::endl;;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
ParserV4::parsePlayerParam( const int n_line,
                            const std::string & line,
                            Handler & handler ) const
{
    if ( ! handler.handlePlayerParam( line ) )
    {
        std::cerr << n_line << ": error: "
                  << "Illegal player_param line. \"" << line << "\"" << std::endl;;
    }

    return true;
}

} // end of namespace
} // end of namespace
