// -*-c++-*-

/*!
	\file formation_uva.cpp
	\brief UvA Trilearn type formation method classes Source File.
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

#include "formation_uva.h"

#include <rcsc/math_util.h>

#include <cstdio>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
FormationUvA::RoleParam::RoleParam()
    : M_name( "null" )
    , M_attr_x( 0.0 )
    , M_attr_y( 0.0 )
    , M_behind_ball( false )
    , M_min_x( 0.0 )
    , M_max_x( 0.0 )
{

}


/*-------------------------------------------------------------------*/
/*!

*/
FormationUvA::RoleParam::RoleParam( const std::string & name,
                                    const double & attr_x,
                                    const double & attr_y,
                                    const bool behind_ball,
                                    const double & min_x,
                                    const double & max_x )
    : M_name( name )
    , M_attr_x( attr_x )
    , M_attr_y( attr_y )
    , M_behind_ball( behind_ball )
    , M_min_x( min_x )
    , M_max_x( max_x )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
const
FormationUvA::RoleParam &
FormationUvA::RoleParam::assign( const std::string & name,
                                 const double & attr_x,
                                 const double & attr_y,
                                 const bool behind_ball,
                                 const double & min_x,
                                 const double & max_x )
{
    M_name = name;
    M_attr_x = attr_x;
    M_attr_y = attr_y;
    M_behind_ball = behind_ball;
    M_min_x = min_x;
    M_max_x = max_x;

    return *this;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
FormationUvA::RoleParam::print( std::ostream & os ) const
{
    //os << "Role=" << M_name << ": attr=("
    //   << M_attr_x << ", " << M_attr_y << ") x-range=("
    //   << M_min_x  << ", " << M_maxy << ") behind_ball="
    //   << M_behind_ball;

    os << attrX() << ' '
       << attrY() << ' '
       << ( behindBall() ? 1 : 0 ) << ' '
       << minX() << ' '
       << maxX();

    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
FormationUvA::FormationUvA()
    : Formation()
    , M_max_y_percentage( 0.75 )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
Formation::Snapshot
FormationUvA::createDefaultParam()
{
    createNewRole( 1, "Goalie", Formation::CENTER );
    createNewRole( 2, "Sweeper", Formation::CENTER );
    createNewRole( 3, "CenterBack", Formation::CENTER );
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
FormationUvA::createNewRole( const int unum,
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
FormationUvA::setRoleName( const int unum,
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
FormationUvA::getRoleName( const int unum ) const
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
FormationUvA::getPosition( const int unum,
                           const Vector2D & focus_point ) const
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << __FILE__ << ':' << __LINE__
                  << " **ERROR*** Invalid player number " << unum
                  << std::endl;
        return Vector2D::INVALIDATED;
    }

    Vector2D home_pos = M_home_pos[unum - 1];

    std::map< std::string, RoleParam >::const_iterator it
        = M_role_params.find( M_role_names[unum - 1] );
    if ( it == M_role_params.end() )
    {
        std::cerr << __FILE__ << ':' << __LINE__
                  << " **ERROR*** parameters for  " << M_role_names[unum - 1]
                  << " cannot be found!"
                  << std::endl;
        return home_pos;
    }

    home_pos.x += focus_point.x * it->second.attrX();
    home_pos.y += focus_point.y * it->second.attrY();

    if ( it->second.behindBall()
         && home_pos.x > focus_point.x )
    {
        home_pos.x = focus_point.x;
    }

    home_pos.x = bound( it->second.minX(),
                        home_pos.x,
                        it->second.maxX() );

//     home_pos.y = bound( - ServerParam::i().pitchHalfWidth() * maxYPercentage(),
//                         home_pos.y,
//                         + ServerParam::i().pitchHalfWidth() * maxYPercentage() );
    home_pos.y = bound( - 52.5 * maxYPercentage(),
                        home_pos.y,
                        + 52.5 * maxYPercentage() );

    return home_pos;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FormationUvA::getPositions( const Vector2D & focus_point,
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
FormationUvA::train( const std::list< Snapshot > & train_data )
{


}

/*-------------------------------------------------------------------*/
/*!

*/
bool
FormationUvA::read( std::istream & is )
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

    if ( ! readRoles( is ) )
    {
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
FormationUvA::readPlayers( std::istream & is )
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
            double home_x, home_y;

            if ( std::sscanf( line_buf.c_str(),
                              " %d %s %lf %lf ",
                              &unum, role_name, &home_x, &home_y ) != 4 )
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

            M_home_pos[i].assign( home_x, home_y );
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
bool
FormationUvA::readRoles( std::istream & is )
{
    std::string line_buf;

    while ( std::getline( is, line_buf ) )
    {
        if ( line_buf.empty()
             || line_buf[0] == '#'
             || ! line_buf.compare( 0, 2, "//" ) )
        {
            continue;
        }

        // <name> <attr-x> <attr-y> <behind-ball> <min-x> <max-x>
        char role_name[128];
        double attr_x, attr_y;
        int behind_ball;
        double min_x, max_x;

        if ( std::sscanf( line_buf.c_str(),
                          " %s %lf %lf %d %lf %lf ",
                          role_name,
                          &attr_x, &attr_y,
                          &behind_ball, &min_x, &max_x ) != 6 )
        {
            continue;
        }

        M_role_params.insert( std::make_pair( std::string( role_name ),
                                              RoleParam( role_name,
                                                         attr_x, attr_y,
                                                         static_cast< bool >( behind_ball ),
                                                         min_x, max_x ) ) );
    }

    // check player's dependency
    for ( int i = 0; i < 11; ++i )
    {
        if ( M_role_params.find( M_role_names[i] ) == M_role_params.end() )
        {
            std::cerr << __FILE__ << ':' << __LINE__
                      << " ***ERROR*** The role " << M_role_names[i]
                      << " does not exist!"
                      << std::endl;
            return false;
        }
    }

    return true;
}


/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
FormationUvA::print( std::ostream & os ) const
{
    os << "Formation " << methodName() << '\n';

    for ( int i = 0; i < 11; ++i )
    {
        os << i + 1 << ' '
           << M_role_names[i] << ' '
           << M_home_pos[i].x << ' '
           << M_home_pos[i].y << '\n';
    }

    const std::map< std::string, RoleParam >::const_iterator end
       = M_role_params.end();

    for ( std::map< std::string, RoleParam >::const_iterator it
              = M_role_params.begin();
          it != end;
          ++it )
    {
        os << it->first << ' ';
        it->second.print( os );
        os << "    // role param\n";
    }

    return os << std::flush;
}

}
