// -*-c++-*-

/*!
  \file polygon_2d_test.cpp
  \brief test code for rcsc::Polygon2D
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

#include "polygon_2d.h"

#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <rcsc/geom/vector_2d.h>
#include <rcsc/geom/rect_2d.h>

#include <vector>
#include <cstdlib>


using namespace boost::unit_test_framework;

namespace {
static const double EPS = 1.0e-10;
}

#define CHECK_DOUBLE_CLOSE( d1, d2 ) \
        (BOOST_CHECK_SMALL( ((d2)-(d1)), EPS))

#define CHECK_VECTOR2D_CLOSE( v1, v2 ) \
        (BOOST_CHECK_SMALL( ((v2)-(v1)).r(), EPS))


static void checkPolygon2D()
{
    //
    // empty
    //
    {
        const rcsc::Polygon2D empty_polygon;

        BOOST_CHECK( !empty_polygon.contains( rcsc::Vector2D( 0.0, 0.0 ) ) );
    }


    //
    // point polygon
    //
    {
        const rcsc::Vector2D p( +100.0, +100.0 );

        std::vector< rcsc::Vector2D > v;
        v.push_back( p );

        const rcsc::Polygon2D point_polygon( v );

        BOOST_CHECK( !point_polygon.contains( rcsc::Vector2D( 0.0, 0.0 ) ) );

        // strict checks
        BOOST_CHECK(  point_polygon.contains( p ) );
        BOOST_CHECK( !point_polygon.contains( p, false ) );
    }


    std::vector< rcsc::Vector2D > rect;
    rect.push_back( rcsc::Vector2D( +200.0, +100.0 ) );
    rect.push_back( rcsc::Vector2D( -200.0, +100.0 ) );
    rect.push_back( rcsc::Vector2D( -200.0, -100.0 ) );
    rect.push_back( rcsc::Vector2D( +200.0, -100.0 ) );

    const rcsc::Polygon2D rectangle( rect );


    //
    // getBoundingBox()
    //
    {
        const rcsc::Rect2D r = rectangle.getBoundingBox();

        CHECK_DOUBLE_CLOSE( -200.0, r.minX() );
        CHECK_DOUBLE_CLOSE( +200.0, r.maxX() );
        CHECK_DOUBLE_CLOSE( -100.0, r.minY() );
        CHECK_DOUBLE_CLOSE( +100.0, r.maxY() );
    }


    //
    // contains
    //
    {
        BOOST_CHECK(  rectangle.contains( rcsc::Vector2D(    0.0,    0.0 ) ) );
        BOOST_CHECK(  rectangle.contains( rcsc::Vector2D(   50.0,   50.0 ) ) );
        BOOST_CHECK(  rectangle.contains( rcsc::Vector2D(  199.9,   99.9 ) ) );
        BOOST_CHECK(  rectangle.contains( rcsc::Vector2D( -199.9, - 99.9 ) ) );
        BOOST_CHECK( !rectangle.contains( rcsc::Vector2D(  200.1,  100.1 ) ) );
        BOOST_CHECK( !rectangle.contains( rcsc::Vector2D( -200.1, -100.1 ) ) );
        BOOST_CHECK( !rectangle.contains( rcsc::Vector2D( +500.0, +500.0 ) ) );
        BOOST_CHECK( !rectangle.contains( rcsc::Vector2D(    0.0, +500.0 ) ) );
    }


    //
    // contains 2
    //
    {
        std::vector< rcsc::Vector2D > tri;
        tri.push_back( rcsc::Vector2D( -200.0, -100.0 ) );
        tri.push_back( rcsc::Vector2D(    0.0, +100.0 ) );
        tri.push_back( rcsc::Vector2D( +200.0, -100.0 ) );

        const rcsc::Polygon2D triangle( tri );

        BOOST_CHECK(  triangle.contains( rcsc::Vector2D( 0.0,    0.0 ) ) );
        BOOST_CHECK( !triangle.contains( rcsc::Vector2D( 0.0, -300.0 ) ) );
        BOOST_CHECK( !triangle.contains( rcsc::Vector2D( 0.1, -300.0 ) ) );
    }


    //
    // contains 3
    //
    {
        std::vector< rcsc::Vector2D > tri2;
        tri2.push_back( rcsc::Vector2D(   0.0,   0.0 ) );
        tri2.push_back( rcsc::Vector2D( 100.0, 100.0 ) );
        tri2.push_back( rcsc::Vector2D(   0.0, 200.0 ) );

        const rcsc::Polygon2D triangle2( tri2 );

        BOOST_CHECK( !triangle2.contains( rcsc::Vector2D( -100.0, 100.0 ) ) );
        BOOST_CHECK(  triangle2.contains( rcsc::Vector2D(   50.0, 100.0 ) ) );
    }


    //
    // contains 4
    //
    {
        std::vector< rcsc::Vector2D > tri3;
        tri3.push_back( rcsc::Vector2D(   0.0,   0.0 ) );
        tri3.push_back( rcsc::Vector2D( 100.0, 100.0 ) );
        tri3.push_back( rcsc::Vector2D( 100.0, 100.0 ) );
        tri3.push_back( rcsc::Vector2D(   0.0, 200.0 ) );

        const rcsc::Polygon2D triangle3( tri3 );

        BOOST_CHECK( !triangle3.contains( rcsc::Vector2D( -100.0, 100.0 ) ) );
    }


    //
    // contains 5
    //
    {
        std::vector< rcsc::Vector2D > tri4;
        tri4.push_back( rcsc::Vector2D(   0.0,   0.0 ) );
        tri4.push_back( rcsc::Vector2D( 100.0, 100.0 ) );
        tri4.push_back( rcsc::Vector2D( 100.0, 100.0 ) );
        tri4.push_back( rcsc::Vector2D( 100.0, 100.0 ) );
        tri4.push_back( rcsc::Vector2D(   0.0, 200.0 ) );

        const rcsc::Polygon2D triangle4( tri4 );

        BOOST_CHECK( !triangle4.contains( rcsc::Vector2D( -100.0, 100.0 ) ) );
    }


    //
    // contains 6
    //
    {
        std::vector< rcsc::Vector2D > rect;
        rect.push_back( rcsc::Vector2D(  0,  0 ) );
        rect.push_back( rcsc::Vector2D( 10,  0 ) );
        rect.push_back( rcsc::Vector2D( 10, 10 ) );
        rect.push_back( rcsc::Vector2D(  0, 10 ) );

        const rcsc::Polygon2D r( rect );

        BOOST_CHECK( ! r.contains( rcsc::Vector2D( -100, 0 ) ) );
    }


    //
    // contains (grid)
    //
    {
        std::vector< rcsc::Vector2D > rect;
        rect.push_back( rcsc::Vector2D(  0,  0 ) );
        rect.push_back( rcsc::Vector2D( 10,  0 ) );
        rect.push_back( rcsc::Vector2D( 10, 10 ) );
        rect.push_back( rcsc::Vector2D(  0, 10 ) );

        const rcsc::Polygon2D r( rect );

        int count = 0;

        for ( int x = -100; x <= +100; ++x )
        {
            for ( int y = -100; y <= +100; ++y )
            {
                if (    0 <= x && x <= 10
                     && 0 <= y && y <= 10 )
                {
                    continue;
                }

                if ( r.contains( rcsc::Vector2D( x, y ) ) )
                {
                    ++count;
                }
            }
        }

        BOOST_CHECK_EQUAL( 0, count );
    }


    //
    // contains
    //
    {
        std::vector< rcsc::Vector2D > v;
        v.push_back( rcsc::Vector2D( 100, 100 ) );
        v.push_back( rcsc::Vector2D( 200, 100 ) );
        v.push_back( rcsc::Vector2D( 200, 500 ) );

        const rcsc::Polygon2D tri( v );

        //                    //
        //  po1               //
        //                    //
        //  po2          p5   //
        //              /|    //
        //             / |    //
        //            /  |    //
        //           /   |    //
        //          /    |    //
        //         /     |    //
        //  po3  p7  p1  p6   //
        //       /       |    //
        //  po4 p4---p2--p3   //
        //                    //
        //  po5               //

        rcsc::Vector2D p1( 150, 150 );
        rcsc::Vector2D p2( 150, 100 );
        rcsc::Vector2D p3( 200, 100 );
        rcsc::Vector2D p4( 100, 100 );
        rcsc::Vector2D p5( 200, 500 );
        rcsc::Vector2D p6( 200, 150 );
        rcsc::Vector2D p7( 200, 150 );

        rcsc::Vector2D po1( 50, 600 );
        rcsc::Vector2D po2( 50, 500 );
        rcsc::Vector2D po3( 50, 150 );
        rcsc::Vector2D po4( 50, 100 );
        rcsc::Vector2D po5( 50,   0 );


        BOOST_CHECK( tri.contains( p1 ) );
        BOOST_CHECK( tri.contains( p1, false ) );

        BOOST_CHECK(  tri.contains( p2 ) );
        BOOST_CHECK( !tri.contains( p2, false ) );

        BOOST_CHECK(  tri.contains( p3 ) );
        BOOST_CHECK( !tri.contains( p3, false ) );

        BOOST_CHECK(  tri.contains( p4 ) );
        BOOST_CHECK( !tri.contains( p4, false ) );

        BOOST_CHECK(  tri.contains( p5 ) );
        BOOST_CHECK( !tri.contains( p5, false ) );

        BOOST_CHECK(  tri.contains( p6 ) );
        BOOST_CHECK( !tri.contains( p6, false ) );

        BOOST_CHECK(  tri.contains( p7 ) );
        BOOST_CHECK( !tri.contains( p7, false ) );


        BOOST_CHECK( !tri.contains( po1 ) );
        BOOST_CHECK( !tri.contains( po1, false ) );

        BOOST_CHECK( !tri.contains( po2 ) );
        BOOST_CHECK( !tri.contains( po2, false ) );

        BOOST_CHECK( !tri.contains( po3 ) );
        BOOST_CHECK( !tri.contains( po3, false ) );

        BOOST_CHECK( !tri.contains( po4 ) );
        BOOST_CHECK( !tri.contains( po4, false ) );

        BOOST_CHECK( !tri.contains( po5 ) );
        BOOST_CHECK( !tri.contains( po5, false ) );
    }


    //
    // empty area
    //
    {
        std::vector< rcsc::Vector2D > a0;
        a0.push_back( rcsc::Vector2D( 100.0, 100.0 ) );
        a0.push_back( rcsc::Vector2D( 100.0, 100.0 ) );
        a0.push_back( rcsc::Vector2D( 100.0, 100.0 ) );
        a0.push_back( rcsc::Vector2D( 100.0, 100.0 ) );
        a0.push_back( rcsc::Vector2D( 100.0, 100.0 ) );

        const rcsc::Polygon2D area_1( a0 );

        a0.push_back( rcsc::Vector2D( 100.0, 100.0 ) );
        const rcsc::Polygon2D area_2( a0 );


        BOOST_CHECK( !area_1.contains( rcsc::Vector2D( 0.0, 0.0 ) ) );
        BOOST_CHECK( !area_2.contains( rcsc::Vector2D( 0.0, 0.0 ) ) );

        // strict checks
        BOOST_CHECK(  area_1.contains( rcsc::Vector2D( 100.0, 100.0 ) ) );
        BOOST_CHECK( !area_1.contains( rcsc::Vector2D( 100.0, 100.0 ), false ) );

        // strict checks
        BOOST_CHECK(  area_2.contains( rcsc::Vector2D( 100.0, 100.0 ) ) );
        BOOST_CHECK( !area_2.contains( rcsc::Vector2D( 100.0, 100.0 ), false ) );
    }


    //
    // scissoring
    //
    {
        const rcsc::Rect2D rectangle( -100, -100,
                                      /* length of x */ 200, /* length of y */ 200 );

        //                         //
        //              (200,200)  //
        //           +---------+   //
        //           |         |   //
        //    -100   |         |   //
        // +100 +----|----+    |   //
        //      |    |    |    |   //
        //      |    |    |    |   //
        //      |    +---------+   //
        //      |   (0,0) |        //
        //      |         |        //
        // -100 +---------+        //
        //                         //

        std::vector< rcsc::Vector2D > v;
        v.push_back( rcsc::Vector2D(   0,   0 ) );
        v.push_back( rcsc::Vector2D( 200,   0 ) );
        v.push_back( rcsc::Vector2D( 200, 200 ) );
        v.push_back( rcsc::Vector2D(   0, 200 ) );
        v.push_back( rcsc::Vector2D(   0,   0 ) );

        const rcsc::Polygon2D polygon( v );

        const rcsc::Polygon2D result = polygon.getScissoredConnectedPolygon( rectangle );

        CHECK_DOUBLE_CLOSE( 10000.0, result.area() );

        const rcsc::Rect2D bbox = result.getBoundingBox();

        CHECK_DOUBLE_CLOSE(   0.0, bbox.minX() );
        CHECK_DOUBLE_CLOSE( 100.0, bbox.maxX() );
        CHECK_DOUBLE_CLOSE(   0.0, bbox.minY() );
        CHECK_DOUBLE_CLOSE( 100.0, bbox.maxY() );
    }


    //
    // get_distance
    //
    {
        std::vector< rcsc::Vector2D > rect;
        rect.push_back( rcsc::Vector2D(  0,  0 ) );
        rect.push_back( rcsc::Vector2D( 10,  0 ) );
        rect.push_back( rcsc::Vector2D( 10, 10 ) );
        rect.push_back( rcsc::Vector2D(  0, 10 ) );

        const rcsc::Polygon2D r( rect );

        // out of polygon
        CHECK_DOUBLE_CLOSE( 1.0, r.dist( rcsc::Vector2D( 11.0, 10.0 ) ) );

        // in polygon, check as plane
        CHECK_DOUBLE_CLOSE( 0.0, r.dist( rcsc::Vector2D( 5.0, 5.0 ) ) );

        // in polygon, check as polyline
        CHECK_DOUBLE_CLOSE( 5.0, r.dist( rcsc::Vector2D( 5.0, 5.0 ), false ) );
    }


    //
    // area, xyCenter
    //
    {
        std::vector< rcsc::Vector2D > rect;
        rect.push_back( rcsc::Vector2D( 10, 10 ) );
        rect.push_back( rcsc::Vector2D( 20, 10 ) );
        rect.push_back( rcsc::Vector2D( 20, 20 ) );
        rect.push_back( rcsc::Vector2D( 10, 20 ) );

        const rcsc::Polygon2D r( rect );

        CHECK_DOUBLE_CLOSE( +100.0, r.area() );
        CHECK_DOUBLE_CLOSE( +200.0, r.signedArea2() );

        CHECK_VECTOR2D_CLOSE( rcsc::Vector2D( 15.0, 15.0 ),
                              r.xyCenter() );
    }


    //
    // counter clockwise/clockwise, signedArea2
    //
    {
        std::vector< rcsc::Vector2D > points;
        const rcsc::Polygon2D empty(points);

        points.push_back( rcsc::Vector2D( 10, 10 ) );
        const rcsc::Polygon2D point(points);

        points.push_back( rcsc::Vector2D( 20, 10 ) );
        const rcsc::Polygon2D line(points);

        points.push_back( rcsc::Vector2D( 20, 20 ) );
        const rcsc::Polygon2D triangle(points);

        points.push_back( rcsc::Vector2D( 10, 20 ) );
        const rcsc::Polygon2D rectangle(points);

        CHECK_DOUBLE_CLOSE(    0.0, empty    .signedArea2() );
        CHECK_DOUBLE_CLOSE(    0.0, point    .signedArea2() );
        CHECK_DOUBLE_CLOSE(    0.0, line     .signedArea2() );
        CHECK_DOUBLE_CLOSE( +100.0, triangle .signedArea2() );
        CHECK_DOUBLE_CLOSE( +200.0, rectangle.signedArea2() );


        BOOST_CHECK_EQUAL( false, empty.isCounterclockwise() );
        BOOST_CHECK_EQUAL( false, empty.isClockwise() );

        BOOST_CHECK_EQUAL( false, point.isCounterclockwise() );
        BOOST_CHECK_EQUAL( false, point.isClockwise() );

        BOOST_CHECK_EQUAL( false, line.isCounterclockwise() );
        BOOST_CHECK_EQUAL( false, line.isClockwise() );

        BOOST_CHECK_EQUAL( true , triangle.isCounterclockwise() );
        BOOST_CHECK_EQUAL( false, triangle.isClockwise() );

        BOOST_CHECK_EQUAL( true , triangle.isCounterclockwise() );
        BOOST_CHECK_EQUAL( false, triangle.isClockwise() );


        std::vector< rcsc::Vector2D > r_points;
        r_points.push_back( rcsc::Vector2D( 10, 20 ) );
        r_points.push_back( rcsc::Vector2D( 20, 20 ) );
        r_points.push_back( rcsc::Vector2D( 20, 10 ) );
        const rcsc::Polygon2D r_triangle(r_points);

        r_points.push_back( rcsc::Vector2D( 10, 10 ) );
        const rcsc::Polygon2D r_rectangle(r_points);

        CHECK_DOUBLE_CLOSE( -100.0, r_triangle .signedArea2() );
        CHECK_DOUBLE_CLOSE( -200.0, r_rectangle.signedArea2() );

        BOOST_CHECK_EQUAL( false, r_triangle.isCounterclockwise() );
        BOOST_CHECK_EQUAL( true , r_triangle.isClockwise() );

        BOOST_CHECK_EQUAL( false, r_rectangle.isCounterclockwise() );
        BOOST_CHECK_EQUAL( true , r_rectangle.isClockwise() );
    }
}


test_suite *
init_unit_test_suite( int argc, char * argv[] )
{
    test_suite * test = BOOST_TEST_SUITE( "rcsc::Polygon2D test" );

    test -> add( BOOST_TEST_CASE( &checkPolygon2D ) );

    return test;
}
