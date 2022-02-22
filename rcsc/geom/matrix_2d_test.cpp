// -*-c++-*-

/*!
  \file matrix_2d_test.cpp
  \brief test code for rcsc::Matrix2D
*/

/*
 *Copyright:

 Copyright (C) Hidehisa Akiyama, Hiroki Shimora

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

#include "matrix_2d.h"

using namespace boost::unit_test_framework;

namespace {
static const double EPS = 1.0e-10;
}

static
void
translate_test()
{
    {
        rcsc::Matrix2D m;
        m.translate( 2.0, 4.0 );

        rcsc::Vector2D v( 0.0, 0.0 );
        m.transform( &v );

        BOOST_CHECK_SMALL( rcsc::Vector2D( 2.0, 4.0 ).dist( v ), EPS );
    }

    {
        rcsc::Matrix2D m = rcsc::Matrix2D::make_translation( 3.0, 4.0 );

        rcsc::Vector2D v( 0.0, 0.0 );
        m.transform( &v );

        BOOST_CHECK_SMALL( rcsc::Vector2D( 3.0, 4.0 ).dist( v ), EPS );
    }
}

static
void
scale_test()
{
    {
        rcsc::Matrix2D m;
        m.scale( 2.0, 4.0 );

        rcsc::Vector2D v1( 0.0, 0.0 );
        rcsc::Vector2D v2( 1.0, 1.0 );
        rcsc::Vector2D v3( 2.0, 3.0 );

        m.transform( &v1 );
        m.transform( &v2 );
        m.transform( &v3 );

        BOOST_CHECK_SMALL( rcsc::Vector2D( 0.0, 0.0 ).dist( v1 ), EPS );
        BOOST_CHECK_SMALL( rcsc::Vector2D( 2.0, 4.0 ).dist( v2 ), EPS );
        BOOST_CHECK_SMALL( rcsc::Vector2D( 4.0, 12.0 ).dist( v3 ), EPS );
    }

    {
        rcsc::Matrix2D m = rcsc::Matrix2D::make_scaling( -3.0, 1.0 );

        rcsc::Vector2D v1( 0.0, 0.0 );
        rcsc::Vector2D v2( -1.0, 1.0 );
        rcsc::Vector2D v3( 2.0, -3.0 );

        m.transform( &v1 );
        m.transform( &v2 );
        m.transform( &v3 );

        BOOST_CHECK_SMALL( rcsc::Vector2D( 0.0, 0.0 ).dist( v1 ), EPS );
        BOOST_CHECK_SMALL( rcsc::Vector2D( 3.0, 1.0 ).dist( v2 ), EPS );
        BOOST_CHECK_SMALL( rcsc::Vector2D( -6.0, -3.0 ).dist( v3 ), EPS );
    }
}

static
void
rotate_test()
{
    {
        rcsc::AngleDeg angle = 90.0;
        rcsc::Matrix2D m = rcsc::Matrix2D::make_rotation( angle );
        //rcsc::Matrix2D m;
        //m.rotate( 90.0 );

        rcsc::Vector2D v1( 0.0, 0.0 );
        rcsc::Vector2D v2( 1.0, 0.0 );
        rcsc::Vector2D v3( 0.0, 1.0 );
        rcsc::Vector2D v4( 2.0, 3.0 );

        rcsc::Vector2D rv1 = v1.rotatedVector( angle );
        rcsc::Vector2D rv2 = v2.rotatedVector( angle );
        rcsc::Vector2D rv3 = v3.rotatedVector( angle );
        rcsc::Vector2D rv4 = v4.rotatedVector( angle );

        m.transform( &v1 );
        m.transform( &v2 );
        m.transform( &v3 );
        m.transform( &v4 );

        BOOST_CHECK_SMALL( rcsc::Vector2D( 0.0, 0.0 ).dist( v1 ), EPS );
        BOOST_CHECK_SMALL( rcsc::Vector2D( 0.0, 1.0 ).dist( v2 ), EPS );
        BOOST_CHECK_SMALL( rcsc::Vector2D( -1.0, 0.0 ).dist( v3 ), EPS );
        BOOST_CHECK_SMALL( rcsc::Vector2D( -3.0, 2.0 ).dist( v4 ), EPS );

        BOOST_CHECK_SMALL( rv1.dist( v1 ), EPS );
        BOOST_CHECK_SMALL( rv2.dist( v2 ), EPS );
        BOOST_CHECK_SMALL( rv3.dist( v3 ), EPS );
        BOOST_CHECK_SMALL( rv4.dist( v4 ), EPS );    }

}

static
void
multiplication_test()
{
    {
        rcsc::Vector2D v( 3.5821, -292.23 );
        rcsc::Vector2D scale( 5.62, 92.092 );
        rcsc::Vector2D translate( 3.2, 5.4 );
        rcsc::AngleDeg rotate = 14.0;

        rcsc::Matrix2D m1;
        m1.rotate( rotate );
        m1.translate( translate.x, translate.y );
        m1.scale( scale.x, scale.y );

        rcsc::Matrix2D m2
            = rcsc::Matrix2D::make_scaling( scale.x, scale.y )
            * rcsc::Matrix2D::make_translation( translate.x, translate.y )
            * rcsc::Matrix2D::make_rotation( rotate );

        BOOST_CHECK_SMALL( m1.transform( v ).dist( m2.transform( v ) ), EPS );
    }
}

test_suite *
init_unit_test_suite( int argc, char * argv[] )
{
    test_suite * test = BOOST_TEST_SUITE( "rcsc::Matrix2D test" );

    test -> add( BOOST_TEST_CASE( &translate_test ) );
    test -> add( BOOST_TEST_CASE( &scale_test ) );
    test -> add( BOOST_TEST_CASE( &rotate_test ) );
    test -> add( BOOST_TEST_CASE( &multiplication_test ) );

    return test;
}
