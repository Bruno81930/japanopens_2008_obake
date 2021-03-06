// -*-c++-*-

/*!
  \file see_state.cpp
  \brief see synch state manager Source File
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

#include "see_state.h"

#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>

namespace rcsc {

bool SeeState::S_synch_see_mode = false;

/*-------------------------------------------------------------------*/
/*!

*/
SeeState::SeeState()
    : M_current_time( -1, 0 )
    , M_last_see_time( -1, 0 )
    , M_synch_type( SYNCH_NO )
    , M_last_timing( TIME_NOSYNCH )
    , M_current_see_count( 0 )
    , M_cycles_till_next_see( 100 )
    , M_view_width( ViewWidth::NORMAL )
    , M_view_quality( ViewQuality::HIGH )
{
    for ( int i = 0; i < HISTORY_SIZE; i++ )
    {
        M_see_count_history[i] = 0;
    }
}
/*-------------------------------------------------------------------*/
/*!

*/
SeeState::Timing
SeeState::getNextTiming( const ViewWidth & vw,
                         const ViewQuality & vq ) const
{
    if ( S_synch_see_mode )
    {
        return TIME_SYNC;
    }

    if ( vq.type() == ViewQuality::LOW )
    {
        return TIME_NOSYNCH;
    }

    Timing timing = TIME_NOSYNCH;

    switch ( lastTiming() ) {
    case TIME_0_00:
        switch ( vw.type() ) {
        case ViewWidth::WIDE:
            timing = TIME_0_00;
            break;
        case ViewWidth::NORMAL:
            timing = TIME_50_0;
            break;
        case ViewWidth::NARROW:
            // no synch
            break;
        default:
            break;
        }
        break;
    case TIME_50_0:
        switch ( vw.type() ) {
        case ViewWidth::WIDE:
            timing = TIME_50_0;
            break;
        case ViewWidth::NORMAL:
            timing = TIME_0_00;
            break;
        case ViewWidth::NARROW:
            timing = TIME_22_5;
            break;
        default:
            break;
        }
        break;
    case TIME_22_5:
        switch ( vw.type() ) {
        case ViewWidth::WIDE:
            timing = TIME_22_5;
            break;
        case ViewWidth::NORMAL:
            // no synch
            break;
        case ViewWidth::NARROW:
            timing = TIME_0_00;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    return timing;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
SeeState::updateBySenseBody( const GameTime & sense_time,
                             const ViewWidth & vw,
                             const ViewQuality & vq )
{
    setNewCycle( sense_time );

    if ( M_view_width != vw )
    {
        dlog.addText( Logger::SYSTEM,
                      "SeeState. updateBySenseBody. view_width does not match. old=%d sense=%d",
                      M_view_width.type(), vw.type() );
        std::cerr << sense_time
                  << " view width does not match . old=" << M_view_width.type()
                  << " sense=" << vw.type()
                  << std::endl;

        M_view_width = vw;
    }

    if ( M_view_quality != vq )
    {
        dlog.addText( Logger::SYSTEM,
                      "SeeState. updateBySenseBody. view_quality does not match. old=%d sense=%d",
                      M_view_quality.type(), vq.type() );
        std::cerr << sense_time
                  << " view quality does not match. old=" << M_view_quality.type()
                  << " sense=" << vq.type()
                  << std::endl;

        M_view_quality = vq;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
SeeState::updateBySee( const GameTime & see_time,
                       const ViewWidth & vw,
                       const ViewQuality & vq )
{
    // update counter
    if ( see_time == M_last_see_time )
    {
        M_current_see_count += 1;
        if ( isSynch() )
        {
            std::cerr << see_time
                      << "SeeState.updateSee. estimated synch, but duplicated"
                      << std::endl;
            M_last_timing = TIME_NOSYNCH;
        }
    }
    else
    {
        setNewCycle( see_time );
        M_last_see_time = see_time;
        M_current_see_count = 1;
    }

    // check view quality
    if ( vq == ViewQuality::LOW )
    {
        M_last_timing = TIME_NOSYNCH;
        return;
    }

    if ( ! isSynch() )
    {
        dlog.addText( Logger::SYSTEM,
                      "SeeState.updateSee. but no synch" );
        return;
    }

    //
    // see timing is synchronized.
    //

    if ( M_cycles_till_next_see > 0 )
    {
        M_cycles_till_next_see = 0;
        setViewMode( vw, vq );
    }

    // update current see arrival timing.
    Timing new_timing = getNextTiming( vw, vq );
    if ( new_timing == TIME_NOSYNCH )
    {
        std::cerr << see_time
                  << " Invalid view width. no synchronization... "
                  << std::endl;
    }
    dlog.addText( Logger::SYSTEM,
                  "SeeState. see update, prev timing = %d.  current timing = %d",
                  M_last_timing, new_timing );

    M_last_timing = new_timing;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
SeeState::setNewCycle( const GameTime & new_time )
{
    if ( new_time == M_current_time )
    {
        return;
    }
    M_current_time = new_time;

    if ( --M_cycles_till_next_see < 0 )
    {
        M_cycles_till_next_see = 0;
    }

    // std::rotate( [0], [1], [SIZE] );
    for ( int i = HISTORY_SIZE - 1; i > 0; --i )
    {
        M_see_count_history[i] = M_see_count_history[i-1];
    }
    M_see_count_history[0] = M_current_see_count;
    M_current_see_count = 0;

    dlog.addText( Logger::SYSTEM,
                  "SeeState. set new cycle. cycle till next see = %d",
                  M_cycles_till_next_see );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
SeeState::setLastSeeTiming( const Timing last_timing )
{
    dlog.addText( Logger::SYSTEM,
                  "SeeState.setTiming %d.",
                  last_timing );
    M_last_timing = last_timing;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
SeeState::isSynch() const
{
    if ( S_synch_see_mode )
    {
        return true;
    }

    if ( M_synch_type == SYNCH_SYNC )
    {
        return true;
    }

    return ( M_last_timing == TIME_0_00
             || M_last_timing == TIME_50_0
             || M_last_timing == TIME_22_5 );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
SeeState::isSynchedSeeCountNormalMode() const
{
    return ( M_see_count_history[0] == 2 // previous cycle
             && M_see_count_history[1] == 3 // 2 cycles before
             // && M_see_count_history[2] == 3 // 3 cycles before
             );
    // this condition means last see timing is TIME_0_00
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
SeeState::isSynchedSeeCountSynchMode() const
{
    if ( M_current_see_count == 2 // current cycle
         && M_see_count_history[0] == 3 // previous cycle
         && M_see_count_history[1] == 2 // 2 cycles before
         && M_see_count_history[2] == 3 // 3 cycles before
         )
    {
        // this means last see timing is TIME_50_0
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
SeeState::canChangeViewTo( const ViewWidth & next_width,
                           const GameTime & current ) const
{
    if ( current != M_last_see_time )
    {
        return false;
    }

    if ( S_synch_see_mode )
    {
        return true;
    }

    if ( next_width.type() == ViewWidth::NARROW )
    {
        if ( lastTiming() == TIME_0_00 )
        {
            return false;
        }
        return true;
    }

    if ( next_width.type() == ViewWidth::NORMAL )
    {
        switch ( lastTiming() ) {
        case TIME_0_00:
        case TIME_50_0:
            return true;
        default:
            return false;
        }

        return false;
    }

    if ( next_width.type() == ViewWidth::WIDE )
    {
        return true;
    }

    std::cerr << __FILE__ << ':' << __LINE__
              << " unexpected reach..." << std::endl;
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
int
SeeState::cyclesTillNextSee() const
{
    return M_cycles_till_next_see;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
SeeState::setViewMode( const ViewWidth & new_width,
                       const ViewQuality & new_quality )
{
    if ( M_last_see_time != M_current_time )
    {
        dlog.addText( Logger::SYSTEM,
                      "SeeState. setViewMode. no current cycle see arrival" );
        return;
    }

    M_view_width = new_width;
    M_view_quality = new_quality;

    if ( S_synch_see_mode )
    {
        switch ( new_width.type() ) {
        case ViewWidth::WIDE:
            M_cycles_till_next_see = 3;
            M_synch_type = SYNCH_WIDE;
            dlog.addText( Logger::SYSTEM,
                          "SeeState. setViewMode. synch wide: cycle = 3" );
            break;
        case ViewWidth::NORMAL:
            M_cycles_till_next_see = 2;
            M_synch_type = SYNCH_NORMAL;
            dlog.addText( Logger::SYSTEM,
                          "SeeState. setViewMode. synch normal: cycle = 2" );
            break;
        case ViewWidth::NARROW:
            M_cycles_till_next_see = 1;
            M_synch_type = SYNCH_NARROW;
            dlog.addText( Logger::SYSTEM,
                          "SeeState. setViewMode. synch narrow: cycle = 1" );
            break;
        default:
            break;
        }

        return;
    }

    // case 1
    if ( lastTiming() == TIME_0_00 )
    {
        switch ( new_width.type() ) {
        case ViewWidth::WIDE:
            M_cycles_till_next_see = 3;
            M_synch_type = SYNCH_WIDE;
            dlog.addText( Logger::SYSTEM,
                          "SeeState. setViewMode. 00:wide: cycle = %d",
                          M_cycles_till_next_see );
            break;
        case ViewWidth::NORMAL:
            M_cycles_till_next_see = 1;
            M_synch_type = SYNCH_EVERY;
            dlog.addText( Logger::SYSTEM,
                          "SeeState. setViewMode. 00:normal: cycle = %d",
                          M_cycles_till_next_see );
            break;
        case ViewWidth::NARROW:
            std::cerr << M_current_time
                      << " SeeState. TIME_0_00. Narrow is illegal."
                      << std::endl;
            M_synch_type = SYNCH_NO;
            break;
        default:
            break;
        }

        return;
    }

    // case 2
    if ( lastTiming() == TIME_50_0 )
    {
        switch ( new_width.type() ) {
        case ViewWidth::WIDE:
            M_cycles_till_next_see = 3;
            M_synch_type = SYNCH_WIDE;
            dlog.addText( Logger::SYSTEM,
                          "SeeState. setViewMode. 50:wide: cycle = %d",
                          M_cycles_till_next_see );
            break;
        case ViewWidth::NORMAL:
            M_cycles_till_next_see = 2;
            M_synch_type = SYNCH_NORMAL;
            dlog.addText( Logger::SYSTEM,
                          "SeeState. setViewMode. 50:normal: cycle = %d",
                          M_cycles_till_next_see );
            break;
        case ViewWidth::NARROW:
            M_cycles_till_next_see = 1;
            M_synch_type = SYNCH_EVERY;
            dlog.addText( Logger::SYSTEM,
                          "SeeState. setViewMode. 50:narrow: cycle = %d",
                          M_cycles_till_next_see );
            break;
        default:
            break;
        }

        return;
    }

    // case 3
    if ( lastTiming() == TIME_22_5 )
    {
        switch ( new_width.type() ) {
        case ViewWidth::WIDE:
            M_cycles_till_next_see = 3;
            M_synch_type = SYNCH_WIDE;
            break;
        case ViewWidth::NORMAL:
            std::cerr << "SeeState. TIME_22_5. Normal is illegal."
                      << std::endl;
            M_synch_type = SYNCH_NO;
            break;
        case ViewWidth::NARROW:
            M_cycles_till_next_see = 1;
            M_synch_type = SYNCH_EVERY;
            break;
        default:
            break;
        }
        return;
    }

    // case no synchronization...
    M_synch_type = SYNCH_NO;
    switch ( new_width.type() ) {
    case ViewWidth::WIDE:
        M_cycles_till_next_see = 3;
        break;
    case ViewWidth::NORMAL:
        M_cycles_till_next_see = 2;
        break;
    case ViewWidth::NARROW:
        M_cycles_till_next_see = 1;
        break;
    default:
        break;
    }

}

}
