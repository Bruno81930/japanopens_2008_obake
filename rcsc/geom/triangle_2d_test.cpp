// -*-c++-*-

/*!
  \file triangle_2d_test.cpp
  \brief test code for rcsc::Triangle2D
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

#include "triangle_2d.h"
#include "vector_2d.h"

using namespace boost::unit_test_framework;

#define CHECK_DOUBLE_CLOSE( d1, d2 ) \
        (BOOST_CHECK_SMALL( ((d2)-(d1)), EPS))


static void checkSignedArea()
{
    static const double EPS = 1.0e-10;

    //
    // basic checks
    //
    {
        const rcsc::Vector2D p1( 0.0, 0.0 );
        const rcsc::Vector2D p2( 3.0, 0.0 );
        const rcsc::Vector2D p3( 3.0, 4.0 );

        const rcsc::Triangle2D t1( p1, p2, p3 );
        const rcsc::Triangle2D t2( p3, p2, p1 );

        CHECK_DOUBLE_CLOSE( + 6.0, t1.signedArea() );
        CHECK_DOUBLE_CLOSE( +12.0, t1.signedArea2() );

        CHECK_DOUBLE_CLOSE( - 6.0, t2.signedArea() );
        CHECK_DOUBLE_CLOSE( -12.0, t2.signedArea2() );
    }


    //
    // points on a line
    //
    {
        const rcsc::Vector2D p1( -100, 200 );
        const rcsc::Vector2D p2(  600, 200 );
        const rcsc::Vector2D p3(    0, 200 );

        const rcsc::Triangle2D tri( p1, p2, p3 );

        // should be EXACTRY equal to 0
        BOOST_CHECK_EQUAL( 0.0, tri.signedArea2() );
    }


    //
    // same 2 points
    //
    {
        const rcsc::Vector2D p1( -100, 200 );
        const rcsc::Vector2D p2( + 50, 100 );

        const rcsc::Triangle2D tri1( p1, p1, p2 );
        const rcsc::Triangle2D tri2( p1, p2, p1 );
        const rcsc::Triangle2D tri3( p2, p1, p1 );

        // should be EXACTRY equal to 0
        BOOST_CHECK_EQUAL( 0.0, tri1.signedArea2() );
        BOOST_CHECK_EQUAL( 0.0, tri2.signedArea2() );
        BOOST_CHECK_EQUAL( 0.0, tri3.signedArea2() );
    }


    //
    // same 3 points
    //
    {
        const rcsc::Vector2D p( -100, 200 );

        const rcsc::Triangle2D tri( p, p, p );

        // should be EXACTRY equal to 0
        BOOST_CHECK_EQUAL( 0.0, tri.signedArea2() );
    }
}


test_suite *
init_unit_test_suite( int argc, char * argv[] )
{
    test_suite * test = BOOST_TEST_SUITE( "rcsc::Triangle2D test" );

    test -> add( BOOST_TEST_CASE( &checkSignedArea ) );

    return test;
}
