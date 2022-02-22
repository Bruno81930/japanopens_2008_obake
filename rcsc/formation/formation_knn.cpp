// -*-c++-*-

/*!
	\file formation_knn.cpp
	\brief k-nearest neighbor formation class Source File.
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

#include "formation_knn.h"

#include <rcsc/math_util.h>

#include <algorithm>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

 */
FormationKNN::Data::Data()
    : ball_( 0.0, 0.0 )
    , players_( 11 )
{

}

/*-------------------------------------------------------------------*/
/*!

 */
FormationKNN::Data::Data( const Snapshot & snapshot )
    : ball_( snapshot.ball_ )
    , players_( snapshot.players_ )
{

}

/*-------------------------------------------------------------------*/
/*!

 */
FormationKNN::Data::Data( const Vector2D & ball,
                          const std::vector< Vector2D > & players )
    : ball_( ball )
    , players_( players )
{

}

/*-------------------------------------------------------------------*/
/*!

 */
const
FormationKNN::Data &
FormationKNN::Data::assign( const Vector2D & ball,
                            const std::vector< Vector2D > & players )
{
    ball_ = ball;
    players_ = players;
    return *this;
}

/*-------------------------------------------------------------------*/
/*!

 */
Vector2D
FormationKNN::Data::getPosition( const int unum ) const
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " *** ERROR *** invalid unum " << unum
                  << std::endl;
        return Vector2D( 0.0, 0.0 );
    }

    return players_[unum - 1];
}

/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/

/*-------------------------------------------------------------------*/
/*!

 */
FormationKNN::FormationKNN()
    : Formation()
    , M_k( 3 )
{
    for ( int i = 0; i < 11; ++i )
    {
        M_role_name[i] = "Dummy";
    }
}



/*-------------------------------------------------------------------*/
/*!

 */
Formation::Snapshot
FormationKNN::createDefaultParam()
{
#if 1
    // 4-3-3

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

#elif 0
    // 1-3-4-2

    // 1: goalie
    // 2: sweeper
    // 3: left side back
    // 4: right side back
    // 5: center back
    // 6: left offensive half
    // 7: right offensive half
    // 8: left side half
    // 9: right side half
    // 10: left forward
    // 11: right forward
    createNewRole( 1, "Goalie", Formation::CENTER );
    createNewRole( 2, "Sweeper", Formation::CENTER );
    createNewRole( 3, "SideBack", Formation::SIDE );
    setSynmetryType( 4, 3 );
    createNewRole( 5, "DefensiveHalf", Formation::CENTER );
    createNewRole( 6, "OffensiveHalf", Formation::SIDE );
    setSynmetryType( 7, 6 );
    createNewRole( 8, "SideHalf", Formation::SIDE );
    setSynmetryType( 9, 8 );
    createNewRole( 10, "Forward", Formation::SIDE );
    setSynmetryType( 11, 10 );

    Snapshot snap;

    snap.ball_.assign( 0.0, 0.0 );
    snap.players_.push_back( Vector2D( -50.0, 0.0 ) );
    snap.players_.push_back( Vector2D( -15.0, 0.0 ) );
    snap.players_.push_back( Vector2D( -15.0, -8.0 ) );
    snap.players_.push_back( Vector2D( -15.0, 8.0 ) );
    snap.players_.push_back( Vector2D( -9.0, 0.0 ) );
    snap.players_.push_back( Vector2D( -5.0, -16.0 ) );
    snap.players_.push_back( Vector2D( -5.0, 16.0 ) );
    snap.players_.push_back( Vector2D( 0.0, -25.0 ) );
    snap.players_.push_back( Vector2D( 0.0, 25.0 ) );
    snap.players_.push_back( Vector2D( 10.0, -10.0 ) );
    snap.players_.push_back( Vector2D( 10.0, 10.0 ) );
#else
    // 3-5-2

    // 1: goalie
    // 2: sweeper
    // 3: left side back
    // 4: right side back
    // 5: left defensive half
    // 6: right defensive half
    // 7: offensive half
    // 8: left side half
    // 9: right side half
    // 10: left forward
    // 11: right forward
    createNewRole( 1, "Goalie", Formation::CENTER );
    createNewRole( 2, "Sweeper", Formation::CENTER );
    createNewRole( 3, "SideBack", Formation::SIDE );
    setSynmetryType( 4, 3 );
    createNewRole( 5, "DefensiveHalf", Formation::SIDE );
    setSynmetryType( 6, 5 );
    createNewRole( 7, "OffensiveHalf", Formation::CENTER );
    createNewRole( 8, "SideHalf", Formation::SIDE );
    setSynmetryType( 9, 8 );
    createNewRole( 10, "Forward", Formation::SIDE );
    setSynmetryType( 11, 10 );

    Snapshot snap;

    snap.ball_.assign( 0.0, 0.0 );
    snap.players_.push_back( Vector2D( -50.0, 0.0 ) );
    snap.players_.push_back( Vector2D( -15.0, 0.0 ) );
    snap.players_.push_back( Vector2D( -15.0, -8.0 ) );
    snap.players_.push_back( Vector2D( -15.0, 8.0 ) );
    snap.players_.push_back( Vector2D( -5.0, -16.0 ) );
    snap.players_.push_back( Vector2D( -5.0, 16.0 ) );
    snap.players_.push_back( Vector2D( -9.0, 0.0 ) );
    snap.players_.push_back( Vector2D( 0.0, -25.0 ) );
    snap.players_.push_back( Vector2D( 0.0, 25.0 ) );
    snap.players_.push_back( Vector2D( 10.0, -10.0 ) );
    snap.players_.push_back( Vector2D( 10.0, 10.0 ) );
#endif
    return snap;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
FormationKNN::setRoleName( const int unum,
                           const std::string & name )
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " *** ERROR *** invalid unum " << unum
                  << std::endl;
        return;
    }

    M_role_name[unum - 1] = name;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::string
