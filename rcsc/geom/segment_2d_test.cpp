// -*-c++-*-

/*!
  \file segment_2d_test.cpp
  \brief test code for rcsc::Segment2D
*/

/*
 *Copyright:

 Copyright (C) Hiroki Shimora

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

#ifdef HAVE_CONFIG
#include <config.h>
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include "segment_2d.h"
#include "vector_2d.h"

using namespace boost::unit_test_framework;

namespace {
static const double EPS = 1.0e-10;
}

#define CHECK_DOUBLE_CLOSE( d1, d2 ) \
        (BOOST_CHECK_SMALL( ((d2)-(d1)), EPS))

#define CHECK_VECTOR2D_CLOSE( v1, v2 ) \
        (BOOST_CHECK_SMALL( ((v2)-(v1)).r(), EPS))


static void checkSegment2D()
{
    //
    // check length of segment
    //
    {
        const rcsc::Segment2D s1( rcsc::Vector2D( 0.0, 0.0 ),
                                  rcsc::Vector2D( 3.0, 4.0 ) );

        CHECK_DOUBLE_CLOSE( 5.0, s1.length() );
    }



    //
    // check existIntersectionExceptTerminalPoint()
    //
    {
        const rcsc::Segment2D s1( rcsc::Vector2D( 0.0, 0.0 ),
                                  rcsc::Vector2D( 3.0, 4.0 ) );

        const rcsc::Segment2D s2( rcsc::Vector2D( 0.0, 2.0 ),
                                  rcsc::Vector2D( 5.0, 2.0 ) );

        BOOST_CHECK( s1.existIntersectionExceptEndpoint( s2 ) );
        BOOST_CHECK( s2.existIntersectionExceptEndpoint( s1 ) );


        const rcsc::Segment2D s3( rcsc::Vector2D( 100.0, 200.0 ),
                                  rcsc::Vector2D( 300.0, 400.0 ) );

        BOOST_CHECK( ! s3.existIntersectionExceptEndpoint( s1 ) );
        BOOST_CHECK( ! s3.existIntersectionExceptEndpoint( s2 ) );
        BOOST_CHECK( ! s1.existIntersectionExceptEndpoint( s3 ) );
        BOOST_CHECK( ! s2.existIntersectionExceptEndpoint( s3 ) );


        // 2 segments on a line
        const rcsc::Segment2D s1_2( rcsc::Vector2D( 6.0,  8.0 ),
                                    rcsc::Vector2D( 9.0, 12.0 ) );

        BOOST_CHECK( ! s1.existIntersectionExceptEndpoint( s1_2 ) );
        BOOST_CHECK( ! s1_2.existIntersectionExceptEndpoint( s1 ) );

        const rcsc::Segment2D s4( rcsc::Vector2D( -100.0, 4.0 ),
                                  rcsc::Vector2D( +100.0, 4.0 ) );

        BOOST_CHECK( s1.existIntersection( s4 ) );
        BOOST_CHECK( s4.existIntersection( s1 ) );
    }



    //
    // check existIntersection()
    //
    {
        const rcsc::Segment2D t1( rcsc::Vector2D( 100, 100 ),
                                  rcsc::Vector2D(   0, 200 ) );

        const rcsc::Segment2D t2( rcsc::Vector2D( -100, 200 ),
                                  rcsc::Vector2D(  600, 200 ) );

        BOOST_CHECK( t1.existIntersection( t2 ) );
        BOOST_CHECK( t2.existIntersection( t1 ) );
    }

    // existIntersection at terminal points
    {
        const rcsc::Segment2D t1( rcsc::Vector2D( -200.0, -100.0 ),
                                  rcsc::Vector2D(    0.0, +100.0 ) );

        const rcsc::Segment2D t2( rcsc::Vector2D(    0.0, +100.0 ),
                                  rcsc::Vector2D( +200.0, -100.0 ) );

        const rcsc::Segment2D t_check( rcsc::Vector2D( 0.0, -300.0 ),
                                       rcsc::Vector2D( 0.0, +900.0 ) );

        BOOST_CHECK( t1.existIntersection( t_check ) );
        BOOST_CHECK( t_check.existIntersection( t1 ) );

        BOOST_CHECK( t2.existIntersection( t_check ) );
        BOOST_CHECK( t_check.existIntersection( t2 ) );
    }

    // intersects at terminal points
    {
        const rcsc::Segment2D t1( rcsc::Vector2D(  200, 100 ),
                                  rcsc::Vector2D( 2000, 100 ) );

        const rcsc::Segment2D t2( rcsc::Vector2D(  200, 100 ),
                                  rcsc::Vector2D(  200, 500 ) );

        BOOST_CHECK( t1.existIntersection( t2 ) );
        BOOST_CHECK( t2.existIntersection( t1 ) );
    }

    // intersects at terminal points (parallel, horizontal)
    {
        const rcsc::Segment2D t1( rcsc::Vector2D( +200, +100 ),
                                  rcsc::Vector2D( +500, +100 ) );

        const rcsc::Segment2D t2( rcsc::Vector2D( +200, +100 ),
                                  rcsc::Vector2D( -100, +100 ) );

        BOOST_CHECK( t1.existIntersection( t2 ) );
        BOOST_CHECK( t2.existIntersection( t1 ) );
    }

    // intersects with terminal points (parallel, vertical)
    {
            const rcsc::Segment2D t1( rcsc::Vector2D( +100, +200 ),
                                      rcsc::Vector2D( +100, +500 ) );

            const rcsc::Segment2D t2( rcsc::Vector2D( +100, +200 ),
                                      rcsc::Vector2D( +100, -100 ) );

            BOOST_CHECK( t1.existIntersection( t2 ) );
            BOOST_CHECK( t2.existIntersection( t1 ) );

            CHECK_DOUBLE_CLOSE( 0.0, t1.dist( t2 ) );
            CHECK_DOUBLE_CLOSE( 0.0, t2.dist( t1 ) );
    }

    // intersect with point segment 1
    {
            const rcsc::Segment2D t1( rcsc::Vector2D( 0,    0 ),
                                      rcsc::Vector2D( 0, +500 ) );

            const rcsc::Segment2D t2( rcsc::Vector2D( +100, +500 ),
                                      rcsc::Vector2D( +100, +500 ) );

            BOOST_CHECK( ! t1.existIntersection( t2 ) );
            BOOST_CHECK( ! t2.existIntersection( t1 ) );
    }

    // intersect with point segment 2
    {
            const rcsc::Segment2D t1( rcsc::Vector2D( +500, +500 ),
                                      rcsc::Vector2D( +500, +500 ) );

            const rcsc::Segment2D t2( rcsc::Vector2D( +300, +500 ),
                                      rcsc::Vector2D( +200, +400 ) );


            BOOST_CHECK( ! t1.existIntersection( t2 ) );
            BOOST_CHECK( ! t2.existIntersection( t1 ) );
    }

    // intersect with point segment 3
    {
            const rcsc::Segment2D t1( rcsc::Vector2D( +500, +500 ),
                                      rcsc::Vector2D( +500, +500 ) );

            const rcsc::Segment2D t2( rcsc::Vector2D( +300, +300 ),
                                      rcsc::Vector2D( +300, +300 ) );


            BOOST_CHECK( ! t1.existIntersection( t2 ) );
            BOOST_CHECK( ! t2.existIntersection( t1 ) );

            BOOST_CHECK( t1.existIntersection( t1 ) );
            BOOST_CHECK( t2.existIntersection( t2 ) );
    }

    // intersect with point segment 4
    {
            const rcsc::Segment2D t1( rcsc::Vector2D( +500, +500 ),
                                      rcsc::Vector2D( +500, +500 ) );

            const rcsc::Segment2D t2( rcsc::Vector2D(    0, +500 ),
                                      rcsc::Vector2D( +100, +500 ) );


            BOOST_CHECK( ! t1.existIntersection( t2 ) );
            BOOST_CHECK( ! t2.existIntersection( t1 ) );
    }

    // intersect with point segment 5
    {
            const rcsc::Segment2D t1( rcsc::Vector2D( +500, +500 ),
                                      rcsc::Vector2D( +500, +500 ) );

            const rcsc::Segment2D t2( rcsc::Vector2D( +500,    0 ),
                                      rcsc::Vector2D( +500, +100 ) );

            BOOST_CHECK( ! t1.existIntersection( t2 ) );
            BOOST_CHECK( ! t2.existIntersection( t1 ) );
    }



    //
    // check nearestPoint()
    //
    {
        const rcsc::Vector2D s1( -500, 100 );
        const rcsc::Vector2D s2( +500, 100 );

        const rcsc::Segment2D s( s1, s2 );

        CHECK_VECTOR2D_CLOSE
                ( rcsc::Vector2D( 0.0, 100.0 ),
                  s.nearestPoint( rcsc::Vector2D( 0.0, 0.0 ) ) );

        CHECK_VECTOR2D_CLOSE
                ( rcsc::Vector2D( 200.0, 100.0 ),
                  s.nearestPoint( rcsc::Vector2D( 200.0, 0.0 ) ) );

        for ( long i = 0 ; i < 100000 ; i += 10 )
        {
            const rcsc::Vector2D p( i, +500 );

            rcsc::Vector2D c;

            if ( i <= 500 )
            {
                c = s.nearestPoint( +p );
                CHECK_VECTOR2D_CLOSE( rcsc::Vector2D( (+p).x, 100 ), c );

                c = s.nearestPoint( -p );
                CHECK_VECTOR2D_CLOSE( rcsc::Vector2D( (-p).x, 100 ), c );
            }
            else
            {
                c = s.nearestPoint( +p );
                CHECK_VECTOR2D_CLOSE( s2, c );

                c = s.nearestPoint( -p );
                CHECK_VECTOR2D_CLOSE( s1, c );
            }
        }
    }



    //
    // check distance of segment and point
    //
    {
        const rcsc::Segment2D seg1( rcsc::Vector2D( -100.0, 0.0 ),
                                    rcsc::Vector2D(    0.0, 0.0 ) );
        const rcsc::Segment2D seg2( rcsc::Vector2D(    0.0, 0.0 ),
                                    rcsc::Vector2D( -100.0, 0.0 ) );

        const rcsc::Vector2D p( 400.0, 300.0 );

        CHECK_DOUBLE_CLOSE( 500.0, seg1.dist( p ) );
        CHECK_DOUBLE_CLOSE( 500.0, seg2.dist( p ) );
    }

    // distance from point (segment and point are on a line)
    {
        const rcsc::Segment2D seg( rcsc::Vector2D( -100, 0.0 ),
                                   rcsc::Vector2D( +100, 0.0 ) );

        const rcsc::Vector2D p( +150.0, 0.0 );

        CHECK_DOUBLE_CLOSE(  50.0, seg.dist( p ) );
        CHECK_DOUBLE_CLOSE( 250.0, seg.farthestDist( p ) );
    }

    // distance from point (complex)
    {
        const rcsc::Vector2D s1( -100, 0 );
        const rcsc::Vector2D s2( +100, 0 );

        const rcsc::Segment2D seg( s1, s2 );

        const rcsc::Vector2D p1( 0, +150 );

        CHECK_DOUBLE_CLOSE( 150.0, seg.dist( +p1 ) );
        CHECK_DOUBLE_CLOSE( 150.0, seg.dist( -p1 ) );

        const rcsc::Vector2D p2( 300, 0 );
        CHECK_DOUBLE_CLOSE( 200.0, seg.dist( +p2 ) );
        CHECK_DOUBLE_CLOSE( 200.0, seg.dist( -p2 ) );

        const rcsc::Vector2D p3( 20000, 0 );
        CHECK_DOUBLE_CLOSE( 19900.0, seg.dist( +p3 ) );
        CHECK_DOUBLE_CLOSE( 19900.0, seg.dist( -p3 ) );

        for ( long  i = 0  ;  i < 100000  ;  i += 10 )
        {
            const rcsc::Vector2D p( i, +500 );

            if ( i <= 100 )
            {
                CHECK_DOUBLE_CLOSE( 500.0, seg.dist( +p ) );

                CHECK_DOUBLE_CLOSE( 500.0, seg.dist( -p ) );
            }
            else
            {
                CHECK_DOUBLE_CLOSE( (s2 - p).r(), seg.dist( +p ) );

                CHECK_DOUBLE_CLOSE( (s1 - (-p)).r(), seg.dist( -p ) );
            }
        }
    }



    //
    // distance segment and segment
    //
    {
        const rcsc::Segment2D seg1( rcsc::Vector2D( +100.0, 100.0 ),
                                    rcsc::Vector2D( -100.0, 100.0 ) );

        const rcsc::Segment2D seg2( rcsc::Vector2D(    0.0, 300.0 ),
                                    rcsc::Vector2D( +100.0, 400.0 ) );

        CHECK_DOUBLE_CLOSE( 200.0, seg1.dist( seg2 ) );
        CHECK_DOUBLE_CLOSE( 200.0, seg2.dist( seg1 ) );
    }
}


test_suite *
init_unit_test_suite( int argc, char * argv[] )
{
    test_suite * test = BOOST_TEST_SUITE( "rcsc::Segment2D test" );

    test -> add( BOOST_TEST_CASE( &checkSegment2D ) );

    return test;
}
