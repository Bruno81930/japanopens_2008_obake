// -*-c++-*-

/*!
	\file formation_dt.cpp
	\brief Delaunay Triangulation formation data classes Source File.
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

#include "formation_dt.h"

#include <rcsc/geom/segment_2d.h>
#include <rcsc/geom/line_2d.h>
#include <rcsc/math_util.h>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

 */
FormationDT::Param::Param( const Snapshot & data )
    : ball_( data.ball_ )
    , players_( data.players_ )
{

}

/*-------------------------------------------------------------------*/
/*!

 */
Vector2D
FormationDT::Param::getPosition( const int unum ) const
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
FormationDT::FormationDT()
    : Formation()
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
FormationDT::createDefaultParam()
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
FormationDT::setRoleName( const int unum,
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
FormationDT::getRoleName( const int unum ) const
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
FormationDT::createNewRole( const int unum,
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
}

/*-------------------------------------------------------------------*/
/*!

 */
Vector2D
FormationDT::getPosition( const int unum,
                          const Vector2D & focus_point ) const
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " *** ERROR *** invalid unum " << unum
                  << std::endl;
        return Vector2D::INVALIDATED;
    }

    DelaunayTriangulation::TrianglePtr tri
        = M_triangulation.findTriangleContains( focus_point );

    // linear interpolation
    return interpolate( unum, focus_point, tri );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
FormationDT::getPositions( const Vector2D & focus_point,
                           std::vector< Vector2D > & positions ) const
{
    positions.clear();

    DelaunayTriangulation::TrianglePtr tri
        = M_triangulation.findTriangleContains( focus_point );

    for ( int unum = 1; unum <= 11; ++unum )
    {
        positions.push_back( interpolate( unum, focus_point, tri ) );
    }
}


/*-------------------------------------------------------------------*/
/*!

 */
