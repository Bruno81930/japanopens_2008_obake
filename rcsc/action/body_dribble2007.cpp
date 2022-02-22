// -*-c++-*-

/*!
  \file body_dribble2007.cpp
  \brief advanced dribble action. player agent can avoid opponent
  players.
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

#include "body_dribble2007.h"
#include "intention_dribble2007.h"

#include <rcsc/action/basic_actions.h>
#include <rcsc/action/body_go_to_point.h>
#include <rcsc/action/body_hold_ball.h>
#include <rcsc/action/body_intercept.h>
#include <rcsc/action/body_kick_to_relative.h>
#include <rcsc/action/body_stop_ball.h>
#include <rcsc/action/neck_scan_field.h>

#include <rcsc/player/audio_sensor.h>
#include <rcsc/player/player_agent.h>
#include <rcsc/player/debug_client.h>
#include <rcsc/player/say_message_builder.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include <rcsc/geom/ray_2d.h>
#include <rcsc/geom/rect_2d.h>
#include <rcsc/geom/sector_2d.h>
#include <rcsc/soccer_math.h>
#include <rcsc/math_util.h>

#include <functional>

//#define USE_CHANGE_VIEW

namespace rcsc {

/*!
  \struct KeepDribbleCmp
  \brief function object to evaluate the keep dribble
 */
struct KeepDribbleCmp
    : public std::binary_function< Body_Dribble2007::KeepDribbleInfo,
                                   Body_Dribble2007::KeepDribbleInfo,
                                   bool > {
    /*!
      \brief compare operator
      \param lhs left hand side argument
      \param rhs right hand side argument
     */
    result_type operator()( const first_argument_type & lhs,
                            const second_argument_type & rhs ) const
      {
          if ( lhs.dash_count_ > rhs.dash_count_ )
          {
              return true;
          }

          if ( lhs.dash_count_ == rhs.dash_count_ )
          {
              if ( lhs.min_opp_dist_ > 5.0
                   && rhs.min_opp_dist_ > 5.0 )
              {
                  return lhs.ball_forward_travel_ > rhs.ball_forward_travel_;
              }

              return lhs.min_opp_dist_ > rhs.min_opp_dist_;
          }

          return false;
      }
};

