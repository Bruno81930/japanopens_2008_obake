// -*-c++-*-

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA

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

/////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <rcsc/geom/rect_2d.h>
#include <rcsc/common/server_param.h>

#include <rcsc/common/logger.h>
#include <rcsc/player/player_agent.h>
#include <rcsc/player/intercept_table.h>
#include <rcsc/player/debug_client.h>
#include <rcsc/player/world_model.h>
#include <rcsc/formation/formation.h>
#include <rcsc/action/body_clear_ball.h>
#include <rcsc/action/neck_scan_field.h>
#include <rcsc/action/intention_kick.h>
#include <rcsc/action/body_go_to_point.h>
#include "bhv_goalie_basic_move.h"
#include "bhv_goalie_chase_ball.h"
#include "bhv_goalie_free_kick.h"
#include "obake_fuzzy_grade.h"

#include "obake_analysis.h"

#include "role_goalie.h"

/*-------------------------------------------------------------------*/
/*!

 */
void
RoleGoalie::execute( rcsc::PlayerAgent * agent )
{
    static const
        rcsc::Rect2D our_penalty( -rcsc::ServerParam::i().pitchHalfLength(),
                                  -rcsc::ServerParam::i().penaltyAreaHalfWidth() + 1.0,
                                  rcsc::ServerParam::i().penaltyAreaLength() - 1.0,
                                  rcsc::ServerParam::i().penaltyAreaWidth() - 2.0 );

    //////////////////////////////////////////////////////////////
    // play_on play

    // catchable
    if ( agent->world().time().cycle() > M_last_goalie_kick_time.cycle() + 5
         && agent->world().ball().distFromSelf() < agent->world().self().catchableArea() - 0.05
         && our_penalty.contains( agent->world().ball().pos() ) )
    {
        rcsc::dlog.addText( rcsc::Logger::ROLE,
                            "%s:%d: catchable. ball dist=%f, my_catchable=%f"
                            ,__FILE__, __LINE__,
                            agent->world().ball().distFromSelf(),
                            agent->world().self().catchableArea() );
        agent->doCatch();
    }
    else if ( agent->world().self().isKickable() )
    {
        doKick( agent );
    }
    else
    {
        doMove( agent );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
RoleGoalie::doKick( rcsc::PlayerAgent * agent )
{
    rcsc::Body_ClearBall().execute( agent );
    agent->setNeckAction( new rcsc::Neck_ScanField() );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
RoleGoalie::doMove( rcsc::PlayerAgent * agent )
{

    if ( Bhv_GoalieChaseBall::is_ball_chase_situation( agent ) )
    {
        Bhv_GoalieChaseBall().execute( agent );
    }
    else
    {
        const rcsc::WorldModel & wm = agent->world();
        const double pitch_half_length = rcsc::ServerParam::i().pitchLength() / 2;
        const double near_penalty_area_x = -pitch_half_length + 2.5;
        const double far_penalty_area_x = rcsc::ServerParam::i().ourPenaltyAreaLineX() - 5.0;
        const double far_goal_y = (wm.ball().pos().y / wm.ball().pos().absY())
	    * rcsc::ServerParam::i().goalAreaHalfWidth();
        const double degree_near_penalty_area_x = Obake_FuzzyGrade().degreeNearMatePenaltyAreaX(wm.ball().pos().x);
        //const double degree_far_penalty_area_x = Obake_FuzzyGrade().degreeFarMatePenaltyAreaX(wm.ball().pos().x);
        const double degree_very_far_penalty_area_x
            = std::pow(Obake_FuzzyGrade().degreeFarMatePenaltyAreaX(wm.ball().pos().x), 2.0);
        const double degree_far_goal_y = Obake_FuzzyGrade().degreeFarMateGoalY(wm.ball().pos().y);
        const double sum = degree_near_penalty_area_x + degree_very_far_penalty_area_x;//degree_far_penalty_area_x;
        const double move_pos_x = (near_penalty_area_x * degree_near_penalty_area_x
                                   + far_penalty_area_x * degree_very_far_penalty_area_x) / sum;
        const double move_pos_y = far_goal_y * degree_far_goal_y;
        const rcsc::Vector2D move_pos(move_pos_x, move_pos_y);
        Bhv_GoalieBasicMove(move_pos).execute( agent );
    }
}
