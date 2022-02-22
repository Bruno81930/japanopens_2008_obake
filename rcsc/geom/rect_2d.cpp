// -*-c++-*-

/*!
  \file rect_2d.cpp
  \brief 2D rectangle region Source File.
*/

/*
 *Copyright:

 Copyright (C) Hidehisa Akiyama

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

#include "rect_2d.h"

#include "segment_2d.h"
#include "ray_2d.h"

#include <iostream>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

 */
int
Rect2D::intersection( const Line2D & line,
                      Vector2D * sol1,
                      Vector2D * sol2 ) const
{
    int n_sol = 0;
    Vector2D tsol[2];

    const double left_x = left();
    const double right_x = right();
    const double top_y = top();
    const double bottom_y = bottom();

    if ( n_sol < 2
         && ( tsol[n_sol] = leftEdge().intersection( line ) ).valid()
         && top_y <= tsol[n_sol].y && tsol[n_sol].y <= bottom_y )
    {
        ++n_sol;
    }

    if ( n_sol < 2
         && ( tsol[n_sol] = rightEdge().intersection( line ) ).valid()
         && top_y <= tsol[n_sol].y && tsol[n_sol].y <= bottom_y )
    {
        ++n_sol;
    }

    if ( n_sol < 2
         && ( tsol[n_sol] = topEdge().intersection( line ) ).valid()
         && left_x <= tsol[n_sol].x && tsol[n_sol].x <= right_x )
    {
        ++n_sol;
    }

    if ( n_sol < 2
         && ( tsol[n_sol] = bottomEdge().intersection( line ) ).valid()
         && left_x <= tsol[n_sol].x && tsol[n_sol].x <= right_x )
    {
        ++n_sol;
    }

    if ( n_sol > 0
         && sol1 )
    {
        *sol1 = tsol[0];
    }

    if ( n_sol > 1
         && sol2 )
    {
        *sol2 = tsol[1];
    }

    return n_sol;
}

/*-------------------------------------------------------------------*/
/*!

 */
int
Rect2D::intersection( const Ray2D & ray,
                      Vector2D * sol1,
                      Vector2D * sol2 ) const
{
    Vector2D tsol1, tsol2;
    int n_sol = intersection( ray.line(), &tsol1, &tsol2 );

    if ( n_sol > 1
         && ! ray.inRightDir( tsol2, 1.0 ) )
    {
        --n_sol;
    }

    if ( n_sol > 0
         && ! ray.inRightDir( tsol1, 1.0 ) )
    {
        tsol1 = tsol2;
        --n_sol;
    }

    if ( n_sol > 0
         && sol1 )
    {
        *sol1 = tsol1;
    }

    if ( n_sol > 1
         && sol2 )
    {
        *sol2 = tsol2;
    }

    return n_sol;
}

/*-------------------------------------------------------------------*/
/*!

 */
int
Rect2D::intersection( const Segment2D & segment,
                      Vector2D * sol1,
                      Vector2D * sol2 ) const
{
    Vector2D tsol1, tsol2;
    int n_sol = intersection( segment.line(), &tsol1, &tsol2 );

    if ( n_sol > 1
         && ! segment.contains( tsol2 ) )
    {
        --n_sol;
    }

    if ( n_sol > 0
         && ! segment.contains( tsol1 ) )
    {
        tsol1 = tsol2;
        --n_sol;
    }

    if ( n_sol > 0
         && sol1 )
    {
        *sol1 = tsol1;
    }

    if ( n_sol > 1
         && sol2 )
    {
        *sol2 = tsol2;
    }

    return n_sol;
}


}
