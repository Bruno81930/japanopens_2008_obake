// -*-c++-*-

/*!
  \file segment_2d.cpp
  \brief 2D segment line class Source File.
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

/////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "segment_2d.h"
#include "triangle_2d.h"

#include <algorithm>
#include <iostream>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
Vector2D
Segment2D::intersection( const Segment2D & other ) const
{
    Line2D my_line = this->line();
    Line2D other_line = other.line();

    Vector2D tmp_sol = my_line.intersection( other_line );

    if ( ! tmp_sol.valid() )
    {
        return Vector2D::INVALIDATED;
    }

    // check if intersection point is on the line segment
    if ( ! this->contains( tmp_sol )
         || ! other.contains( tmp_sol ) )
    {
        return Vector2D::INVALIDATED;
    }

    return tmp_sol;

#if 0
    // Following algorithm seems faster ther abover method.
    // In fact, following algorithm slower...

    Vector2D ab = b() - a();
    Vector2D dc = other.a() - other.b();
    Vector2D ad = other.b() - a();

    double det = dc.outerProduct( ab );

    if ( std::fabs( det ) < 0.001 )
    {
        // area size is 0.
        // segments has same slope.
        std::cerr << "Segment2D::intersection()"
                  << " ***ERROR*** parallel segments"
                  << std::endl;
        return Vector2D::INVALIDATED;
    }

    double s = (dc.x * ad.y - dc.y * ad.x) / det;
    double t = (ab.x * ad.y - ab.y * ad.x) / det;

    if ( s < 0.0 || 1.0 < s || t < 0.0 || 1.0 < t )
    {
        return Vector2D::INVALIDATED;
    }

    return Vector2D( a().x + ab.x * s, a().y + ab.y * s );
#endif
}

/*-------------------------------------------------------------------*/
/*!

*/
Vector2D
Segment2D::intersection( const Line2D & other ) const
{
    Line2D my_line = this->line();

    Vector2D tmp_sol = my_line.intersection( other );

    if ( ! tmp_sol.valid() )
    {
        return Vector2D::INVALIDATED;
    }

    // check if intersection point is on the line segment
    if ( ! this->contains( tmp_sol ) )
    {
        return Vector2D::INVALIDATED;
    }

    return tmp_sol;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Segment2D::existIntersectionExceptEndpoint( const Segment2D & other ) const
{
    return    Triangle2D( *this, other.a() ).signedArea2()
            * Triangle2D( *this, other.b() ).signedArea2() < 0.0
         &&   Triangle2D( other, this -> a() ).signedArea2()
            * Triangle2D( other, this -> b() ).signedArea2() < 0.0;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Segment2D::existIntersection( const Segment2D & other ) const
{
    double a0 = Triangle2D( *this, other.a() ).signedArea2();
    double a1 = Triangle2D( *this, other.b() ).signedArea2();
    double b0 = Triangle2D( other, this -> a() ).signedArea2();
    double b1 = Triangle2D( other, this -> b() ).signedArea2();

    if ( a0 * a1 < 0.0 && b0 * b1 < 0.0 )
    {
        return true;
    }

    if ( this -> a() == this -> b() )
    {
        if ( other.a() == other.b() )
        {
            return this -> a() == other.a();
        }

        return b0 == 0.0 && other.checkIntersectsOnLine( this -> a() );
    }
    else if ( other.a() == other.b() )
    {
        return a0 == 0.0 && this -> checkIntersectsOnLine( other.a() );
    }


    if (    (a0 == 0.0 && this -> checkIntersectsOnLine( other.a() ))
         || (a1 == 0.0 && this -> checkIntersectsOnLine( other.b() ))
         || (b0 == 0.0 && other.checkIntersectsOnLine( this -> a() ))
         || (b1 == 0.0 && other.checkIntersectsOnLine( this -> b() )) )
    {
        return true;
    }

    return false;
}

/*-------------------------------------------------------------------*/
/*!

*/
// private
bool
Segment2D::checkIntersectsOnLine( const Vector2D & p ) const
{
    if ( a().x == b().x )
    {
        return (   (a().y <= p.y && p.y <= b().y)
                || (b().y <= p.y && p.y <= a().y) );
    }
    else
    {
        return (   (a().x <= p.x && p.x <= b().x)
                || (b().x <= p.x && p.x <= a().x) );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
Vector2D
Segment2D::nearestPoint( const Vector2D & p ) const
{
    const Vector2D vec = b() - a();

    const double len_square = vec.r2();

    if ( len_square == 0.0 )
    {
        return a();
    }


    double inner_product = vec.innerProduct( (p - a()) );

    //
    // A: p1 - p0
    // B: p - p0
    //
    // check if 0 <= |B|cos(theta) <= |A|
    //       -> 0 <= |A||B|cos(theta) <= |A|^2
    //       -> 0 <= A.B <= |A|^2  // A.B = |A||B|cos(theta)
    //
    if ( inner_product <= 0.0 )
    {
            return a();
    }
    else if ( inner_product >= len_square )
    {
            return b();
    }

    return a() + vec * inner_product / len_square;
}

/*-------------------------------------------------------------------*/
/*!

*/
double
Segment2D::dist( const Vector2D & p ) const
{
    double len = this -> length();

    if ( len == 0.0 )
    {
        return (p - a()).r();
    }

    const Vector2D vec = b() - a();
    const double prod = vec.innerProduct( p - a() );

    //
    // A: p1 - p0
    // A: p - p0
    //
    // check if 0 <= |B|cos(theta) <= |A|
    //       -> 0 <= |A||b|cos(theta) <= |A|^2
    //       -> 0 <= A.B <= |A|^2  // A.B = |A||B|cos(theta)
    //
    if ( 0.0 <= prod && prod <= len * len )
    {
        // return perpendicular distance
        return std::fabs( Triangle2D( *this, p ).signedArea2() / len );
    }

    return std::min( (p - a()).r(),
                     (p - b()).r() );
}

/*-------------------------------------------------------------------*/
/*!

*/
double
Segment2D::dist( const Segment2D &  seg ) const
{
    if ( this -> existIntersection( seg ) )
    {
        return 0.0;
    }

    return std::min( std::min( this -> dist( seg.a() ),
                               this -> dist( seg.b() ) ),
                     std::min( seg.dist( a() ),
                               seg.dist( b() ) ) );
}

/*-------------------------------------------------------------------*/
/*!

*/
double
Segment2D::farthestDist( const Vector2D & p ) const
{
    return std::max( (a() - p).r(), (b() - p).r() );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Segment2D::onSegment( const Vector2D & p ) const
{
    return Triangle2D( *this, p ).signedArea2() == 0.0
           && this -> checkIntersectsOnLine( p );
}

}