/*-------------------------------------------------------------------*/
/*!
  execute action
*/
bool
Body_Dribble2007::execute( PlayerAgent * agent )
{
    dlog.addText( Logger::DRIBBLE,
                  "%s: Body_Dribble. to(%.1f, %.1f) dash_power=%.1f"
                  " dash_count=%d"
                  ,__FILE__,
                  M_target_point.x, M_target_point.y,
                  M_dash_power, M_dash_count );

    if ( ! agent->world().self().isKickable() )
    {
        dlog.addText( Logger::DRIBBLE,
                      "%s: Body_Dribble. not kickable"
                      ,__FILE__ );
        return Body_Intercept().execute( agent );
    }

    if ( ! agent->world().ball().velValid() )
    {
        dlog.addText( Logger::DRIBBLE,
                      "%s: Body_Dribble. invalid ball vel"
                      ,__FILE__ );
        return Body_StopBall().execute( agent );
    }

    M_dash_power = agent->world().self().getSafetyDashPower( M_dash_power );

    doAction( agent,
              M_target_point,
              M_dash_power,
              M_dash_count,
              M_dodge_mode );  // dodge mode;
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
Body_Dribble2007::say( PlayerAgent * agent,
                       const Vector2D & target_point,
                       const int queue_count )
{
    if ( agent->config().useCommunication() )
    {
        dlog.addText( Logger::ACTION,
                      __FILE__":  set dribble target communication." );
        agent->debugClient().addMessage( "SayD" );
        agent->addSayMessage( new DribbleMessage( target_point, queue_count ) );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_Dribble2007::doAction( PlayerAgent * agent,
                            const Vector2D & target_point,
                            const double & dash_power,
                            const int dash_count,
                            const bool dodge,
                            const bool dodge_mode )
{
    // try to create the action queue.
    // kick -> dash -> dash -> ...
    // the number of dash is specified by M_dash_count

    /*--------------------------------------------------------*/
    // do dodge dribble
    if ( dodge
         && isDodgeSituation( agent, target_point ) )
    {
        agent->debugClient().addMessage( "DribDodge" );
        return doDodge( agent, target_point );
    }

    /*--------------------------------------------------------*/
    // normal dribble

    const WorldModel & wm = agent->world();
    const Vector2D my_final_pos
        = wm.self().playerType().inertiaFinalPoint( wm.self().pos(),
                                                    wm.self().vel() );
    const Vector2D target_rel = target_point - my_final_pos;
    const double target_dist = target_rel.r();

    // already reach the target point
    if ( target_dist < M_dist_thr )
    {
        dlog.addText( Logger::DRIBBLE,
                      "%s: doAction() already there. hold"
                      ,__FILE__ );
        return Body_HoldBall().execute( agent );
    }

    agent->debugClient().setTarget( M_target_point );


    /*--------------------------------------------------------*/
    // decide dribble angle & dist

    if ( doTurn( agent, target_point, dash_power, dash_count, dodge ) )
    {
        return true;
    }

    /*--------------------------------------------------------*/
    // after one dash, ball will kickable
    double used_dash_power = dash_power;
    if ( canKickAfterDash( agent, &used_dash_power ) )
    {
        dlog.addText( Logger::DRIBBLE,
                      "%s:%d: next kickable. after dash. dash_power=%.1f"
                      ,__FILE__, __LINE__,
                      used_dash_power );
        return agent->doDash( used_dash_power );
    }

    /*--------------------------------------------------------*/
    // do kick first

    if ( ( ! dodge_mode || dash_count >= 2 )
         && doKickDashesWithBall( agent, target_point, dash_power, dash_count,
                                  dodge_mode ) )
    {
        return true;
    }

    return doKickDashes( agent, target_point, dash_power, dash_count );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_Dribble2007::doTurn( PlayerAgent * agent,
                          const Vector2D & target_point,
                          const double & dash_power,
                          const int dash_count,
                          const bool dodge )
{
    const WorldModel & wm = agent->world();

    const Vector2D my_final_pos
        = wm.self().playerType().inertiaFinalPoint( wm.self().pos(),
                                                    wm.self().vel() );
    const Vector2D target_rel = target_point - my_final_pos;
    const AngleDeg target_angle = target_rel.th();
    const double dir_diff
        = ( dash_power > 0.0
            ? ( target_angle - wm.self().body() ).degree()
            : ( target_angle - wm.self().body() - 180.0 ).degree() );
    const double dir_margin_abs
        = std::max( 15.0,
                    std::fabs( AngleDeg::atan2_deg( M_dist_thr, target_rel.r() ) ) );

    dlog.addText( Logger::DRIBBLE,
                  "%s: doTurn dir_diff=%.1f dir_margin=%.1f"
                  ,__FILE__,
                  dir_diff, dir_margin_abs );

    /*--------------------------------------------------------*/
    // already facing to the target
    if ( std::fabs( dir_diff ) < dir_margin_abs )
    {
        return false;
    }

    if ( doTurnOnly( agent, target_point, dash_power,
                     dir_diff ) )
    {
        return true;
    }

    if ( doKickTurnsDash( agent, target_point, dash_power,
                          dir_diff, dir_margin_abs ) )
    {
        return true;
    }

    // just stop the ball

    double kick_power = wm.ball().vel().r() / wm.self().kickRate();
    AngleDeg kick_dir = ( wm.ball().vel().th() - 180.0 ) - wm.self().body();

    dlog.addText( Logger::DRIBBLE,
                  "%s: doTurn just stop the ball."
                  ,__FILE__ );
    agent->doKick( kick_power, kick_dir );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_Dribble2007::doTurnOnly( PlayerAgent * agent,
                              const Vector2D & target_point,
                              const double & dash_power,
                              const double & dir_diff )
{
    const WorldModel & wm = agent->world();

    //---------------------------------------------------------//
    // check opponent
    if ( wm.interceptTable()->opponentReachCycle() <= 1 )
    {
        dlog.addText( Logger::DRIBBLE,
                      "%s: doTurnOnly  exist near opponent"
                      ,__FILE__ );
        // TODO:
        //   emergent avoidance action
        return false;
    }

    //---------------------------------------------------------//
    // check required turn step.
    if ( ! wm.self().canTurn( dir_diff ) )
    {
        // it is necessary to turn more than one step.
        dlog.addText( Logger::DRIBBLE,
                      "%s:%d: doTurnOnly. cannot turn by 1 step. angle_diff = %.1f"
                      ,__FILE__, __LINE__,
                      dir_diff );
        return false;
    }

    //---------------------------------------------------------//
    // check next ball dist after turn
    Vector2D my_next = wm.self().pos() + wm.self().vel();
    Vector2D ball_next = wm.ball().pos() + wm.ball().vel();
    const double ball_next_dist = my_next.dist( ball_next );

    dlog.addText( Logger::DRIBBLE,
                  "%s: doTurnOnly next_ball_dist=%.2f"
                  ,__FILE__,
                  ball_next_dist );

    // not kickable at next cycle, if do turn at current cycle.
    if ( ball_next_dist
         > ( wm.self().kickableArea()
             - wm.ball().vel().r() * ServerParam::i().ballRand()
             - wm.self().vel().r() * ServerParam::i().playerRand()
             - 0.15 ) )
    {
        dlog.addText( Logger::DRIBBLE,
                      "%s: doTurnOnly  not kickable at next"
                      ,__FILE__ );
        return false;
    }

    const PlayerObject * nearest_opp = wm.getOpponentNearestToBall( 5 );
    if ( nearest_opp )
    {
        Vector2D opp_next = nearest_opp->pos() + nearest_opp->vel();
        AngleDeg opp_angle = ( nearest_opp->bodyCount() == 0
                               ? nearest_opp->body()
                               : nearest_opp->vel().th() );
        Line2D opp_line( opp_next, opp_angle );
        if ( opp_line.dist( ball_next ) < 1.1
             && nearest_opp->pos().dist( ball_next ) < 2.0 )
        {
            dlog.addText( Logger::DRIBBLE,
                          __FILE__": doTurnOnly  opponent maybe reach the ball" );
            return false;
        }
    }

    //---------------------------------------------------------//
    // turn only
    agent->debugClient().addMessage( "TurnOnly" );
    dlog.addText( Logger::DRIBBLE,
                  "%s: doTurnOnly  done. required_moment = %.1f"
                  ,__FILE__,
                  dir_diff );

    agent->doTurn( dir_diff );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_Dribble2007::doCollideWithBall( PlayerAgent * agent )
{
    const WorldModel & wm = agent->world();

    Vector2D required_accel = wm.self().vel(); // target relative pos
    required_accel -= wm.ball().rpos(); // required vel
    required_accel -= wm.ball().vel(); // ball next rpos

    double required_power = required_accel.r()/ wm.self().kickRate();
    if ( required_power > ServerParam::i().maxPower() * 1.1 )
    {
        dlog.addText( Logger::DRIBBLE,
                      "%s:%d: doCollideWithBall.  over max power(%f). never collide"
                      ,__FILE__, __LINE__,
                      required_power );
        return false;
    }


    agent->doKick( std::min( required_power, ServerParam::i().maxPower() ),
                   required_accel.th() - wm.self().body() );
    return true;

}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_Dribble2007::doCollideForTurn( PlayerAgent * agent,
                                    const double & dir_diff_abs,
                                    const bool kick_first )
{
    const WorldModel & wm = agent->world();
    double my_speed = wm.self().vel().r();

    if ( kick_first )
    {
        my_speed *= wm.self().playerType().playerDecay();
    }

    const double max_turn_moment
        = wm.self().playerType().effectiveTurn( ServerParam::i().maxMoment(),
                                                my_speed );

    if ( max_turn_moment > dir_diff_abs * 0.9 )
    {
        dlog.addText( Logger::DRIBBLE,
                      "%s:%d: doCollideForTurn.  can face to target by next turn"
                      ,__FILE__, __LINE__ );
        return false;
    }

    if ( doCollideWithBall( agent ) )
    {
        agent->debugClient().addMessage( "CollideForTurn" );
        return true;
    }

    return false;
}

/*-------------------------------------------------------------------*/
/*!
  if back dash mode, dash_power is negative value.
*/
bool
Body_Dribble2007::doKickTurnsDash( PlayerAgent * agent,
                                   const Vector2D & target_point,
                                   const double & dash_power,
                                   const double & dir_diff,
                                   const double & dir_margin_abs )
{
    // try to create these action queue
    // kick -> turn -> turn -> ... -> one dash -> normal dribble kick

    // assume that ball kickable and require to turn to target.

    const WorldModel & wm = agent->world();

    const Vector2D my_final_pos
        = wm.self().playerType().inertiaFinalPoint( wm.self().pos(),
                                                    wm.self().vel() );
    const Vector2D target_rel = target_point - my_final_pos;
    const AngleDeg target_angle = target_rel.th();

    // simulate kick - turn - dash

    AngleDeg keep_global_angle;
#if 0
    bool exist_opp = existCloseOpponent( agent, &keep_global_angle );

    if ( ! exist_opp
         && doCollideForTurn( agent, std::fabs( dir_diff ), true ) )
    {
        // severa turns are required after kick.
        // try to collide with ball.
        dlog.addText( Logger::DRIBBLE,
                      "%s: doKickTurnsDash() no opp. collide with ball"
                      ,__FILE__ );
        return true;
    }
#endif

    // first step is kick
    double my_speed
        = wm.self().vel().r()
        * wm.self().playerType().playerDecay();
    int n_turn = 0;
    double dir_diff_abs = std::fabs( dir_diff );

    while ( dir_diff_abs > 0.0 )
    {
        // !!! it is necessary to consider about self inertia moment

        double moment_abs
            = effective_turn
            (ServerParam::i().maxMoment(),
             my_speed,
             wm.self().playerType().inertiaMoment());
        moment_abs = std::min( dir_diff_abs, moment_abs );
        dir_diff_abs -= moment_abs;
        my_speed *= wm.self().playerType().playerDecay();
        ++n_turn;
    }

#if 1
    if ( n_turn <= 2
         //&& wm.dirCount( target_angle ) <= 5
         && wm.ball().pos().x > 0.0
         && doKickTurnsDashes( agent, target_point, dash_power, n_turn ) )
    {
        return true;
    }

    bool exist_opp = existCloseOpponent( agent, &keep_global_angle );

    if ( ! exist_opp
         && doCollideForTurn( agent, std::fabs( dir_diff ), true ) )
    {
        // severa turns are required after kick.
        // try to collide with ball.
        dlog.addText( Logger::DRIBBLE,
                      "%s: doKickTurnsDash() no opp. collide with ball"
                      ,__FILE__ );
        return true;
    }
#endif

    const Vector2D my_pos
        = inertia_n_step_point
        ( Vector2D( 0.0, 0.0 ),
          wm.self().vel(),
          1 + n_turn, // kick + turns
          wm.self().playerType().playerDecay() );

    const double control_dist = wm.self().kickableArea() * 0.7;

    const PlayerObject * nearest_opp = wm.getOpponentNearestToBall( 5 );
    if ( nearest_opp
         && nearest_opp->distFromBall() < 5.0 )
    {
        double best_angle = 0.0;
        double min_dist2 = 100000.0;
        for ( double angle = -90.0; angle <= 91.0; angle += 10.0 )
        {
            Vector2D keep_pos = my_pos + Vector2D::from_polar( control_dist, target_angle + angle );
            double d2 = nearest_opp->pos().dist2( keep_pos );
            if ( d2 < min_dist2 )
            {
                best_angle = angle;
                min_dist2 = d2;
            }
        }
        keep_global_angle = target_angle + best_angle;
        dlog.addText( Logger::DRIBBLE,
                      "%s: doKickTurnsDash. target_angle = %.0f best_keep_angle = %.0f  rel = %.0f"
                      ,__FILE__,
                      target_angle.degree(),
                      keep_global_angle.degree(), best_angle );
    }
    else if ( ! exist_opp )
    {
        if ( target_angle.isLeftOf( wm.ball().angleFromSelf() ) )
        {
            keep_global_angle = target_angle + 35.0;
        }
        else
        {
            keep_global_angle = target_angle - 35.0;
        }
    }

    // relative to current my pos, angle is global
    Vector2D required_ball_rel_pos
        = my_pos
        + Vector2D::polar2vector( control_dist, keep_global_angle );


    // travel = firstvel * (1 + dec + dec^2 + ...)
    // firstvel = travel / (1 + dec + dec^2 + ...)
    const double term
        = ( 1.0 - std::pow( ServerParam::i().ballDecay(),
                            static_cast< double >( n_turn + 2 ) ) )
        / ( 1.0 - ServerParam::i().ballDecay() );
    const Vector2D required_first_vel
        = ( required_ball_rel_pos - wm.ball().rpos() ) / term;
    const Vector2D required_accel
        = required_first_vel
        - wm.ball().vel();

    // check power overflow
    const double required_kick_power
        = required_accel.r() / wm.self().kickRate();

    // cannot get the required accel using only one kick
    if ( required_kick_power > ServerParam::i().maxPower()
         || required_first_vel.r() > ServerParam::i().ballSpeedMax() )
    {
        dlog.addText( Logger::DRIBBLE,
                      "%s: doKickTurnsDash. kick power over= %.2f or required speed over= %.2f"
                      ,__FILE__,
                      required_kick_power, required_first_vel.r() );

        Vector2D ball_next
            = wm.self().pos() + wm.self().vel()
            + Vector2D::polar2vector( control_dist, keep_global_angle );
        if ( ball_next.absX() > ServerParam::i().pitchHalfLength() - 0.5
             || ball_next.absY() > ServerParam::i().pitchHalfLength() - 0.5 )
        {
            dlog.addText( Logger::DRIBBLE,
                          "%s: doKickTurnsDash. maybe out of pitch. keep_pos=(%.1f %.1f)"
                          ,__FILE__,
                          ball_next.x, ball_next.y );
            return false;
        }

        agent->debugClient().addMessage( "DribRotPowerOver" );
        return Body_KickToRelative( control_dist,
                                    keep_global_angle - wm.self().body(),
                                    false // not need to stop
                                    ).execute( agent );
    }

    // check collision
    {
        Vector2D tmp_my_pos( 0.0, 0.0 );
        Vector2D my_vel = wm.self().vel();
        Vector2D ball_pos = wm.ball().rpos();
        Vector2D ball_vel = required_first_vel;
        const double collide_dist2
            = square( wm.self().playerType().playerSize()
                      + ServerParam::i().ballSize() );
        for ( int i = 1; i < n_turn + 1; i++ )
        {
            tmp_my_pos += my_vel;
            ball_pos += ball_vel;

            if ( tmp_my_pos.dist2( ball_pos ) < collide_dist2 )
            {
                dlog.addText( Logger::DRIBBLE,
                              "%s: doKickTurnsDash. maybe cause collision. keep_angle=%.0f"
                              ,__FILE__,
                              keep_global_angle.degree() );

                Vector2D ball_next
                    = wm.self().pos() + wm.self().vel()
                    + Vector2D::polar2vector( control_dist, keep_global_angle );
                if ( ball_next.absX() > ServerParam::i().pitchHalfLength() - 0.5
                     || ball_next.absY() > ServerParam::i().pitchHalfLength() - 0.5 )
                {
                    dlog.addText( Logger::DRIBBLE,
                                  "%s: doKickTurnsDash. maybe out of pitch. keep_pos=(%.1f %.1f)"
                                  ,__FILE__,
                                  ball_next.x, ball_next.y );
                    return false;
                }

                agent->debugClient().addMessage( "DribRotNoColKick" );
                return Body_KickToRelative( control_dist,
                                            keep_global_angle - wm.self().body(),
                                            false // not need to stop
                                            ).execute( agent );
            }

            my_vel *= wm.self().playerType().playerDecay();
            ball_vel *= ServerParam::i().ballDecay();
        }
    }

    // can archieve required vel

    dlog.addText( Logger::DRIBBLE,
                  "%s: doKickTurnsDash() kick -> turn[%d]"
                  ,__FILE__,
                  n_turn );
    agent->debugClient().addMessage( "DribKT%dD", n_turn );

    //////////////////////////////////////////////////////////
    // register intention
    agent->setIntention
        ( new IntentionDribble2007( target_point,
                                    M_dist_thr,
                                    n_turn,
                                    1, // one dash
                                    std::fabs( dash_power ),
                                    ( dash_power < 0.0 ), // back_dash
                                    wm.time() ) );
    say( agent, target_point, n_turn + 1 );
#ifdef USE_CHANGE_VIEW
    if ( n_turn + 1 >= 3 )
    {
        agent->setViewAction( new View_Normal() );
    }
#endif

    // execute first kick
    return agent->doKick( required_kick_power,
                          required_accel.th() - wm.self().body() );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_Dribble2007::doKickTurnsDashes(  PlayerAgent * agent,
                                      const Vector2D & target_point,
                                      const double & dash_power,
                                      const int n_turn )
{
    const int max_dash = 5;

    const WorldModel & wm = agent->world();

    const double trap_ball_dist = ( wm.self().playerType().kickableArea()
                                    - wm.self().playerType().playerSize() ) * 0.4;

    std::vector< Vector2D > self_cache;
    createSelfCache( agent,
                     target_point, dash_power,
                     n_turn, max_dash, self_cache );

    const AngleDeg accel_angle = ( target_point - self_cache[n_turn] ).th();

    for ( int n_dash = max_dash; n_dash >= 2; --n_dash )
    {
        const Vector2D ball_trap_pos
            = self_cache[n_turn + n_dash]
            + Vector2D::polar2vector( trap_ball_dist, accel_angle );

        dlog.addText( Logger::DRIBBLE,
                      __FILE__": doKickTurnsDashes() target=(%.1f %.1f) n_turn=%d n_dash=%d"
                      " ball_trap=(%.1f %.1f)",
                      target_point.x, target_point.y,
                      n_turn, n_dash,
                      ball_trap_pos.x, ball_trap_pos.y );

        if ( ball_trap_pos.absX() > ServerParam::i().pitchHalfLength() - 0.5
             || ball_trap_pos.absY() > ServerParam::i().pitchHalfWidth() - 0.5 )
        {
            dlog.addText( Logger::DRIBBLE,
                          "---> pitch over" );
            continue;
        }

        bool failed = false;

        const PlayerPtrCont::const_iterator o_end = wm.opponentsFromSelf().end();
        for ( PlayerPtrCont::const_iterator o = wm.opponentsFromSelf().begin();
              o != o_end;
              ++o )
        {
            if ( (*o)->distFromSelf() > 30.0 ) break;

            double dist = (*o)->pos().dist( ball_trap_pos );
            int dash_step = std::min( 10, (*o)->posCount() )
                + 1 + n_turn + n_dash
                //- 1 // opponent starts to intercept after the kick
                ;
            if ( (*o)->goalie() )
            {
                if ( dist < ( ServerParam::i().defaultRealSpeedMax() * dash_step
                              + ServerParam::i().catchableArea() )
                     )
                {
                    dlog.addText( Logger::DRIBBLE,
                                  "--->detect goalie reachable." );
                    failed = true;
                    break;
                }
            }
            else
            {
                const PlayerType * player_type = (*o)->playerTypePtr();
                double speed_max = ( player_type
                                     ? player_type->realSpeedMax()
                                     : ServerParam::i().defaultPlayerSpeedMax() );
                if ( dist < ( speed_max * dash_step + ServerParam::i().tackleDist() )
                     )
                {
                    dlog.addText( Logger::DRIBBLE,
                                  "--->detect opponent %d(%.1f %.1f) reachable.",
                                  (*o)->unum(), (*o)->pos().x, (*o)->pos().y );
                    failed = true;
                    break;
                }
            }
        }

        if ( failed ) continue;

        const double term
            = ( 1.0 - std::pow( ServerParam::i().ballDecay(), 1 + n_turn + n_dash ) )
            / ( 1.0 - ServerParam::i().ballDecay() );
        Vector2D first_vel = ( ball_trap_pos - wm.ball().pos() ) / term;
        Vector2D kick_accel = first_vel - wm.ball().vel();
        double kick_power = kick_accel.r() / wm.self().kickRate();

        if ( kick_power > ServerParam::i().maxPower()
             || kick_accel.r() > ServerParam::i().ballAccelMax()
             || first_vel.r() > ServerParam::i().ballSpeedMax() )
        {
            dlog.addText( Logger::DRIBBLE,
                          "-->cannot achieve. first_vel=(%.1f %.1f)r%.2f accel=(%.1f %.1f)r%.2f power=%.1f",
                          first_vel.x, first_vel.y, first_vel.r(),
                          kick_accel.x, kick_accel.y, kick_accel.r(),
                          kick_power );
            continue;;
        }

        agent->debugClient().addMessage( "DribKT%dD%d:%.0f",
                                         n_turn, n_dash, dash_power );
        agent->debugClient().addLine( wm.self().pos(), self_cache[n_turn + n_dash] );
        dlog.addCircle( Logger::DRIBBLE,
                        ball_trap_pos, 0.1, "#FF00FF" );
        dlog.addText( Logger::DRIBBLE,
                      "<--- turn=%d dash=%d. first_vel=(%.1f %.1f) accel=(%.1f %.1f) power=%.1f",
                      n_turn, n_dash,
                      first_vel.x, first_vel.y,
                      kick_accel.x, kick_accel.y,
                      kick_power );

        agent->doKick( kick_power, kick_accel.th() - wm.self().body() );

        agent->setIntention
            ( new IntentionDribble2007( target_point,
                                        M_dist_thr,
                                        n_turn,
                                        n_dash,
                                        std::fabs( dash_power ),
                                        ( dash_power < 0.0 ), // back_dash
                                        wm.time() ) );
        say( agent, target_point, n_dash + n_turn );
#ifdef USE_CHANGE_VIEW
        if ( n_turn + n_dash >= 3 )
        {
            agent->setViewAction( new View_Normal() );
        }
#endif
        return true;
    }

    return false;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_Dribble2007::doKickDashes( PlayerAgent * agent,
                                const Vector2D & target_point,
                                const double & dash_power,
                                const int dash_count )
{
    // do dribble kick. simulate next action queue.
    // kick -> dash -> dash -> ...

    const WorldModel & wm = agent->world();

    ////////////////////////////////////////////////////////
    // simulate my pos after one kick & dashes
    std::vector< Vector2D > self_cache;
    createSelfCache( agent,
                     target_point, dash_power,
                     0, dash_count, // no turn
                     self_cache );

    // my moved position after 1 kick and n dashes
    const Vector2D my_pos = self_cache.back() - wm.self().pos();
    const double my_move_dist = my_pos.r();
    // my move direction
    const AngleDeg my_move_dir = my_pos.th();

    const AngleDeg accel_angle = ( dash_power > 0.0
                                   ? wm.self().body()
                                   : wm.self().body() - 180.0 );

    ////////////////////////////////////////////////////////
    // estimate required kick param
    dlog.addText( Logger::DRIBBLE,
                  "%s: doKickDashes() my move dist = %.3f  dir = %.1f  accel_angle=%.1f"
                  ,__FILE__,
                  my_move_dist, my_move_dir.degree(),
                  accel_angle.degree() );


    // decide next ball control point

    AngleDeg keep_global_angle;
    bool exist_close_opp = existCloseOpponent( agent, &keep_global_angle );

    double control_dist;
    double add_angle_abs;
    {
        double y_dist
            = wm.self().playerType().playerSize()
            + ServerParam::i().ballSize()
            + 0.2;
        Vector2D cur_ball_rel = wm.ball().rpos().rotatedVector( - my_move_dir );
        if ( cur_ball_rel.absY() < y_dist )
        {
            dlog.addText( Logger::DRIBBLE,
                          "%s: doKickDashes() y_dist(%.2f) is"
                          " inner from keep Y(%2.f). correct."
                          ,__FILE__,
                          cur_ball_rel.absY(), y_dist );
            //y_dist = ( y_dist + cur_ball_rel.absY() ) * 0.5;
            y_dist += 0.1;
            y_dist = std::min( y_dist, cur_ball_rel.absY() );
        }
        double x_dist
            = std::sqrt( std::pow( wm.self().playerType().kickableArea(), 2.0 )
                         - std::pow( y_dist, 2.0 ) )
            - 0.2 - std::min( 0.6, my_move_dist * 0.05 );
        control_dist = std::sqrt( std::pow( x_dist, 2.0 )
                                  + std::pow( y_dist, 2.0 ) );
        add_angle_abs = std::fabs( AngleDeg::atan2_deg( y_dist, x_dist ) );
    }

    if ( exist_close_opp )
    {
//         if ( my_move_dir.isLeftOf( keep_global_angle ) )
//         {
//             keep_global_angle = my_move_dir + add_angle_abs;
//             dlog.addText( Logger::DRIBBLE,
//                           "%s: doKickDashes() avoid. keep right"
//                           ,__FILE__ );
//         }
//         else
//         {
//             keep_global_angle = my_move_dir - add_angle_abs;
//             dlog.addText( Logger::DRIBBLE,
//                           "%s: doKickDashes() avoid. keep left"
//                           ,__FILE__ );
//         }
    }
    else
    {
        if ( my_move_dir.isLeftOf( wm.ball().angleFromSelf() ) )
        {
            keep_global_angle = my_move_dir + add_angle_abs;
            dlog.addText( Logger::DRIBBLE,
                          "%s: doKickDashes() keep right."
                          " accel_angle= %.1f < ball_angle=%.1f"
                          ,__FILE__,
                          accel_angle.degree(),
                          wm.ball().angleFromSelf().degree() );
        }
        else
        {
            keep_global_angle = my_move_dir - add_angle_abs;
            dlog.addText( Logger::DRIBBLE,
                          "%s: doKickDashes() keep left."
                          " accel_angle= %.1f > ball_angle=%.1f"
                          ,__FILE__,
                          accel_angle.degree(),
                          wm.ball().angleFromSelf().degree() );
        }
    }

    const Vector2D next_ball_rel
        = Vector2D::polar2vector( control_dist, keep_global_angle );
    Vector2D next_ctrl_ball_pos = wm.self().pos() + my_pos + next_ball_rel;

    dlog.addText( Logger::DRIBBLE,
                  "%s: doKickDashes() next_ball_rel=(%.2f, %.2f) global(%.2f %.2f)"
                  " ctrl_dist= %.2f, keep_anggle=%.1f"
                  ,__FILE__,
                  next_ball_rel.x, next_ball_rel.y,
                  next_ctrl_ball_pos.x, next_ctrl_ball_pos.y,
                  control_dist, keep_global_angle.degree() );

    // calculate required kick param

    // relative to current my pos
    const Vector2D required_ball_pos = my_pos + next_ball_rel;
    const double term
        = ( 1.0 - std::pow( ServerParam::i().ballDecay(), dash_count + 1 ) )
        / ( 1.0 - ServerParam::i().ballDecay() );
    const Vector2D required_first_vel
        = (required_ball_pos - wm.ball().rpos()) / term;
    const Vector2D required_accel
        = required_first_vel
        - wm.ball().vel();
    const double required_kick_power
        = required_accel.r() / wm.self().kickRate();

    ////////////////////////////////////////////////////////
    // never kickable
    if ( required_kick_power > ServerParam::i().maxPower()
         || required_first_vel.r() > ServerParam::i().ballSpeedMax() )
    {
        dlog.addText( Logger::DRIBBLE,
                      "%s: doKickDashes() never reach. rotate."
                      ,__FILE__ );
        agent->debugClient().addMessage( "DribKDFail" );

        return Body_KickToRelative( wm.self().kickableArea() * 0.7,
                                    keep_global_angle - wm.self().body(),
                                    false  // not need to stop
                                    ).execute( agent );
    }


    ////////////////////////////////////////////////////////
    // check next collision

    const double collide_dist2
        = std::pow( wm.self().playerType().playerSize()
                    + ServerParam::i().ballSize()
                    + 0.15,
                    2.0);
    if ( ( wm.ball().rpos()
           + required_first_vel - wm.self().vel() ).r2() // next rel pos
         < collide_dist2 )
    {
        AngleDeg rotate_global_angle = keep_global_angle;
        if ( ( wm.ball().angleFromSelf() - my_move_dir ).abs() > 90.0 )
        {
            if ( keep_global_angle.isLeftOf( my_move_dir ) )
            {
                rotate_global_angle = my_move_dir + 90.0;
            }
            else
            {
                rotate_global_angle = my_move_dir + 90.0;
            }
        }
        AngleDeg rotate_rel_angle = rotate_global_angle - wm.self().body();
        dlog.addText( Logger::DRIBBLE,
                      "%s: doKickDashes() maybe collision. rotate. rel_angle=%f"
                      ,__FILE__,
                      rotate_rel_angle.degree() );
        agent->debugClient().addMessage( "DribKDCol" );
        return Body_KickToRelative( wm.self().kickableArea() * 0.7,
                                    rotate_rel_angle,
                                    false  // not need to stop
                                    ).execute( agent );
    }

    agent->debugClient().addMessage( "DribKD%d:%.0f", dash_count, dash_power );
    agent->debugClient().addLine( wm.self().pos(), wm.self().pos() + my_pos );
    //////////////////////////////////////////////////////////
    // register intention
    agent->setIntention
        ( new IntentionDribble2007( target_point,
                                    M_dist_thr,
                                    0, // zero turn
                                    dash_count,
                                    std::fabs( dash_power ),
                                    ( dash_power < 0.0 ), // back_dash
                                    wm.time() ) );
    say( agent, target_point, dash_count );
#ifdef USE_CHANGE_VIEW
    if ( dash_count >= 2 )
    {
        agent->setViewAction( new View_Normal() );
    }
#endif

    dlog.addText( Logger::DRIBBLE,
                  "%s: doKickDashes() register intention. dash_count=%d"
                  ,__FILE__,
                  dash_count );

#if 1
    {
        char msg[16];
        int count = 0;
        char r = (char)255, g = (char)0, b = (char)0;
        Vector2D bpos = wm.ball().pos() + required_first_vel;
        Vector2D bvel = required_first_vel;
        for ( std::vector< Vector2D >::iterator p = self_cache.begin();
              p != self_cache.end();
              ++p, ++count )
        {
            std::snprintf( msg, 16, "d%d", count );
            dlog.addCircle( Logger::DRIBBLE,
                            *p, 0.1, r, g, b );
            dlog.addMessage( Logger::DRIBBLE,
                             p->x, p->y - 0.1, msg, r, g, b );
            b += 16;

            dlog.addCircle( Logger::DRIBBLE,
                            *p, 0.1, r, 255, b );
            bvel *= ServerParam::i().ballDecay();
            bpos += bvel;
        }
    }
#endif

    // execute first kick
    return agent->doKick( required_kick_power,
                          required_accel.th() - wm.self().body() );
}


/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_Dribble2007::doKickDashesWithBall( PlayerAgent * agent,
                                        const Vector2D & target_point,
                                        const double & dash_power,
                                        const int dash_count,
                                        const bool dodge_mode )
{
    // do dribble kick. simulate next action queue.
    // kick -> dash -> dash -> ...
    dlog.addText( Logger::DRIBBLE,
                  "%s:%d: doKickDashesWithBall."
                  ,__FILE__, __LINE__ );

    const WorldModel & wm = agent->world();

    // estimate my move positions
    std::vector< Vector2D > my_state;
    createSelfCache( agent,
                     target_point, dash_power,
                     0, // no turn
                     std::max( 4, dash_count ),
                     my_state );


    const double collide_dist
        = wm.self().playerType().playerSize()
        + ServerParam::i().ballSize();
    const double kickable_area = wm.self().kickableArea();
    const double ball_decay = ServerParam::i().ballDecay();

    const Rect2D pitch_rect( - ServerParam::i().pitchHalfLength() + 1.0,
                             - ServerParam::i().pitchHalfWidth() + 1.0,
                             ServerParam::i().pitchLength() - 1.0,
                             ServerParam::i().pitchWidth() - 1.0 );

    const std::vector< Vector2D >::const_iterator my_pos_end = my_state.end();


    std::list< KeepDribbleInfo > dribble_info;

    const AngleDeg accel_angle = ( dash_power > 0.0
                                   ? wm.self().body()
                                   : wm.self().body() - 180.0 );

    const int DIST_DIVS = 10;

    const double max_dist = wm.self().kickableArea() + 0.2;
    double first_ball_dist = ( wm.self().playerType().playerSize()
                               + ServerParam::i().ballSize()
                               + 0.15 );
    const double dist_step = ( max_dist - first_ball_dist ) / ( DIST_DIVS - 1 );

    const double angle_range = 240.0;
    const double angle_range_forward = 160.0;
    const double arc_dist_step = 0.1;

    int total_loop_count = 0;

    for ( int dist_loop = 0;
          dist_loop < DIST_DIVS;
          ++dist_loop, first_ball_dist += dist_step )
    {
        const double angle_step
            = ( arc_dist_step * 360.0 )
            / ( 2.0 * first_ball_dist * M_PI );
        const int ANGLE_DIVS
            = ( first_ball_dist < wm.self().kickableArea() - 0.1
                ? static_cast< int >( std::ceil( angle_range / angle_step ) ) + 1
                : static_cast< int >( std::ceil( angle_range_forward / angle_step ) ) + 1 );

        AngleDeg first_ball_angle = accel_angle - angle_step * ( ANGLE_DIVS/2 );

        // angle loop
        for ( int angle_loop = 0;
              angle_loop < ANGLE_DIVS;
              ++angle_loop, first_ball_angle += angle_step )
        {
            ++total_loop_count;

            Vector2D ball_pos
                = my_state.front()
                + Vector2D::polar2vector( first_ball_dist, first_ball_angle );
            double min_opp_dist = 1000.0;

            if ( ! pitch_rect.contains( ball_pos ) )
            {
                // out of pitch
                continue;
            }

            const Vector2D first_ball_vel = ball_pos - wm.ball().pos();
            const Vector2D first_ball_accel = first_ball_vel - wm.ball().vel();
            const double first_ball_accel_r = first_ball_accel.r();

            if ( first_ball_vel.r() > ServerParam::i().ballSpeedMax()
                 || first_ball_accel_r > ServerParam::i().ballAccelMax()
                 || ( first_ball_accel_r
                      > wm.self().kickRate() * ServerParam::i().maxPower() )
                 )
            {
                // cannot acccelerate to the desired speed
                continue;
            }

            if ( existKickableOpponent( wm, ball_pos, &min_opp_dist ) )
            {
                continue;
            }

            Vector2D ball_vel = first_ball_vel;
            ball_vel *= ball_decay;

            int tmp_dash_count = 0;
            Vector2D ball_move;

            // future state loop
            for ( std::vector< Vector2D >::const_iterator my_pos = my_state.begin() + 1;
                  my_pos != my_pos_end;
                  ++my_pos )
            {
                ball_pos += ball_vel;

                // out of pitch
                if ( ! pitch_rect.contains( ball_pos ) ) break;

                const Vector2D ball_rel = ( ball_pos - *my_pos ).rotatedVector( - accel_angle );
                const double new_ball_dist = ball_rel.r();

                const double ball_travel = ball_pos.dist( wm.ball().pos() );
                const double my_travel = my_pos->dist( wm.self().pos() );

                // check collision
                //double dist_buf = std::min( 0.01 * ball_travel + 0.02 * my_travel,
                //                            0.3 );
                //if ( new_ball_dist < collide_dist - dist_buf + 0.1 ) break;
                //double dist_buf = std::min( 0.02 * ball_travel + 0.03 * my_travel,
                //                            0.3 );
                //if ( new_ball_dist < collide_dist - dist_buf + 0.15 ) break;
                //double dist_buf = std::min( 0.02 * ball_travel + 0.03 * my_travel,
                //                            0.1 );
                //if ( new_ball_dist < collide_dist - dist_buf + 0.15 ) break;
                double dist_buf = std::min( 0.02 * ball_travel + 0.03 * my_travel,
                                            0.1 );
                if ( new_ball_dist < collide_dist - dist_buf + 0.2 ) break;

                // check kickable

                if ( tmp_dash_count == dash_count - 1
                     && ball_rel.x > 0.0
                     && new_ball_dist > kickable_area - 0.25 ) break;

                //dist_buf = std::min( 0.025 * ball_travel + 0.05 * my_travel,
                //                     0.2 );
                //if ( new_ball_dist > kickable_area + dist_buf ) break;
                //if ( new_ball_dist > kickable_area - 0.2 * std::pow( 0.8, tmp_dash_count ) ) break;
                if ( new_ball_dist > kickable_area - 0.2 ) break;

                // front x buffer
                dist_buf = std::min( 0.02 * ball_travel + 0.04 * my_travel,
                                     0.2 );
                if ( ball_rel.x > kickable_area - dist_buf - 0.2 ) break;

                // side y buffer
                //dist_buf = std::min( 0.02 * ball_travel + 0.04 + my_travel,
                //                     0.2 );
                //if ( ball_rel.absY() > kickable_area - dist_buf - 0.25 ) break;
                dist_buf = std::min( 0.02 * ball_travel + 0.055 + my_travel,
                                     0.35 );
                if ( ball_rel.absY() > kickable_area - dist_buf - 0.15 ) break;

                // check opponent kickable possibility
                if ( existKickableOpponent( wm, ball_pos, &min_opp_dist ) )
                {
                    break;
                }

                ball_move = ball_pos - wm.ball().pos();
                ++tmp_dash_count;
                ball_vel *= ball_decay;
            }
#if 0
            if ( tmp_dash_count >= 1 )
            {
                ball_pos = wm.ball().pos() + ball_move;
                Vector2D ball_rel = ( ball_pos - my_state[tmp_dash_count + 1] ).rotatedVector( - accel_angle );
                if ( ball_rel.x > 0.2
                     && ball_rel.r() > kickable_area - 0.4 )//0.15 - ball_travel * 0.05 * 1.1414 )
                {
                    continue;
                }
            }
#endif
            if ( tmp_dash_count <= 0 )
            {
                //dlog.addText( Logger::DRIBBLE,
                //              "_____ failed bdist=%.2f bangle=%.1f"
                //              " vel=(%.1f %.1f)",
                //              first_ball_dist,
                //              (first_ball_angle - accel_angle).degree(),
                //              first_ball_vel.x, first_ball_vel.y );
                continue;
            }

            dribble_info.push_back( KeepDribbleInfo( first_ball_vel,
                                                     ball_move.rotate( - accel_angle ).x,
                                                     tmp_dash_count,
                                                     min_opp_dist ) );
            dlog.addText( Logger::DRIBBLE,
                          "_____ add bdist=%.2f bangle=%.1f"
                          " vel=(%.1f %.1f) dash=%d  opp_dist=%.1f",
                          first_ball_dist,
                          //(first_ball_angle - accel_angle).degree(),
                          first_ball_angle.degree(),
                          first_ball_vel.x, first_ball_vel.y,
                          tmp_dash_count,
                          min_opp_dist );
        }
    }

    dlog.addText( Logger::DRIBBLE,
                  "___ total loop = %d, solution size = %d",
                  total_loop_count, dribble_info.size() );

    if ( dribble_info.empty() )
    {
        dlog.addText( Logger::DRIBBLE,
                      "%s: doKickDashesWithBall() WARNING. no solution!"
                      ,__FILE__ );

        return false;
    }

    std::list< KeepDribbleInfo >::const_iterator dribble
        = std::min_element( dribble_info.begin(),
                            dribble_info.end(),
                            KeepDribbleCmp() );

    if ( dribble == dribble_info.end() )
    {
        dlog.addText( Logger::DRIBBLE,
                      "%s: doKickDashesWithBall() WARNING. no solution!"
                      ,__FILE__ );
        return false;
    }

    if ( dodge_mode
         && dash_count > dribble->dash_count_ )
    {
        dlog.addText( Logger::DRIBBLE,
                      "%s: doKickDashesWithBall() dodge mode. but not found. required_dash=%d found_dash=%d"
                      ,__FILE__,
                      dash_count, dribble->dash_count_ );
        return false;
    }

    agent->debugClient().addMessage( "DribKDKeep%d:%.0f",
                                     dribble->dash_count_,
                                     dash_power );

    dlog.addText( Logger::DRIBBLE,
                  "%s: doKickDashesWithBall() dash_count=%d, ball_vel=(%.1f %.1f) ball_travel_x=%.1f"
                  ,__FILE__,
                  dribble->dash_count_,
                  dribble->first_ball_vel_.x,
                  dribble->first_ball_vel_.y,
                  dribble->ball_forward_travel_ );

#ifdef DEBUG
    {
        Vector2D ball_pos = wm.ball().pos();
        Vector2D ball_vel = dribble->first_ball_vel_;
        const int DASH = dribble->dash_count_ + 1;
        for ( int i = 0; i < DASH; ++i )
        {
            ball_pos += ball_vel;
            ball_vel *= ServerParam::i().ballDecay();
            dlog.addCircle( Logger::DRIBBLE,
                            ball_pos, 0.05, "#0000FF" );
            dlog.addCircle( Logger::DRIBBLE,
                            my_state[i], wm.self().kickableArea(), "#0000FF" );
            //agent->debugClient().addCircle( ball_pos, 0.05 );
            //agent->debugClient().addCircle( my_state[i], wm.self().kickableArea() );
        }
    }
#endif

    Vector2D kick_accel = dribble->first_ball_vel_ - wm.ball().vel();

    // execute first kick
    agent->doKick( kick_accel.r() / wm.self().kickRate(),
                   kick_accel.th() - wm.self().body() );


    //////////////////////////////////////////////////////////
    // register intention

    agent->setIntention
        ( new IntentionDribble2007( target_point,
                                    M_dist_thr,
                                    0, // zero turn
                                    std::min( dribble->dash_count_, dash_count ),
                                    std::fabs( dash_power ),
                                    ( dash_power < 0.0 ), // back_dash
                                    wm.time() ) );
    say( agent, target_point, std::min( dribble->dash_count_, dash_count ) );
#ifdef USE_CHANGE_VIEW
    if ( std::min( dribble->dash_count_, dash_count ) >= 2 )
    {
        agent->setViewAction( new View_Normal() );
    }
#endif

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
Body_Dribble2007::createSelfCache( PlayerAgent * agent,
                                   const Vector2D & target_point,
                                   const double & dash_power,
                                   const int turn_count,
                                   const int dash_count,
                                   std::vector< Vector2D > & self_cache )
{
    const WorldModel & wm = agent->world();

    self_cache.clear();
    self_cache.reserve( turn_count + dash_count + 1 );

    double my_stamina = wm.self().stamina();
    double my_effort = wm.self().effort();
    double my_recovery = wm.self().recovery();

    Vector2D my_pos = wm.self().pos();
    Vector2D my_vel = wm.self().vel();

    my_pos += my_vel;
    my_vel *= wm.self().playerType().playerDecay();

    self_cache.push_back( my_pos ); // first element is next cycle just after kick

    for ( int i = 0; i < turn_count; ++i )
    {
        my_pos += my_vel;
        my_vel *= wm.self().playerType().playerDecay();
        self_cache.push_back( my_pos );
    }

    wm.self().playerType().predictStaminaAfterWait( ServerParam::i(),
                                                    1 + turn_count,
                                                    &my_stamina,
                                                    &my_effort,
                                                    my_recovery );
    AngleDeg accel_angle;
    if ( turn_count == 0 )
    {
        accel_angle = ( dash_power > 0.0
                        ? wm.self().body()
                        : wm.self().body() - 180.0 );
    }
    else
    {
        accel_angle = ( target_point - wm.self().inertiaFinalPoint() ).th();
    }

    for ( int i = 0; i < dash_count; ++i )
    {
        double available_stamina
            =  std::max( 0.0,
                         my_stamina
                         - ServerParam::i().recoverDecThrValue()
                         - 300.0 );
        double consumed_stamina = ( dash_power > 0.0
                                    ? dash_power
                                    : dash_power * -2.0 );
        consumed_stamina = std::min( available_stamina,
                                     consumed_stamina );
        double used_power = ( dash_power > 0.0
                              ? consumed_stamina
                              : consumed_stamina * -0.5 );
        double max_accel_mag = ( std::fabs( used_power )
                                 * wm.self().playerType().dashPowerRate()
                                 * my_effort );
        double accel_mag = max_accel_mag;
        if ( wm.self().playerType().normalizeAccel( my_vel,
                                                    accel_angle,
                                                    &accel_mag ) )
        {
            used_power *= accel_mag / max_accel_mag;
        }

        Vector2D dash_accel
            = Vector2D::polar2vector( std::fabs( used_power )
                                      * my_effort
                                      * wm.self().playerType().dashPowerRate(),
                                      accel_angle );
        my_vel += dash_accel;
        my_pos += my_vel;

        self_cache.push_back( my_pos );

        my_vel *= wm.self().playerType().playerDecay();

        wm.self().playerType().predictStaminaAfterDash( ServerParam::i(),
                                                        used_power,
                                                        &my_stamina,
                                                        &my_effort,
                                                        &my_recovery );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_Dribble2007::existKickableOpponent( const WorldModel & wm,
                                         const Vector2D & ball_pos,
                                         double * min_opp_dist ) const
{
    static const double kickable_area
        = ServerParam::i().defaultKickableArea() + 0.2;

    const PlayerPtrCont::const_iterator end = wm.opponentsFromSelf().end();
    for ( PlayerPtrCont::const_iterator it = wm.opponentsFromSelf().begin();
          it != end;
          ++it )
    {
        if ( (*it)->posCount() > 5 )
        {
            continue;
        }

        if ( (*it)->distFromSelf() > 30.0 )
        {
            break;
        }

        // goalie's catchable check
        if ( (*it)->goalie() )
        {
            if ( ball_pos.x > ServerParam::i().theirPenaltyAreaLineX()
                 && ball_pos.absY() < ServerParam::i().penaltyAreaHalfWidth() )
            {
                double d = (*it)->pos().dist( ball_pos );
                if ( d < ServerParam::i().catchableArea() )
                {
                    return true;
                }

                d -= ServerParam::i().catchableArea();
                if ( *min_opp_dist > d )
                {
                    *min_opp_dist = d;
                }
            }
        }

        // normal kickable check
        double d = (*it)->pos().dist( ball_pos );
        if ( d < kickable_area )
        {
            return true;
        }

        if ( *min_opp_dist > d )
        {
            *min_opp_dist = d;
        }
    }

    return false;
}


/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_Dribble2007::doDodge( PlayerAgent * agent,
                           const Vector2D & target_point )
{
    const WorldModel & wm = agent->world();

    const double new_target_dist = 6.0;

    AngleDeg avoid_angle
        = getAvoidAngle( agent,
                         ( target_point - wm.self().pos() ).th() );

    const Vector2D new_target_rel
        = Vector2D::polar2vector( new_target_dist, avoid_angle );
    Vector2D new_target
        = wm.self().pos()
        + new_target_rel;

    dlog.addText( Logger::DRIBBLE,
                  "%s: doDodge. avoid_angle=%.1f"
                  ,__FILE__,
                  avoid_angle.degree() );
    agent->debugClient().addCircle( new_target, 0.7 );

    const PlayerPtrCont & opponents = wm.opponentsFromSelf();
    {
        const PlayerPtrCont::const_iterator end = opponents.end();
        for ( PlayerPtrCont::const_iterator it = opponents.begin();
              it != end;
              ++it )
        {
            if ( (*it)->posCount() >= 5 ) break;
            if ( (*it)->distFromSelf() > 3.0 ) break;

            if ( ( (*it)->angleFromSelf() - avoid_angle ).abs() > 90.0 )
            {
                continue;
            }

            if ( (*it)->distFromSelf()
                 < ServerParam::i().defaultKickableArea() + 0.3 )
            {
                dlog.addText( Logger::DRIBBLE,
                              "%s: doDodge. emergency avoidance"
                              ,__FILE__ );
                return doAvoidKick( agent, avoid_angle );
            }
        }
    }

    double min_opp_dist = ( opponents.empty()
                            ? 100.0
                            : opponents.front()->distFromSelf() );

    double dir_diff_abs = ( avoid_angle - wm.self().body() ).abs();
    double avoid_dash_power;
    if ( min_opp_dist > 3.0
         || dir_diff_abs < 120.0
         || agent->world().self().stamina() < ServerParam::i().staminaMax() * 0.5
         )
    {
        avoid_dash_power
            = agent->world().self().getSafetyDashPower( ServerParam::i().maxPower() );
    }
    else
    {
        // backward
        avoid_dash_power
            = agent->world().self().getSafetyDashPower( - ServerParam::i().maxPower() );

    }

    const double pitch_buffer = 1.0;
    if ( new_target.absX() > ServerParam::i().pitchHalfLength() - pitch_buffer )
    {
        double diff
            = new_target.absX()
            - (ServerParam::i().pitchHalfLength() - pitch_buffer);
        double rate = 1.0 - diff / new_target_rel.absX();
        new_target
            = wm.self().pos()
            + Vector2D::polar2vector( new_target_dist * rate,
                                      avoid_angle );
    }
    if ( new_target.absY() > ServerParam::i().pitchHalfWidth() - pitch_buffer )
    {
        double diff
            = new_target.absY()
            - (ServerParam::i().pitchHalfWidth() - pitch_buffer);
        double rate = 1.0 - diff / new_target_rel.absY();
        new_target
            = wm.self().pos()
            + Vector2D::polar2vector( new_target_dist * rate,
                                      avoid_angle );
    }

    int n_dash = 2;

    if ( avoid_dash_power > 0.0
         && wm.self().pos().x > -20.0
         && new_target.absY() > 15.0 )
    {
        double dist_to_target = wm.self().pos().dist( new_target );
        n_dash = wm.self().playerType().cyclesToReachDistance( dist_to_target );
        n_dash = std::min( 3, n_dash );

        dlog.addText( Logger::DRIBBLE,
                      "%s: doDodge. dash step = %d"
                      ,__FILE__,
                      n_dash );
    }

    {
        Ray2D drib_ray( wm.self().pos(), avoid_angle );
        Rect2D pitch_rect( - ServerParam::i().pitchHalfLength() + 0.5,
                           - ServerParam::i().pitchHalfWidth() + 0.5,
                           ServerParam::i().pitchLength() - 1.0,
                           ServerParam::i().pitchWidth() - 1.0 );
        Vector2D pitch_intersect;
        if ( pitch_rect.intersection( drib_ray, &pitch_intersect, NULL ) == 1 )
        {
            if ( wm.self().pos().dist( pitch_intersect ) < 7.0 )
            {
                dlog.addText( Logger::DRIBBLE,
                              "%s: doDodge. pitch intersection near."
                              " enforce 1 dash step"
                              ,__FILE__ );
                n_dash = 1;
            }
        }
    }

    return doAction( agent, new_target, avoid_dash_power,
                     n_dash, false, true ); // no dodge & dodge_mode flag
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_Dribble2007::doAvoidKick( PlayerAgent * agent,
                               const AngleDeg & avoid_angle )
{
    dlog.addText( Logger::DRIBBLE,
                  "%s: doAvoidKick"
                  ,__FILE__ );

    const WorldModel & wm = agent->world();

    const double ball_move_radius = 2.0;

    const Vector2D target_rel_point
        = Vector2D::polar2vector( ball_move_radius, avoid_angle );

    // my max turnable moment with current speed
    const double next_turnable
        = ServerParam::i().maxMoment()
        / ( 1.0
            + wm.self().playerType().inertiaMoment()
            * (wm.self().vel().r()
               * wm.self().playerType().playerDecay()) );
    // my inernia move vector
    const Vector2D my_final_rel_pos
        = wm.self().vel()
        / ( 1.0 - wm.self().playerType().playerDecay() );

    AngleDeg target_angle = (target_rel_point - my_final_rel_pos).th();
    double dir_diff_abs = (target_angle - wm.self().body()).abs();
    double dir_margin_abs
        = std::max( 12.0,
                    std::fabs( AngleDeg::atan2_deg( wm.self().kickableArea() * 0.8,
                                                    ball_move_radius ) ) );

    double ball_first_speed;
    // kick -> dash -> dash -> dash -> ...
    if ( dir_diff_abs < dir_margin_abs
         || dir_diff_abs > 180.0 - dir_margin_abs ) // backward dash
    {
        ball_first_speed = 0.7;
    }
    // kick -> turn -> dash -> dash -> ...
    else if ( dir_diff_abs < next_turnable
              || dir_diff_abs > 180.0 - next_turnable )
    {
        ball_first_speed = 0.5;
    }
    // kick -> turn -> turn -> dash -> ...
    else
    {
        ball_first_speed = 0.3;
    }

    Vector2D required_first_vel
        = Vector2D::polar2vector( ball_first_speed,
                                  ( target_rel_point - wm.ball().rpos() ).th() );
    Vector2D required_accel = required_first_vel - wm.ball().vel();
    double required_kick_power = required_accel.r() / wm.self().kickRate();

    // over max power
    if ( required_kick_power > ServerParam::i().maxPower() )
    {
        dlog.addText( Logger::DRIBBLE,
                      "%s:%d: doAvoidKick. power over. hold"
                      ,__FILE__, __LINE__ );
        Vector2D face_point
            = wm.self().pos()
            + Vector2D::polar2vector( 20.0, target_angle );
        return Body_HoldBall( true, face_point ).execute( agent );
    }

    // check collision
    if ( ( wm.ball().rpos() + required_first_vel ).dist( wm.self().vel() )
         < wm.self().playerType().playerSize() + ServerParam::i().ballSize() )
    {
        dlog.addText( Logger::DRIBBLE,
                      "%s:%d: doAvoidKick. detect collision. hold"
                      ,__FILE__, __LINE__ );
        Vector2D face_point
            = wm.self().pos()
            + Vector2D::polar2vector(20.0, target_angle);
        return Body_HoldBall( true, face_point ).execute( agent );
    }

    dlog.addText( Logger::DRIBBLE,
                  "%s:%d: doAvoidKick. done"
                  ,__FILE__, __LINE__ );
    agent->debugClient().addMessage( "AvoidKick" );

    return agent->doKick( required_kick_power,
                          required_accel.th() - wm.self().body() );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_Dribble2007::isDodgeSituation( const PlayerAgent * agent,
                                    const Vector2D & target_point )
{
    const WorldModel & wm = agent->world();

    //////////////////////////////////////////////////////

    const AngleDeg target_angle = (target_point - wm.self().pos()).th();
    // check if opponent on target dir
    const Sector2D sector( wm.self().pos(),
                           0.6,
                           std::min( 5, M_dash_count )
                           * ServerParam::i().defaultPlayerSpeedMax()
                           * 1.5, //10.0,
                           target_angle - 20.0, target_angle + 20.0 );
    const double base_safety_dir_diff = 60.0;
    double dodge_consider_dist
        = ( static_cast< double >( M_dash_count )
            * ServerParam::i().defaultPlayerSpeedMax() * 2.0 )
        + 4.0;

    if ( dodge_consider_dist > 10.0 ) dodge_consider_dist = 10.0;


    const PlayerPtrCont::const_iterator end = wm.opponentsFromSelf().end();
    for ( PlayerPtrCont::const_iterator it = wm.opponentsFromSelf().begin();
          it != end;
          ++it )
    {
        if ( (*it)->posCount() >= 10 ) continue;

        if ( sector.contains( (*it)->pos() ) )
        {
            dlog.addText( Logger::DRIBBLE,
                          "%s:%d: exist obstacle on dir"
                          ,__FILE__, __LINE__ );
            return true;
        }

        const double dir_diff = ( (*it)->angleFromSelf() - target_angle ).abs();
        double add_buf = 0.0;
        if ( (*it)->distFromSelf() < dodge_consider_dist
             && (*it)->distFromSelf() > 3.0 )
        {
            add_buf = 30.0 / (*it)->distFromSelf();
        }

        if ( (*it)->distFromSelf() < 1.0
             || ( (*it)->distFromSelf() < 1.5 && dir_diff < 120.0 )
             || ( (*it)->distFromSelf() < dodge_consider_dist
                  && dir_diff < base_safety_dir_diff + add_buf ) )
        {
            dlog.addText( Logger::DRIBBLE,
                          "%s:%d: exist obstacle (%.1f, %.1f) dist=%.2f"
                          " dir_diff=%,1f dir_buf=%.1f"
                          ,__FILE__, __LINE__,
                          (*it)->pos().x, (*it)->pos().y,
                          (*it)->distFromSelf(),
                          dir_diff, base_safety_dir_diff + add_buf );
            return true;
        }
    }

    return false;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_Dribble2007::canKickAfterDash( const PlayerAgent * agent,
                                    double * dash_power )
{
    const WorldModel & wm = agent->world();

    if ( wm.interceptTable()->opponentReachCycle() <= 1 )
    {
        dlog.addText( Logger::DRIBBLE,
                      "%s: canKickAfterDash..exist reachable opponent"
                      ,__FILE__ );
        return false;
    }

    const Vector2D ball_next
        = wm.ball().pos()
        + wm.ball().vel();

    const PlayerObject * opp = wm.getOpponentNearestToSelf( 5 );
    if ( opp
         && ( opp->pos().dist( ball_next )
              < ( ServerParam::i().defaultKickableArea()
                  + ServerParam::i().defaultPlayerSpeedMax()
                  + 0.3 ) )
         )
    {
        dlog.addText( Logger::DRIBBLE,
                      "%s:%d: canKickAfterDash."
                      " next_ball=(%.2f %.2f)"
                      " exist near opp(%.1f %.1f)"
                      ,__FILE__, __LINE__,
                      ball_next.x, ball_next.y,
                      opp->pos().x, opp->pos().y );
        return false;
    }

    // check safety
    AngleDeg accel_angle = ( *dash_power < 0.0
                             ? wm.self().body() - 180.0
                             : wm.self().body() );
    Vector2D my_pos( 0.0, 0.0 );
    Vector2D my_vel = wm.self().vel();

    const double max_accel_mag = ( std::fabs( *dash_power )
                                   * wm.self().playerType().dashPowerRate()
                                   * wm.self().effort() );
    double accel_mag = max_accel_mag;
    if ( wm.self().playerType().normalizeAccel( wm.self().vel(),
                                                accel_angle,
                                                &accel_mag ) )
    {
        *dash_power *= accel_mag / max_accel_mag;
    }

    Vector2D dash_accel
        = Vector2D::polar2vector( accel_mag, accel_angle );

    Vector2D ball_pos = wm.ball().rpos();
    Vector2D ball_vel = wm.ball().vel();

    my_vel += dash_accel;
    my_pos += my_vel;
    ball_pos += ball_vel;

    double ball_dist = my_pos.dist( ball_pos );
    double noise_buf
        = my_vel.r() * ServerParam::i().playerRand() * 0.5
        + ball_vel.r() * ServerParam::i().ballRand() * 0.5;

    dlog.addText( Logger::DRIBBLE,
                  "%s:%d: canKickAfterDash. ball_dist= %.2f, noise= %.2f"
                  ,__FILE__, __LINE__,
                  ball_dist, noise_buf );

    if ( ( ( ball_pos - my_pos ).th() - accel_angle ).abs() < 150.0
         && ball_dist < wm.self().kickableArea() - noise_buf - 0.2
         && ( ball_dist - noise_buf
              > ( wm.self().playerType().playerSize()
                  + ServerParam::i().ballSize() ) ) )
    {
        dlog.addText( Logger::DRIBBLE,
                      "%s:%d: canKickAfterDash. kickable after one dash"
                      ,__FILE__, __LINE__ );
        return true;
    }

    dlog.addText( Logger::DRIBBLE,
                  "%s:%d: canKickAfterDash. ball must be kicked."
                  ,__FILE__, __LINE__ );
    return false;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_Dribble2007::existCloseOpponent( const PlayerAgent * agent,
                                      AngleDeg * keep_angle )
{
    const WorldModel & wm = agent->world();

    const PlayerObject * opp = wm.getOpponentNearestToBall( 5 );
    if ( ! opp )
    {
        dlog.addText( Logger::DRIBBLE,
                      "%s: existCloseOppnent. No opponent."
                      ,__FILE__ );
        return false;
    }

    if ( opp->distFromBall()
         > ( ServerParam::i().defaultPlayerSpeedMax()
             + ServerParam::i().tackleDist() )
         )
    {
        dlog.addText( Logger::DRIBBLE,
                      "%s: existCloseOpponent. No dangerous opponent"
                      ,__FILE__ );
        return false;
    }


    // opp is in dangerous range

    Vector2D my_next = wm.self().pos() + wm.self().vel();
    Vector2D opp_next = opp->pos() + opp->vel();

    if ( opp->bodyCount() == 0
         || ( opp->velCount() <= 1 && opp->vel().r() > 0.2 ) )
    {
        Line2D opp_line( opp_next,
                         ( opp->bodyCount() == 0
                           ? opp->body()
                           : opp->vel().th() ) );
        Vector2D proj_pos = opp_line.projection( my_next );

        *keep_angle = ( my_next - proj_pos ).th();

        dlog.addText( Logger::DRIBBLE,
                      "%s:%d: existCloseOpponent  found interfere opponent (%.1f %.1f). avoid line."
                      " keep_angle=%.1f"
                      ,__FILE__, __LINE__,
                      opp_next.x, opp_next.y,
                      keep_angle->degree() );

        return true;
    }

    *keep_angle = ( my_next - opp_next ).th();

    dlog.addText( Logger::DRIBBLE,
                  "%s:%d: existCloseOpponent  found interfere opponent (%.1f %.1f). opposite side."
                  " keep_angle=%.1f"
                  ,__FILE__, __LINE__,
                  opp_next.x, opp_next.y,
                  keep_angle->degree() );
    return true;
}

/*-------------------------------------------------------------------*/
/*!
  TODO: angles which agent can turn by 1 step should have the priority
*/
AngleDeg
Body_Dribble2007::getAvoidAngle( const PlayerAgent * agent,
                                 const AngleDeg & target_angle )
{
    const WorldModel & wm = agent->world();

    if ( wm.opponentsFromSelf().empty() )
    {
        return target_angle;
    }

    const double avoid_radius = 5.0;
    const double safety_opp_dist = 5.0;
    const double safety_space_body_ang_radius2 = 3.0 * 3.0;


    const PlayerPtrCont & opps = wm.opponentsFromSelf();
    const PlayerPtrCont::const_iterator opps_end = opps.end();

    // at first, check my body dir and opposite dir of my body
    if ( ! opps.empty()
         && opps.front()->distFromSelf() < 3.0 )
    {
        dlog.addText( Logger::DRIBBLE,
                      "%s: getAvoidAngle. check body line. base_target_angle=%.0f"
                      ,__FILE__,
                      target_angle.degree() );

        AngleDeg new_target_angle = wm.self().body();
        for ( int i = 0; i < 2; i++ )
        {
            const Vector2D sub_target
                = wm.self().pos()
                + Vector2D::polar2vector( avoid_radius,
                                          new_target_angle );

            if ( sub_target.absX() > ServerParam::i().pitchHalfLength() - 1.8
                 || sub_target.absY() > ServerParam::i().pitchHalfWidth() - 1.8 )
            {
                // out of pitch
                continue;
            }

            bool success = true;
            for ( PlayerPtrCont::const_iterator it = opps.begin();
                  it != opps_end;
                  ++it )
            {
                if ( (*it)->posCount() >= 10 ) continue;

                if ( (*it)->distFromSelf() > 20.0 )
                {
                    break;
                }

                if ( (*it)->distFromSelf() < safety_opp_dist
                     && ((*it)->angleFromSelf() - new_target_angle).abs() < 30.0 )
                {
                    dlog.addText( Logger::DRIBBLE,
                                  "____ body line dir=%.1f"
                                  " exist near opp(%.1f, %.1f)",
                                  new_target_angle.degree(),
                                  (*it)->pos().x, (*it)->pos().y );
                    success = false;
                    break;
                }

                if ( sub_target.dist2( (*it)->pos() )
                     < safety_space_body_ang_radius2 )
                {
                    dlog.addText( Logger::DRIBBLE,
                                  "____ body line dir=%.1f"
                                  " exist opp(%.1f, %.1f) "
                                  "close to subtarget(%.1f, %.1f)",
                                  new_target_angle.degree(),
                                  (*it)->pos().x, (*it)->pos().y,
                                  sub_target.x, sub_target.y );
                    success = false;
                    break;
                }
            }

            if ( success )
            {
                dlog.addText( Logger::DRIBBLE,
                              "---> avoid to body line. angle=%.1f",
                              new_target_angle.degree() );
                return new_target_angle;
            }

            new_target_angle -= 180.0;
        }
    }

    // search divisions

    const int search_divs = 10;
    const double div_dir = 360.0 / static_cast< double >( search_divs );
    const double safety_angle = 60.0;
    const double safety_space_radius2 = avoid_radius * avoid_radius;

    dlog.addText( Logger::DRIBBLE,
                  "%s: getAvoidAngle. search angles. base_target_angle=%.0f"
                  ,__FILE__,
                  target_angle.degree() );

    double angle_sign = 1.0;
    if ( agent->world().self().pos().y < 0.0 ) angle_sign = -1.0;

    for ( int i = 1; i < search_divs; ++i, angle_sign *= -1.0 )
    {
        const AngleDeg new_target_angle
            = target_angle
            + ( angle_sign * div_dir * ( (i+1)/2 ) );

        const Vector2D sub_target
            = wm.self().pos()
            + Vector2D::polar2vector( avoid_radius,
                                      new_target_angle );

        if ( sub_target.absX()
             > ServerParam::i().pitchHalfLength() - wm.self().kickableArea() - 0.2
             || sub_target.absY()
             > ServerParam::i().pitchHalfWidth() - wm.self().kickableArea() - 0.2 )
        {
            dlog.addText( Logger::DRIBBLE,
                          "avoid angle. out of pitch. angle=%.0f pos=(%.1f %.1f)",
                          new_target_angle.degree(),
                          sub_target.x, sub_target.y );
            // out of pitch
            continue;
        }

        if ( sub_target.x < 30.0
             && sub_target.x < wm.self().pos().x - 2.0 )
        {
            dlog.addText( Logger::DRIBBLE,
                          "avoid angle. backword.. angle=%.0f pos=(%.1f %.1f)",
                          new_target_angle.degree(),
                          sub_target.x, sub_target.y );
            continue;
        }

        dlog.addText( Logger::DRIBBLE,
                      "avoid angle=%.0f pos=(%.1f %.1f)",
                      new_target_angle.degree(),
                      sub_target.x, sub_target.y );

        bool success = true;
        for ( PlayerPtrCont::const_iterator it = opps.begin();
              it != opps_end;
              ++it )
        {
            if ( (*it)->posCount() >= 10 ) continue;

            if ( (*it)->distFromSelf() > 20.0 ) break;

            double add_dir = 5.8 / (*it)->distFromSelf();
            add_dir = std::min( 180.0 - safety_angle, add_dir );
            if ( (*it)->distFromSelf() < safety_opp_dist
                 && ( ( (*it)->angleFromSelf() - new_target_angle ).abs()
                      < safety_angle + add_dir ) )
            {
                dlog.addText( Logger::DRIBBLE,
                              "____ opp angle close. cannot avoid to %.1f",
                              new_target_angle.degree() );
                success = false;
                break;
            }

            if ( sub_target.dist2( (*it)->pos() ) < safety_space_radius2 )
            {
                dlog.addText( Logger::DRIBBLE,
                              "____ opp dist close. cannot avoid to %.1f",
                              new_target_angle.degree() );
                success = false;
                break;
            }
        }

        if ( success )
        {
            dlog.addText( Logger::DRIBBLE,
                          "---> avoid to angle= %.1f",
                          new_target_angle.degree() );
            return new_target_angle;
        }

    }


    // Best angle is not found.
    // go to the least congestion point

    dlog.addText( Logger::DRIBBLE,
                  "%s: getAvoidAngle. search least congestion point."
                  ,__FILE__ );

    Rect2D target_rect( wm.self().pos().x - 4.0,
                        wm.self().pos().y - 4.0,
                        8.0,
                        8.0 );

    double best_score = 10000.0;
    Vector2D best_target = wm.self().pos();

    double x_i = 30.0 - wm.self().pos().x;
    if ( x_i > 0.0 ) x_i = 0.0;
    if ( x_i < -8.0 ) x_i = -8.0;

    for ( ; x_i < 8.5; x_i += 1.0 )
    {
        for ( double y_i = - 8.0; y_i < 8.5; y_i += 1.0 )
        {
            Vector2D candidate = wm.self().pos();
            candidate.add( x_i, y_i );

            if ( candidate.absX() > ServerParam::i().pitchHalfLength() - 2.0
                 || candidate.absY() > ServerParam::i().pitchHalfWidth() - 2.0 )
            {
                continue;
            }

            double tmp_score = 0.0;
            for ( PlayerPtrCont::const_iterator it = opps.begin();
                  it != opps_end;
                  ++it )
            {
                double d2 = (*it)->pos().dist2( candidate );

                if ( d2 > 15.0 * 15.0 ) continue;

                tmp_score += 1.0 / d2;
            }

            if ( tmp_score < best_score )
            {
                dlog.addText( Logger::DRIBBLE,
                              "    update least congestion point to"
                              " (%.2f, %.2f) score=%.4f",
                              candidate.x, candidate.y, tmp_score );
                best_target = candidate;
                best_score = tmp_score;
            }
        }
    }

    dlog.addText( Logger::DRIBBLE,
                  "  avoid to point (%.2f, %.2f)",
                  best_target.x, best_target.y );

    return ( best_target - wm.self().pos() ).th();
}

}