FormationKNN::getRoleName( const int unum ) const
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " *** ERROR *** invalid unum " << unum
                  << std::endl;
        return std::string( "" );
    }

    return M_role_name[unum - 1];
}

/*-------------------------------------------------------------------*/
/*!

 */
void
FormationKNN::createNewRole( const int unum,
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
                  << " ***ERROR*** Unsupported side type "
                  << std::endl;
        break;
    default:
        break;
    }
}


/*-------------------------------------------------------------------*/

class DataCmp {
private:
    const Vector2D M_point;
public:

    DataCmp( const Vector2D & point )
        : M_point( point )
      { }

    bool operator()( const FormationKNN::Data * lhs,
                     const FormationKNN::Data * rhs )
      {
          return lhs->ball_.dist2( M_point ) < rhs->ball_.dist2( M_point );
      }
};


/*-------------------------------------------------------------------*/
/*!

 */
Vector2D
FormationKNN::getPosition( const int unum,
                           const Vector2D & focus_point ) const
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " *** ERROR *** Illegal unum " << unum
                  << std::endl;
        return Vector2D::INVALIDATED;
    }

    if ( M_data_ptr.empty() )
    {
        return Vector2D( 0.0, 0.0 );
    }

    const size_t size = std::min( M_data_ptr.size(), M_k );

    std::partial_sort( M_data_ptr.begin(),
                       M_data_ptr.begin() + size,
                       M_data_ptr.end(),
                       DataCmp( focus_point ) );

    std::vector< double > inv_dist2( size, 0.0 );
    double sum_inv_dist2 = 0.0;

    for ( size_t i = 0; i < size; ++i )
    {
        double d2 = M_data_ptr[i]->ball_.dist2( focus_point );
        if ( d2 < 1.0e-10 )
        {
            return M_data_ptr[i]->getPosition( unum );
        }
        inv_dist2[i] = 1.0 / d2;
        sum_inv_dist2 += inv_dist2[i];
    }

    Vector2D pos( 0.0, 0.0 );

    for ( size_t i = 0; i < size; ++i )
    {
        pos += M_data_ptr[i]->getPosition( unum ) * inv_dist2[i];
    }

    pos /= sum_inv_dist2;
#if 0
    if ( unum == 11 )
    {
        std::cerr << "sum_inv_dist2=" << sum_inv_dist2 << '\n';
        for ( size_t i = 0; i < size; ++i )
        {
            std::cerr << "  ball=" << M_data_ptr[i]->ball_
                      << " pos=" << M_data_ptr[i]->getPosition( unum )
                      << " inv_dist2=" << inv_dist2[i]
                      << " rate=" << ( inv_dist2[i] / sum_inv_dist2 )
                      << '\n';
        }
        std::cerr << "  pos = " << pos << std::endl;
    }
#endif
    return pos;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
