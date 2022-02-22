// -*-c++-*-

/*!
  \file logger.cpp
  \brief Logger class Source File
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

#include "logger.h"
#include <rcsc/game_time.h>

#include <string>
#include <iostream>
#include <cstdarg>

namespace rcsc {

namespace  {

//! buffer size for the log message.
#define G_BUFFER_SIZE 2048

//! temporary buffer
char g_buffer[G_BUFFER_SIZE];

//! main buffer
std::string g_str;

}

//! global variable
Logger dlog;

/*-------------------------------------------------------------------*/
/*!

*/
Logger::Logger()
    : M_time( static_cast< GameTime * >( 0 ) )
    , M_fout( NULL )
    , M_flags( 0 )
{
    g_str.reserve( 8192 * 4 );
    std::strcpy( g_buffer, "" );
}

/*-------------------------------------------------------------------*/
/*!

*/
Logger::~Logger()
{
    if ( M_fout )
    {
        this->flush();
        fclose( M_fout );
        M_fout = NULL;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
Logger::open( const char * file_path )
{
    M_fout = std::fopen( file_path, "w" );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
Logger::setLogFlag( const GameTime * time,
                    const boost::int32_t id,
                    const bool on )
{
    M_time = time;
    if ( on )
    {
        M_flags |= id;
    }
    else
    {
        M_flags &= ~id;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
Logger::print( const char * msg )
{
    if ( M_fout )
    {
        fputs( msg, M_fout );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
Logger::flush()
{
    if ( M_fout && g_str.length() > 0 )
    {
        fputs( g_str.c_str(), M_fout );
        //fwrite( g_str.c_str(), sizeof( char ), g_str.length(), M_fout );
        fflush( M_fout );
    }
    g_str.erase();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
Logger::clear()
{
    g_str.erase();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
Logger::addText( const boost::int32_t id,
                 char * msg, ... )
{
    if ( M_fout && ( id & M_flags ) && M_time )
    {
        va_list argp;
        va_start( argp, msg );
        vsnprintf( g_buffer, G_BUFFER_SIZE, msg, argp );
        va_end( argp );

        char header[32];
        std::snprintf( header, 32, "%ld %d T ",
                       M_time->cycle(),
                       id );

        g_str += header;
        g_str += g_buffer;
        g_str += '\n';
        if ( g_str.length() > 8192 * 3 )
        {
            flush();
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
Logger::addPoint( const boost::int32_t id,
                  const double & x,
                  const double & y,
                  const char * color )
{
    if ( M_fout && ( id & M_flags ) && M_time )
    {
        char msg[128];
        std::snprintf( msg, 128, "%ld %d p %.4f %.4f ",
                       M_time->cycle(),
                       id,
                       x, y );
        g_str += msg;
        if ( color )
        {
            g_str += color;
        }
        g_str += '\n';
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
Logger::addPoint( const boost::int32_t id,
                  const double & x,
                  const double & y,
                  const char r, const char g, const char b )
{
    if ( M_fout && ( id & M_flags ) && M_time )
    {
        char msg[128];
        std::snprintf( msg, 128, "%ld %d p %.4f %.4f ",
                       M_time->cycle(),
                       id,
                       x, y );
        g_str += msg;

        char col[8];
        std::snprintf( col, 8, "#%02x%02x%02x", r, g, b );
        g_str += col;

        g_str += '\n';
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
Logger::addLine( const boost::int32_t id,
                 const double & x1,
                 const double & y1,
                 const double & x2,
                 const double & y2,
                 const char * color )
{
    if ( M_fout && ( id & M_flags ) && M_time )
    {
        char msg[128];
        std::snprintf( msg, 128, "%ld %d l %.4f %.4f %.4f %.4f ",
                       M_time->cycle(),
                       id,
                       x1, y1, x2, y2 );
        g_str += msg;
        if ( color )
        {
            g_str += color;
        }
        g_str += '\n';
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
Logger::addLine( const boost::int32_t id,
                 const double & x1,
                 const double & y1,
                 const double & x2,
                 const double & y2,
                 const char r, const char g, const char b )
{
    if ( M_fout && ( id & M_flags ) && M_time )
    {
        char msg[128];
        std::snprintf( msg, 128, "%ld %d l %.4f %.4f %.4f %.4f ",
                       M_time->cycle(),
                       id,
                       x1, y1, x2, y2 );
        g_str += msg;

        char col[8];
        std::snprintf( col, 8, "#%02x%02x%02x", r, g, b );
        g_str += col;

        g_str += '\n';
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
Logger::addCircle( const boost::int32_t id,
                   const double & x,
                   const double & y,
                   const double & radius,
                   const char * color )
{
    if ( M_fout && ( id & M_flags ) && M_time )
    {
        char msg[128];
        std::snprintf( msg, 128, "%ld %d c %.4f %.4f %.4f ",
                       M_time->cycle(),
                       id,
                       x, y, radius );
        g_str += msg;
        if ( color )
        {
            g_str += color;
        }
        g_str += '\n';
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
Logger::addCircle( const boost::int32_t id,
                   const double & x,
                   const double & y,
                   const double & radius,
                   const char r, const char g, const char b )
{
    if ( M_fout && ( id & M_flags ) && M_time )
    {
        char msg[128];
        std::snprintf( msg, 128, "%ld %d c %.4f %.4f %.4f ",
                       M_time->cycle(),
                       id,
                       x, y, radius );
        g_str += msg;

        char col[8];
        std::snprintf( col, 8, "#%02x%02x%02x", r, g, b );
        g_str += col;

        g_str += '\n';
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
Logger::addTriangle( const boost::int32_t id,
                     const double & x1,
                     const double & y1,
                     const double & x2,
                     const double & y2,
                     const double & x3,
                     const double & y3,
                     const char * color )
{
    if ( M_fout && ( id & M_flags ) && M_time )
    {
        char msg[128];
        std::snprintf( msg, 128, "%ld %d t %.4f %.4f %.4f %.4f %.4f %.4f ",
                       M_time->cycle(),
                       id,
                       x1, y1, x2, y2, x3, y3 );
        g_str += msg;
        if ( color )
        {
            g_str += color;
        }
        g_str += '\n';
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
Logger::addTriangle( const boost::int32_t id,
                     const double & x1,
                     const double & y1,
                     const double & x2,
                     const double & y2,
                     const double & x3,
                     const double & y3,
                     const char r, const char g, const char b )
{
    if ( M_fout && ( id & M_flags ) && M_time )
    {
        char msg[128];
        std::snprintf( msg, 128, "%ld %d t %.4f %.4f %.4f %.4f %.4f %.4f ",
                       M_time->cycle(),
                       id,
                       x1, y1, x2, y2, x3, y3 );
        g_str += msg;

        char col[8];
        std::snprintf( col, 8, "#%02x%02x%02x", r, g, b );
        g_str += col;

        g_str += '\n';
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
Logger::addRect( const boost::int32_t id,
                 const double & left,
                 const double & top,
                 const double & length,
                 const double & width,
                 const char * color )
{
    if ( M_fout && ( id & M_flags ) && M_time )
    {
        char msg[128];
        std::snprintf( msg, 128, "%ld %d r %.4f %.4f %.4f %.4f ",
                       M_time->cycle(),
                       id,
                       left, top, length, width );
        g_str += msg;
        if ( color )
        {
            g_str += color;
        }
        g_str += '\n';
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
Logger::addRect( const boost::int32_t id,
                 const double & left,
                 const double & top,
                 const double & length,
                 const double & width,
                 const char r, const char g, const char b )
{
    if ( M_fout && ( id & M_flags ) && M_time )
    {
        char msg[128];
        std::snprintf( msg, 128, "%ld %d r %.4f %.4f %.4f %.4f ",
                       M_time->cycle(),
                       id,
                       left, top, length, width );
        g_str += msg;

        char col[8];
        std::snprintf( col, 8, "#%02x%02x%02x", r, g, b );
        g_str += col;

        g_str += '\n';
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
Logger::addMessage( const boost::int32_t id,
                    const double & x,
                    const double & y,
                    const char * msg,
                    const char * color )
{
    if ( M_fout && ( id & M_flags ) && M_time )
    {
        char header[128];
        std::snprintf( header, 128, "%ld %d m %.4f %.4f ",
                       M_time->cycle(),
                       id,
                       x, y );
        g_str += header;

        if ( color )
        {
            g_str += "(c ";
            g_str += color;
            g_str += ") ";
        }

        g_str += msg;
        g_str += '\n';
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
Logger::addMessage( const boost::int32_t id,
                    const double & x,
                    const double & y,
                    const char * msg,
                    const char r, const char g, const char b )
{
    if ( M_fout && ( id & M_flags ) && M_time )
    {
        char header[128];
        std::snprintf( header, 128, "%ld %d m %.4f %.4f ",
                       M_time->cycle(),
                       id,
                       x, y );
        g_str += header;

        char col[8];
        std::snprintf( col, 8, "#%02x%02x%02x", r, g, b );
        g_str += "(c ";
        g_str += col;
        g_str += ") ";

        g_str += msg;
        g_str += '\n';
    }
}

}
