// -*-c++-*-

/*
*Copyright:

Copyright Copyright (C) Hidehisa AKIYAMA

This code is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this code; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

*EndCopyright:
*/

/*
In this file, the code which is between "begin of the added code" 
and "end of the added code" 
has been added by Shogo TAKAGI
*/

/*
*Copyright:

Copyright (C) Shogo TAKAGI

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*EndCopyright:
*/

/////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <rcsc/geom/sector_2d.h>
#include <rcsc/common/server_param.h>

#include <rcsc/common/logger.h>
#include <rcsc/player/player_agent.h>
#include <rcsc/player/debug_client.h>

#include <rcsc/action/body_advance_ball.h>
#include <rcsc/action/body_kick_multi_step.h>
#include <rcsc/action/body_dribble.h>
#include <rcsc/action/body_hold_ball.h>
#include <rcsc/action/body_pass.h>
#include <rcsc/action/neck_scan_field.h>
#include <rcsc/action/neck_turn_to_low_conf_teammate.h>

#include "body_kick_to_corner.h"
#include "body_obake_pass.h"
#include "bhv_obake_action_strategy.h"
#include "bhv_obake_pass.h"
#include "body_obake_clear.h"
#include "obake_strategy.h"
#include "obake_analysis.h"
#include "bhv_basic_offensive_kick.h"