FormationKNN::getPositions( const Vector2D & focus_point,
                            std::vector< Vector2D > & positions ) const
{
    positions.clear();

    const size_t size = std::min( M_data_ptr.size(), M_k );

    std::partial_sort( M_data_ptr.begin(),
                       M_data_ptr.begin() + size,
                       M_data_ptr.end(),
                       DataCmp( focus_point ) );

    std::vector< double > dist( size, 0.0 );
    double sum_dist = 0.0;

    for ( size_t i = 0; i < size; ++i )
    {
        dist[i] = M_data_ptr[i]->ball_.dist( focus_point );
        sum_dist += dist[i];
    }

    for ( int unum = 1; unum <= 11; ++unum )
    {
        Vector2D pos( 0.0, 0.0 );

        for ( size_t i = 0; i < size; ++i )
        {
            pos += M_data_ptr[i]->getPosition( unum ) * ( ( sum_dist - dist[i] ) / sum_dist );
        }

        positions.push_back( pos );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
FormationKNN::train( const std::list< Snapshot > & train_data )
{
    M_data.clear();
    M_data_ptr.clear();

    M_data.reserve( train_data.size() );

    for ( std::list< Formation::Snapshot >::const_iterator it
              = train_data.begin();
          it != train_data.end();
          ++it )
    {
        M_data.push_back( Data( *it ) );
    }

    M_data.reserve( M_data.size() );

    for ( std::vector< Data >::iterator it = M_data.begin();
          it != M_data.end();
          ++it )
    {
        M_data_ptr.push_back( &(*it) );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationKNN::read( std::istream & is )
{
    M_data.clear();

    int n_line = 0;

    // read formation type name
    n_line += readName( is );
    if ( n_line < 0 )
    {
        return false;
    }

    std::string line_buf;

    // read role assignment
    if ( ! readRoles( is ) )
    {
        return false;
    }

    //---------------------------------------------------
    // read sample data
    if ( ! readSamples( is ) )
    {
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationKNN::readRoles( std::istream & is )
{
    std::string line_buf;

    if ( ! std::getline( is, line_buf ) )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " *** ERROR *** readRoles(). Failed getline "
                  << std::endl;
        return false;
    }

    const char * msg = line_buf.c_str();

    for ( int unum = 1; unum <= 11; ++unum )
    {
        char role_name[128];
        int synmetry_number;
        int n_read = 0;

        if ( std::sscanf( msg, " %s %d %n ",
                          role_name, &synmetry_number, &n_read ) != 2 )
        {
            std::cerr << __FILE__ << ":" << __LINE__
                      << " *** ERROR *** readRoles(). Failed to read player "
                      << unum
                      << std::endl;
            return false;
        }
        msg += n_read;

        const Formation::SideType type = ( synmetry_number == 0
                                           ? Formation::CENTER
                                           : synmetry_number < 0
                                           ? Formation::SIDE
                                           : Formation::SYNMETRY );
        if ( type == Formation::CENTER )
        {
            createNewRole( unum, role_name, type );
        }
        else if ( type == Formation::SIDE )
        {
            createNewRole( unum, role_name, type );
        }
        else
        {
            setSynmetryType( unum, synmetry_number );
        }
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationKNN::readSamples( std::istream & is )
{
    std::string line_buf;

    while ( std::getline( is, line_buf ) )
    {
        if ( line_buf == "End" )
        {
            break;
        }

        M_data.push_back( Data() );

        const char * msg = line_buf.c_str();

        double read_x, read_y;
        int n_read = 0;

        // read ball pos
        if ( std::sscanf( msg, " %lf %lf %n ",
                          &read_x, &read_y, &n_read ) != 2 )
        {
            std::cerr << __FILE__ << ":" << __LINE__
                      << " *** ERROR *** Invalid ball data ["
                      << line_buf << "]"
                      << std::endl;
            M_data.pop_back();
            return false;
        }
        msg += n_read;

        M_data.back().ball_.assign( read_x, read_y );

        for ( int unum = 1; unum <= 11; ++unum )
        {
            if ( std::sscanf( msg, " %lf %lf %n ",
                              &read_x, &read_y, &n_read ) != 2 )
            {
                std::cerr << __FILE__ << ":" << __LINE__
                          << " *** ERROR *** Invalid player data. unum = "
                          << unum
                          << " [" << line_buf << "]"
                          << std::endl;
                M_data.pop_back();
                return false;
            }
            msg += n_read;

            M_data.back().players_[unum - 1].assign( read_x, read_y );
        }
    }


    M_data_ptr.reserve( M_data.size() );

    for ( std::vector< Data >::iterator it = M_data.begin();
          it != M_data.end();
          ++it )
    {
        M_data_ptr.push_back( &(*it) );
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
FormationKNN::print( std::ostream & os ) const
{
    printName( os );

    // put role assignment
    for ( int unum = 1; unum <= 11; ++unum )
    {
        os << M_role_name[unum - 1] << ' '
           << M_synmetry_number[unum - 1] << ' ';
    }
    os << '\n';

    for ( std::vector< Data >::const_iterator it = M_data.begin();
          it != M_data.end();
          ++it )
    {
        os << it->ball_.x << ' '
           << it->ball_.y << ' ';

        for ( std::vector< Vector2D >::const_iterator p
                  = it->players_.begin();
              p != it->players_.end();
              ++p )
        {
            os << p->x << ' '
               << p->y << ' ';
        }
        os << '\n';
    }

    os << "End" << std::endl;
    return os;
}

}