Vector2D
FormationDT::interpolate( const int unum,
                          const Vector2D & focus_point,
                          const DelaunayTriangulation::TrianglePtr tri ) const
{
    if ( ! tri )
    {
        const DelaunayTriangulation::Vertex * v
            = M_triangulation.findNearestVertex( focus_point );

        if ( ! v )
        {
            std::cerr << __FILE__ << ":" << __LINE__
                      << " *** ERROR *** No vertex!"
                      << std::endl;
            return Vector2D::INVALIDATED;
        }

        try
        {
            //std::cerr << "found nearest vertex id= " << v->id() << std::endl;
            return M_param.at( v->id() ).getPosition( unum );
        }
        catch ( std::exception & e )
        {
            std::cerr << __FILE__ << ":" << __LINE__
                      << " exception caught! "
                      << e.what() << std::endl;
            return Vector2D::INVALIDATED;
        }
    }

    try
    {
        Vector2D result_0 = M_param.at( tri->vertex( 0 )->id() ).getPosition( unum );
        Vector2D result_1 = M_param.at( tri->vertex( 1 )->id() ).getPosition( unum );
        Vector2D result_2 = M_param.at( tri->vertex( 2 )->id() ).getPosition( unum );

        Line2D line_0( tri->vertex( 0 )->pos(),
                       focus_point );

        Segment2D segment_12( tri->vertex( 1 )->pos(),
                              tri->vertex( 2 )->pos() );

        Vector2D intersection_12 = segment_12.intersection( line_0 );

        if ( ! intersection_12.valid() )
        {
            if ( focus_point.dist2( tri->vertex( 0 )->pos() ) < 1.0e-5 )
            {
                return result_0;
            }

            std::cerr << __FILE__ << ":" << __LINE__
                      << "***ERROR*** No intersection!\n"
                      << " focus=" << focus_point
                      << " line_intersection=" << intersection_12
                      << "\n v0=" << tri->vertex( 0 )->pos()
                      << " v1=" << tri->vertex( 1 )->pos()
                      << " v2=" << tri->vertex( 2 )->pos()
                      << std::endl;

            return ( result_0 + result_1 + result_2 ) / 3.0;
        }

        // distance from vertex_? to interxection_12
        double dist_1i = tri->vertex( 1 )->pos().dist( intersection_12 );
        double dist_2i = tri->vertex( 2 )->pos().dist( intersection_12 );

        // interpolation result of vertex_1 & vertex_2
        Vector2D result_12
            = result_1
            + ( result_2 - result_1 ) * ( dist_1i / ( dist_1i + dist_2i ) );

        // distance from vertex_0 to ball
        double dist_0b = tri->vertex( 0 )->pos().dist( focus_point );
        // distance from interxectin_12 to ball
        double dist_ib = intersection_12.dist( focus_point );

        // interpolation result of vertex_0 & intersection_12
        Vector2D result_012
            = result_0
            + ( result_12 - result_0 ) * ( dist_0b / ( dist_0b + dist_ib ) );

        return result_012;
    }
    catch ( std::exception & e )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " Exception caught!! "
                  << e.what()
                  << std::endl;
        return  Vector2D::INVALIDATED;
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
FormationDT::train( const std::list< Snapshot > & train_data )
{
    Rect2D pitch( - 60.0, - 45.0,
                  120.0, 90.0 );
    M_triangulation.init( pitch );
    M_param.clear();

    const std::list< Formation::Snapshot >::const_iterator end
        = train_data.end();
    for ( std::list< Formation::Snapshot >::const_iterator it
              = train_data.begin();
          it != end;
          ++it )
    {
        M_triangulation.addVertex( it->ball_ );
        M_param.push_back( Param( *it ) );
    }

    M_triangulation.compute();
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationDT::read( std::istream & is )
{
    M_param.clear();

    int n_line = 0;

    // read formation type name
    n_line += readName( is );
    if ( n_line < 0 )
    {
        return false;
    }

    // read role assignment
    if ( ! readRoles( is ) )
    {
        return false;
    }

#if 0
    std::cerr << "Loaded Role Assignment" << std::endl;
    for ( int unum = 1; unum <= 11; ++unum )
    {
        std::cerr << "Role=" << getRoleName( unum )
                  << " Synmetry=" << getSynmetryNumber( unum )
                  << std::endl;
    }
#endif

    //---------------------------------------------------
    // read sample data
    if ( ! readSamples( is ) )
    {
        return false;
    }

    if ( is.eof() )
    {
        std::cerr << "Input stream reaches EOF"
                  << std::endl;
    }

    ////////////////////////////////////////////////////////
    {
        Rect2D pitch( - 60.0, - 45.0,
                      120.0, 90.0 );
        M_triangulation.init( pitch );

        for ( std::vector< Param >::const_iterator it = M_param.begin();
              it != M_param.end();
              ++it )
        {
            M_triangulation.addVertex( it->ball_ );
        }

        M_triangulation.compute();
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationDT::readRoles( std::istream & is )
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
FormationDT::readSamples( std::istream & is )
{
    std::string line_buf;

    while ( std::getline( is, line_buf ) )
    {
        if ( line_buf == "End" )
        {
            break;
        }

        M_param.push_back( Param() );


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
            M_param.pop_back();
            return false;
        }
        msg += n_read;

        M_param.back().ball_.assign( read_x, read_y );

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
                M_param.pop_back();
                return false;
            }
            msg += n_read;

            M_param.back().players_[unum - 1].assign( read_x, read_y );
        }
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
FormationDT::print( std::ostream & os ) const
{
    printName( os );

    // put role assignment
    for ( int unum = 1; unum <= 11; ++unum )
    {
        os << M_role_name[unum - 1] << ' '
           << M_synmetry_number[unum - 1] << ' ';
    }
    os << '\n';

    for ( std::vector< Param >::const_iterator it = M_param.begin();
          it != M_param.end();
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
