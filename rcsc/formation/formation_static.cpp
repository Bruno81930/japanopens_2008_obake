// -*-c++-*-

/*!
	\file formation_static.cpp
	\brief static type formation method classes Source File.
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

#include "formation_static.h"

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
FormationStatic::FormationStatic()
    : Formation()
{

}

/*-------------------------------------------------------------------*/
/*!

*/
Formation::Snapshot
FormationStatic::createDefaultParam()
{
    createNewRole( 1, "Goalie", Formation::CENTER );
    createNewRole( 2, "CenterBack", Formation::SIDE );
    setSynmetryType( 3, 2 );
    createNewRole( 4, "SideBack", Formation::SIDE );
    setSynmetryType( 5, 4 );
    createNewRole( 6, "DefensiveHalf", Formation::CENTER );
    createNewRole( 7, "OffensiveHalf", Formation::SIDE );
    setSynmetryType( 8, 7 );
    createNewRole( 9, "SideForward", Formation::SIDE );
    setSynmetryType( 10, 9 );
    createNewRole( 11, "CenterForward", Formation::CENTER );

    Snapshot snapshot;

    snapshot.ball_.assign( 0.0, 0.0 );
    snapshot.players_.push_back( Vector2D( -50.0, 0.0 ) );
    snapshot.players_.push_back( Vector2D( -20.0, -8.0 ) );
    snapshot.players_.push_back( Vector2D( -20.0, 8.0 ) );
    snapshot.players_.push_back( Vector2D( -18.0, -18.0 ) );
    snapshot.players_.push_back( Vector2D( -18.0, 18.0 ) );
    snapshot.players_.push_back( Vector2D( -15.0, 0.0 ) );
    snapshot.players_.push_back( Vector2D( 0.0, -12.0 ) );
    snapshot.players_.push_back( Vector2D( 0.0, 12.0 ) );
    snapshot.players_.push_back( Vector2D( 10.0, -22.0 ) );
    snapshot.players_.push_back( Vector2D( 10.0, 22.0 ) );
    snapshot.players_.push_back( Vector2D( 10.0, 0.0 ) );

    return snapshot;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FormationStatic::createNewRole( const int unum,
                                const std::string & role_name,
                                const SideType type )
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " *** ERROR *** invalid unum " << unum
                  << std::endl;
        return;
    }

    setRoleName( unum, role_name );

    switch ( type ) {
    case Formation::CENTER:
        setCenterType( unum );
        break;
    case Formation::SIDE:
        setSideType( unum );
        break;
    case Formation::SYNMETRY:
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** Invalid side type "
                  << std::endl;
        break;
    default:
        break;
    }

}

/*-------------------------------------------------------------------*/
/*!

*/
void
FormationStatic::setRoleName( const int unum,
                              const std::string & name )
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << __FILE__ << ':' << __LINE__
                  << " ***ERROR*** invalid unum " << unum
                  << std::endl;
        return;
    }

    M_role_names[unum - 1] = name;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::string
FormationStatic::getRoleName( const int unum ) const
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << __FILE__ << ':' << __LINE__
                  << " ***ERROR*** Invalid unum"
                  << std::endl;
        return std::string( "null" );
    }

    return M_role_names[unum - 1];
}

/*-------------------------------------------------------------------*/
/*!

*/
Vector2D
FormationStatic::getPosition( const int unum,
                              const Vector2D & focus_point ) const
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << __FILE__ << ':' << __LINE__
                  << " **ERROR*** Invalid player number " << unum
                  << std::endl;
        return Vector2D( 0.0, 0.0 );
    }

    return M_pos[unum - 1];
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FormationStatic::getPositions( const Vector2D & focus_point,
                               std::vector< Vector2D > & positions ) const
{
    positions.clear();

    for ( int unum = 1; unum <= 11; ++unum )
    {
        positions.push_back( getPosition( unum, focus_point ) );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FormationStatic::train( const std::list< Snapshot > & train_data )
{


}

/*-------------------------------------------------------------------*/
/*!

*/
bool
FormationStatic::read( std::istream & is )
{
    int n_line = 0;

    n_line += readName( is );
    if ( n_line < 0 )
    {
        return false;
    }

    if ( ! readPlayers( is ) )
    {
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
FormationStatic::readPlayers( std::istream & is )
{
    int n_read = 0;

    std::string line_buf;

    for ( int i = 0; i < 11; ++i )
    {
        while ( std::getline( is, line_buf ) )
        {
            if ( line_buf.empty()
                 || line_buf[0] == '#'
                 || ! line_buf.compare( 0, 2, "//" ) )
            {
                continue;
            }

            int unum;
            char role_name[128];
            double pos_x, pos_y;

            if ( std::sscanf( line_buf.c_str(),
                              " %d %s %lf %lf ",
                              &unum, role_name, &pos_x, &pos_y ) != 4 )
            {
                continue;
            }

            if ( unum != i + 1 )
            {
                std::cerr << __FILE__ << ':' << __LINE__
                          << " ***ERROR*** Invalid formation formart ["
                          << line_buf << "]"
                          << std::endl;
                return false;
            }

            M_pos[i].assign( pos_x, pos_y );
            M_role_names[i] = role_name;
            ++n_read;
            break;
        }
    }

    if ( n_read != 11 )
    {
        std::cerr << __FILE__ << ':' << __LINE__
                  << " ***ERROR*** Invalid formation format."
                  << " The number of read player is " << n_read
                  << std::endl;
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
FormationStatic::print( std::ostream & os ) const
{
    os << "Formation " << methodName() << '\n';

    for ( int i = 0; i < 11; ++i )
    {
        os << i + 1 << ' '
           << M_role_names[i] << ' '
           << M_pos[i].x << ' '
           << M_pos[i].y << '\n';
    }

    return os << std::flush;
}

}
