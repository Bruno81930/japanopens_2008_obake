// -*-c++-*-

/*!
	\file formation_sbsp.cpp
	\brief simple SBSP formation Source File.
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

#include "formation_sbsp.h"

#include <rcsc/random.h>
#include <rcsc/math_util.h>

#include <sstream>
#include <algorithm>
#include <cstdio>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
FormationSBSP::Role::Role()
    : number_( -1 )
{
    randomize();
}


/*-------------------------------------------------------------------*/
/*!

*/
FormationSBSP::Role::Role( const Vector2D & attract,
                           const Rect2D & region,
                           const bool behind_ball )
    : attract_( attract )
    , region_( region )
    , behind_ball_( behind_ball )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
void
FormationSBSP::Role::randomize()
{
    UniformReal rng( -1.0, 1.0 );

    pos_.x = 52.5 * rng();
    pos_.y = 34.0 * rng();

    attract_.x = std::fabs( rng() );
    attract_.y = std::fabs( rng() );


    region_.assign( -52.5, -34.0,
                    105.0, 68.0 );

    behind_ball_ = false;

}

/*-------------------------------------------------------------------*/
/*!

*/
bool
FormationSBSP::Role::read( std::istream & is )
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

        break;
    }

    if ( line_buf.empty() )
    {
        return false;
    }

    std::istringstream istr( line_buf );

    istr >> number_
         >> synmetry_;

    if ( synmetry_ > 0 )
    {
        // synmetry type
        return true;
    }

    double left_x, right_x, top_y, bottom_y;

    istr >> name_
         >> pos_.x >> pos_.y
         >> attract_.x >> attract_.y
         >> left_x >> right_x >> top_y >> bottom_y
         >> behind_ball_;

    if ( istr.fail() )
    {
        return false;
    }

    region_.assign( left_x, top_y,
                    right_x - left_x, bottom_y - top_y );

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
FormationSBSP::Role::print( std::ostream & os ) const
{
    char buf[512];

    std::sprintf( buf,
                  "%2d %2d %16s "
                  "%6.2f %6.2f "
                  "%4.2f %4.2f "
                  "%6.2f %6.2f %6.2f %6.2f "
                  "%d",
                  number_, synmetry_, name_.c_str(),
                  pos_.x, pos_.y,
                  attract_.x, attract_.y,
                  region_.left(), region_.right(),
                  region_.top(), region_.bottom(),
                  behind_ball_ ? 1 : 0 );
    os << buf;
    return os;
}

