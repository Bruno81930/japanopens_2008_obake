// -*-c++-*-

/*!
  \file bhv_scan_field.cpp
  \brief behavior to scan the field evenly.
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

#include "bhv_scan_field.h"

#include "basic_actions.h"
#include "neck_scan_field.h"

#include <rcsc/player/player_agent.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
bool
Bhv_ScanField::execute( PlayerAgent * agent )
{
    dlog.addText( Logger::ACTION,
                  "%s:%d: Bhf_ScanField"
                  ,__FILE__, __LINE__ );

    const WorldModel & wm = agent->world();

    if ( ! wm.self().posValid() )
    {
        dlog.addText( Logger::ACTION,
                      "%s:%d: invalid my pos"
                      ,__FILE__, __LINE__ );
        agent->doTurn( 60.0 );
        agent->setNeckAction( new Neck_TurnToRelative( 0.0 ) );
        return true;
    }

    if ( wm.ball().posValid() )
    {
        agent->doTurn( 90.0 );
        agent->setNeckAction( new Neck_ScanField() );
        return true;
    }

    /////////////////////////////////////////////////////////////

    // ball search mode

    if ( wm.seeTime() != wm.time() )
    {
        dlog.addText( Logger::ACTION,
                      "%s:%d: no current see info"
                      ,__FILE__, __LINE__ );
        agent->doTurn( 0.0 );
        agent->setNeckAction( new Neck_TurnToRelative( wm.self().neck() ) );
        return false;
    }

    // change to WIDE view mode
    // and only face to last ball seen pos or reverse side
    if ( wm.self().viewWidth() != ViewWidth::WIDE )
    {
        agent->doChangeView( ViewWidth::WIDE );
    }

    Vector2D mynext
        = wm.self().pos()
        + wm.self().vel();
    AngleDeg face_angle
        = ( wm.ball().seenPos().valid()
            ? ( wm.ball().seenPos() - mynext ).th()
            : ( -mynext ).th()  // face to field center
            );

    int search_flag = wm.ball().lostCount() / 3;
//         = ( agent->config().synchSee()
//             ? wm.ball().lostCount() / 3
//            : wm.ball().lostCount() / 2 );

    if ( search_flag % 2 )
    {
        face_angle += 180.0;
    }

    Vector2D face_point
        = mynext
        + Vector2D::polar2vector( 10.0, face_angle );

    dlog.addText( Logger::ACTION,
                  "%s:%d: ball search. lost_count=%d, search_flag=%d, angle=%f"
                  ,__FILE__, __LINE__,
                  wm.ball().lostCount(),
                  search_flag,
                  face_angle.degree() );
    return Bhv_NeckBodyToPoint( face_point ).execute( agent );
}

}
