// -*-c++-*-

/*!
  \file neck_scan_field.cpp
  \brief scan field with neck evenly
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

#include "neck_scan_field.h"

#include "basic_actions.h"

#include <rcsc/player/player_agent.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include <rcsc/geom/rect_2d.h>

#include <numeric>
#include <deque>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!
  scan field by turn_neck
*/
bool
Neck_ScanField::execute( PlayerAgent * agent )
{
    static GameTime s_last_calc_time( 0, 0 );
    static ViewWidth s_last_calc_view_width = ViewWidth::NORMAL;
    static AngleDeg s_cached_target_angle( 0.0 );

    if ( s_last_calc_time != agent->world().time()
         || s_last_calc_view_width != agent->effector().queuedNextViewWidth() )
    {
        s_last_calc_time = agent->world().time();
        s_last_calc_view_width = agent->effector().queuedNextViewWidth();

        s_cached_target_angle = calcAngle( agent );
    }

    return agent->doTurnNeck( s_cached_target_angle
                              - agent->effector().queuedNextMyBody()
                              - agent->world().self().neck() );
}

/*-------------------------------------------------------------------*/
/*!

*/
AngleDeg
Neck_ScanField::calcAngle( const PlayerAgent * agent )
{
    static const Rect2D pitch_rect( -ServerParam::i().pitchHalfLength(),
                                    -ServerParam::i().pitchHalfWidth(),
                                    ServerParam::i().pitchLength(),
                                    ServerParam::i().pitchWidth() );
    static const Rect2D expand_pitch_rect( -ServerParam::i().pitchHalfLength() - 3.0,
                                           -ServerParam::i().pitchHalfWidth() - 3.0,
                                           ServerParam::i().pitchLength() + 6.0,
                                           ServerParam::i().pitchWidth() + 6.0 );
    static const Rect2D goalie_rect( ServerParam::i().pitchHalfLength() - 3.0,
                                     -15.0,
                                     10.0,
                                     30.0 );

    const WorldModel & wm = agent->world();

    const double next_view_width = agent->effector().queuedNextViewWidth().width();

    const AngleDeg left_start
        = agent->effector().queuedNextMyBody()
        + ( ServerParam::i().minNeckAngle() - ( next_view_width * 0.5 ) );
    const double scan_range
        = ( ( ServerParam::i().maxNeckAngle() - ServerParam::i().minNeckAngle() )
            + next_view_width );

    dlog.addText( Logger::ACTION,
                  "%s:%d: next_left_limit=%.0f, next_neck_range=%.0f"
                  ,__FILE__, __LINE__,
                  left_start.degree(), scan_range );

    const double shrinked_scan_range = scan_range - WorldModel::DIR_STEP * 1.5;
    const double shrinked_next_view_width = next_view_width - WorldModel::DIR_STEP * 1.5;

    AngleDeg sol_angle = left_start + scan_range * 0.5;

    if ( scan_range < shrinked_next_view_width )
    {
        dlog.addText( Logger::ACTION,
                      "%s:%d: scan reange is smaller than next view width."
                      ,__FILE__, __LINE__ );
        return sol_angle;
    }


    AngleDeg tmp_angle = left_start;

    const std::size_t size_of_view_width
        = static_cast< std::size_t >
        ( rint( shrinked_next_view_width / WorldModel::DIR_STEP ) );

    std::deque< int > dir_counts( size_of_view_width );

    // generate first visible cone score list
    {

        const std::deque< int >::iterator end = dir_counts.end();
        for ( std::deque< int >::iterator it = dir_counts.begin();
              it != end;
              ++it )
        {
            *it = wm.dirCount( tmp_angle );
            tmp_angle += WorldModel::DIR_STEP;
        }
    }

    int max_count_sum = 0;
    double add_dir = shrinked_next_view_width;

    dlog.addText( Logger::ACTION,
                  "%s:%d loop start. left_start=%.0f shrinked_can_range=%.0f"
                  ,__FILE__, __LINE__,
                  left_start.degree(),
                  shrinked_scan_range );

    const Vector2D my_next = agent->effector().queuedNextMyPos();

    do
    {
        int tmp_count_sum = std::accumulate( dir_counts.begin(), dir_counts.end(), 0 );

        AngleDeg angle = tmp_angle - shrinked_next_view_width * 0.5;
#ifdef DEBUG
        dlog.addText( Logger::ACTION,
                      "___ angle=%.0f tmp_count_sum=%d",
                      angle.degree(),
                      tmp_count_sum );
#endif
        if ( tmp_count_sum > max_count_sum )
        {
            bool update = true;
            {
                Vector2D face_point
                    = my_next
                    + Vector2D::polar2vector( 20.0, angle );
                if ( ! pitch_rect.contains( face_point )
                     && ! goalie_rect.contains( face_point ) )
                {
                    update = false;
                }
            }

            if ( update )
            {
                Vector2D left_face_point
                    = my_next
                    + Vector2D::polar2vector( 20.0, angle - next_view_width*0.5 );
                if ( ! expand_pitch_rect.contains( left_face_point )
                     && ! goalie_rect.contains( left_face_point ) )
                {
                    update = false;
                }
            }

            if ( update )
            {
                Vector2D right_face_point
                    = my_next
                    + Vector2D::polar2vector( 20.0, angle + next_view_width*0.5 );
                if ( ! expand_pitch_rect.contains( right_face_point )
                     && ! goalie_rect.contains( right_face_point ) )
                {
                    update = false;
                }
            }

            if ( update )
            {
#ifdef DEBUG
                dlog.addText( Logger::ACTION,
                              "___-- updated" );
#endif
                sol_angle = angle;
                max_count_sum = tmp_count_sum;
            }
        }

        dir_counts.pop_front();
        add_dir += WorldModel::DIR_STEP;
        tmp_angle += WorldModel::DIR_STEP;
        dir_counts.push_back( wm.dirCount( tmp_angle ) );
    }
    while ( add_dir <= scan_range );


    dlog.addText( Logger::ACTION,
                  "%s:%d: target angle = %.0f"
                  ,__FILE__, __LINE__,
                  sol_angle.degree() );

    return sol_angle;
}

}