///////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------*/
/*!

*/
Vector2D
FormationSBSP::Param::getPosition( const int unum,
                                   const Vector2D & ball_pos ) const
{
    const Role & role = getRole( unum );

    Vector2D position = role.pos_;

    position.x += ball_pos.x * role.attract_.x;
    position.y += ball_pos.y * role.attract_.y;

    position.x = min_max( role.region_.left(),
                          position.x,
                          role.region_.right() );
    position.y = min_max( role.region_.top(),
                          position.y,
                          role.region_.bottom() );
    if ( role.behind_ball_ )
    {
        position.x = std::min( ball_pos.x, position.x );
    }

    return position;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FormationSBSP::getPositions( const Vector2D & focus_point,
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
bool
FormationSBSP::Param::check()
{
    for ( std::size_t i = 0; i < 11; ++i )
    {
        // check if number is read or not
        if ( M_roles[i].number_ == -1 )
        {
            std::cerr <<  __FILE__ << ':' << __LINE__
                      << " ***ERROR*** player number is not read "
                      << " at " << i + 1
                      << std::endl;
            return false;
        }

        // check number order
        if ( M_roles[i].number_ != static_cast< int >( i + 1 ) )
        {
            std::cerr <<  __FILE__ << ':' << __LINE__
                      << " ***ERROR*** Invalid player number order "
                      << M_roles[i].number_ << " at " << i + 1
                      << std::endl;
            return false;
        }

        // check duplicate number
        for ( std::size_t j = i + 1; j < 11; ++j )
        {
            if ( M_roles[i].number_ == M_roles[j].number_ )
            {
                std::cerr << "***ERRORR*** Duplicated player number"
                          << std::endl;
                return false;

            }
        }

        // check synmetry number
        if ( M_roles[i].synmetry_ > 0 )
        {
            const int synmetry = M_roles[i].synmetry_;

            if ( synmetry > 11 )
            {
                std::cerr << "***ERROR*** synmetry number is over 11."
                          << " synmetry = " << synmetry
                          << std::endl;
                return false;
            }

            if ( M_roles[i].number_ == synmetry )
            {
                std::cerr << "***ERROR*** refered player is same as self number"
                          << " number = " << i + 1
                          << " synmetry = " << synmetry
                          << std::endl;
                return false;
            }

            if ( M_roles[synmetry-1].synmetry_ >= 0 )
            {
                // refered player is not a side type
                std::cerr << "***ERROR*** refered player is not a side type."
                          << " number = " << i + 1
                          << " synmetry = " << synmetry
                          << std::endl;
                return false;
            }
        }

    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FormationSBSP::Param::createSynmetryParam()
{
    for ( std::size_t i = 0; i < 11; ++i )
    {
        int synmetry = M_roles[i].synmetry_;
        if ( synmetry <= 0 )
        {
            // not a synmetry type
            continue;
        }

        Role & synmetry_role = M_roles[synmetry-1];

        M_roles[i].name_ = synmetry_role.name_;
        M_roles[i].pos_.x = synmetry_role.pos_.x;
        M_roles[i].pos_.y = - synmetry_role.pos_.y;
        M_roles[i].attract_ = synmetry_role.attract_;
        M_roles[i].region_.assign( Vector2D( synmetry_role.region_.left(),
                                             - synmetry_role.region_.bottom() ),
                                   synmetry_role.region_.size() );
        M_roles[i].behind_ball_ = synmetry_role.behind_ball_;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
FormationSBSP::Param::read( std::istream & is )
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

        break;
    }

    if ( line_buf.empty() )
    {
        return false;
    }

    std::istringstream istr( line_buf );
    istr >> M_name;

    for ( std::size_t i = 0; i < 11; ++i )
    {
        if ( ! M_roles[i].read( is ) )
        {
            std::cerr << __FILE__ << ':' << __LINE__
                      << " ***ERROR*** Failed to read role of number "
                      << i + 1
                      << std::endl;
            return false;
        }
    }

    if ( ! check() )
    {
        return false;
    }

    createSynmetryParam();

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
FormationSBSP::Param::print( std::ostream & os ) const
{
    os << M_name << '\n';

    for ( std::size_t i = 0; i < 11; ++i )
    {
        M_roles[i].print( os );
        os << '\n';
    }

    os << std::flush;
    return os;
}

///////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------*/
/*!

*/
FormationSBSP::FormationSBSP()
    : Formation()
    , M_param( "Default" )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
Formation::Snapshot
FormationSBSP::createDefaultParam()
{
    // 1: goalie
    // 2: left center back
    // 3(2): right center back
    // 4: left side back
    // 5(4): right side back
    // 6: defensive half
    // 7: left offensive half
    // 8(7): left side half
    // 9(8): right side half
    // 10: left forward
    // 11(10): right forward
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

    Snapshot snap;

    snap.ball_.assign( 0.0, 0.0 );
    snap.players_.push_back( Vector2D( -50.0, 0.0 ) );
    snap.players_.push_back( Vector2D( -20.0, -8.0 ) );
    snap.players_.push_back( Vector2D( -20.0, 8.0 ) );
    snap.players_.push_back( Vector2D( -18.0, -18.0 ) );
    snap.players_.push_back( Vector2D( -18.0, 18.0 ) );
    snap.players_.push_back( Vector2D( -15.0, 0.0 ) );
    snap.players_.push_back( Vector2D( 0.0, -12.0 ) );
    snap.players_.push_back( Vector2D( 0.0, 12.0 ) );
    snap.players_.push_back( Vector2D( 10.0, -22.0 ) );
    snap.players_.push_back( Vector2D( 10.0, 22.0 ) );
    snap.players_.push_back( Vector2D( 10.0, 0.0 ) );

    return snap;
}


/*-------------------------------------------------------------------*/
/*!

*/
void
FormationSBSP::setRoleName( const int unum,
                            const std::string & name )
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " *** ERROR *** invalid unum " << unum
                  << std::endl;
        return;
    }

    M_param.getRole( unum ).name_ =  name;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::string
FormationSBSP::getRoleName( const int unum ) const
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " *** ERROR *** invalid unum " << unum
                  << std::endl;
        return std::string( "" );
    }

    return M_param.getRole( unum ).name_;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FormationSBSP::createNewRole( const int unum,
                              const std::string & role_name,
                              const Formation::SideType type )
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


    M_param.getRole( unum ).randomize();
}

/*-------------------------------------------------------------------*/
/*!

*/
Vector2D
FormationSBSP::getPosition( const int unum,
                            const Vector2D & ball_pos ) const
{
    try
    {
        return param().getPosition( unum, ball_pos );
    }
    catch ( std::exception & e )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " *** ERROR *** invalid unum " << unum
                  << std::endl;
        return Vector2D( 0.0, 0.0 );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FormationSBSP::train( const std::list< Snapshot > & train_data )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
bool
FormationSBSP::read( std::istream & is )
{
    int n_line = 0;

    n_line += readName( is );
    if ( n_line < 0 )
    {
        return false;
    }

    if ( ! M_param.read( is ) )
    {
        return false;
    }

    for ( int unum = 1; unum <= 11; ++unum )
    {
        int synmetry = M_param.getRole( unum ).synmetry_;
        if ( synmetry == 0 )
        {
            setCenterType( unum );
        }
        else if ( synmetry < 0 )
        {
            setSideType( unum );
        }
        else
        {
            setSynmetryType( unum, synmetry );
        }
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
FormationSBSP::print( std::ostream & os ) const
{
    printName( os );
    return M_param.print( os );
}


}
