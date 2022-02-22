// -*-c++-*-

/*!
  \file body_pass.cpp
  \brief advanced pass planning & behavior.
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

#include "body_obake_pass.h"

#include <rcsc/action/intention_kick.h>
#include <rcsc/action/body_kick_two_step.h>
#include <rcsc/action/body_kick_multi_step.h>
#include <rcsc/action/body_stop_ball.h>
#include <rcsc/action/body_hold_ball.h>

#include <rcsc/player/interception.h>
#include <rcsc/common/logger.h>
#include <rcsc/player/player_agent.h>
#include <rcsc/player/debug_client.h>
#include <rcsc/player/audio_sensor.h>
#include <rcsc/player/say_message_builder.h>

#include "obake_analysis.h"
#include <rcsc/common/server_param.h>
#include <rcsc/geom/rect_2d.h>
#include <rcsc/geom/sector_2d.h>
#include <rcsc/soccer_math.h>
#include <rcsc/math_util.h>
#include "obake_strategy.h"
#include "obake_fuzzy_grade.h"
#include "obake_analysis.h"
#include <fstream>
#define DEBUG


std::vector< Body_ObakePass::PassRoute > Body_ObakePass::S_cached_pass_route;

/*-------------------------------------------------------------------*/
/*!
  execute action
*/
bool
Body_ObakePass::execute(rcsc::PlayerAgent * agent)
{
    rcsc::dlog.addText( rcsc::Logger::ACTION,
                  "%s:%d: Body_ObakePass. execute()"
                  ,__FILE__, __LINE__ );
    const rcsc::WorldModel & wm = agent->world();
    if ( ! wm.self().isKickable() )
    {
        std::cerr << __FILE__ << ": " << __LINE__
                  << " not ball kickable!"
                  << std::endl;
        rcsc::dlog.addText( rcsc::Logger::ACTION,
			    "%s:%d:  not kickable"
			    ,__FILE__, __LINE__ );
        return false;
    }

    rcsc::Vector2D target_point(50.0, 0.0);
    double first_speed = 0.0;
    int receiver = 0;
    double score = 0.0;
    bool can_assist, can_shoot;
    if ( ! get_best_pass(agent, &target_point, &first_speed, &receiver, &score, &can_shoot, &can_assist) )
    {
        return false;
    }


    // evaluation
    //   judge situation
    //   decide max kick step
    //

    agent->debugClient().addMessage( "pass" );
    agent->debugClient().setTarget( target_point );

    rcsc::Body_KickMultiStep( target_point,
                              first_speed,
                              false
	).execute( agent ); // not enforce

    rcsc::dlog.addText( rcsc::Logger::ACTION,
			"%s:%d: execute() register pass intention"
			,__FILE__, __LINE__ );

    if ( wm.gameMode().type() != rcsc::GameMode::PlayOn )
    {
        agent->setIntention
            (new rcsc::IntentionKick(target_point,
                                     first_speed,
                                     3, // max kick step
                                     false, // not enforce
                                     wm.time() ) );
    }

    if ( agent->config().useCommunication()
         && receiver != Unum_Unknown )
    {
        rcsc::dlog.addText( rcsc::Logger::ACTION,
			    "%s:%d: execute() set pass communication."
			    ,__FILE__, __LINE__ );
        rcsc::Vector2D target_buf = target_point - wm.self().pos();
        target_buf.setLength( 1.0 );

        agent->addSayMessage( new rcsc::PassMessage( receiver,
                                                     target_point + target_buf,
                                                     agent->effector().queuedNextBallPos(),
                                                     agent->effector().queuedNextBallVel() ) );
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!
  static method
*/
bool
Body_ObakePass::get_best_pass(rcsc::PlayerAgent * agent,
			      rcsc::Vector2D * target_point,
			      double * first_speed,
			      int * receiver,
                              double * score,
                              bool * can_shoot,
                              bool * can_assist)
{
    const rcsc::WorldModel & wm = agent->world();
    static rcsc::GameTime S_last_calc_time( 0, 0 );
    static bool S_last_calc_valid = false;
    static rcsc::Vector2D S_last_calc_target;
    static double S_last_calc_speed = 0.0;
    static int S_last_calc_receiver = Unum_Unknown;
    *score = 0.0;
    if ( S_last_calc_time == wm.time() )
    {
        if ( S_last_calc_valid )
        {
            if ( target_point )
            {
                *target_point = S_last_calc_target;
            }
            if ( first_speed )
            {
                *first_speed = S_last_calc_speed;
            }
            if ( receiver )
            {
                *receiver = S_last_calc_receiver;
            }
            return true;
        }
        return false;
    }

    S_last_calc_time = wm.time();
    S_last_calc_valid = false;

    // create route
    create_routes(agent);

    if ( ! S_cached_pass_route.empty() )
    {
        std::vector< PassRoute >::iterator max_it
            = std::max_element( S_cached_pass_route.begin(),
                                S_cached_pass_route.end(),
                                PassRouteScoreComp() );
        S_last_calc_target = max_it->receive_point_;
        S_last_calc_speed = max_it->first_speed_;
        S_last_calc_receiver = max_it->receiver_->unum();
        S_last_calc_valid = true;
        *score = max_it->score_;
        *can_shoot = max_it->can_shoot_;
        *can_assist = max_it->can_assist_;
        rcsc::dlog.addText( rcsc::Logger::ACTION,
			    "%s:%d: get_best_pass() size=%d. target=(%.1f %.1f)"
			    " speed=%.3f  receiver=%d"
			    ,__FILE__, __LINE__,
			    S_cached_pass_route.size(),
			    S_last_calc_target.x, S_last_calc_target.y,
			    S_last_calc_speed,
			    S_last_calc_receiver );
    }

    if ( S_last_calc_valid )
    {
        if ( target_point )
        {
            *target_point = S_last_calc_target;
        }
        if ( first_speed )
        {
            *first_speed = S_last_calc_speed;
        }
        if ( receiver )
        {
            *receiver = S_last_calc_receiver;
        }

        rcsc::dlog.addText( rcsc::Logger::ACTION,
			    "%s:%d: best pass (%.2f, %.2f). speed=%.2f. receiver=%d"
			    ,__FILE__, __LINE__,
			    S_last_calc_target.x, S_last_calc_target.y,
			    S_last_calc_speed, S_last_calc_receiver );
    }

    return S_last_calc_valid;
}

/*-------------------------------------------------------------------*/
/*!
  static method
*/
void
Body_ObakePass::create_routes(rcsc::PlayerAgent * agent )
{
    const rcsc::WorldModel & wm = agent->world();
    // reset old info
    S_cached_pass_route.clear();

    // loop candidate teammates
    const rcsc::PlayerPtrCont::const_iterator
        t_end = wm.teammatesFromSelf().end();
    for ( rcsc::PlayerPtrCont::const_iterator
              it = wm.teammatesFromSelf().begin();
          it != t_end;
          ++it )
    {
        if ( (*it)->goalie() && (*it)->pos().x < -22.0 )
        {
            // goalie is rejected.
            continue;
        }
        if ( (*it)->posCount() > 3 )
        {
            // low confidence players are rejected.
            continue;
        }
        if ( (*it)->pos().x > wm.offsideLineX() + 1.0 )
        {
            // offside players are rejected.
            continue;
        }
        if ( (*it)->pos().x < wm.ball().pos().x - 25.0 )
        {
            // too back
            continue;
        }

        // create & verify each route
        create_direct_pass(agent, *it );
        create_lead_pass(agent, *it );
        if ( wm.self().pos().x > wm.offsideLineX() - 20.0 )
        {
            create_through_pass(agent, *it );
        }
    }

    ////////////////////////////////////////////////////////////////
    // evaluation
/*
     std::ofstream fout;
     fout.open("pass.txt", std::ios::out | std::ios::app);
     fout<<"ball = "<<wm.ball().pos()<<std::endl;
     evaluate_routes(agent);
     fout<<std::endl<<std::endl;
     fout.close();
*/
    evaluate_routes(agent);
}

/*-------------------------------------------------------------------*/
/*!
  static method
*/
void
Body_ObakePass::create_direct_pass(rcsc::PlayerAgent * agent,
				   const rcsc::PlayerObject * receiver)
{ 
/*
    std::ofstream fout;
    fout.open("pass.txt", std::ios::out | std::ios::app);
    fout<<"create_pass"<<std::endl;
*/
    const rcsc::WorldModel & wm = agent->world();
    static const double MAX_DIRECT_PASS_DIST
        = 38.0;

 /*0.8 * rcsc::inertia_final_distance( rcsc::ServerParam::i().ballSpeedMax(),
            rcsc::ServerParam::i().ballDecay() );*/
#ifdef DEBUG
    rcsc::dlog.addText( rcsc::Logger::PASS,
			"Create_direct_pass() to %d(%.1f %.1f)",
			receiver->unum(),
			receiver->pos().x, receiver->pos().y );
#endif

    // out of pitch?
    if ( receiver->pos().absX() > rcsc::ServerParam::i().pitchHalfLength() - 3.0
         || receiver->pos().absY() > rcsc::ServerParam::i().pitchHalfWidth() - 3.0 )
    {
#ifdef DEBUG
        rcsc::dlog.addText( rcsc::Logger::PASS,
			    "__ out of pitch" );
#endif
        return;
    }

    /////////////////////////////////////////////////////////////////
    // too far
    if ( receiver->distFromSelf() > MAX_DIRECT_PASS_DIST )
    {
#ifdef DEBUG
        rcsc::dlog.addText( rcsc::Logger::PASS,
			    "__ over max distance %.2f > %.2f",
			    receiver->distFromSelf(),
			    MAX_DIRECT_PASS_DIST );
#endif
        return;
    }
    // too close
    // if ( receiver->distFromSelf() < 6.0 )
    if((ObakeStrategy().getArea(receiver->pos()) != ObakeStrategy::ShootChance
        && receiver->distFromSelf() < 6.0/*4.5*/)
       ||  receiver->distFromSelf() < 3.0)
    {
#ifdef DEBUG
        rcsc::dlog.addText( rcsc::Logger::PASS,
			    "__ too close. dist = %.2f  canceled",
			    receiver->distFromSelf() );
#endif
        return;
    }
    /////////////////
    if ( receiver->pos().x < wm.self().pos().x + 5.0 )
    {
        if ( receiver->angleFromSelf().abs() < 40.0
             || ( receiver->pos().x > wm.defenseLineX() + 10.0
                  && receiver->pos().x > 0.0 )
	    )
        {
            // "DIRECT: not defender." <<;
        }
        else
        {
            // "DIRECT: back.;
#ifdef DEBUG
            rcsc::dlog.addText( rcsc::Logger::PASS,
				"__ looks back pass. DF Line=%.1f. canceled",
				wm.defenseLineX() );
#endif
            return;
        }
    }

    // not safety area
    if ( receiver->pos().x < -20.0 )
    {
        if ( receiver->pos().x > wm.self().pos().x + 13.0 )
        {
            // safety clear??
        }
        else if ( receiver->pos().x > wm.self().pos().x + 5.0
                  && receiver->pos().absY() > 20.0
                  && fabs(receiver->pos().y - wm.self().pos().y) < 20.0
	    )
        {
            // safety area
        }

        else
        {
            // dangerous
#ifdef DEBUG
            rcsc::dlog.addText( rcsc::Logger::PASS,
				"__ receiver is in dangerous area. canceled" );
#endif
            return;
        }
    }

    /////////////////////////////////////////////////////////////////

    // set base target
    rcsc::Vector2D base_player_pos = receiver->pos();
    if ( receiver->velCount() < 3 )
    {
        rcsc::Vector2D fvel = receiver->vel();
        fvel /= rcsc::ServerParam::i().defaultPlayerDecay();
        fvel *= std::min( receiver->velCount() + 1, 2 );
        base_player_pos += fvel;
    }


    const rcsc::Vector2D receiver_rel = base_player_pos - wm.ball().pos();
    const double receiver_dist = receiver_rel.r();
    const rcsc::AngleDeg receiver_angle = receiver_rel.th();

#ifdef DEBUG
    rcsc::dlog.addText( rcsc::Logger::PASS,
			"__ receiver. predict pos(%.2f %.2f) rel(%.2f %.2f)"
			"  dist=%.2f  angle=%.1f",
			base_player_pos.x, base_player_pos.y,
			receiver_rel.x, receiver_rel.y,
			receiver_dist,
			receiver_angle.degree() );

#endif
/*
    if(Obake_Analysis().checkExistOppPenaltyAreaIn(base_player_pos))
    {
        fout<<base_player_pos<<":PA"<<std::endl;
    }
*/
    const double criterion = (ObakeStrategy().getArea(base_player_pos) == ObakeStrategy::ShootChance)
        ? 0.3 : 0.8;

    double end_speed = 1.5;
    double first_speed = 100.0;
    do
    {
        first_speed
            = rcsc::calc_first_term_geom_series_last
            ( end_speed,
              receiver_dist,
              rcsc::ServerParam::i().ballDecay() );
        //= required_first_speed( receiver_dist,
        //ServerParam::i().ballDecay(),
        //end_speed );
        if ( first_speed < rcsc::ServerParam::i().ballSpeedMax() )
        {
            break;
        }
        end_speed -= 0.1;
    }
    while ( end_speed > criterion );


    if ( first_speed > rcsc::ServerParam::i().ballSpeedMax() )
    {
#ifdef DEBUG
        rcsc::dlog.addText( rcsc::Logger::PASS,
			    "__ ball first speed= %.3f.  too high. canceled",
			    first_speed );
#endif
/*
        if(Obake_Analysis().checkExistOppPenaltyAreaIn(base_player_pos))
        {
            fout<<"first_speed = "<<first_speed<<std::endl;
        }
*/
        return;
    }


    if(wm.ball().pos().dist(base_player_pos) <= 17.5
       && ObakeStrategy().getArea(base_player_pos) == ObakeStrategy::ShootChance)
    {
        const double pass_angle = 45.0;//40.0;//30.0;
        if(Obake_Analysis().checkPassCourse(agent,
                                            wm.ball().pos(),
                                            base_player_pos,
                                            pass_angle))   
        {
            S_cached_pass_route
                .push_back( PassRoute( DIRECT,
                                       receiver,
                                       base_player_pos,
                                       first_speed,
                                       can_kick_by_one_step(agent,
                                                            first_speed,
                                                            receiver_angle )
                                )
                    );
        }
    }

    // add strictly direct pass
    if ( verify_direct_pass(agent,
                            receiver,
                            base_player_pos,
                            receiver_dist,
                            receiver_angle,
                            first_speed ) )
    {
        
        S_cached_pass_route
            .push_back( PassRoute( DIRECT,
                                   receiver,
                                   base_player_pos,
                                   first_speed,
                                   can_kick_by_one_step(agent,
                                                        first_speed,
                                                        receiver_angle )
			    )
		);
/*
         if(Obake_Analysis().checkExistOppPenaltyAreaIn(base_player_pos))
         {
             fout<<"push"<<std::endl;
         }
*/
    }

    // add kickable edge points
    double kickable_angle_buf = 360.0 * ( rcsc::ServerParam::i().defaultKickableArea()
                                          / (2.0 * M_PI * receiver_dist) );
    first_speed *= rcsc::ServerParam::i().ballDecay();

    // right side
    rcsc::Vector2D target_new = wm.ball().pos();
    rcsc::AngleDeg angle_new = receiver_angle;
    angle_new += kickable_angle_buf;
    target_new += rcsc::Vector2D::polar2vector(receiver_dist, angle_new);

    if ( verify_direct_pass(agent,
                            receiver,
                            target_new,
                            receiver_dist,
                            angle_new,
                            first_speed))
    {
        S_cached_pass_route
            .push_back( PassRoute( DIRECT,
                                   receiver,
                                   target_new,
                                   first_speed,
                                   can_kick_by_one_step(agent,
                                                        first_speed,
                                                        angle_new)
			    )
		);
/*
         if(Obake_Analysis().checkExistOppPenaltyAreaIn(base_player_pos))
         {
             fout<<"push right"<<std::endl;
         }
*/
    }
    // left side
    target_new = wm.ball().pos();
    angle_new = receiver_angle;
    angle_new -= kickable_angle_buf;
    target_new += rcsc::Vector2D::polar2vector( receiver_dist, angle_new );

    if ( verify_direct_pass(agent,
                            receiver,
                            target_new,
                            receiver_dist,
                            angle_new,
                            first_speed))
    {
        S_cached_pass_route
            .push_back( PassRoute( DIRECT,
                                   receiver,
                                   target_new,
                                   first_speed,
                                   can_kick_by_one_step(agent,
                                                         first_speed,
                                                         angle_new )
			    )
		);
/*
         if(Obake_Analysis().checkExistOppPenaltyAreaIn(base_player_pos))
         {
             fout<<"push left"<<std::endl;
         }
*/
        rcsc::dlog.addText( rcsc::Logger::PASS,
			    "Pass Success direct unum=%d pos=(%.1f %.1f). first_speed= %.1f",
			    receiver->unum(),
			    target_new.x, target_new.y,
			    first_speed );
    }
    else
    {
        rcsc::dlog.addText( rcsc::Logger::PASS,
			    "Pass Failed direct unum=%d pos=(%.1f %.1f). first_speed= %.1f",
			    receiver->unum(),
			    target_new.x, target_new.y,
			    first_speed );
    }
//    fout.close();
}

/*-------------------------------------------------------------------*/
/*!
  Static method
*/
void
Body_ObakePass::create_lead_pass(rcsc::PlayerAgent * agent,
				 const rcsc::PlayerObject * receiver )
{
    const rcsc::WorldModel & wm = agent->world();
    static const double MAX_LEAD_PASS_DIST
        = 38.0;
/*0.7 * rcsc::inertia_final_distance( rcsc::ServerParam::i().ballSpeedMax(),
            rcsc::ServerParam::i().ballDecay() );*/
    static const
        rcsc::Rect2D shrinked_pitch( -rcsc::ServerParam::i().pitchHalfLength() + 3.0,
                                     -rcsc::ServerParam::i().pitchHalfWidth() + 3.0,
                                     rcsc::ServerParam::i().pitchLength() - 6.0,
                                     rcsc::ServerParam::i().pitchWidth() - 6.0 );

    //static const double receiver_dash_speed = 0.9;
    static const double receiver_dash_speed = 0.8;

#ifdef DEBUG
    rcsc::dlog.addText( rcsc::Logger::PASS,
			"Create_lead_pass() to %d(%.1f %.1f)",
			receiver->unum(),
			receiver->pos().x, receiver->pos().y );
#endif

    /////////////////////////////////////////////////////////////////
    // too far
    if ( receiver->distFromSelf() > MAX_LEAD_PASS_DIST )
    {
#ifdef DEBUG
        rcsc::dlog.addText( rcsc::Logger::PASS,
			    "__ over max distance %.2f > %.2f",
			    receiver->distFromSelf(), MAX_LEAD_PASS_DIST );
#endif
        return;
    }
    // too close
    if ( receiver->distFromSelf() < 2.0 )
    {
#ifdef DEBUG
        rcsc::dlog.addText( rcsc::Logger::PASS,
			    "__ too close %.2f",
			    receiver->distFromSelf() );
#endif
        return;
    }
    if ( receiver->pos().x < wm.self().pos().x - 15.0
         && receiver->pos().x < 15.0 )
    {
#ifdef DEBUG
        rcsc::dlog.addText( rcsc::Logger::PASS,
			    "__ receiver is back cancel" );
#endif
        return;
    }
    //
    if ( receiver->pos().x < -10.0
         && std::fabs( receiver->pos().y - wm.self().pos().y ) > 20.0 )
    {
#ifdef DEBUG
        rcsc::dlog.addText( rcsc::Logger::PASS,
			    "__ receiver is in our field. or Y diff is big" );
#endif
        return;
    }

    const rcsc::Vector2D receiver_rel = receiver->pos() - wm.ball().pos();
    const double receiver_dist = receiver_rel.r();
    const rcsc::AngleDeg receiver_angle = receiver_rel.th();

    const double circum = 2.0 * receiver_dist * M_PI;
    const double angle_step_abs = std::max( 5.0, 360.0 * ( 2.0 / circum ));
    /// angle loop
    double total_add_angle_abs;
    int count;
    for ( total_add_angle_abs = angle_step_abs, count = 0;
          total_add_angle_abs < 31.0 && count < 5;
          total_add_angle_abs += angle_step_abs, ++count )
    {
        /////////////////////////////////////////////////////////////
        // estimate required first ball speed
        const double dash_step
            = ( (angle_step_abs / 360.0) * circum ) / receiver_dash_speed + 2.0;

        double end_speed = 1.2 - 0.11 * count; // Magic Number
        double first_speed = 100.0;
        double ball_steps_to_target = 100.0;
        do
        {
            first_speed
                = rcsc::calc_first_term_geom_series_last
                (end_speed,
                 receiver_dist,
                 rcsc::ServerParam::i().ballDecay());
            //= required_first_speed( receiver_dist,
            //ServerParam::i().ballDecay(),
            //end_speed );
            if ( first_speed < rcsc::ServerParam::i().ballSpeedMax() )
            {
                break;
            }
            ball_steps_to_target
                = rcsc::calc_length_geom_series(first_speed,
                                                receiver_dist,
                                                rcsc::ServerParam::i().ballDecay() );
            //= move_step_for_first( receiver_dist,
            //first_speed,
            //ServerParam::i().ballDecay() );
            if ( dash_step < ball_steps_to_target )
            {
                break;
            }
            end_speed -= 0.1;
        }
        while ( end_speed > 0.5 );


        if ( first_speed > rcsc::ServerParam::i().ballSpeedMax() )
        {
            continue;
        }

        /////////////////////////////////////////////////////////////
        // angle plus minus loop
        for ( int i = 0; i < 2; i++ )
        {
            rcsc::AngleDeg target_angle = receiver_angle;
            if ( i == 0 )
            {
                target_angle -= total_add_angle_abs;
            }
            else
            {
                target_angle += total_add_angle_abs;
            }

            // check dir confidence
            int max_count = 100, ave_count = 100;
            wm.dirRangeCount( target_angle, 20.0,
			      &max_count, NULL, &ave_count );
            if ( max_count > 9 || ave_count > 3 )
            {
                continue;
            }

            const rcsc::Vector2D target_point
                = wm.ball().pos()
                + rcsc::Vector2D::polar2vector(receiver_dist, target_angle);

            /////////////////////////////////////////////////////////////////
            // ignore back pass
            if ( target_point.x < 0.0
                 && target_point.x <  wm.self().pos().x )
            {
                continue;
            }

            if ( target_point.x < 0.0
                 && target_point.x < receiver->pos().x - 3.0 )
            {
                continue;
            }
            if ( target_point.x < receiver->pos().x - 6.0 )
            {
                continue;
            }
            // out of pitch
            if ( ! shrinked_pitch.contains( target_point ) )
            {
                continue;
            }
            // not safety area
            if ( target_point.x < -10.0 )
            {
                if ( target_point.x < wm.defenseLineX() + 10.0 )
                {
                    continue;
                }
                else if ( target_point.x > wm.self().pos().x + 20.0
                          && fabs(target_point.y - wm.self().pos().y) < 20.0 )
                {
                    // safety clear ??
                }
                else if ( target_point.x > wm.self().pos().x + 5.0 // forward than me
                          && std::fabs( target_point.y - wm.self().pos().y ) < 20.0
		    ) // out side of me
                {
                    // safety area
                }
                else if ( target_point.x > wm.defenseLineX() + 20.0 )
                {
                    // safety area
                }
                else
                {
                    // dangerous
                    continue;
                }
            }
            /////////////////////////////////////////////////////////////////

#ifdef DEBUG
            rcsc::dlog.addText( rcsc::Logger::PASS,
				"__ lead pass to (%.1f %.1f). first_speed= %.3f. angle = %.1f",
				target_point.x, target_point.y,
				first_speed, target_angle.degree() );
#endif
            // add lead pass route
            // this methid is same as through pass verification method.
            if ( verify_through_pass(agent,
                                     receiver,
                                     receiver->pos(),
                                     target_point,
                                     receiver_dist,
                                     target_angle,
                                     first_speed,
                                     ball_steps_to_target))
            {
                S_cached_pass_route
                    .push_back( PassRoute( LEAD,
                                           receiver,
                                           target_point,
                                           first_speed,
                                           can_kick_by_one_step(agent,
                                                                first_speed,
                                                                target_angle)));
                rcsc::dlog.addText( rcsc::Logger::PASS,
				    "Pass Success lead unum=%d pos=(%.1f %.1f) angle=%.1f first_speed=%.1f",
				    receiver->unum(),
				    target_point.x, target_point.y,
				    target_angle.degree(),
				    first_speed );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::PASS,
				    "Pass Failed lead unum=%d pos=(%.1f %.1f) angle=%.1f first_speed=%.1f",
				    receiver->unum(),
				    target_point.x, target_point.y,
				    target_angle.degree(),
				    first_speed );

            }
        }
    }
}

/*-------------------------------------------------------------------*/
/*!
  static method
*/
void
Body_ObakePass::create_through_pass(rcsc::PlayerAgent * agent,
				    const rcsc::PlayerObject * receiver)
{
    const rcsc::WorldModel & wm = agent->world();
    static const double MAX_THROUGH_PASS_DIST
        = 35.0;
/*0.9 * rcsc::inertia_final_distance( rcsc::ServerParam::i().ballSpeedMax(),
            rcsc::ServerParam::i().ballDecay() );*/

    ////////////////////////////////////////////////////////////////
    static const
        rcsc::Rect2D shrinked_pitch( -rcsc::ServerParam::i().pitchHalfLength() + 3.0,
                                     -rcsc::ServerParam::i().pitchHalfWidth() + 3.0,
                                     rcsc::ServerParam::i().pitchLength() - 6.0,
                                     rcsc::ServerParam::i().pitchWidth() - 6.0 );

    static const double receiver_dash_speed = 1.0;
    //static const double receiver_dash_speed = 0.85;

    static const double S_min_dash = 5.0;
    static const double S_max_dash = 25.0;
    static const double S_dash_range = S_max_dash - S_min_dash;
    static const double S_dash_inc = 4.0;
    static const int S_dash_loop
        = static_cast< int >( std::ceil( S_dash_range / S_dash_inc ) ) + 1;

    static const rcsc::AngleDeg S_min_angle = -20.0;
    static const rcsc::AngleDeg S_max_angle = 20.0;
    static const double S_angle_range = ( S_min_angle - S_max_angle ).abs();
    static const double S_angle_inc = 10.0;
    static const int S_angle_loop
        = static_cast< int >( std::ceil( S_angle_range / S_angle_inc ) ) + 1;

#ifdef DEBUG
    rcsc::dlog.addText( rcsc::Logger::PASS,
			"Create_through_pass() to %d(%.1f %.1f)",
			receiver->unum(),
			receiver->pos().x, receiver->pos().y );
#endif

    /////////////////////////////////////////////////////////////////
    // check receiver position
    if ( receiver->pos().x > wm.offsideLineX() - 0.5 )
    {
#ifdef DEBUG
        rcsc::dlog.addText( rcsc::Logger::PASS,
			    "__ receiver is offside" );
#endif
        return;
    }
    if ( receiver->pos().x < wm.self().pos().x - 10.0 )
    {
#ifdef DEBUG
        rcsc::dlog.addText( rcsc::Logger::PASS,
			    "__ receiver is back" );
#endif
        return;
    }
    if ( std::fabs( receiver->pos().y - wm.self().pos().y ) > 35.0 )
    {
#ifdef DEBUG
        rcsc::dlog.addText( rcsc::Logger::PASS,
			    "__ receiver Y diff is big" );
#endif
        return;
    }
    if ( wm.defenseLineX() < 0.0
         && receiver->pos().x < wm.defenseLineX() - 15.0 )
    {
#ifdef DEBUG
        rcsc::dlog.addText( rcsc::Logger::PASS,
			    "__ receiver is near to defense line" );
#endif
        return;
    }
    if ( wm.offsideLineX() < 30.0
         && receiver->pos().x < wm.offsideLineX() - 15.0 )
    {
#ifdef DEBUG
        rcsc::dlog.addText( rcsc::Logger::PASS,
			    "__ receiver is far from offside line" );
#endif
        return;
    }
    if ( receiver->angleFromSelf().abs() > 135.0 )
    {
#ifdef DEBUG
        rcsc::dlog.addText( rcsc::Logger::PASS,
			    "__ receiver angle is too back" );
#endif
        return;
    }

    // angle loop
    rcsc::AngleDeg dash_angle = S_min_angle;
    for ( int i = 0; i < S_angle_loop; ++i, dash_angle += S_angle_inc )
    {
        const rcsc::Vector2D base_dash = rcsc::Vector2D::polar2vector( 1.0, dash_angle );

        // dash dist loop
        double dash_dist = S_min_dash;
        for ( int j = 0; j < S_dash_loop; ++j, dash_dist += S_dash_inc )
        {
            rcsc::Vector2D target_point = base_dash;
            target_point *= dash_dist;
            target_point += receiver->pos();

            if ( ! shrinked_pitch.contains( target_point ) )
            {
                // out of pitch
                continue;
            }

            if ( target_point.x < wm.self().pos().x + 3.0 )
            {
                continue;
            }

            const rcsc::Vector2D target_rel = target_point - wm.ball().pos();
            const double target_dist = target_rel.r();
            const rcsc::AngleDeg target_angle = target_rel.th();

            // check dir confidence
            int max_count = 100, ave_count = 100;
            wm.dirRangeCount( target_angle, 20.0,
			      &max_count, NULL, &ave_count );
            if ( max_count > 9 || ave_count > 3 )
            {
                continue;
            }

            if ( target_dist > MAX_THROUGH_PASS_DIST ) // dist range over
            {
                continue;
            }

            if ( target_dist < dash_dist ) // I am closer than receiver
            {
                continue;
            }

            const double dash_step = dash_dist / receiver_dash_speed + 7.0;//+2.0

            double end_speed = 0.81;//0.65
            double first_speed = 100.0;
            double ball_steps_to_target = 100.0;
            double min_speed = 0.5;
            if(target_point.x >rcsc::ServerParam::i().theirPenaltyAreaLineX()
               && target_point.absY() <= rcsc::ServerParam::i().goalAreaHalfWidth())
            {
                min_speed = 0.3;
            }
            else if(wm.ball().pos().x <= wm.offsideLineX() - 8.0 
                    &&target_point.x >= wm.offsideLineX() + 5.0
                    && std::abs(target_point.y - wm.ball().pos().y) <= 12.5)
            {
                min_speed = 0.3;
//                std::cout<<"min_speed"<<std::endl;
            }
            else if(target_point.x  > 20.0
                    && target_point.absY() <= rcsc::ServerParam::i().penaltyAreaHalfWidth())
            {
                min_speed = 0.4;
            }
            do
            {
                first_speed
                    = rcsc::calc_first_term_geom_series_last
                    (end_speed,
                     target_dist,
                     rcsc::ServerParam::i().ballDecay());
                //= required_first_speed(target_dist,
                //ServerParam::i().ballDecay(),
                //end_speed);
                if ( first_speed > rcsc::ServerParam::i().ballSpeedMax() )
                {
                    end_speed -= 0.1;
                    continue;
                }

                ball_steps_to_target
                    = rcsc::calc_length_geom_series( first_speed,
                                                     target_dist,
                                                     rcsc::ServerParam::i().ballDecay());
                //= move_step_for_first( target_dist,
                //first_speed,
                //ServerParam::i().ballDecay() );
                //if ( ball_steps_to_target < dash_step )
                if ( dash_step < ball_steps_to_target )
                {
                    break;
                }

                end_speed -= 0.1;
            }
            while ( end_speed > min_speed);

            if ( first_speed > rcsc::ServerParam::i().ballSpeedMax()
                 || dash_step > ball_steps_to_target )
            {
                continue;
            }

#ifdef DEBUG
            rcsc::dlog.addText( rcsc::Logger::PASS,
				"__ throug pass to (%.1f %.1f). first_speed= %.3f",
				target_point.x, target_point.y,
				first_speed );
#endif
            if ( verify_through_pass(agent,
                                     receiver,
                                     receiver->pos(),
                                     target_point, target_dist, target_angle,
                                     first_speed,
                                     ball_steps_to_target))
            {
                S_cached_pass_route
                    .push_back( PassRoute( THROUGH,
                                           receiver,
                                           target_point,
                                           first_speed,
                                           can_kick_by_one_step(agent,
                                                                first_speed,
                                                                target_angle )
				    )
			);
                rcsc::dlog.addText( rcsc::Logger::PASS,
				    "Pass Success through pass unum=%d pos=(%.1f %.1f) angle=%.1f first_speed=%.1f dash_step=%.1f ball_step=%.1f",
				    receiver->unum(),
				    target_point.x, target_point.y,
				    target_angle.degree(),
				    first_speed,
				    dash_step,
				    ball_steps_to_target );
            }
            else
            {
                rcsc::dlog.addText( rcsc::Logger::PASS,
				    "Pass Failed through unum=%d pos=(%.1f %.1f) angle=%.1f first_speed=%.1f dash_step-%.1f ball_step=%.1f",
				    receiver->unum(),
				    target_point.x, target_point.y,
				    target_angle.degree(),
				    first_speed,
				    dash_step,
				    ball_steps_to_target );
            }
        }
    }
}

/*-------------------------------------------------------------------*/
/*!
  static method
*/
bool
Body_ObakePass::verify_direct_pass( rcsc::PlayerAgent * agent,
				    const rcsc::PlayerObject * receiver,
				    const rcsc::Vector2D & target_point,
				    const double & target_dist,
				    const rcsc::AngleDeg & target_angle,
				    const double & first_speed )
{
    /*
      std::ofstream fout;
    fout.open("pass.txt", std::ios::out | std::ios::app);
    */
/*    static const double player_dash_speed =(Obake_Analysis().checkExistOppPenaltyAreaIn(target_point)) 
      ? 0.65 : 1.0;*/
    const rcsc::WorldModel & wm = agent->world();
    static const double player_dash_speed = 1.0;

    const rcsc::Vector2D first_vel = rcsc::Vector2D::polar2vector( first_speed, target_angle );
    const rcsc::AngleDeg minus_target_angle = -target_angle;
    const double next_speed = first_speed * rcsc::ServerParam::i().ballDecay();

#ifdef DEBUG
    rcsc::dlog.addText( rcsc::Logger::PASS,
			"____ verify direct pass to(%.1f %.1f). first_speed=%.3f. angle=%.1f",
			target_point.x, target_point.y,
			first_speed, target_angle.degree() );
#endif

#if 0
    const Interception interception( wm.ball().pos() + first_vel,
                                     first_speed * rcsc::ServerParam::i().ballDecay(),
                                     target_angle );
    {
        int cycle = static_cast< int >
            ( std::ceil( interception.getReachCycle( receiver->pos(),
                                                     &(receiver->vel()),
                                                     ( receiver->bodyCount() == 0
                                                       ? &(receiver->body()) : NULL ),
                                                     receiver->posCount(),
                                                     0.9, // kickable area + buf
                                                     1.0 ) ) ); // dash speed
        rcsc::dlog.addText( rcsc::Logger::PASS,
			    "______ receiver reach cycle by util = %d.",
			    cycle );
    }
#endif
    const int turn_cycle =(Obake_Analysis().checkExistOppPenaltyAreaIn(target_point)/*ObakeStrategy().getArea(target_point) == ObakeStrategy::ShootChance*/) 
        ? 4 : 0;
    const double near_penalty_area_base_opp_r = 1.5;
    const double not_near_penalty_area_base_opp_r = 3.0;
    const double very_near_penalty_area_base_virtual_dash_rate = 0.45;//0.35;
    const double not_very_near_penalty_area_base_virtual_dash_rate = 0.8;
    double degree_near_penalty_area_x, degree_not_near_penalty_area_x, 
        degree_very_near_penalty_area_x, degree_not_very_near_penalty_area_x,
        base_dist_from_opp, virtual_dash_rate;
    const rcsc::PlayerPtrCont::const_iterator
        o_end = wm.opponentsFromSelf().end();
    for ( rcsc::PlayerPtrCont::const_iterator
              it = wm.opponentsFromSelf().begin();
          it != o_end;
          ++it )
    {
        if ( (*it)->posCount() > 10 ) continue;
        if ( (*it)->isGhost() && (*it)->posCount() >= 4 ) continue;

        /*
          const double virtual_dash
          = player_dash_speed * 0.8 * std::min( 5, (*it)->posCount() );
*/
        degree_near_penalty_area_x = Obake_FuzzyGrade().degreeNearOppPenaltyAreaX(target_point.x);
        degree_not_near_penalty_area_x = 1 - degree_near_penalty_area_x;
        degree_very_near_penalty_area_x = std::pow(degree_near_penalty_area_x, 2.0);
        degree_not_very_near_penalty_area_x = 1 - degree_very_near_penalty_area_x;
        virtual_dash_rate = very_near_penalty_area_base_virtual_dash_rate * degree_very_near_penalty_area_x
            + not_very_near_penalty_area_base_virtual_dash_rate * degree_not_very_near_penalty_area_x;
        const double virtual_dash
          = player_dash_speed * virtual_dash_rate * std::min( 5, (*it)->posCount() );

//         if ( (*it)->pos().dist( target_point ) - virtual_dash > target_dist + 2.0 )
//         {
// #ifdef DEBUG
//             rcsc::dlog.addText( rcsc::Logger::PASS,
//                           "______ opp%d(%.1f %.1f) is too far. not calculated.",
//                           (*it)->unum(),
//                           (*it)->pos().x, (*it)->pos().y );
// #endif
//             continue;
//         }


        if ( ( (*it)->angleFromSelf() - target_angle ).abs() > 100.0 )
        {
// #ifdef DEBUG
//             rcsc::dlog.addText( rcsc::Logger::PASS,
//                           "______ opp%d(%.1f %.1f) is back of target dir. not calculated.",
//                           (*it)->unum(),
//                           (*it)->pos().x, (*it)->pos().y );
//#endif
            continue;
        }
        
        base_dist_from_opp = near_penalty_area_base_opp_r * degree_near_penalty_area_x
            + not_near_penalty_area_base_opp_r * degree_not_near_penalty_area_x;
        //std::cout<<"base_dist_from_opp = "<<base_dist_from_opp<<std::endl;
        //std::cout<<"opp_to_target^2 = "<<(*it)->pos().dist2( target_point )<<", dist = "<<(*it)->pos().dist(target_point)<<std::endl;
        if ( (*it)->pos().dist2( target_point ) < base_dist_from_opp * base_dist_from_opp)

        {
#ifdef DEBUG
            rcsc::dlog.addText( rcsc::Logger::PASS,
				"______ opp%d(%.1f %.1f) is already on target point(%.1f %.1f).",
				(*it)->unum(),
				(*it)->pos().x, (*it)->pos().y,
				target_point.x, target_point.y );
#endif
            return false;
        }

#if 0
        int cycle = static_cast< int >
            ( std::ceil( interception
                         .getReachCycle( (*it)->pos(),
                                         ( (*it)->velCount() <= 1 ? &((*it)->vel()) : NULL ),
                                         ( (*it)->bodyCount() == 0 ? &((*it)->body()) : NULL ),
                                         (*it)->posCount(),
                                         1.2, // kickable area + buf
                                         1.1 ) ) ); // dash speed
        rcsc::dlog.addText( rcsc::Logger::PASS,
			    "______ opp%d(%.1f %.1f) reach cycle by util = %d.",
			    (*it)->unum(),
			    (*it)->pos().x, (*it)->pos().y, cycle );
#endif


        rcsc::Vector2D ball_to_opp = (*it)->pos();
        ball_to_opp -= wm.ball().pos();
        ball_to_opp -= first_vel;
        ball_to_opp.rotate( minus_target_angle );
/*        if(wm.ball().pos().x >= rcsc::ServerParam::i().theirPenaltyAreaLineX()
               && Obake_Analysis().checkExistOppPenaltyAreaIn(target_point))
        {
            fout<<"ball_to_opp.x = "<<ball_to_opp.x<<", receiver = "<<(*it)->pos()<<std::endl;
        }
*/
        if ( 0.0 < ball_to_opp.x && ball_to_opp.x < target_dist )
        {
            double opp2line_dist = ball_to_opp.absY();
            opp2line_dist -= virtual_dash;
            opp2line_dist -= rcsc::ServerParam::i().defaultKickableArea();
            opp2line_dist -= 0.1;

            if ( opp2line_dist < 0.0 )
            {
#ifdef DEBUG
                rcsc::dlog.addText( rcsc::Logger::PASS,
				    "______ opp%d(%.1f %.1f) can reach pass line. rejected. vdash=%.1f",
				    (*it)->unum(),
				    (*it)->pos().x, (*it)->pos().y,
				    virtual_dash );
#endif
                return false;
            }

            const double ball_steps_to_project
                = rcsc::calc_length_geom_series(next_speed,
                                                ball_to_opp.x,
                                                rcsc::ServerParam::i().ballDecay() );
            //= move_step_for_first(ball_to_opp.x,
            //next_speed,
            //ServerParam::i().ballDecay());
/*
            if(wm.ball().pos().x >= rcsc::ServerParam::i().theirPenaltyAreaLineX()
               && Obake_Analysis().checkExistOppPenaltyAreaIn(target_point))
            {
                
                fout<<"ball_steps_to_project = "<<ball_steps_to_project<<std::endl;
                fout<<"player_dash = 1, "<<opp2line_dist/ 1.0<<", player_dash = 0.5, " << opp2line_dist/ 0.5<<std::endl;
            }
*/
            if ( ball_steps_to_project < 0.0
                 || opp2line_dist / player_dash_speed + turn_cycle < ball_steps_to_project )
            {
#ifdef DEBUG
                rcsc::dlog.addText( rcsc::Logger::PASS,
				    "______ opp%d(%.1f %.1f) can reach pass line."
				    " ball reach step to project= %.1f",
				    (*it)->unum(),
				    (*it)->pos().x, (*it)->pos().y,
				    ball_steps_to_project );
#endif
                return false;
            }
#ifdef DEBUG
            rcsc::dlog.addText( rcsc::Logger::PASS,
				"______ opp%d(%.1f %.1f) cannot intercept.",
				(*it)->unum(),
				(*it)->pos().x, (*it)->pos().y );
#endif
        }
        /*else if(0.0 < ball_to_opp.x && (*it)->pos().dist(target_point) < target_dist)
        {
	    const double dist_buf = 1.5;
            double opp_to_target_point_dist = (*it)->pos().dist(target_point);
            opp_to_target_point_dist -= virtual_dash;
            opp_to_target_point_dist -= rcsc::ServerParam::i().defaultKickableArea();
            opp_to_target_point_dist -= 0.1;
            const double ball_steps_to_project
                = rcsc::calc_length_geom_series(next_speed,
                                                ball_to_opp.x,
                                                rcsc::ServerParam::i().ballDecay());
            if((opp_to_target_point_dist + dist_buf)/ player_dash_speed < ball_steps_to_project)
            {
                return false;
            }
            }*/
    }
//    fout.close();
#ifdef DEBUG
    rcsc::dlog.addText( rcsc::Logger::PASS,
			"__ Success!" );
#endif
    return true;
}

/*-------------------------------------------------------------------*/
/*!
  static method
*/
bool
Body_ObakePass::verify_through_pass(rcsc::PlayerAgent * agent,
                                    const rcsc::PlayerObject * receiver,
                                    const rcsc::Vector2D & receiver_pos,
                                    const rcsc::Vector2D & target_point,
                                    const double & target_dist,
                                    const rcsc::AngleDeg & target_angle,
                                    const double & first_speed,
                                    const double & reach_step)
{
    const rcsc::WorldModel & wm = agent->world();

    static const double player_dash_speed = 1.0;

    const rcsc::Vector2D first_vel = rcsc::Vector2D::polar2vector( first_speed, target_angle );
    const rcsc::AngleDeg minus_target_angle = -target_angle;
    const double next_speed = first_speed * rcsc::ServerParam::i().ballDecay();

    const double receiver_to_target = receiver_pos.dist( target_point );


#if 0
    const Interception interception( wm.ball().pos() + first_vel,
                                     first_speed * rcsc::ServerParam::i().ballDecay(),
                                     target_angle );
    {
        int cycle = static_cast< int >
            ( std::ceil( interception.getReachCycle( receiver->pos(),
                                                     &(receiver->vel()),
                                                     ( receiver->bodyCount() == 0
                                                       ? &(receiver->body()) : NULL ),
                                                     receiver->posCount(),
                                                     0.9, // kickable area + buf
                                                     1.0 ) ) ); // dash speed
        rcsc::dlog.addText( rcsc::Logger::PASS,
			    "______ receiver reach cycle by util = %d.",
			    cycle );
    }
#endif
    bool very_aggressive = false;
    const rcsc::Vector2D receiver_to_target_vector = target_point - receiver_pos;
    double difference_angle = std::abs((receiver)->body().degree()
                                       - receiver_to_target_vector.th().degree());
//    std::cout<<"dash"<<(receiver)->vel().x<<","<<(receiver)->vel().y<<std::endl;
//    std::cout<<"difference_angle"<<difference_angle<<std::endl;
    const double max_angle = 45.0;
    if(difference_angle <= max_angle)
    {
        if ( target_point.x > 28.0
             && target_point.x > wm.ball().pos().x + 20.0 
             && !(wm.ball().pos().absY() <= rcsc::ServerParam::i().goalAreaHalfWidth()
                  && target_point.x < rcsc::ServerParam::i().theirPenaltyAreaLineX()))
        {
            very_aggressive = true;
        }
        else if(wm.ball().pos().x > wm.offsideLineX() - 10.0
                && target_point.x > wm.offsideLineX() + 5.0)
        {
            very_aggressive = true;
        }
        else if(wm.ball().pos().x >= wm.offsideLineX() - 15.5
                && receiver_pos.x - wm.ball().pos().x >= 5.0
                && target_point.x > wm.offsideLineX()
                && ((receiver)->vel().x >= 
                    rcsc::ServerParam::i().defaultPlayerSpeedMax() * 0.35
                    || difference_angle < 30.0))
        {
//            std::cout<<"dash or angle through pass"<<std::endl;
            very_aggressive = true;
        }
        else if ( target_point.x > wm.offsideLineX() + 15.0
                  && target_point.x > wm.ball().pos().x + 15.0 
                  && !(wm.ball().pos().absY() <= rcsc::ServerParam::i().goalAreaHalfWidth()
                       && target_point.x < rcsc::ServerParam::i().theirPenaltyAreaLineX()))
        {
            very_aggressive = true;
        }
        else if(target_point.x > wm.offsideLineX() + 5.0 
                && target_point.absY() <= (rcsc::ServerParam::i().goalAreaHalfWidth() + 3.0)
                && target_point.x >= 20.0
                && target_point.x < rcsc::ServerParam::i().theirPenaltyAreaLineX())
        {
            very_aggressive = true;
        }
    }
    bool agressivle = false;
    if(difference_angle <= max_angle)
    {
        if(target_point.x > wm.offsideLineX()
           && target_point.absY() <= rcsc::ServerParam::i().penaltyAreaHalfWidth()
           && target_point.x >= 15.0
           && target_point.x < rcsc::ServerParam::i().theirPenaltyAreaLineX())
        {
            agressivle = true;
        }
    }
    const rcsc::PlayerPtrCont::const_iterator
        o_end = wm.opponentsFromSelf().end();
    for ( rcsc::PlayerPtrCont::const_iterator
              it = wm.opponentsFromSelf().begin();
          it != o_end;
          ++it )
    {
        if ( (*it)->posCount() > 10 ) continue;

        if ( (*it)->goalie() )
        {
            if ( target_point.absY() > rcsc::ServerParam::i().penaltyAreaWidth() - 3.0
                 || target_point.x < rcsc::ServerParam::i().theirPenaltyAreaLineX() + 2.0 )
            {
                continue;
            }
        }

#if 0
        int cycle = static_cast< int >
            ( std::ceil( interception
                         .getReachCycle( (*it)->pos(),
                                         ( (*it)->velCount() <= 1 ? &((*it)->vel()) : NULL ),
                                         ( (*it)->bodyCount() == 0 ? &((*it)->body()) : NULL ),
                                         (*it)->posCount(),
                                         1.2, // kickable area + buf
                                         1.1 ) ) ); // dash speed
        rcsc::dlog.addText( rcsc::Logger::PASS,
			    "______ opp%d(%.1f %.1f) reach cycle by util = %d.",
			    (*it)->unum(),
			    (*it)->pos().x, (*it)->pos().y, cycle );
#endif

        const double virtual_dash
            = player_dash_speed * std::min( 2, (*it)->posCount() );
        double turn_advantage = 0.0;
        if((*it)->goalie())
        {
            turn_advantage = 8.5;//6.5;
        }
        const double opp_to_target = (*it)->pos().dist( target_point );
        double dist_rate = ( very_aggressive ? 0.83/*0.8*/ : 1.0);
        double dist_buf = ( very_aggressive ? 0.8/*0.5*/ : 1.5);
        if(!very_aggressive && agressivle)
        {
            dist_rate = 0.9;
            dist_buf = 1.0;
        }
        if(opp_to_target - virtual_dash - turn_advantage < receiver_to_target * dist_rate + dist_buf )
        {
/*            if(target_point.absY() <= rcsc::ServerParam::i().penaltyAreaHalfWidth())
            {
                std::cout<<"offside:"<<wm.offsideLineX()<<std::endl;

                std::cout<<"target:"<<target_point<<",rate:"<<dist_rate<<",buf:"<<dist_buf
                         <<","<< (receiver_to_target * dist_rate + dist_buf)<<std::endl;
                std::cout<<"opp:"<<opp_to_target - virtual_dash<<"unum:"<<(*it)->unum()<<std::endl;
            }
*/
#ifdef DEBUG
            rcsc::dlog.addText( rcsc::Logger::PASS,
				"______ opp%d(%.1f %.1f) is closer than receiver.",
				(*it)->unum(),
				(*it)->pos().x, (*it)->pos().y );
#endif
            return false;
        }

        rcsc::Vector2D ball_to_opp = (*it)->pos();
        ball_to_opp -= wm.ball().pos();
        ball_to_opp -= first_vel;
        ball_to_opp.rotate(minus_target_angle);

        if ( 0.0 < ball_to_opp.x && ball_to_opp.x < target_dist )
        {
            double opp2line_dist = ball_to_opp.absY();
            opp2line_dist -= virtual_dash;
            if((*it)->goalie())
            {
                opp2line_dist -= rcsc::ServerParam::i().catchableArea();
            }
            else
            {
                opp2line_dist -= rcsc::ServerParam::i().defaultKickableArea();
            }
            opp2line_dist -= 0.1;
            if ( opp2line_dist < 0.0 )
            {
#ifdef DEBUG
                rcsc::dlog.addText( rcsc::Logger::PASS,
				    "______ opp%d(%.1f %.1f) is already on pass line.",
				    (*it)->unum(),
				    (*it)->pos().x, (*it)->pos().y );
#endif
                return false;
            }

            const double ball_steps_to_project
                = rcsc::calc_length_geom_series( next_speed,
					   ball_to_opp.x,
					   rcsc::ServerParam::i().ballDecay() );
            //= move_step_for_first( ball_to_opp.x,
            //next_speed,
            //ServerParam::i().ballDecay() );
            if ( ball_steps_to_project < 0.0
                 || opp2line_dist / player_dash_speed < ball_steps_to_project )
            {
#ifdef DEBUG
                rcsc::dlog.addText( rcsc::Logger::PASS,
				    "______ opp%d(%.1f %.1f) can reach pass line."
				    " ball reach step to project= %.1f",
				    (*it)->unum(),
				    (*it)->pos().x, (*it)->pos().y,
				    ball_steps_to_project );
#endif
                return false;
            }

        }
        else if(0.0 < ball_to_opp.x && (*it)->pos().dist(target_point) < target_dist)
        {
            double opp_to_target_point_dist = (*it)->pos().dist(target_point);
            opp_to_target_point_dist -= virtual_dash;
            if((*it)->goalie())
            {
                opp_to_target_point_dist -= rcsc::ServerParam::i().catchableArea();
            }
            else
            {
                opp_to_target_point_dist -= rcsc::ServerParam::i().defaultKickableArea();
            }
            opp_to_target_point_dist -= 0.1;
            const double ball_steps_to_project
                = rcsc::calc_length_geom_series(next_speed,
                                                ball_to_opp.x,
                                                rcsc::ServerParam::i().ballDecay());
            if(opp_to_target_point_dist / player_dash_speed < ball_steps_to_project)
            {
                return false;
            }
        }
    }

#ifdef DEBUG
    rcsc::dlog.addText( rcsc::Logger::PASS,
			"__ Success!" );
#endif
    return true;
}

/*-------------------------------------------------------------------*/
/*!
  static method
*/
void
Body_ObakePass::evaluate_routes( rcsc::PlayerAgent * agent )
{
    const rcsc::WorldModel & wm = agent->world();
    const rcsc::AngleDeg min_angle = -45.0;
    const rcsc::AngleDeg max_angle = 45.0;
/*
    std::ofstream fout;
     fout.open("pass.txt", std::ios::out | std::ios::app);
*/
    const std::vector< PassRoute >::iterator it_end = S_cached_pass_route.end();
    for ( std::vector< PassRoute >::iterator it = S_cached_pass_route.begin();
          it != it_end;
          ++it )
    {
/*	
        if(ObakeStrategy().getArea(it->receive_point_) == ObakeStrategy::ShootChance)
        {
            fout<<"shoot chance"<<std::endl;
        }
	fout<<"receive_point = "<<it->receive_point_<<std::endl;
*/        
        //-----------------------------------------------------------
        /*
          double opp_dist_rate = 1.0;
        {
            double opp_dist = 100.0;
            //wm.getOpponentNearestTo( it->receive_point_, 20, &opp_dist );
            wm.getOpponentNearestTo( it->receive_point_, 10, &opp_dist );
            const double base_dist =(Obake_Analysis().checkExistOppPenaltyAreaIn(it->receive_point_))
                ? 15.0 : 30.0;
            //opp_dist_rate = std::pow( 0.99, std::max(0.0, 30.0 - opp_dist));
            const rcsc::PlayerObject * opp = wm.getOpponentNearestTo( it->receive_point_, 10, &opp_dist );
            if(opp)
            {
                fout<<"opp = "<<(opp)->pos()<<", count = "<<(opp)->posCount()<<std::endl;
            }
            opp_dist_rate = std::pow(0.99, std::max(0.0, base_dist - opp_dist));
	    fout<<"opp_dist_rate: base_dist = "<<base_dist<<", opp_dist = ";
	    fout<<opp_dist<<", rate = "<<opp_dist_rate<<std::endl;
	    
        }
        */
        double opp_dist = 100.0;
        wm.getOpponentNearestTo( it->receive_point_, 10, &opp_dist );	
        const double base_big_rate = 1.0;
        const double base_moderate_rate = 0.9;
        const double base_small_rate = 0.8;
        const double degree_near_offside_line = Obake_FuzzyGrade().degreeNearOffsideLine(agent,
                                                                                         it->receive_point_.x);
        const double degree_far_offside_line = Obake_FuzzyGrade().degreeFarOffsideLine(agent,
                                                                                       it->receive_point_.x);
        const double degree_near_opp_goal_y = Obake_FuzzyGrade().degreeNearOppGoalY(it->receive_point_.y);
        const double degree_far_opp_goal_y = Obake_FuzzyGrade().degreeFarOppGoalY(it->receive_point_.y);
        const double degree_near_from_opp = Obake_FuzzyGrade().degreeNearFromOpp(opp_dist);
        const double degree_moderate_from_opp = Obake_FuzzyGrade().degreeModerateFromOpp(opp_dist);
        const double degree_far_from_opp = Obake_FuzzyGrade().degreeFarFromOpp(opp_dist);
        const double big_rate = std::min(std::min(degree_near_offside_line,
                                                    degree_near_opp_goal_y),
                                           std::max(degree_moderate_from_opp,
                                                    degree_far_from_opp));
        double moderate_rate = std::max(std::min(std::min(degree_near_offside_line,
                                                                degree_near_opp_goal_y),
						 degree_near_from_opp),
                                        std::min(degree_far_offside_line,
                                                 degree_far_from_opp));
        moderate_rate = std::max(moderate_rate,
                                 std::min(std::min(degree_near_offside_line,
                                                   degree_far_opp_goal_y),
                                          std::max(degree_moderate_from_opp,
                                                   degree_far_from_opp)));
        
        const double small_rate = std::max(std::min(std::min(degree_near_offside_line,
								  degree_far_opp_goal_y),
							 degree_near_from_opp),
						std::min(degree_far_offside_line,
							 std::max(degree_near_from_opp,
								  degree_moderate_from_opp)));
        double sum = big_rate + moderate_rate + small_rate;
        const double opp_dist_rate = (base_big_rate * big_rate + base_moderate_rate * moderate_rate
				      + base_small_rate * small_rate) / sum;
/*
	fout<<"offside line = "<<wm.offsideLineX()<<std::endl;
	fout<< "opp pos = "<<wm.getOpponentNearestTo( it->receive_point_, 10, &opp_dist )->pos()
	    <<", opp_dist = "<<opp_dist<<std::endl;
	fout<<"near_offsideline = "<<degree_near_offside_line<<", far_offsideline = "<<degree_far_offside_line;
	fout<<"near_opp_goal_y = "<<degree_near_opp_goal_y<<", far_opp_goal_y = "<<degree_far_opp_goal_y<<std::endl;
	fout<<"near form opp = "<<degree_near_from_opp<<", moderate from opp = "<<degree_moderate_from_opp;
	fout<<"far from opp = "<<degree_far_from_opp<<std::endl;
	fout<<"small = "<<small_rate<<", moderate = "<<moderate_rate<<", big = "<<big_rate<<std::endl;
	fout<<"opp_dist_rate = "<<opp_dist_rate<<std::endl;
*/
        //-----------------------------------------------------------
/*
        double x_diff_rate = 1.0;
        {
            if(!Obake_Analysis().checkExistOppPenaltyAreaIn(wm.ball().pos()))
            {
                double x_diff = it->receive_point_.x - wm.self().pos().x;
                double base = 0.98;
                if(it->receive_point_.x > wm.offsideLineX() - 15.0
                   && it->receive_point_.x > wm.ball().pos().x)
                {
                    base -= 0.03;
                }
                x_diff_rate = std::pow(0.98,
                                       std::max(0.0, 30.0 - x_diff));
                fout<<"x_diff_rate: x_diff = "<<x_diff;
                fout<<", rate = "<<x_diff_rate<<std::endl;
            }
        }
*/
        const double near_offside_line_base_rate = 1.2;
        const double far_offside_line_base_rate = 2.5;       
        sum = degree_near_offside_line + degree_far_offside_line;  
        const double offside_line_rate = (near_offside_line_base_rate * degree_near_offside_line
                                          + far_offside_line_base_rate * degree_far_offside_line) / sum;
//        fout<<"offside rate = "<<offside_line_rate<<std::endl;
//-----------------------------------------------------------
//        double receiver_move_rate = 1.0;
        //= std::pow( 0.995,
        //it->receiver_->pos().dist( it->receive_point_ ) );
        //-----------------------------------------------------------
        /*
        double pos_conf_rate = std::pow( 0.98, it->receiver_->posCount() );        
        */
        const double many_pos_conf_base_rate = 1.1;
        const double any_pos_conf_base_rate = 1.0;
        const double degree_many_pos_conf = Obake_FuzzyGrade().degreeManyPosConf(it->receiver_->posCount());
        const double degree_any_pos_conf = 1 - degree_many_pos_conf;
        const double pos_conf_rate = many_pos_conf_base_rate * degree_many_pos_conf
            + any_pos_conf_base_rate * degree_any_pos_conf;
/*
        fout<<"receiver pos conf = "<<it->receiver_->posCount();
        fout<<"pos conf rate = "<<pos_conf_rate<<std::endl;
*/
        //-----------------------------------------------------------
        /*
        double dir_conf_rate = 1.0;
        {
            rcsc::AngleDeg pass_angle = ( it->receive_point_ - wm.self().pos() ).th();
            int max_count = 0;
            wm.dirRangeCount( pass_angle, 20.0, &max_count, NULL, NULL );
            dir_conf_rate = std::pow( 0.95, max_count );            
        }
        */
        rcsc::AngleDeg pass_angle = ( it->receive_point_ - wm.self().pos() ).th();
        int max_count = 0;
        wm.dirRangeCount( pass_angle, 20.0, &max_count, NULL, NULL );
        const double many_dir_conf_base_rate = 1.6;
        const double any_dir_conf_base_rate = 1.0;
        const double degree_many_dir_conf = Obake_FuzzyGrade().degreeManyDirConf(max_count);
        const double degree_any_dir_conf = 1 - degree_many_dir_conf;
        const double dir_conf_rate = many_dir_conf_base_rate * degree_many_dir_conf 
            + any_dir_conf_base_rate * degree_any_dir_conf;
/*
        fout<<"dir_conf_rate: max_count = "<<max_count<<std::endl;
        fout<<"rate = "<<dir_conf_rate<<std::endl;
*/
        //-----------------------------------------------------------
/*
        double offense_rate
            = std::pow( 0.98,
                        std::max( 5.0, std::fabs( it->receive_point_.y
                                                  - wm.ball().pos().y ) ) );
*/
        //-----------------------------------------------------------
 
        const double angle = 90.0;
        
        /*
          const rcsc::Sector2D sector(it->receive_point_,
                                    0.0, 10.0,
                                    min_angle, max_angle );
        */
        // opponent check with goalie
//        double front_space_rate = 0.95;
//        const double dribble_angle = 90.0;
	double front_space_rate = 1.05;
	const double dribble_dist = 10.0;
        std::vector<rcsc::Vector2D> dribble_target_point_vector
            = Obake_Analysis().getDribbleTargetPointVector(agent,
                                                           it->receive_point_,
                                                           dribble_dist);
        const std::vector<rcsc::Vector2D>::const_iterator 
            t_end = dribble_target_point_vector.end();
        for(std::vector<rcsc::Vector2D>::const_iterator
                t = dribble_target_point_vector.begin();
            t != t_end;
            t++)
        {
            if(!Obake_Analysis().checkExistOpponent(agent,
                                                    it->receive_point_,
                                                    (*t),
                                                    angle))
            {
                front_space_rate = 1.0;
                break;
            }
        }
        /*
          if (wm.existOpponentIn( sector, 10, true )
            && !(Obake_Analysis().checkExistOppPenaltyAreaIn(wm.ball().pos())
                && !Obake_Analysis().checkExistOppPenaltyAreaIn(it->receive_point_)))
        {
            front_space_rate = 0.95;
        }
        */
        //-----------------------------------------------------------
        it->score_ = 1000.0;
        it->score_ /= opp_dist_rate;
        it->score_ /= offside_line_rate;
        it->score_ /= pos_conf_rate;
        it->score_ /= dir_conf_rate;
        it->score_ /= front_space_rate;

        if ( it->one_step_kick_ )
        {
            it->score_ *= 1.2;
//            fout<<"can one_step_kick_"<<std::endl;
        }
        /*the begin of the added code*/
       
        const double assist_angle = 30.0;
        const double shoot_angle = 30.0;
        const double max_pass_dist = 25.0;
        const double shoot_r = 2.5;
        //can assist
        if(Obake_Analysis().getAssistCourseNumber(agent,
                                                  it->receiver_,
                                                  assist_angle,
                                                  max_pass_dist) >= 1)
        {

            it->score_ *= 1.3;
            it->can_assist_ = true;
//            fout<<"assist"<<std::endl;
        }
        //shoot
        if(ObakeStrategy().getArea(it->receive_point_) == ObakeStrategy::ShootChance)
        {
            if(Obake_Analysis().checkExistShootCourse(agent,
                                                      it->receive_point_,
                                                      shoot_angle,
                                                      shoot_r))
            {
                it->score_ *= 1.4;
                it->can_shoot_ = true;
            }
//            fout<<"shoot"<<std::endl;
        }
        //thorough_pass
/*
        if(it->receive_point_.x > wm.offsideLineX()
           && it->receiver_->distFromBall() > 12.0)
        {
          it->score_ *= 1.05;
          fout<<"through_pass"<<std::endl;
        }
*/
        //side attack
        if(it->receive_point_.absY() >= rcsc::ServerParam::i().pitchHalfWidth() - 8.0)
        {
            it->score_ *= 1.05;
//            fout<<"side attack"<<std::endl;
        }
        
        //back
/*
        if(it->receiver_->unum() >= 2
           && it->receiver_->unum() <= 5)
        {
            it->score_ *= 0.95;
        }
        //defensive half
        if(it->receiver_->unum() == 6
           || it->receiver_->unum() == 7)
        {
            it->score_ *= 0.98;
        }
*/

        /*the end of the added code*/
/*   
        if(it->receiver_->distFromBall() > 15.0
           && it->receive_point_.x > wm.ball().pos().x - 5.0)
        {
            it->score_ *= 1.1;
        }
*/
/*        if(it->receive_point_.x > wm.offsideLineX())
        {
            it->score_ *= 1.05;
        }
*/
        rcsc::dlog.addText( rcsc::Logger::PASS,
			    "PASS Score %6.2f -- to%d(%.1f %.1f) recv_pos(%.1f %.1f) type %d "
			    " speed=%.2f",
			    it->score_,
			    it->receiver_->unum(),
			    it->receiver_->pos().x, it->receiver_->pos().y,
			    it->receive_point_.x, it->receive_point_.y,
			    it->type_,
			    it->first_speed_ );
/*
        rcsc::dlog.addText( rcsc::Logger::PASS,
			    "____ opp_dist=%.2f x_diff=%.2f pos_conf=%.2f"
			    " dir_conf=%.2f space=%.1f %s",
			    opp_dist_rate, x_diff_rate, pos_conf_rate,
			    dir_conf_rate, front_space_rate,
			    ( it->one_step_kick_ ? "one_step" : "" ) );
*/
/*
        fout<<"pos = "<<it->receive_point_<<std::endl;
        fout<<"score = "<<it->score_<<std::endl<<std::endl;
*/
    }
//    fout.close();
}

/*-------------------------------------------------------------------*/
/*!
  static method
*/
bool
Body_ObakePass::can_kick_by_one_step( rcsc::PlayerAgent * agent,
				      const double & first_speed,
				      const rcsc::AngleDeg & target_angle )
{
    const rcsc::WorldModel & wm = agent->world();
    rcsc::Vector2D required_accel = rcsc::Vector2D::polar2vector( first_speed, target_angle );
    required_accel -= wm.ball().vel();
    return ( wm.self().kickRate() * rcsc::ServerParam::i().maxPower()
             > required_accel.r() );
}


