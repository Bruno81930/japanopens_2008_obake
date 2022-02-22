// -*-c++-*-

/*!
  \file monitor_command.cpp
  \brief monitor command classes Source File
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

#include "monitor_command.h"

#include <rcsc/rcg/types.h>

#include <cmath>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
MonitorInitCommand::MonitorInitCommand( const int version  )
    : M_version( version )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
MonitorInitCommand::toStr( std::ostream & to ) const
{
    if ( M_version )
    {
        return to << "(dispinit version " << M_version << ")";
    }
    else
    {
        return to << "(dispinit)";
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
MonitorByeCommand::toStr( std::ostream & to ) const
{
    return to << "(dispbye)";
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
MonitorKickOffCommand::toStr( std::ostream & to ) const
{
    return to << "(dispstart)";
}

/*-------------------------------------------------------------------*/
/*!

*/
MonitorDropBallCommand::MonitorDropBallCommand( const double & x,
                                                const double & y )
    : M_x( x )
    , M_y( y )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
MonitorDropBallCommand::toStr( std::ostream & to ) const
{
    using namespace std;

    to << "(dispfoul "
       << static_cast< int >( rint( M_x * rcg::SHOWINFO_SCALE ) ) << " "
       << static_cast< int >( rint( M_y * rcg::SHOWINFO_SCALE ) ) << " "
       << NEUTRAL << ")";
    return to;
}

/*-------------------------------------------------------------------*/
/*!

*/
MonitorFreeKickCommand::MonitorFreeKickCommand( const double & x,
                                                const double & y,
                                                const SideID side )
    : M_x( x )
    , M_y( y )
    , M_side( side )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
MonitorFreeKickCommand::toStr( std::ostream & to ) const
{
    using namespace std;

    to << "(dispfoul "
       << static_cast< int >( rint( M_x * rcg::SHOWINFO_SCALE ) ) << " "
       << static_cast< int >( rint( M_y * rcg::SHOWINFO_SCALE ) ) << " "
       << M_side << ")";
    return to;
}

/*-------------------------------------------------------------------*/
/*!

*/
MonitorMovePlayerCommand::MonitorMovePlayerCommand( const SideID side,
                                                    const int unum,
                                                    const double & x,
                                                    const double & y,
                                                    const double & angle )
    : M_side( side )
    , M_unum( unum )
    , M_x( x )
    , M_y( y )
    , M_angle( angle )
{
    if ( M_unum < 1 || 11 < M_unum )
    {
        std::cerr << "MonitorMovePlayerCommand illegal uniform number "
                  << M_unum << std::endl;
    }

    if ( M_side != LEFT
         && M_side != RIGHT )
    {
        std::cerr << "MonitorMovePlayerCommand illegal side type "
                  << M_side << std::endl;
        M_unum = 0;
    }

}

/*-------------------------------------------------------------------*/
/*!

*/
bool
MonitorMovePlayerCommand::check() const
{

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
MonitorMovePlayerCommand::toStr( std::ostream & to ) const
{
    using namespace std;

    if ( 1 <= M_unum && M_unum <= 11 )
    {
        to << "(dispplayer "
           << M_side << " "
           << M_unum << " "
           << static_cast< int >( rint( M_x * rcg::SHOWINFO_SCALE ) ) << " "
           << static_cast< int >( rint( M_y * rcg::SHOWINFO_SCALE ) ) << " "
           << static_cast< int >( rint( M_angle * rcg::SHOWINFO_SCALE ) ) << ")";
    }
    return to;
}

/*-------------------------------------------------------------------*/
/*!

*/
MonitorDiscardPlayerCommand::MonitorDiscardPlayerCommand( const SideID side,
                                                          const int unum )
    : M_side( side )
    , M_unum( unum )
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << "MonitorDiscardPlayerCommand: illegal uniform number "
                  << M_unum << std::endl;
        M_unum = 1;
    }

    if ( side != LEFT
         && side != RIGHT )
    {
        std::cerr << "MonitorDiscardPlayerCommand: illegal side type "
                  << M_side << std::endl;
        M_unum = 0;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
MonitorDiscardPlayerCommand::toStr( std::ostream & to ) const
{
    if ( 1 <= M_unum && M_unum <= 11 )
    {
        to << "(dispdiscard "
           << M_side << " "
           << M_unum << ")";
    }

    return to;
}

/*-------------------------------------------------------------------*/
/*!

*/
MonitorCompressionCommand::MonitorCompressionCommand( const int level )
    : M_level( level )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
MonitorCompressionCommand::toStr( std::ostream & to ) const
{
    return to << "(compression " << M_level << ")";
}

}