/*-------------------------------------------------------------------*/
/*!

*/
bool
Bhv_BasicOffensiveKick::execute( rcsc::PlayerAgent * agent )
{
    rcsc::dlog.addText( rcsc::Logger::TEAM,
                        "%s:%d: Bhv_BasicOffensiveKick"
                        ,__FILE__, __LINE__ );

    const rcsc::WorldModel & wm = agent->world();

    const rcsc::PlayerPtrCont & opps = wm.opponentsFromSelf();
    const rcsc::PlayerObject * nearest_opp
        = ( opps.empty()
            ? static_cast< rcsc::PlayerObject * >( 0 )
            : opps.front() );
    const double nearest_opp_dist = ( nearest_opp
                                      ? nearest_opp->distFromSelf()
                                      : 1000.0 );
    const rcsc::Vector2D nearest_opp_pos = ( nearest_opp
                                             ? nearest_opp->pos()
                                             : rcsc::Vector2D( -1000.0, 0.0 ) );

    /***************************/
    //begin of the added code
    /********************************************/
    /*test*/    

    if(wm.ball().pos().x < rcsc::ServerParam::i().theirPenaltyAreaLineX() - 7.5
       && wm.ball().pos().x - wm.offsideLineX() > 10.0)
    {
        /*
         if(wm.ball().pos().x - wm.offsideLineX() <= 10.0)
        {
            if(Bhv_ObakeDribbleStrategy().execute(agent))
            {
                return true;
                agent->setNeckAction( new rcsc::Neck_TurnToLowConfTeammate() );
            }
        }
        */
        if(Body_ObakePass().execute(agent))
        {
            return true;
            agent->setNeckAction( new rcsc::Neck_TurnToLowConfTeammate() );
        }
        
    }
    /**************/
/*    if((wm.ball().pos().x >= 39.0
        && wm.ball().pos().absY() <= rcsc::ServerParam::i().goalAreaHalfWidth())
*/
       /*|| (wm.ball().pos().x < rcsc::ServerParam::i().theirPenaltyAreaLineX()
           &&wm.ball().pos().x > wm.offsideLineX() - 7.5
           && wm.ball().pos().absY() <= rcsc::ServerParam::i().penaltyAreaHalfWidth())*//*)
    {
        if(Bhv_ObakeActionStrategy().checkStrategicDribble(agent))
        {
            return true;
        }
        }*/

    if(Bhv_ObakePass(agent,
                     M_previous_number).execute(agent))
    {
        return true;
        agent->setNeckAction( new rcsc::Neck_TurnToLowConfTeammate() );
    }
    const double defense_line = Obake_Analysis().getMateDefenseLine(agent);    
    double nearest_dist_criterion = 4.0;
    if(wm.ball().pos().x <= rcsc::ServerParam::i().ourPenaltyAreaLineX()
       && wm.ball().pos().absY() <= rcsc::ServerParam::i().goalAreaHalfWidth())
    {
        nearest_dist_criterion += 0.5;
    }
    if(nearest_opp_dist <= nearest_dist_criterion
       && (wm.ball().pos().x <= rcsc::ServerParam::i().ourPenaltyAreaLineX()
           || (wm.ball().pos().x < 0.0
               && std::abs(defense_line - wm.ball().pos().x) <= 10.0/*5.0*/)))
    {
        if(Body_ObakeClear().execute(agent))
        {
            return true;
            agent->setNeckAction( new rcsc::Neck_TurnToLowConfTeammate() );
        }
    }

    if(Bhv_ObakeActionStrategy().execute(agent))
    {
        return true;
    }

    /**************/

    /*if(nearest_opp_dist < 10.0)
    {
        if(Bhv_ObakeClear().execute(agent))
        {
            return true;
            agent->setNeckAction( new rcsc::Neck_TurnToLowConfTeammate() );
        }
    }*/

    /* if(Bhv_ObakePass(M_previous_number).execute(agent))
    {
        return true;
        agent->setNeckAction( new rcsc::Neck_TurnToLowConfTeammate() );
    }
    */
    /********************************************/
    //end of the added code

    if ( nearest_opp_dist < 7.0 )
    {
        if (Body_ObakePass().execute( agent ) )
        {
            rcsc::dlog.addText( rcsc::Logger::TEAM,
                                "%s:%d: do best pass"
                                ,__FILE__, __LINE__ );
            agent->debugClient().addMessage( "OffKickPass(2)" );
            agent->setNeckAction( new rcsc::Neck_TurnToLowConfTeammate() );
            return true;
        }
    }

    // dribble to my body dir
    if ( nearest_opp_dist < 5.0
         && nearest_opp_dist > ( rcsc::ServerParam::i().tackleDist()
                                 + rcsc::ServerParam::i().defaultPlayerSpeedMax() * 1.5 )
         && wm.self().body().abs() < 70.0 )
    {
        const rcsc::Vector2D body_dir_drib_target
            = wm.self().pos()
            + rcsc::Vector2D::polar2vector(5.0, wm.self().body());

        int max_dir_count = 0;
        wm.dirRangeCount( wm.self().body(), 20.0, &max_dir_count, NULL, NULL );

        if ( body_dir_drib_target.x < rcsc::ServerParam::i().pitchHalfLength() - 1.0
             && body_dir_drib_target.absY() < rcsc::ServerParam::i().pitchHalfWidth() - 1.0
             && max_dir_count < 3
             )
        {
            // check opponents
            // 10m, +-30 degree
            const rcsc::Sector2D sector( wm.self().pos(),
                                         0.5, 10.0,
                                         wm.self().body() - 30.0,
                                         wm.self().body() + 30.0 );
            // opponent check with goalie
            if ( ! wm.existOpponentIn( sector, 10, true ) )
            {
                rcsc::dlog.addText( rcsc::Logger::TEAM,
                                    "%s:%d: dribble to my body dir"
                                    ,__FILE__, __LINE__ );
                agent->debugClient().addMessage( "OffKickDrib(1)" );
                const double power = Bhv_ObakeActionStrategy().getDashPower(agent,
                                                                             body_dir_drib_target);
                rcsc::Body_Dribble( body_dir_drib_target,
                                    1.0,
                                    power,
                                    2
                                    ).execute( agent );
                agent->setNeckAction( new rcsc::Neck_TurnToLowConfTeammate() );
                return true;
            }
        }
    }

    rcsc::Vector2D drib_target( 50.0, wm.self().pos().absY() );
    if ( drib_target.y < 20.0 ) drib_target.y = 20.0;
    if ( drib_target.y > 29.0 ) drib_target.y = 27.0;
    if ( wm.self().pos().y < 0.0 ) drib_target.y *= -1.0;
    const rcsc::AngleDeg drib_angle = ( drib_target - wm.self().pos() ).th();

    // opponent is behind of me
    if ( nearest_opp_pos.x < wm.self().pos().x + 1.0 )
    {
        // check opponents
        // 15m, +-30 degree
        const rcsc::Sector2D sector( wm.self().pos(),
                                     0.5, 15.0,
                                     drib_angle - 30.0,
                                     drib_angle + 30.0 );
        // opponent check with goalie
        if ( ! wm.existOpponentIn( sector, 10, true ) )
        {
            const int max_dash_step
                = wm.self().playerType()
                .cyclesToReachDistance( wm.self().pos().dist( drib_target ) );
            if ( wm.self().pos().x > 35.0 )
            {
                drib_target.y *= ( 10.0 / drib_target.absY() );
            }

            rcsc::dlog.addText( rcsc::Logger::TEAM,
                                "%s:%d: fast dribble to (%.1f, %.1f) max_step=%d"
                                ,__FILE__, __LINE__,
                                drib_target.x, drib_target.y,
                                max_dash_step );
            agent->debugClient().addMessage( "OffKickDrib(2)" );
            const double power = Bhv_ObakeActionStrategy().getDashPower(agent,
                                                                         drib_target);
            rcsc::Body_Dribble( drib_target,
                                1.0,
                                power,
                                std::min( 5, max_dash_step )
                                ).execute( agent );
        }
        else
        {
            rcsc::dlog.addText( rcsc::Logger::TEAM,
                                "%s:%d: slow dribble to (%f, %f)"
                                ,__FILE__, __LINE__,
                                drib_target.x, drib_target.y );
            agent->debugClient().addMessage( "OffKickDrib(3)" );
            rcsc::Body_Dribble( drib_target,
                                1.0,
                                rcsc::ServerParam::i().maxPower(),
                                2
                                ).execute( agent );

        }
        agent->setNeckAction( new rcsc::Neck_TurnToLowConfTeammate() );
        return true;
    }

    // opp is far from me
    if ( nearest_opp_dist > 5.0 )
    {
        rcsc::dlog.addText( rcsc::Logger::TEAM,
                            "%s:%d: opp far. dribble(%f, %f)"
                            ,__FILE__, __LINE__,
                            drib_target.x, drib_target.y );
        agent->debugClient().addMessage( "OffKickDrib(4)" );
        const double power = Bhv_ObakeActionStrategy().getDashPower(agent,
                                                                     drib_target);
        rcsc::Body_Dribble( drib_target,
                            1.0,
                            power,
                            1
                            ).execute( agent );
        agent->setNeckAction( new rcsc::Neck_TurnToLowConfTeammate() );
        return true;
    }

    // opp is near

    // can pass
    if (Body_ObakePass().execute( agent ) )
    {
        rcsc::dlog.addText( rcsc::Logger::TEAM,
                            "%s:%d: pass"
                            ,__FILE__, __LINE__ );
        agent->debugClient().addMessage( "OffKickPass(3)" );
        agent->setNeckAction( new rcsc::Neck_TurnToLowConfTeammate() );
        return true;
    }

    // opp is far from me
    if ( nearest_opp_dist > 3.0 )
    {
        rcsc::dlog.addText( rcsc::Logger::TEAM,
                            "%s:%d: opp far. dribble(%f, %f)"
                            ,__FILE__, __LINE__,
                            drib_target.x, drib_target.y );
        agent->debugClient().addMessage( "OffKickDrib(5)" );
        const double power = Bhv_ObakeActionStrategy().getDashPower(agent,
                                                                    drib_target);
        rcsc::Body_Dribble( drib_target,
                            1.0,
                            power,
                            1
                            ).execute( agent );
        agent->setNeckAction( new rcsc::Neck_TurnToLowConfTeammate() );
        return true;
    }

    if ( nearest_opp_dist > 2.5 )
    {
        rcsc::dlog.addText( rcsc::Logger::TEAM,
                            "%s:%d: hold"
                            ,__FILE__, __LINE__ );
        agent->debugClient().addMessage( "OffKickHold" );
        rcsc::Body_HoldBall().execute( agent );
        agent->setNeckAction( new rcsc::Neck_TurnToLowConfTeammate() );
        return true;
    }

    if ( wm.self().pos().x > wm.offsideLineX() - 10.0 )
    {
        rcsc::dlog.addText( rcsc::Logger::TEAM,
                            "%s:%d: kick near side"
                            ,__FILE__, __LINE__ );
        agent->debugClient().addMessage( "OffKickToCorner" );
        Body_KickToCorner( (wm.self().pos().y < 0.0) ).execute( agent );
        agent->setNeckAction( new rcsc::Neck_ScanField() );
    }
    else
    {
        rcsc::dlog.addText( rcsc::Logger::TEAM,
                            "%s:%d: advance"
                            ,__FILE__, __LINE__ );
        agent->debugClient().addMessage( "OffKickAdvance" );
        rcsc::Body_AdvanceBall().execute( agent );
        agent->setNeckAction( new rcsc::Neck_ScanField() );
    }

    return true;
}

