// -*-c++-*-

/*!
  \file world_model.cpp
  \brief world model Source File
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

#include "world_model.h"

#include "action_effector.h"
#include "localization.h"
#include "body_sensor.h"
#include "visual_sensor.h"
#include "fullstate_sensor.h"
#include "debug_client.h"
#include "intercept_table.h"
#include "penalty_kick_state.h"
#include "player_command.h"
#include "player_predicate.h"

#include <rcsc/common/audio_memory.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/player_param.h>
#include <rcsc/common/server_param.h>
#include <rcsc/soccer_math.h>
#include <rcsc/math_util.h>

#include <iterator>
#include <algorithm>
#include <cassert>
#include <cmath>

//#define DEBUG

namespace rcsc {

/*!
  \class WMImpl
  \brief the implementation of WorldModel
 */
class WMImpl {
public:

    /*!

    */
    inline
    static
    void create_player_set( rcsc::PlayerCont & players,
                            rcsc::PlayerPtrCont & players_from_self,
                            rcsc::PlayerPtrCont & players_from_ball,
                            const rcsc::Vector2D & self_pos,
                            const rcsc::Vector2D & ball_pos )

      {
          const rcsc::PlayerCont::iterator end = players.end();
          for ( rcsc::PlayerCont::iterator it = players.begin();
                it != end;
                ++it )
          {
              it->updateSelfBallRelated( self_pos, ball_pos );
              players_from_self.push_back( &( *it ) );
              players_from_ball.push_back( &( *it ) );
          }
      }

    /*!

    */
    inline
    static
    bool check_player_kickable( rcsc::PlayerPtrCont::iterator first,
                                const rcsc::PlayerPtrCont::iterator last,
                                const int ball_count )
      {
          for ( ; first != last; ++first )
          {
              if ( (*first)->isGhost()
                   || (*first)->posCount() > ball_count + 1 )
              {
                  continue;
              }

              if ( (*first)->isKickable() )
              {
                  return true;
              }

              return false;
          }

          return false;
      }

};


/////////////////////////////////////////////////////////////////////

//! view cone area
struct ViewArea {
    double width_half_;
    Vector2D origin_;
    double angle_;
    GameTime time_;

    ViewArea( const double & width,
              const Vector2D & origin,
              const double & angle,
              const GameTime & t )
        : width_half_( width * 0.5 )
        , origin_( origin )
        , angle_( angle )
        , time_( t )
      { }

    bool contains( const Vector2D & p,
                   const double & dir_thr,
                   const double & vis_dist2 ) const
      {
          Vector2D rpos( p - origin_ );
          if ( rpos.r2() < vis_dist2 )
          {
              return true;
          }
          AngleDeg angle_diff = rpos.th() - angle_;
          //rpos.rotate( -angle_ );
          //if ( rpos.th().abs() < width_half_ - dir_thr )
          if ( angle_diff.abs() < width_half_ - dir_thr )
          {
              return true;
          }
          return false;
      }
};

/*-------------------------------------------------------------------*/


const double WorldModel::DIR_STEP = 360.0 / static_cast< double >( DIR_CONF_DIVS );

/*-------------------------------------------------------------------*/
/*!

*/
WorldModel::WorldModel()
    : M_localize( static_cast< Localization * >( 0 ) )
    , M_intercept_table( new InterceptTable( *this ) )
    , M_penalty_kick_state( new PenaltyKickState )
    , M_our_side( NEUTRAL )
    , M_time( -1, 0 )
    , M_sense_body_time( -1, 0 )
    , M_see_time( -1, 0 )
    , M_last_set_play_start_time( 0, 0 )
    , M_setplay_count( 0 )
    , M_game_mode()
    , M_self()
    , M_ball()
    , M_opponent_goalie_unum( Unum_Unknown )
    , M_offside_line_x( 0.0 )
    , M_offside_line_count( 0 )
    , M_defense_line_x( 0.0 )
    , M_exist_kickable_teammate( false )
    , M_exist_kickable_opponent( false )
{
    assert( M_intercept_table );
    assert( M_penalty_kick_state );

    for ( int i = 0; i < DIR_CONF_DIVS; i++ )
    {
        M_dir_count[i] = 1000;
    }

    for ( int i = 0; i < 12; ++i )
    {
        M_known_teammates[i] = static_cast< AbstractPlayerObject * >( 0 );
        M_known_opponents[i] = static_cast< AbstractPlayerObject * >( 0 );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
WorldModel::~WorldModel()
{
    if ( M_localize )
    {
        delete M_localize;
        M_localize = static_cast< Localization * >( 0 );
    }

    if ( M_intercept_table )
    {
        delete M_intercept_table;
        M_intercept_table = static_cast< InterceptTable * >( 0 );
    }

    if ( M_penalty_kick_state )
    {
        delete M_penalty_kick_state;
        M_penalty_kick_state = static_cast< PenaltyKickState * >( 0 );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
const
InterceptTable *
WorldModel::interceptTable() const
{
    assert( M_intercept_table );
    return M_intercept_table;
}

/*-------------------------------------------------------------------*/
/*!

*/
const
PenaltyKickState *
WorldModel::penaltyKickState() const
{
    assert( M_penalty_kick_state );
    return M_penalty_kick_state;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
WorldModel::initTeamInfo( const std::string & teamname,
                          const SideID ourside,
                          const int my_unum,
                          const bool my_goalie )
{
    M_localize = new Localization( ourside );

    if ( ! M_localize )
    {
        std::cerr << teamname << ' '
                  << my_unum << ':'
                  << " ***ERROR*** Failed to create localization object."
                  << std::endl;
        return false;
    }

    if ( ! M_audio_memory )
    {
        std::cerr << teamname << ' '
                  << my_unum << ':'
                  << " ***ERROR*** No audio message holder."
                  << std::endl;
        return false;
    }

    M_teamname = teamname;
    M_our_side = ourside;
    M_self.init( ourside, my_unum, my_goalie );

    for ( int i = 0; i < 11; i++ )
    {
        M_teammate_types[i] = Hetero_Default;
        M_opponent_types[i] = Hetero_Default;
    }

    PlayerTypeSet::instance().resetDefaultType( ServerParam::i() );
    M_self.setPlayerType( Hetero_Default );

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
WorldModel::setAudioMemory( boost::shared_ptr< AudioMemory > memory )
{
    M_audio_memory = memory;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
WorldModel::setTeammatePlayerType( const int unum,
                                   const int id )
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << teamName() << " : " << self().unum()
                  << " ***ERROR*** WorldModel:: setTeammatePlayerType "
                  << " Illegal uniform number" << unum
                  << std::endl;
        return;
    }

    dlog.addText( Logger::WORLD,
                  "WorldModel: teammate %d to player_type %d",
                  unum, id );

    M_teammate_types[unum - 1] = id;

    if ( unum == self().unum() )
    {
        const PlayerType * tmp = PlayerTypeSet::i().get( id );
        if ( ! tmp )
        {
            std::cerr << teamName() << " : " << self().unum()
                      << "WorldModel: Illega player type id??"
                      << " player type param not found, id = "
                      << id << std::endl;
            return;
        }
        M_self.setPlayerType( id );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
WorldModel::setOpponentPlayerType( const int unum,
                                   const int id )
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << teamName() << " : " << self().unum()
                  << " ***ERROR*** WorldModel:: setOpponentPlayerType "
                  << " Illegal uniform number"
                  << unum << std::endl;
        return;
    }


    dlog.addText( Logger::WORLD,
                  "WorldModel: opponent %d to player_type %d",
                  unum, id );

    M_opponent_types[unum - 1] = id;
}

/*-------------------------------------------------------------------*/
/*!

*/
const
PlayerType *
WorldModel::teammatePlayerType( const int unum ) const
{
    if ( unum < 1 || 11 < unum )
    {
        return PlayerTypeSet::i().get( Hetero_Default );
    }

    const PlayerType * p = PlayerTypeSet::i().get( teammateHeteroID( unum ) );
    if ( ! p )
    {
        p = PlayerTypeSet::i().get( Hetero_Default );
    }
    return p;
}

/*-------------------------------------------------------------------*/
/*!

*/
const
PlayerType *
WorldModel::opponentPlayerType( const int unum ) const
{
    if ( unum < 1 || 11 < unum )
    {
        return PlayerTypeSet::i().get( Hetero_Unknown );
    }

    const PlayerType * p = PlayerTypeSet::i().get( opponentHeteroID( unum ) );
    if ( ! p )
    {
        p = PlayerTypeSet::i().get( Hetero_Unknown );
    }
    return p;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
WorldModel::update( const ActionEffector & act,
                    const GameTime & current )
{
    // this function called from updateAfterSense()
    // or, if player could not receive sense_body,
    // this function is called at the each update operations

    // simplest estimational update
    if ( time() == current )
    {
        std::cerr << teamName() << " : " << self().unum()
                  << current << "internal update called twice ??"
                  << std::endl;
        return;
    }

    M_time = current;

    // playmode is updated in updateJustBeforeDecision

    // update each object
    M_self.update( act, current );
    M_ball.update( act, gameMode(), current );

    if ( M_ball.rposValid() )
    {
        dlog.addText( Logger::WORLD,
                      "world.update. internal update. bpos=(%.2f, %.2f)"
                      " brpos=(%.2f, %.2f) bvel=(%.2f, %.2f)",
                      M_ball.pos().x, M_ball.pos().y,
                      M_ball.rpos().x, M_ball.rpos().y,
                      M_ball.vel().x, M_ball.vel().y );
    }
    else
    {
        dlog.addText( Logger::WORLD,
                      "world.update. internal update. bpos=(%.2f, %.2f)"
                      " bvel=(%.2f, %.2f), invalid rpos",
                      M_ball.pos().x, M_ball.pos().y,
                      M_ball.vel().x, M_ball.vel().y );
    }

    //dlog.addText( Logger::WORLD,
    //              "world.update. internal update. ball seen_pos_count=%d"
    //              " heard_pos_count=%d",
    //              ball().seenPosCount(), ball().heardPosCount() );

    // clear pointer reference container
    M_teammates_from_self.clear();
    M_opponents_from_self.clear();
    M_teammates_from_ball.clear();
    M_opponents_from_ball.clear();

    M_all_players.clear();
    M_all_teammates.clear();
    M_all_opponents.clear();

    for ( int i = 0; i < 12; ++i )
    {
        M_known_teammates[i] = static_cast< AbstractPlayerObject * >( 0 );
        M_known_opponents[i] = static_cast< AbstractPlayerObject * >( 0 );
    }

    // update teammates
    std::for_each( M_teammates.begin(),
                   M_teammates.end(),
                   PlayerObject::UpdateOp() );
    M_teammates.remove_if( PlayerObject::IsInvalidOp() );

    // update opponents
    std::for_each( M_opponents.begin(),
                   M_opponents.end(),
                   PlayerObject::UpdateOp() );
    M_opponents.remove_if( PlayerObject::IsInvalidOp() );

    // update unknown players
    std::for_each( M_unknown_players.begin(),
                   M_unknown_players.end(),
                   PlayerObject::UpdateOp() );
    M_unknown_players.remove_if( PlayerObject::IsInvalidOp() );

    for ( int i = 0; i < DIR_CONF_DIVS; i++ )
    {
        M_dir_count[i] = std::min( 10, M_dir_count[i] + 1);
        //dlog.addText( Logger::WORLD,
        //            "  world.dirConf: %4.0f -> %d",
        //            (double)i * 360.0 / static_cast<double>(DIR_CONF_DIVS) - 180.0,
        //            M_dir_conf[i] );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
WorldModel::updateAfterSense( const BodySensor & sense,
                              const ActionEffector & act,
                              const GameTime & current )
{
    // called just after sense_body

    // if I could not get sense_body & could get see before action decision,
    // this method is called before update(VisualSensor &, GameTime &)

    // if I could not get sense_body & see before action decision,
    // this method is called just before action decision.

    if ( M_sense_body_time == current )
    {
        std::cerr << teamName() << " : " << self().unum()
                  << current
                  << " world.updateAfterSense: called twice"
                  << std::endl;
        dlog.addText( Logger::WORLD,
                      "world.updateAfterSense. called twide" );
        return;
    }

    M_sense_body_time = sense.time();

    dlog.addText( Logger::WORLD,
                  "*************** updateAfterSense ***************" );

    if ( sense.time() == current )
    {
        dlog.addText( Logger::WORLD,
                      "world.updateAfterSense. update self" );
        M_self.updateAfterSense( sense, act, current );
    }

    if ( time() != current )
    {
        dlog.addText( Logger::WORLD,
                      "world.updateAfterSense. call internal update" );
        // internal update
        update( act, current );
        // check collision
        //updateCollision();
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
WorldModel::updateCollision()
{
    // called in updateJustBeforeDecision()

    if ( ! ball().posValid()
         || ! ball().velValid()
         || ! self().posValid()
         || ! self().velValid() )
    {

        return;
    }

    if ( ball().velCount() == 0 )
    {
        // already seen the ball velocity directly
        // nothing to do
        return;
    }

    bool collided_with_ball = false;

    if ( self().hasSensedCollision() )
    {
        dlog.addText( Logger::WORLD,
                      "%s: update collision. agent has sensed collision info"
                      ,__FILE__ );

        collided_with_ball = self().collidesWithBall();
        if ( collided_with_ball )
        {
            dlog.addText( Logger::WORLD,
                          "%s: update collision. detected by sense_body"
                          ,__FILE__ );
        }
    }
    else
    {
        // internally updated positions
        const double self_ball_dist
            = ( ball().pos() - self().pos() ).r();

        if ( ( self().collisionEstimated()
               && self_ball_dist < ( self().playerType().playerSize()
                                     + ServerParam::i().ballSize()
                                     + 0.1 )
               )
             || ( ( self().collisionEstimated()
                    || self().vel().r() < ( self().playerType().realSpeedMax()
                                            * self().playerType().playerDecay()
                                            * 0.11 ) )
                  && ( self_ball_dist
                       < ( self().playerType().playerSize()
                           + ServerParam::i().ballSize()
                           - 0.2 ) ) )
             )
        {
            dlog.addText( Logger::WORLD,
                          "%s: update collision. detected. ball_dist= %.3f"
                          ,__FILE__,
                      self_ball_dist );
            collided_with_ball = true;
        }
    }

    if ( collided_with_ball )
    {
        if ( ball().posCount() > 0 )
        {
            Vector2D mid = ball().pos() + self().pos();
            mid *= 0.5;

            Vector2D mid2ball = ball().pos() - mid;
            Vector2D mid2self = self().pos() - mid;
            double ave_size = ( ServerParam::i().ballSize()
                                + self().playerType().playerSize() ) * 0.5;
            mid2ball.setLength( ave_size );
            mid2self.setLength( ave_size );

            Vector2D new_ball_pos = mid + mid2ball;
            Vector2D ball_add = new_ball_pos - ball().pos();
            Vector2D new_ball_rpos = ball().rpos() + ball_add;
            Vector2D new_ball_vel = ball().vel() * -0.1;

            M_ball.updateByCollision( new_ball_pos, ball().posCount() + 1,
                                      new_ball_rpos, ball().rposCount() + 1,
                                      new_ball_vel, ball().velCount() + 1 );
            dlog.addText( Logger::WORLD,
                          "%s: update collision. new bpos(%.2f %.2f) rpos(%.2f %.2f)"
                          " vel(%.2f %.2f)"
                          ,__FILE__,
                          new_ball_pos.x, new_ball_pos.y,
                          new_ball_rpos.x, new_ball_rpos.y,
                          new_ball_vel.x, new_ball_vel.y );

            if ( self().posCount() > 0 )
            {
                Vector2D new_my_pos = mid + mid2self;
                double my_add_r = ( new_my_pos - self().pos() ).r();
                Vector2D new_my_pos_error = self().posError();
                new_my_pos_error.x += my_add_r;
                new_my_pos_error.y += my_add_r;

                M_self.updateByCollision( new_my_pos, new_my_pos_error );

                dlog.addText( Logger::WORLD,
                              "%s: update collision. new mypos(%.2f %.2f) error(%.2f %.2f)"
                              ,__FILE__,
                              new_my_pos.x, new_my_pos.y,
                              new_my_pos_error.x, new_my_pos_error.y );
            }
        }
        else // ball().posCount() == 0
        {
            dlog.addText( Logger::WORLD,
                          "%s: update collision. seen ball"
                          ,__FILE__ );
            int vel_count = ( self().hasSensedCollision()
                              ? ball().velCount()
                              : ball().velCount() + 1 );

            M_ball.updateByCollision( ball().pos(), ball().posCount(),
                                      ball().rpos(), ball().rposCount(),
                                      ball().vel() * -0.1, vel_count );
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
WorldModel::updateAfterSee( const VisualSensor & see,
                            const BodySensor & sense,
                            const ActionEffector & act,
                            const GameTime & current )
{
    //////////////////////////////////////////////////////////////////
    // check internal update time
    if ( time() != current )
    {
        update( act, current );
    }

    //////////////////////////////////////////////////////////////////
    // check last sight update time
    if ( M_see_time == current )
    {
        std::cerr << teamName() << " : " << self().unum()
                  << current << " updateAfterSee : called twice "
                  << std::endl;
        return;
    }
    //////////////////////////////////////////////////////////////////
    // time update
    M_see_time = current;

    dlog.addText( Logger::WORLD,
                  "*************** updateAfterSee *****************" );

    if ( M_fullstate_time == current )
    {
        dlog.addText( Logger::WORLD,
                      "%s: updateAfterSee. already updated by fullstate,"
                      ,__FILE__ );
        // stored info
        ViewArea varea( self().viewWidth().width(),
                        self().pos(),
                        self().face().degree(),
                        current );
        // update dir accuracy
        updateDirCount( varea );
        return;
    }

    //////////////////////////////////////////////////////////////////
    // set opponent teamname
    if ( M_opponent_teamname.empty()
         && ! see.opponentTeamName().empty() )
    {
        M_opponent_teamname = see.opponentTeamName();
    }

    //////////////////////////////////////////////////////////////////
    // self localization
    localizeSelf( see, current );

    // correct vel dir using seen my angle & sense_body's speed magnitude
    M_self.updateVelDirAfterSee( sense, current );

    //////////////////////////////////////////////////////////////////
    // ball localization
    localizeBall( see, act, current );

    //////////////////////////////////////////////////////////////////
    // player localization & matching
    localizePlayers( see );

    //////////////////////////////////////////////////////////////////
    // view cone & ghost check
    // my global position info is successfully updated.
    if ( self().posCount() == 0 )
    {
        // stored info
        ViewArea varea( self().viewWidth().width(),
                        self().pos(),
                        self().face().degree(),
                        current );
        dlog.addText( Logger::WORLD,
                      "%s: viewArea. origin=(%.2f, %.2f) angle=%.1f, width_half=%.1f vwidth=%d,%.2f"
                      ,__FILE__,
                      varea.origin_.x, varea.origin_.y,
                      varea.angle_, varea.width_half_,
                      self().viewWidth().type(),
                      self().viewWidth().width() );

        // check ghost object
        checkGhost( varea );
        // update dir accuracy
        updateDirCount( varea );
    }

    //////////////////////////////////////////////////////////////////
    // debug output
    dlog.addText( Logger::WORLD,
                  "<--- mypos=(%.2f, %.2f) err=(%.3f, %.3f) vel=(%.2f, %.2f)",
                  self().pos().x, self().pos().y,
                  self().posError().x, self().posError().y,
                  self().vel().x, self().vel().y );
    dlog.addText( Logger::WORLD,
                  "<--- seen players t=%d: ut=%d: o=%d: uo=%d: u=%d",
                  see.teammates().size(),
                  see.unknownTeammates().size(),
                  see.opponents().size(),
                  see.unknownOpponents().size(),
                  see.unknownPlayers().size() );
    dlog.addText( Logger::WORLD,
                  "<--- internal players t=%d: o=%d:  u=%d",
                  M_teammates.size(),
                  M_opponents.size(),
                  M_unknown_players.size() );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
WorldModel::updateAfterFullstate( const FullstateSensor & fullstate,
                                  const ActionEffector & act,
                                  const GameTime & current )
{
    // internal update
    if ( time() != current )
    {
        update( act, current );
    }

    if ( M_fullstate_time == current )
    {
        std::cerr << teamName() << " : " << self().unum()
                  << current << " updateAfterFullstate : called twice "
                  << std::endl;
        return;
    }

    M_fullstate_time = current;

    dlog.addText( Logger::WORLD,
                  "*************** updateAfterFullstate ***************" );

    // update players
    const FullstateSensor::PlayerCont & our_players
        = ( isOurLeft()
            ? fullstate.leftTeam()
            : fullstate.rightTeam() );
    const FullstateSensor::PlayerCont & opp_players
        = ( isOurLeft()
            ? fullstate.rightTeam()
            : fullstate.leftTeam() );

    // clean unkown players
    M_unknown_players.clear();

    // update self
    for ( FullstateSensor::PlayerCont::const_iterator fp
              = our_players.begin();
          fp != our_players.end();
          ++fp )
    {
        // update self
        if ( fp->unum_ == self().unum() )
        {
            dlog.addText( Logger::WORLD,
                          "FS updated self" );
            M_self.updateAfterFullstate( *fp, act, current );
            break;
        }
    }

    PlayerCont teammates_temp;
    PlayerCont opponents_temp;

    teammates_temp.splice( teammates_temp.end(),
                           M_teammates );
    opponents_temp.splice( opponents_temp.end(),
                           M_opponents );

    // update teammates
    for ( FullstateSensor::PlayerCont::const_iterator fp
              = our_players.begin();
          fp != our_players.end();
          ++fp )
    {
        // update self
        if ( fp->unum_ == self().unum() )
        {
            continue;
        }

        PlayerObject * player = static_cast< PlayerObject * >( 0 );
        for ( PlayerCont::iterator p = teammates_temp.begin();
              p != teammates_temp.end();
              ++p )
        {
            if ( p->unum() == fp->unum_ )
            {
                M_teammates.splice( M_teammates.end(),
                                    teammates_temp,
                                    p );
                player = &(*p);
                break;
            }
        }

        if ( ! player )
        {
            M_teammates.push_back( PlayerObject() );
            player = &(M_teammates.back());
        }

        dlog.addText( Logger::WORLD,
                      "FS updated teammate %d",
                      fp->unum_ );
        player->updateByFullstate( *fp, self().pos(), fullstate.ball().pos_ );
    }

    // update opponents
    for ( FullstateSensor::PlayerCont::const_iterator fp
              = opp_players.begin();
          fp != opp_players.end();
          ++fp )
    {
        PlayerObject * player = static_cast< PlayerObject * >( 0 );
        for ( PlayerCont::iterator p = opponents_temp.begin();
              p != opponents_temp.end();
              ++p )
        {
            if ( p->unum() == fp->unum_ )
            {
                M_opponents.splice( M_opponents.end(),
                                    opponents_temp,
                                    p );
                player = &(*p);
                break;
            }
        }

        if ( ! player )
        {
            M_opponents.push_back( PlayerObject() );
            player = &(M_opponents.back());
        }

        dlog.addText( Logger::WORLD,
                      "FS updated opponent %d",
                      fp->unum_ );
        player->updateByFullstate( *fp, self().pos(), fullstate.ball().pos_ );
    }

    // update ball
    M_ball.updateByFullstate( fullstate.ball().pos_,
                              fullstate.ball().vel_,
                              self().pos() );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
WorldModel::updateGameMode( const GameMode & game_mode,
                            const GameTime & current )
{
    bool pk_mode = game_mode.isPenaltyKickMode();

    if ( ! pk_mode
         && game_mode.type() != GameMode::PlayOn )// not play_on
    {
        // playmode is changed
        if ( gameMode().type() != game_mode.type() )
        {
            if ( game_mode.type() == GameMode::FreeKick_
                 && ( gameMode().type() == GameMode::OffSide_
                      || gameMode().type() == GameMode::BackPass_
                      || gameMode().type() == GameMode::FreeKickFault_
                      || gameMode().type() == GameMode::CatchFault_
                      || gameMode().type() == GameMode::IndFreeKick_
                      )
                 )
            {
                // nothing to do
            }
            else
            {
                M_last_set_play_start_time = current;
                M_setplay_count = 0;
            }
        }
        // this check supports human referee's interaction
        if ( gameMode().type() == game_mode.type()
             && game_mode.type() == GameMode::FreeKick_ )
        {
            M_last_set_play_start_time = current;
            M_setplay_count = 0;
        }
    }

    // substitute new game mode to member variable
    M_game_mode = game_mode;

    // update penalty kick status
    if ( pk_mode )
    {
        M_penalty_kick_state->update( game_mode, ourSide(), current );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
WorldModel::updateGoalieByHear()
{
    if ( M_fullstate_time == this->time() )
    {
        return;
    }

    if ( M_audio_memory->goalieTime() != this->time()
         || M_audio_memory->goalie().empty() )
    {
        return;
    }

    if ( opponentGoalieUnum() == Unum_Unknown )
    {
        return;
    }

    PlayerObject * goalie = static_cast< PlayerObject * >( 0 );

    PlayerCont::iterator end = M_opponents.end();
    for( PlayerCont::iterator it = M_opponents.begin();
         it != end;
         ++it )
    {
        if ( it->goalie() )
        {
            goalie = &(*it);
            break;
        }
    }

    if ( goalie
         && goalie->posCount() == 0
         && goalie->bodyCount() == 0 )
    {
        // goalie is seen at the current time.
        dlog.addText( Logger::WORLD,
                      __FILE__": update goalie by hear. but already seen" );
        return;
    }

    Vector2D heard_pos( 0.0, 0.0 );
    double heard_body = 0.0;

    //const AudioMemory::Goalie & heard_goalie = M_audio_memory->goalie().front();
    for ( std::vector< AudioMemory::Goalie >::const_iterator it
              = M_audio_memory->goalie().begin();
          it != M_audio_memory->goalie().end();
          ++it )
    {
        heard_pos += it->pos_;
        heard_body += it->body_.degree();
    }

    heard_pos /= static_cast< double >( M_audio_memory->goalie().size() );
    heard_body /= static_cast< double >( M_audio_memory->goalie().size() );

    dlog.addText( Logger::WORLD,
                  __FILE__": update goalie by hear. pos=(%.1f %.1f) body=%.1f",
                  heard_pos.x, heard_pos.y,
                  heard_body );

    if ( goalie )
    {
        goalie->updateByHear( opponentGoalieUnum(), true,
                              heard_pos,
                              AngleDeg( heard_body ) );
        return;
    }

    // goalie not found

    // search the nearest unknown player

    const double goalie_speed_max = ServerParam::i().defaultPlayerSpeedMax();

    double min_dist = 1000.0;

    for( PlayerCont::iterator it = M_opponents.begin();
         it != end;
         ++it )
    {
        if ( it->unum() != Unum_Unknown ) continue;

        if ( it->pos().x < ServerParam::i().theirPenaltyAreaLineX()
             || it->pos().absY() > ServerParam::i().penaltyAreaHalfWidth() )
        {
            // out of penalty area
            continue;
        }

        double d = it->pos().dist( heard_pos );
        if ( d < min_dist
             && d < it->posCount() * goalie_speed_max + it->distFromSelf() * 0.06 )
        {
            min_dist = d;
            goalie = &(*it);
        }
    }

    const PlayerCont::iterator u_end = M_unknown_players.begin();
    for ( PlayerCont::iterator it = M_unknown_players.begin();
          it != u_end;
          ++it )
    {
        if ( it->pos().x < ServerParam::i().theirPenaltyAreaLineX()
             || it->pos().absY() > ServerParam::i().penaltyAreaHalfWidth() )
        {
            // out of penalty area
            continue;
        }

        double d = it->pos().dist( heard_pos );
        if ( d < min_dist
             && d < it->posCount() * goalie_speed_max + it->distFromSelf() * 0.06 )
        {
            min_dist = d;
            goalie = &(*it);
        }
    }


    if ( goalie )
    {
        // found a candidate unknown player
       dlog.addText( Logger::WORLD,
                     __FILE__": updateGoalieByHear(). found."
                     " heard_pos(%.1f %.1f)",
                     heard_pos.x, heard_pos.y );
        goalie->updateByHear( opponentGoalieUnum(),
                              true,
                              heard_pos,
                              AngleDeg( heard_body ) );
    }
    else
    {
        // register new object

        dlog.addText( Logger::WORLD,
                      __FILE__": updateGoalieByHear(). not found."
                      " add new goalie. heard_pos(%.1f %.1f)",
                      heard_pos.x, heard_pos.y );

        M_opponents.push_back( PlayerObject() );
        goalie = &(M_opponents.back());
        goalie->updateByHear( opponentGoalieUnum(), true,
                              heard_pos,
                              AngleDeg( heard_body ) );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
WorldModel::updatePlayerByHear()
{
    if ( M_fullstate_time == this->time() )
    {
        return;
    }

    if ( M_audio_memory->playerTime() != this->time()
         || M_audio_memory->player().empty() )
    {
        return;
    }

    // TODO: consider duplicated player

    const std::vector< AudioMemory::Player >::const_iterator heard_end
        = M_audio_memory->player().end();
    for ( std::vector< AudioMemory::Player >::const_iterator heard_player
              = M_audio_memory->player().begin();
          heard_player != heard_end;
          ++heard_player )
    {
        if ( heard_player->unum_ == Unum_Unknown )
        {
            continue;
        }

        const SideID side = ( heard_player->unum_ <= 11
                              ? ourSide()
                              : theirSide() );
        const int unum = ( heard_player->unum_ <= 11
                           ? heard_player->unum_
                           : heard_player->unum_ - 11 );

        if ( unum < 1 || 11 < unum )
        {
            std::cerr << __FILE__ << ":" << __LINE__
                      << " ***ERROR*** updatePlayerByHear. Illega unum "
                      << unum
                      << " heard_unum=" << heard_player->unum_
                      << " pos=" << heard_player->pos_
                      << std::endl;
            dlog.addText( Logger::WORLD,
                          __FILE__": updatePlayerByHear(). Illega unum %d"
                          " pos=(%.1f %.1f)",
                          unum, heard_player->pos_.x, heard_player->pos_.y );
            continue;
        }

        if ( side == ourSide()
             && unum == self().unum() )
        {
            dlog.addText( Logger::WORLD,
                          __FILE__": updatePlayerByHear(). heard myself. skip" );
            continue;
        }

        PlayerObject * player = static_cast< PlayerObject * >( 0 );

        PlayerCont & players = ( side == ourSide()
                                 ? M_teammates
                                 : M_opponents );
        const PlayerCont::iterator end = players.end();
        for ( PlayerCont::iterator p = players.begin();
              p != end;
              ++p )
        {
            if ( p->unum() == unum )
            {
                player = &(*p);
                dlog.addText( Logger::WORLD,
                              __FILE__": updatePlayerByHear(). found."
                              " side %d, unum %d",
                              side, unum );
                break;
            }
        }

        PlayerCont::iterator unknown = M_unknown_players.end();
        double min_dist = 0.0;
        if ( ! player )
        {
            min_dist = 1000.0;
            for  ( PlayerCont::iterator p = players.begin();
                   p != end;
                   ++p )
            {
                double d = p->pos().dist( heard_player->pos_ );
                if ( d < min_dist
                     && d < p->posCount() * 1.2 + p->distFromSelf() * 0.06 )
                {
                    min_dist = d;
                    player = &(*p);
                }
            }

            const PlayerCont::iterator u_end = M_unknown_players.end();
            for ( PlayerCont::iterator p = M_unknown_players.begin();
                  p != u_end;
                  ++p )
            {
                double d = p->pos().dist( heard_player->pos_ );
                if ( d < min_dist
                     && d < p->posCount() * 1.2 + p->distFromSelf() * 0.06 )
                {
                    min_dist = d;
                    player = &(*p);
                    unknown = p;
                }
            }
        }

        if ( player )
        {
            dlog.addText( Logger::WORLD,
                          __FILE__": updatePlayerByHear(). exist candidate."
                          " heard_pos(%.1f %.1f) pos(%.1f %.1f) count %d  dist=%.2f",
                          heard_player->pos_.x,
                          heard_player->pos_.y,
                          player->pos().x, player->pos().y,
                          player->posCount(),
                          min_dist );

            player->updateByHear( unum, false,
                                  heard_player->pos_,
                                  heard_player->body_ );
            if ( unknown != M_unknown_players.end() )
            {
                dlog.addText( Logger::WORLD,
                              __FILE__": updatePlayerByHear(). splice unknown player to known player list" );
                players.splice( players.end(),
                                M_unknown_players,
                                unknown );
            }
        }
        else
        {
            dlog.addText( Logger::WORLD,
                          __FILE__": updatePlayerByHear(). not found."
                          " add new player heard_pos(%.1f %.1f)",
                          heard_player->pos_.x,
                          heard_player->pos_.y );

            if ( side == ourSide() )
            {
                M_teammates.push_back( PlayerObject() );
                player = &( M_teammates.back() );
            }
            else
            {
                M_opponents.push_back( PlayerObject() );
                player = &( M_opponents.back() );
            }

            player->updateByHear( unum, false,
                                  heard_player->pos_,
                                  heard_player->body_ );
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
WorldModel::updateJustBeforeDecision( const ActionEffector & act,
                                      const GameTime & current )
{
    if ( time() != current )
    {
        update( act, current );
    }

    if ( M_audio_memory->waitRequestTime() == current )
    {
        M_setplay_count = 0;
    }
    else
    {
        // always increment
        ++M_setplay_count;
    }

    // update using sharing network information
    if ( M_audio_memory->ballTime() == current
         && ! M_audio_memory->ball().empty()
         && M_fullstate_time != current )
    {
        Vector2D heard_pos( 0.0, 0.0 );
        Vector2D heard_vel( 0.0, 0.0 );

        for ( std::vector< AudioMemory::Ball >::const_iterator it
                  = M_audio_memory->ball().begin();
              it != M_audio_memory->ball().end();
              ++it )
        {
            heard_pos += it->pos_;
            heard_vel += it->vel_;
        }
        heard_pos /= static_cast< double >( M_audio_memory->ball().size() );
        heard_vel /= static_cast< double >( M_audio_memory->ball().size() );

        M_ball.updateByHear( heard_pos, heard_vel, current );
    }

    updateGoalieByHear();
    updatePlayerByHear();

    updateCollision();

    updatePlayerType();

    // update positional info concerned with other players
    updateObjectRelation();
    updateOffsideLine();
    updateDefenseLine();

    // update interception table
    M_intercept_table->update();

    if ( M_audio_memory->ourInterceptTime() == current )
    {
        const std::vector< AudioMemory::OurIntercept >::const_iterator end
            = M_audio_memory->ourIntercept().end();
        for ( std::vector< AudioMemory::OurIntercept >::const_iterator it
                  = M_audio_memory->ourIntercept().begin();
              it != end;
              ++it )
        {
            M_intercept_table->hearTeammate( it->interceptor_,
                                             it->cycle_ );
        }
    }

    if ( M_audio_memory->oppInterceptTime() == current
         && ! M_audio_memory->oppIntercept().empty() )
    {
        const std::vector< AudioMemory::OppIntercept >::const_iterator end
            = M_audio_memory->oppIntercept().end();
        for ( std::vector< AudioMemory::OppIntercept >::const_iterator it
                  = M_audio_memory->oppIntercept().begin();
              it != end;
              ++it )
        {
            M_intercept_table->hearOpponent( it->interceptor_,
                                             it->cycle_ );
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
WorldModel::setCommandEffect( const ActionEffector & act )
{
    if ( act.changeViewCommand() )
    {
        M_self.setViewMode( act.changeViewCommand()->width(),
                            act.changeViewCommand()->quality() );
    }

    if ( act.pointtoCommand() )
    {
        M_self.setPointto( act.getPointtoPos(),
                           time() );
    }

    const PlayerAttentiontoCommand * attentionto = act.attentiontoCommand();
    if ( attentionto )
    {
        if ( attentionto->isOn() )
        {
            if ( attentionto->side() == PlayerAttentiontoCommand::OUR )
            {
                M_self.setAttentionto( ourSide(),
                                       attentionto->number() );
            }
            else
            {
                SideID opp_side = ( isOurLeft()
                                    ? RIGHT
                                    : LEFT );
                M_self.setAttentionto( opp_side,
                                       attentionto->number() );
            }
        }
        else
        {
            // off
            M_self.setAttentionto( NEUTRAL, 0 );
        }
    }

}

/*-------------------------------------------------------------------*/
/*!

*/
void
WorldModel::localizeSelf( const VisualSensor & see,
                          const GameTime & current )
{
    double angle_face = -360.0, angle_face_error = 0.0;
    Vector2D my_pos( Vector2D::INVALIDATED );
    Vector2D my_pos_error( 0.0, 0.0 );

    //////////////////////////////////////////////////////////////////
    // localization
    M_localize->localizeSelf( see,
                              &angle_face, &angle_face_error,
                              &my_pos, &my_pos_error );

    //////////////////////////////////////////////////////////////////
    // set data
    if ( my_pos.valid() )
    {
        M_self.updatePos( my_pos, my_pos_error,
                          angle_face, std::min( angle_face_error, 180.0 ),
                          current );
    }
    else if ( angle_face != -360.0 )
    {
        M_self.updateAngle( angle_face, std::min( angle_face_error, 180.0 ),
                            current );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
WorldModel::localizeBall( const VisualSensor & see,
                          const ActionEffector & act,
                          const GameTime & current )
{
    if ( ! self().faceValid() )
    {
        //std::cerr << "localizeBall : my face invalid conf= "
        //          << self().faceCount() << std::endl;
        return;
    }

    //////////////////////////////////////////////////////////////////
    // calc relative info

    Vector2D rpos( Vector2D::INVALIDATED );
    Vector2D rpos_error( 0.0, 0.0 );
    Vector2D rvel( Vector2D::INVALIDATED );
    Vector2D vel_error( 0.0, 0.0 );

    M_localize
        -> localizeBallRelative( see,
                                 self().face().degree(), self().faceError(),
                                 &rpos, &rpos_error,
                                 &rvel, &vel_error );
    if ( ! rpos.valid() )
    {
        dlog.addText( Logger::WORLD,
                      "world.localizeBall : invalid rpos. cannot calc current seen pos" );
        return;
    }

    //////////////////////////////////////////////////////////////////
    // Case: invalid self localization
    // to estimate ball global position, self localization is required.
    // in this case, we can estimate only relative info
    if ( ! self().posValid() )
    {
        if ( ball().rposCount() == 1
             && ( see.balls().front().dist_
                  > self().playerType().playerSize() + ServerParam::i().ballSize() + 0.1 )
             && self().lastMove().valid() )
        {
            Vector2D tvel = ( rpos - ball().rposPrev() ) + self().lastMove();
            Vector2D tvel_err = rpos_error + self().velError();
            // set only vel
            tvel *= ServerParam::i().ballDecay();
            tvel_err *= ServerParam::i().ballDecay();
            M_ball.updateOnlyVel( tvel, tvel_err, 1 );

            dlog.addText( Logger::WORLD,
                          "world.localizeBall : only vel (%.3f %.3f)",
                          tvel.x, tvel.y );
        }

        // set relative pos
        M_ball.updateOnlyRelativePos( rpos, rpos_error );

        dlog.addText( Logger::WORLD,
                      "world.localizeBall : only relative pos (%.3f %.3f)",
                      rpos.x, rpos.y );

        return;
    }

    //////////////////////////////////////////////////////////////////
    // calc global pos & vel using visual

    Vector2D pos = self().pos() + rpos;
    Vector2D pos_error = self().posError() + rpos_error;
    Vector2D gvel( Vector2D::INVALIDATED );
    int vel_count = 1000;

    if ( gameMode().type() != GameMode::PlayOn
         && gameMode().type() != GameMode::GoalKick_
         && gameMode().type() != GameMode::PenaltyTaken_ )
    {
        gvel.assign( 0.0, 0.0 );
        vel_error.assign( 0.0, 0.0 );
        vel_count = 0;
    }
    else if ( rvel.valid()
              && self().velValid() )
    {
        gvel = self().vel() + rvel;
        vel_error += self().velError();
        vel_count = 0;
    }

    //////////////////////////////////////////////////////////////////
    // calc global velocity using rpos diff (if ball is out of view cone and within vis_dist)

    if ( ! gvel.valid() )
    {
        estimateBallVelByPosDiff( see, act, current, rpos, rpos_error,
                                  gvel, vel_error, vel_count );
    }

    //////////////////////////////////////////////////////////////////
    // set data

    if ( gvel.valid() )
    {
        dlog.addText( Logger::WORLD,
                      "ball.updateAll. p(%.3f %.3f) rel(%.3f %.3f) v(%.3f %.3f)",
                      pos.x, pos.y, rpos.x, rpos.y, gvel.x, gvel.y );
        M_ball.updateAll( pos, pos_error, self().posCount(),
                          rpos, rpos_error,
                          gvel, vel_error, vel_count );
    }
    else
    {
        dlog.addText( Logger::WORLD,
                      "ball.updatePos. p(%.3f %.3f) rel(%.3f %.3f)",
                      pos.x, pos.y, rpos.x, rpos.y );
        M_ball.updatePos( pos, pos_error, self().posCount(),
                          rpos, rpos_error );
    }


    dlog.addText( Logger::WORLD,
                  "<--- ball pos=(%.2f, %.2f) err=(%.3f, %.3f)"
                  " rpos=(%.2f, %.2f) rpos_err=(%.3f, %.3f)",
                  ball().pos().x, ball().pos().y,
                  ball().posError().x, ball().posError().y,
                  ball().rpos().x, ball().rpos().y,
                  ball().rposError().x, ball().rposError().y );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
WorldModel::estimateBallVelByPosDiff( const VisualSensor & see,
                                      const ActionEffector & act,
                                      const GameTime & current,
                                      const Vector2D & rpos,
                                      const Vector2D & rpos_error,
                                      Vector2D & vel,
                                      Vector2D & vel_error,
                                      int & vel_count )
{
    if ( ball().rposCount() == 1 ) // we saw the ball at prev cycle, too.
    {
        dlog.addText( Logger::WORLD,
                      "world.estimateBallBallVel. update by rpos diff(1)." );

        if ( see.balls().front().dist_ < 3.15 // ServerParam::i().visibleDistance()
             && ball().rposPrev().valid()
             && self().velValid()
             && self().lastMove().valid() )
        {
            Vector2D rpos_diff = rpos - ball().rposPrev();
            vel = rpos_diff;// - ball().rposPrev();
            vel += self().lastMove();
            vel_error = rpos_error + self().velError();
            vel *= ServerParam::i().ballDecay();
            vel_error *= ServerParam::i().ballDecay();
            vel_count = 1;

            dlog.addText( Logger::WORLD,
                          "world.localizeBall: vel update by rpos diff."
                          " cur_rpos(%.2f %.2f) prev_rpos(%.2f %.2f) diff(%.2f %.2f) my_move(%.2f %.2f)"
                          " -> vel(%.2f, %2f)",
                          rpos.x, rpos.y,
                          ball().rposPrev().x, ball().rposPrev().y,
                          rpos_diff.x, rpos_diff.y,
                          self().lastMove().x, self().lastMove().y,
                          vel.x, vel.y );
        }
    }
    else if ( ball().rposCount() == 2 )
    {
        dlog.addText( Logger::WORLD,
                      "world.estimateBallVel update by rpos diff(2)." );

        if ( see.balls().front().dist_ < 3.15
             && act.lastBodyCommandType() != PlayerCommand::KICK
             && ball().seenRPos().valid()
             && ball().seenRPos().r() < 3.15
             && self().velValid()
             && self().lastMove( 0 ).valid()
             && self().lastMove( 1 ).valid() )
        {
            Vector2D ball_move = rpos - ball().seenRPos();
            ball_move += self().lastMove( 0 );
            ball_move += self().lastMove( 1 );
            vel = ball_move * ( square( ServerParam::i().ballDecay() )
                                / ( 1.0 + ServerParam::i().ballDecay() ) );

            double vel_r = vel.r();
            double estimate_speed = ball().vel().r();

            dlog.addText( Logger::WORLD,
                          "world.localizeBall: vel update by rpos diff(2) "
                          "diff_vel=(%.2f %.2f)%.3f   estimate_vel=(%.2f %.2f)%.3f",
                          vel.x, vel.y, vel_r,
                          ball().vel().x, ball().vel().y, estimate_speed );

            if ( vel_r > estimate_speed + 0.1
                 || vel_r < estimate_speed * ( 1.0 - ServerParam::i().ballRand() * 2.0 ) - 0.1
                 || ( vel - ball().vel() ).r() > estimate_speed * ServerParam::i().ballRand() * 2.0 + 0.1 )
            {
                dlog.addText( Logger::WORLD,
                              "world.localizeBall: .failed to update ball vel using pos diff(2) " );
                vel.invalidate();
            }
            else
            {
                vel_error = ( rpos_error * 2.0 ) + self().velError();
                vel_error *= ServerParam::i().ballDecay();
                vel_count = 2;
                dlog.addText( Logger::WORLD,
                              "world.localizeBall: vel update by rpos diff(2)."
                              " cur_rpos(%.2f %.2f) prev_rpos(%.2f %.2f)"
                              " ball_move(%.2f %.2f)"
                              " my_move0(%.2f %.2f) my_move1(%.2f %.2f)"
                              " -> vel(%.2f, %2f)",
                              rpos.x, rpos.y,
                              ball().seenRPos().x, ball().seenRPos().y,
                              ball_move.x, ball_move.y,
                              self().lastMove( 0 ).x, self().lastMove( 0 ).y,
                              self().lastMove( 1 ).x, self().lastMove( 1 ).y,
                              vel.x, vel.y );
            }

        }
    }
#if 0
    else if ( ball().rposCount() == 3 )
    {
        dlog.addText( Logger::WORLD,
                      "world.localizeBall: vel update by rpos diff(3) " );

        if ( see.balls().front().dist_ < 3.15
             && act.lastBodyCommandType( 0 ) != PlayerCommand::KICK
             && act.lastBodyCommandType( 1 ) != PlayerCommand::KICK
             && ball().seenRPos().valid()
             && ball().seenRPos().r() < 3.15
             && self().velValid()
             && self().lastMove( 0 ).valid()
             && self().lastMove( 1 ).valid()
             && self().lastMove( 2 ).valid() )
        {
            Vector2D ball_move = rpos - ball().seenRPos();
            ball_move += self().lastMove( 0 );
            ball_move += self().lastMove( 1 );
            ball_move += self().lastMove( 2 );

            vel = ball_move * ( std::pow( ServerParam::i().ballDecay(), 3.0 )
                                / ( 1.0 + ServerParam::i().ballDecay() + square( ServerParam::i().ballDecay() ) ) );

            double vel_r = vel.r();
            double estimate_speed = ball().vel().r();

            dlog.addText( Logger::WORLD,
                          "world.localizeBall: vel update by rpos diff(2) "
                          "diff_vel=(%.2f %.2f)%.3f   estimate_vel=(%.2f %.2f)%.3f",
                          vel.x, vel.y, vel_r,
                          ball().vel().x, ball().vel().y, estimate_speed );

            if ( vel_r > estimate_speed + 0.1
                 || vel_r < estimate_speed * ( 1.0 - ServerParam::i().ballRand() * 3.0 ) - 0.1
                 || ( vel - ball().vel() ).r() > estimate_speed * ServerParam::i().ballRand() * 3.0 + 0.1 )
            {
                dlog.addText( Logger::WORLD,
                              "world.localizeBall: .failed to update ball vel using pos diff(2) " );
                vel.invalidate();
            }
            else
            {
                vel_error = ( rpos_error * 3.0 ) + self().velError();
                vel_error *= ServerParam::i().ballDecay();
                vel_count = 2;
                dlog.addText( Logger::WORLD,
                              "world.localizeBall: vel update by rpos diff(2)."
                              " cur_rpos(%.2f %.2f) prev_rpos(%.2f %.2f)"
                              " ball_move(%.2f %.2f)"
                              " my_move0(%.2f %.2f) my_move1(%.2f %.2f) my_move2(%.2f %.2f)"
                              " -> vel(%.2f, %2f)",
                              rpos.x, rpos.y,
                              ball().seenRPos().x, ball().seenRPos().y,
                              ball_move.x, ball_move.y,
                              self().lastMove( 0 ).x, self().lastMove( 0 ).y,
                              self().lastMove( 1 ).x, self().lastMove( 1 ).y,
                              self().lastMove( 2 ).x, self().lastMove( 2 ).y,
                              vel.x, vel.y );
            }
        }
    }
#endif
}

/*-------------------------------------------------------------------*/
/*!

*/
void
WorldModel::localizePlayers( const VisualSensor & see )
{
    if ( ! self().faceValid()
         || ! self().posValid() )
    {
        return;
    }

    ////////////////////////////////////////////////////////////////
    // update policy
    //   for each (seen player objects)
    //       if exist matched player in memory list
    //           -> splice from memory to temporary
    //       else
    //           -> assign new data to temporary list
    //   after loop, copy from temporary to memory again

    // temporary data list
    PlayerCont new_teammates;
    PlayerCont new_opponents;
    PlayerCont new_unknown_players;

    const Vector2D MYPOS = self().pos();
    const Vector2D MYVEL = self().vel();
    const double MY_FACE = self().face().degree();
    const double MY_FACE_ERR = self().faceError();

    //////////////////////////////////////////////////////////////////
    // search order is very important !!
    //   If we replace the unknown player to unknown teammate,
    //   it may cause a mistake for pass target selection.

    // current version search order is
    //   [unum opp -> side opp -> unum mate -> side mate -> unknown]

    // if matched, that player is removed from memory list
    // and copy to temporary

    //////////////////////////////////////////////////////////////////
    // localize, matching and splice from memory list to temporary list

    // unum seen opp
    {
        const VisualSensor::PlayerCont::const_iterator o_end
            = see.opponents().end();
        for ( VisualSensor::PlayerCont::const_iterator it
                  = see.opponents().begin();
              it != o_end;
              ++it )
        {
            Localization::PlayerT player;
            // localize
            M_localize->localizePlayer( *it,
                                        MY_FACE, MY_FACE_ERR, MYPOS, MYVEL,
                                        &player );
            // matching, splice or create
            dlog.addText( Logger::WORLD,
                          "-- localized opponent %d pos=(%.2f, %.2f) vel=(%.2f, %.2f)",
                          player.unum_,
                          player.pos_.x, player.pos_.y,
                          player.vel_.x, player.vel_.y );
            checkTeamPlayer( theirSide(),
                             player, it->dist_,
                             M_opponents,
                             M_unknown_players,
                             new_opponents );
        }
    }
    // side seen opp
    {
        const VisualSensor::PlayerCont::const_iterator uo_end
            = see.unknownOpponents().end();
        for ( VisualSensor::PlayerCont::const_iterator it
                  = see.unknownOpponents().begin();
              it != uo_end;
              ++it )
        {
            Localization::PlayerT player;
            // localize
            M_localize->localizePlayer( *it,
                                        MY_FACE, MY_FACE_ERR, MYPOS, MYVEL,
                                        &player );
            dlog.addText( Logger::WORLD,
                          "-- localized u-opponent pos=(%.2f, %.2f)",
                          player.pos_.x, player.pos_.y );
            // matching, splice or create
            checkTeamPlayer( theirSide(),
                             player, it->dist_,
                             M_opponents,
                             M_unknown_players,
                             new_opponents );
        }
    }
    // unum seen mate
    {
        const VisualSensor::PlayerCont::const_iterator t_end
            = see.teammates().end();
        for ( VisualSensor::PlayerCont::const_iterator it
                  = see.teammates().begin();
              it != t_end;
              ++it )
        {
            Localization::PlayerT player;
            // localize
            M_localize->localizePlayer( *it,
                                        MY_FACE, MY_FACE_ERR, MYPOS, MYVEL,
                                        &player );
            dlog.addText( Logger::WORLD,
                          "-- localized teammate %d pos=(%.2f, %.2f) vel=(%.2f, %.2f)",
                          player.unum_,
                          player.pos_.x, player.pos_.y,
                          player.vel_.x, player.vel_.y );
            // matching, splice or create
            checkTeamPlayer( ourSide(),
                             player, it->dist_,
                             M_teammates,
                             M_unknown_players,
                             new_teammates );
        }
    }
    // side seen mate
    {
        const VisualSensor::PlayerCont::const_iterator ut_end
            = see.unknownTeammates().end();
        for ( VisualSensor::PlayerCont::const_iterator it
                  = see.unknownTeammates().begin();
              it != ut_end;
              ++it )
        {
            Localization::PlayerT player;
            // localize
            M_localize->localizePlayer( *it,
                                        MY_FACE, MY_FACE_ERR, MYPOS, MYVEL,
                                        &player );
            dlog.addText( Logger::WORLD,
                          "-- localized u-teammate pos=(%.2f, %.2f)",
                          player.pos_.x, player.pos_.y );
            // matching, splice or create
            checkTeamPlayer( ourSide(),
                             player, it->dist_,
                             M_teammates,
                             M_unknown_players,
                             new_teammates );
        }
    }
    // unknown player
    {
        const VisualSensor::PlayerCont::const_iterator u_end
            = see.unknownPlayers().end();
        for ( VisualSensor::PlayerCont::const_iterator it
                  = see.unknownPlayers().begin();
              it != u_end;
              ++it )
        {
            Localization::PlayerT player;
            // localize
            M_localize->localizePlayer( *it,
                                        MY_FACE, MY_FACE_ERR, MYPOS, MYVEL,
                                        &player );
            dlog.addText( Logger::WORLD,
                          "-- localized unknown: pos=(%.2f, %.2f)",
                          player.pos_.x, player.pos_.y );
            // matching, splice or create
            checkUnknownPlayer( player,
                                it->dist_,
                                M_teammates,
                                M_opponents,
                                M_unknown_players,
                                new_teammates,
                                new_opponents,
                                new_unknown_players );
        }
    }

    //////////////////////////////////////////////////////////////////
    // splice temporary seen players to memory list
    // temporary lists are cleared
    M_teammates.splice( M_teammates.end(),
                        new_teammates );
    M_opponents.splice( M_opponents.end(),
                        new_opponents );
    M_unknown_players.splice( M_unknown_players.end(),
                              new_unknown_players );

    //////////////////////////////////////////////////////////////////
    // create team member pointer vector for sort

    PlayerPtrCont all_teammates_ptr;
    PlayerPtrCont all_opponents_ptr;

    {
        const PlayerCont::iterator end = M_teammates.end();
        for ( PlayerCont::iterator it = M_teammates.begin();
              it != end;
              ++it )
        {
            all_teammates_ptr.push_back( &( *it ) );
        }
    }
    {
        const PlayerCont::iterator end = M_opponents.end();
        for ( PlayerCont::iterator it = M_opponents.begin();
              it != end;
              ++it )
        {
            all_opponents_ptr.push_back( &( *it ) );
        }
    }


    /////////////////////////////////////////////////////////////////
    // sort by accuracy count
    std::sort( all_teammates_ptr.begin(),
               all_teammates_ptr.end(),
               PlayerObject::PtrCountCmp() );
    std::sort( all_opponents_ptr.begin(),
               all_opponents_ptr.end(),
               PlayerObject::PtrCountCmp() );
    M_unknown_players.sort( PlayerObject::CountCmp() );


    //////////////////////////////////////////////////////////////////
    // check the number of players
    // if overflow is detected, player is removed based on confidence value

    // remove from teammates
    PlayerPtrCont::size_type mate_count = all_teammates_ptr.size();
    while ( mate_count > 11 - 1 )
    {
        // reset least confidence value player
        dlog.addText( Logger::WORLD,
                      "%s: localize players: erase overflow teammate pos=(%.2f, %.2f)"
                      ,__FILE__,
                      all_teammates_ptr.back()->pos().x,
                      all_teammates_ptr.back()->pos().y );
        all_teammates_ptr.back()->forget();
        all_teammates_ptr.pop_back();
        --mate_count;
    }

    // remove from not-teammates
    PlayerPtrCont::size_type opp_count = all_opponents_ptr.size();
    while ( opp_count > 15 ) // 11 )
    {
        // reset least confidence value player
        dlog.addText( Logger::WORLD,
                      "%s: localize players: erase overflow opponent pos=(%.2f, %.2f)"
                      ,__FILE__,
                      all_opponents_ptr.back()->pos().x,
                      all_opponents_ptr.back()->pos().y );
        all_opponents_ptr.back()->forget();
        all_opponents_ptr.pop_back();
        --opp_count;
    }

    // remove from unknown players
    PlayerCont::size_type n_size_unknown = M_unknown_players.size();
    size_t n_size_total
        = static_cast< size_t >( n_size_unknown )
        + static_cast< size_t >( mate_count )
        + static_cast< size_t >( opp_count );
    while ( n_size_unknown > 0
            && n_size_total > 11 + 15 - 1 ) //11 * 2 - 1 )
    {
        dlog.addText( Logger::WORLD,
                      "%s: localize players: "
                      " erase over flow unknown player. pos=(%.2f, %.2f)"
                      ,__FILE__,
                      M_unknown_players.back().pos().x,
                      M_unknown_players.back().pos().y );
        if ( M_unknown_players.back().posCount() == 0 )
        {
            // not remove !!!
            break;
        }
        // remove least confidence value player
        M_unknown_players.pop_back();
        --n_size_unknown;
        --n_size_total;
    }


    //////////////////////////////////////////////////////////////////
    // if overflow is detected, instance player must be forget.
    // that player must be removed from memory list.

    // check invalid player
    // if exist, that player is removed from instance list
    M_teammates.remove_if( PlayerObject::IsInvalidOp() );
    M_opponents.remove_if( PlayerObject::IsInvalidOp() );

    //////////////////////////////////////////////////////////////////
    // it is not necessary to check the all unknown list
    // because invalid unknown player is already removed.


    //////////////////////////////////////////////////////////////////
    // ghost check is done in checkGhost()
}

/*-------------------------------------------------------------------*/
/*!

*/
void
WorldModel::checkTeamPlayer( const SideID side,
                             const Localization::PlayerT & player,
                             const double & seen_dist,
                             PlayerCont & old_known_players,
                             PlayerCont & old_unknown_players,
                             PlayerCont & new_known_players )
{
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! //
    //  if matched player is found, that player is removed from old list
    //  and updated data is splice to new container
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! //

    static const
        double player_speed_max
        = ServerParam::i().defaultPlayerSpeedMax() * 1.1;

    const double quantize_buf
        = unquantize_error( seen_dist, ServerParam::i().distQuantizeStep() );


    //////////////////////////////////////////////////////////////////
    // pre check
    // unum is seen -> search the player that has the same uniform number
    if ( player.unum_ != Unum_Unknown )
    {
        // search from old unum known players
        const PlayerCont::iterator end = old_known_players.end();
        for ( PlayerCont::iterator it = old_known_players.begin();
              it != end;
              ++it )
        {
            if ( it->unum() == player.unum_ )
            {
                dlog.addText( Logger::WORLD,
                              "--- check team player: matched!"
                              " unum = %d pos =(%.1f %.1f)",
                              player.unum_, player.pos_.x, player.pos_.y );
                it->updateBySee( side, player );
                new_known_players.splice( new_known_players.end(),
                                          old_known_players,
                                          it );
                return; // success!!
            }
        }
    }

    //////////////////////////////////////////////////////////////////
    // find nearest player

    double min_team_dist = 10.0 * 10.0;
    double min_unknown_dist = 10.0 * 10.0;

    PlayerCont::iterator candidate_team = old_known_players.end();
    PlayerCont::iterator candidate_unknown = old_unknown_players.end();

    //////////////////////////////////////////////////////////////////
    {
        // search from old same team players
        const PlayerCont::iterator end = old_known_players.end();
        for ( PlayerCont::iterator it = old_known_players.begin();
              it != end;
              ++it )
        {
            if ( player.unum_ != Unum_Unknown
                 && it->unum() != Unum_Unknown
                 && it->unum() != player.unum_ )
            {
                // unum is seen
                // and it does not match with old player's unum.
#ifdef DEBUG
                dlog.addText( Logger::WORLD,
                              "______checkTeamPlayer known player: unum is not match."
                              " seen unum = %d, old_unum = %d",
                              player.unum_, it->unum() );
#endif
                continue;
            }

            double d = ( player.pos_ - it->pos() ).r();

            if ( d > ( player_speed_max * it->posCount() + quantize_buf * 2.0
                       + 2.0 )
                 )
            {
                // TODO: inertia movement should be considered.
#ifdef DEBUG
                dlog.addText( Logger::WORLD,
                              "______checkTeamPlayer known player: dist over."
                              " dist=%.2f > buf=%.2f"
                              " seen_pos(%.1f %.1f) old_pos(%.1f %.1f)",
                              d,
                              ( player_speed_max * it->posCount()
                                + quantize_buf * 2.0
                                + 2.0 ),
                              player.pos_.x, player.pos_.y,
                              it->pos().x, it->pos().y );
#endif
                continue;
            }

            if ( d < min_team_dist )
            {
#ifdef DEBUG
                dlog.addText( Logger::WORLD,
                              "______checkTeamPlayer known player: update."
                              " dist=%.2f < min_team_dist=%.2f"
                              " seen_pos(%.1f %.1f) old_pos(%.1f %.1f)",
                              d,
                              min_team_dist,
                              player.pos_.x, player.pos_.y,
                              it->pos().x, it->pos().y );
#endif
                min_team_dist = d;
                candidate_team = it;
            }
        }
    }

    //////////////////////////////////////////////////////////////////
    // search from unknown players
    {
        const PlayerCont::iterator end = old_unknown_players.end();
        for ( PlayerCont::iterator it = old_unknown_players.begin();
              it != end;
              ++it )
        {
            double d = ( player.pos_ - it->pos() ).r();

            if ( d > ( player_speed_max * it->posCount() + quantize_buf * 2.0
                       + 2.0 )
                 )
            {
                // TODO: inertia movement should be considered.
#ifdef DEBUG
                dlog.addText( Logger::WORLD,
                              "______checkTeamPlayer unknown player: dist over. "
                              "dist=%.2f > buf=%.2f"
                              " seen_pos(%.1f %.1f) old_pos(%.1f %.1f)",
                              d,
                              ( player_speed_max * it->posCount()
                                + quantize_buf * 2.0
                                + 2.0 ),
                              player.pos_.x, player.pos_.y,
                              it->pos().x, it->pos().y );
#endif
                continue;
            }

            if ( d < min_unknown_dist )
            {
#ifdef DEBUG
                dlog.addText( Logger::WORLD,
                              "______checkTeamPlayer. unknown player: update. "
                              " dist=%.2f < min_unknown_dist=%.2f"
                              " seen_pos(%.1f %.1f) old_pos(%.1f %.1f)",
                              d,
                              min_unknown_dist,
                              player.pos_.x, player.pos_.y,
                              it->pos().x, it->pos().y );
#endif
                min_unknown_dist = d;
                candidate_unknown = it;
            }
        }
    }


    PlayerCont::iterator candidate = old_unknown_players.end();
    double min_dist = 1000.0;
    PlayerCont * target_list = static_cast< PlayerCont * >( 0 );

    if ( candidate_team != old_known_players.end()
         && min_team_dist < min_unknown_dist )
    {
        candidate = candidate_team;
        min_dist = min_team_dist;
        target_list = &old_known_players;

        dlog.addText( Logger::WORLD,
                      "--- check team player %d (%.1f %.1f)"
                      " -> team player %d (%.2f, %.2f) dist=%.2f",
                      player.unum_,
                      player.pos_.x, player.pos_.y,
                      candidate->unum(),
                      candidate->pos().x, candidate->pos().y,
                      min_dist );
    }

    if ( candidate_unknown != old_unknown_players.end()
         && min_unknown_dist < min_team_dist )
    {
        candidate = candidate_unknown;
        min_dist = min_unknown_dist;
        target_list = &old_unknown_players;

        dlog.addText( Logger::WORLD,
                      "--- check team player %d (%.1f %.1f)"
                      " -> unknown player (%.2f, %.2f) dist=%.2f",
                      player.unum_,
                      player.pos_.x, player.pos_.y,
                      candidate->pos().x, candidate->pos().y,
                      min_dist );
    }


    //////////////////////////////////////////////////////////////////
    // check player movable radius,
    if ( candidate != old_unknown_players.end()
         && target_list )
    {
        // update & splice to new list
        candidate->updateBySee( side, player );

        new_known_players.splice( new_known_players.end(),
                                  *target_list,
                                  candidate );
        return;
    }

    //////////////////////////////////////////////////////////////////
    // generate new player
    dlog.addText( Logger::WORLD,
                  "XXX--- check team player: unmatch. min_dist= %.2f"
                  "  generate new known player pos=(%.2f, %.2f)",
                  min_dist,
                  player.pos_.x, player.pos_.y );

    new_known_players.push_back( PlayerObject( side, player ) );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
WorldModel::checkUnknownPlayer( const Localization::PlayerT & player,
                                const double & seen_dist,
                                PlayerCont & old_teammates,
                                PlayerCont & old_opponents,
                                PlayerCont & old_unknown_players,
                                PlayerCont & new_teammates,
                                PlayerCont & new_opponents,
                                PlayerCont & new_unknown_players )
{
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! //
    //  if matched player is found, that player is removed from old list
    //  and updated data is splice to new container
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! //

    //////////////////////////////////////////////////////////////////
#if 0
    // if seen unknown player is within visible distance(=behind of agents)
    // it is very risky to match the exsiting player
    if ( seen_dist < ServerParam::i().visibleDistance() + 0.2 )
    {
        // generate new player
        dlog.addText( Logger::WORLD,
                      "%s: check unknown player: behind. "
                      "  generate new unknown player. pos=(%.2f, %.2f)"
                      ,__FILE__,
                      player.pos_.x, player.pos_.y );

        new_unknown_players.push_back( PlayerObject( player ) );

        return;
    }
#endif

    static const
        double player_speed_max
        = ServerParam::i().defaultPlayerSpeedMax() * 1.1;

    const double quantize_buf
        = unquantize_error( seen_dist, ServerParam::i().distQuantizeStep() );

    // matching start
    // search the nearest player

    double min_opponent_dist = 10.0 * 10.0;
    double min_teammate_dist = 10.0 * 10.0;
    double min_unknown_dist = 10.0 * 10.0;

    PlayerCont::iterator candidate_opponent = old_opponents.end();
    PlayerCont::iterator candidate_teammate = old_teammates.end();
    PlayerCont::iterator candidate_unknown = old_unknown_players.end();

    //////////////////////////////////////////////////////////////////
    // search from old opponents
    {
        const PlayerCont::iterator end = old_opponents.end();
        for ( PlayerCont::iterator it = old_opponents.begin();
              it != end;
              ++it )
        {
            double d = ( player.pos_ - it->pos() ).r();
            double buf = ( seen_dist < 3.2
                           ? 0.2
                           : 2.0 );

            if ( d > ( player_speed_max * it->posCount()
                       + quantize_buf * 2.0
                       + buf )
                 )
            {
#ifdef DEBUG
                dlog.addText( Logger::WORLD,
                              "______checkUnknownPlayer opp player: dist over."
                              " dist=%.2f > buf=%.2f"
                              " seen_pos(%.1f %.1f) old_pos(%.1f %.1f)",
                              d,
                              ( player_speed_max * it->posCount()
                                + quantize_buf * 2.0
                                + ( it->posCount() * 1.2 )
                                + buf ),
                              player.pos_.x, player.pos_.y,
                              it->pos().x, it->pos().y );
#endif
                continue;
            }

            if ( d < min_opponent_dist )
            {
#ifdef DEBUG
                dlog.addText( Logger::WORLD,
                              "______checkUnknownPlayer. opp player: update."
                              " dist=%.2f < min_opp_dist=%.2f"
                              " seen_pos(%.1f %.1f) old_pos(%.1f %.1f)",
                              d,
                              min_opponent_dist,
                              player.pos_.x, player.pos_.y,
                              it->pos().x, it->pos().y );
#endif
                min_opponent_dist = d;
                candidate_opponent = it;
            }
        }
    }

    //////////////////////////////////////////////////////////////////
    // search from old teammates
    {
        const PlayerCont::iterator end = old_teammates.end();
        for ( PlayerCont::iterator it = old_teammates.begin();
              it != end;
              ++it )
        {
            double d = ( player.pos_ - it->pos() ).r();
            double buf = ( seen_dist < 3.2
                           ? 0.2
                           : 2.0 );

            if ( d > ( player_speed_max * it->posCount()
                       + quantize_buf * 2.0
                       + buf )
                 )
            {
#ifdef DEBUG
                dlog.addText( Logger::WORLD,
                              "______checkUnknownPlayer our player: dist over."
                              " dist=%.2f > buf=%.2f"
                              " seen_pos(%.1f %.1f) old_pos(%.1f %.1f)",
                              d,
                              ( player_speed_max * it->posCount()
                                + quantize_buf * 2.0
                                + buf ),
                              player.pos_.x, player.pos_.y,
                              it->pos().x, it->pos().y );
#endif
                continue;
            }

            if ( d < min_teammate_dist )
            {
#ifdef DEBUG
                dlog.addText( Logger::WORLD,
                              "______checkUnknownPlayer. our player: update."
                              " dist=%.2f < min_our_dist=%.2f"
                              " seen_pos(%.1f %.1f) old_pos(%.1f %.1f)",
                              d,
                              min_teammate_dist,
                              player.pos_.x, player.pos_.y,
                              it->pos().x, it->pos().y );
#endif
                min_teammate_dist = d;
                candidate_teammate = it;
            }
        }
    }

    //////////////////////////////////////////////////////////////////
    // search from old unknown players
    {
        const PlayerCont::iterator end = old_unknown_players.end();
        for ( PlayerCont::iterator it = old_unknown_players.begin();
              it != end;
              ++it )
        {
            double d = ( player.pos_ - it->pos() ).r();
            double buf = ( seen_dist < 3.2
                           ? 0.2
                           : 2.0 );

            if ( d > ( player_speed_max * it->posCount()
                       + quantize_buf * 2.0
                       + buf )
                 )
            {
#ifdef DEBUG
                dlog.addText( Logger::WORLD,
                              "______checkUnknownPlayer unknown player: dist over."
                              " dist=%.2f > buf=%.2f"
                              " seen_pos(%.1f %.1f) old_pos(%.1f %.1f)",
                              d,
                              ( player_speed_max * it->posCount()
                                + quantize_buf * 2.0
                                + buf ),
                              player.pos_.x, player.pos_.y,
                              it->pos().x, it->pos().y );
#endif
                continue;
            }

            if ( d < min_unknown_dist )
            {
#ifdef DEBUG
                dlog.addText( Logger::WORLD,
                              "______checkUnknownPlayer. unknown player: update."
                              " dist=%.2f < min_unknown_dist=%.2f"
                              " seen_pos(%.1f %.1f) old_pos(%.1f %.1f)",
                              d,
                              min_unknown_dist,
                              player.pos_.x, player.pos_.y,
                              it->pos().x, it->pos().y );
#endif
                min_unknown_dist = d;
                candidate_unknown = it;
            }
        }
    }

    PlayerCont::iterator candidate = old_unknown_players.end();;
    double min_dist = 1000.0;
    PlayerCont * new_list = static_cast< PlayerCont * >( 0 );
    PlayerCont * old_list = static_cast< PlayerCont * >( 0 );
    SideID side = NEUTRAL;

    if ( candidate_teammate != old_teammates.end()
         && min_teammate_dist < min_opponent_dist
         && min_teammate_dist < min_unknown_dist )
    {
        candidate = candidate_teammate;
        min_dist = min_teammate_dist;
        new_list = &new_teammates;
        old_list = &old_teammates;
        side = ourSide();

        dlog.addText( Logger::WORLD,
                      "--- check unknown player (%.1f %.1f)"
                      " -> teammate %d (%.1f %.1f) dist=%.2f",
                      player.pos_.x, player.pos_.y,
                      candidate->unum(),
                      candidate->pos().x, candidate->pos().y,
                      min_dist );
    }

    if ( candidate_opponent != old_opponents.end()
         && min_opponent_dist * 0.5 - 3.0 < min_teammate_dist
         && min_opponent_dist < min_unknown_dist )
    {
        candidate = candidate_opponent;
        min_dist = min_opponent_dist;
        new_list = &new_opponents;
        old_list = &old_opponents;
        side = theirSide();

        dlog.addText( Logger::WORLD,
                      "--- check unknown player (%.1f %.1f)"
                      " -> opponent %d (%.1f %.1f) dist=%.2f",
                      player.pos_.x, player.pos_.y,
                      candidate->unum(),
                      candidate->pos().x, candidate->pos().y,
                      min_dist );
    }

    if ( candidate_unknown != old_unknown_players.end()
         && min_unknown_dist * 0.5 - 3.0 < min_teammate_dist
         && min_unknown_dist < min_opponent_dist )
    {
        candidate = candidate_unknown;
        min_dist = min_unknown_dist;
        new_list = &new_unknown_players;
        old_list = &old_unknown_players;
        side = NEUTRAL;

        dlog.addText( Logger::WORLD,
                      "--- check unknown player (%.1f %.1f)"
                      " -> unknown (%.1f %.1f) dist=%.2f",
                      player.pos_.x, player.pos_.y,
                      candidate->pos().x, candidate->pos().y,
                      min_dist );
    }


    //////////////////////////////////////////////////////////////////
    // check player movable radius
    if ( candidate != old_unknown_players.end()
         && new_list
         && old_list )
    {
        // update & splice to new list
        candidate->updateBySee( side, player );
        new_list->splice( new_list->end(),
                          *old_list,
                          candidate );
        return;
    }

    //////////////////////////////////////////////////////////////////
    // generate new player
    dlog.addText( Logger::WORLD,
                  "XXX--- check unknown player: unmatch. quant_buf= %.2f"
                  "  generate new unknown player. pos=(%.2f, %.2f)",
                  quantize_buf,
                  player.pos_.x, player.pos_.y );

    new_unknown_players.push_back( PlayerObject( NEUTRAL, player ) );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
WorldModel::updatePlayerType()
{
   {
        const PlayerCont::iterator end = M_teammates.end();
        for ( PlayerCont::iterator it = M_teammates.begin();
              it != end;
              ++it )
        {
            int n = it->unum() - 1;
            if ( 0 <= n && n < 11 )
            {
                it->setPlayerType( M_teammate_types[n] );
            }
            else
            {
                it->setPlayerType( Hetero_Default );
            }
        }
    }

    {
        const PlayerCont::iterator end = M_opponents.end();
        for ( PlayerCont::iterator it = M_opponents.begin();
              it != end;
              ++it )
        {
            int n = it->unum() - 1;
            if ( 0 <= n && n < 11 )
            {
                it->setPlayerType( M_opponent_types[n] );
            }
            else
            {
                it->setPlayerType( Hetero_Unknown );
            }
        }
    }

    {
        const PlayerCont::iterator end = M_unknown_players.end();
        for ( PlayerCont::iterator it = M_unknown_players.begin();
              it != end;
              ++it )
        {
            it->setPlayerType( Hetero_Unknown );
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
WorldModel::updateObjectRelation()
{
    // update ball matrix
    M_ball.updateSelfRelated( self() );

    // update about ball controll
    M_self.updateBallInfo( ball() );

    // update players matrix
    updatePlayerMatrix();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
WorldModel::updatePlayerMatrix()
{
    //M_teammates_from_self.clear();
    //M_opponents_from_self.clear();
    //M_teammates_from_ball.clear();
    //M_opponents_from_ball.clear();

    if ( ! self().posValid()
         || ! ball().posValid() )
    {
        return;
    }

    dlog.addText( Logger::WORLD,
                  "WorldModel::updatePlayerMatrix()" );

    WMImpl::create_player_set( M_teammates,
                               M_teammates_from_self,
                               M_teammates_from_ball,
                               self().pos(),
                               ball().pos() );
    WMImpl::create_player_set( M_opponents,
                               M_opponents_from_self,
                               M_opponents_from_ball,
                               self().pos(),
                               ball().pos() );
    WMImpl::create_player_set( M_unknown_players,
                               M_opponents_from_self,
                               M_opponents_from_ball,
                               self().pos(),
                               ball().pos() );

    // sort by distance to self or ball
    std::sort( M_teammates_from_self.begin(),
               M_teammates_from_self.end(),
               PlayerObject::PtrSelfDistCmp() );
    std::sort( M_opponents_from_self.begin(),
               M_opponents_from_self.end(),
               PlayerObject::PtrSelfDistCmp() );

    std::sort( M_teammates_from_ball.begin(),
               M_teammates_from_ball.end(),
               PlayerObject::PtrBallDistCmp() );
    std::sort( M_opponents_from_ball.begin(),
               M_opponents_from_ball.end(),
               PlayerObject::PtrBallDistCmp() );

    // check opponent goalie
    if ( M_opponent_goalie_unum == Unum_Unknown )
    {
        const PlayerObject * p = getOpponentGoalie();
        if ( p )
        {
            M_opponent_goalie_unum = p->unum();
        }
    }

    {
        const PlayerPtrCont::iterator t_end = M_teammates_from_ball.end();
        const PlayerPtrCont::iterator o_end = M_opponents_from_ball.end();

        M_all_players.push_back( &M_self );
        M_all_teammates.push_back( &M_self );
        M_known_teammates[self().unum()] = &M_self;

        for ( PlayerPtrCont::iterator t = M_teammates_from_ball.begin();
              t != t_end;
              ++t )
        {
            M_all_players.push_back( *t );
            M_all_teammates.push_back( *t );

            if ( (*t)->unum() != Unum_Unknown )
            {
                M_known_teammates[(*t)->unum()] = *t;
            }
        }

        for ( PlayerPtrCont::iterator o = M_opponents_from_ball.begin();
              o != o_end;
              ++o )
        {
            M_all_players.push_back( *o );
            M_all_opponents.push_back( *o );

            if ( (*o)->unum() != Unum_Unknown )
            {
                M_known_opponents[(*o)->unum()] = *o;
            }
        }

    }

    // check kickable player
    M_exist_kickable_teammate
        = WMImpl::check_player_kickable( M_teammates_from_ball.begin(),
                                         M_teammates_from_ball.end(),
                                         ball().posCount() );
    M_exist_kickable_opponent
        = WMImpl::check_player_kickable( M_opponents_from_ball.begin(),
                                         M_opponents_from_ball.end(),
                                         ball().posCount() );

    dlog.addText( Logger::WORLD,
                  "size of player set"
                  " ourFMself %d"
                  " ourFMball %d"
                  " oppFMself %d"
                  " oppFMball %d",
                  M_teammates_from_self.size(),
                  M_teammates_from_ball.size(),
                  M_opponents_from_self.size(),
                  M_opponents_from_ball.size() );

    dlog.addText( Logger::WORLD,
                  "opponent goalie = %d",
                  M_opponent_goalie_unum );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
WorldModel::updateOffsideLine()
{
    if ( ! ServerParam::i().useOffside() )
    {
        M_offside_line_x = ServerParam::i().pitchHalfLength();
        return;
    }

    if ( gameMode().type() == GameMode::KickIn_
         || gameMode().type() == GameMode::CornerKick_
         || ( gameMode().type() == GameMode::GoalKick_
              && gameMode().side() == ourSide() )
         )
    {
        M_offside_line_count = 0;
        M_offside_line_x = ServerParam::i().pitchHalfLength();
        return;
    }

    if ( gameMode().side() != ourSide()
         && ( gameMode().type() == GameMode::GoalieCatch_
              || gameMode().type() == GameMode::GoalKick_ )
         )
    {
        M_offside_line_count = 0;
        M_offside_line_x = ServerParam::i().theirPenaltyAreaLineX();
        return;
    }

    const double speed_rate
        = ( ball().vel().x < -1.0
            ? ServerParam::i().defaultPlayerSpeedMax() * 0.8
            : ServerParam::i().defaultPlayerSpeedMax() * 0.25 );

    //////////////////////////////////////////////////////////////////
    double first = 0.0, second = 0.0;
    int first_count = 100, second_count = 100;
    int opponent_count = 0;
    {
        const PlayerPtrCont::iterator end = M_opponents_from_self.end();
        for ( PlayerPtrCont::iterator it = M_opponents_from_self.begin();
              it != end;
              ++it )
        {
            ++opponent_count;
            double posx = (*it)->pos().x;
            posx -= speed_rate * std::min( 10, (*it)->posCount() );
            if ( posx > second )
            {
                second = posx;
                second_count = (*it)->posCount();
                if ( second > first )
                {
                    std::swap( first, second );
                    std::swap( first_count, second_count );
                }
            }
        }
    }

    const PlayerObject * goalie = getOpponentGoalie();
    if ( ! goalie )
    {
        if ( 20.0 < ball().pos().x
             && ball().pos().x < ServerParam::i().theirPenaltyAreaLineX() )
        {
            if ( first < ServerParam::i().theirPenaltyAreaLineX() )
            {
                dlog.addText( Logger::WORLD,
                              "%s: offside line. no goalie. %.1f -> %.1f"
                              ,__FILE__,
                              second, first );
                second = first;
                second_count = 30;
            }
        }
    }

    //////////////////////////////////////////////////////////////////
    double new_line = second;
    int count = second_count;

    // consider old offside line

    if ( opponent_count >= 11 )
    {
        // new_line is used directly
    }
    else if ( new_line < M_offside_line_x - 13.0 )
    {
        // new_line is used directly
    }
    else if ( new_line < M_offside_line_x - 5.0 )
    {
        new_line = M_offside_line_x - 1.0;
    }

    if ( new_line < 0.0 )
    {
        new_line = 0.0;
    }

    // ball is more forward than opponent defender line
    if ( gameMode().type() != GameMode::BeforeKickOff
         && gameMode().type() != GameMode::AfterGoal_
         && ball().posCount() <= 3 )
    {
        Vector2D ball_next = ball().pos() + ball().vel();
        if ( ball_next.x > new_line )
        {
            new_line = ball_next.x;
            count = ball().posCount();
        }
    }

    if ( M_audio_memory->offsideLineTime() == this->time()
         && ! M_audio_memory->offsideLine().empty()
         //&& new_line < M_audio_memory->offsideLine().front().x_ - 1.0
         )
    {
        double heard_x = 0.0;
        for ( std::vector< AudioMemory::OffsideLine >::const_iterator it
                  = M_audio_memory->offsideLine().begin();
              it != M_audio_memory->offsideLine().end();
              ++it )
        {
            heard_x += it->x_;
        }
        heard_x /= static_cast< double >( M_audio_memory->offsideLine().size() );

        if ( new_line < heard_x - 1.0 )
        {
            dlog.addText( Logger::WORLD,
                          __FILE__": update offside line by heard info. %.1f -> %.1f",
                          new_line, heard_x );

            new_line = heard_x;
            count = 30;
        }
    }

    M_offside_line_x = new_line;
    M_offside_line_count = count;

    dlog.addText( Logger::WORLD,
                  __FILE__": offside line = %.2f  count = %d",
                  new_line, count );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
WorldModel::updateDefenseLine()
{
    //    const double speed_rate = ServerParam::i().defaultPlayerSpeedMax() * 0.5;

    //////////////////////////////////////////////////////////////////
    double first = 0.0, second = 0.0;
    {
        const PlayerPtrCont::iterator end = M_teammates_from_self.end();
        for ( PlayerPtrCont::iterator it = M_teammates_from_self.begin();
              it != end;
              ++it )
        {
            double posx = (*it)->pos().x;
//             if ( ( (*it)->velCount() <= 3 && (*it)->vel().x > 0.0 )
//                  || M_ball.vel().x > 1.0 )
//             {
//                 posx += speed_rate * std::min( 10, (*it)->posCount() );
//             }
//             else if ( ( (*it)->velCount() <= 3 && (*it)->vel().x < 0.0 )
//                       || M_ball.vel().x < 1.0 )
//             {
//                 posx -= speed_rate * std::min( 10, (*it)->posCount() );
//             }

            if ( posx < second )
            {
                second = posx;
                if ( second < first )
                {
                    std::swap( first, second );
                }
            }
        }
    }

    //////////////////////////////////////////////////////////////////
    double new_line = second;
    // assume that our goalie exist in ourr penalty area
    if ( first > ( -ServerParam::i().pitchHalfLength()
                   + ServerParam::i().penaltyAreaLength() ) )
    {
        new_line = first;
    }

    // consider old line
    if ( M_teammates_from_self.size() >= 10 )
    {
        // new_line is used
    }
    else if ( new_line > M_defense_line_x + 13.0 )
    {
        // new_line is used
    }
    else if ( new_line > M_defense_line_x + 5.0 )
    {
        new_line = M_defense_line_x + 1.0;
    }

    // ball is more forward than defender line
    if ( ball().posValid() && ball().pos().x < new_line )
    {
        new_line = ball().pos().x;
    }

    if ( M_audio_memory->defenseLineTime() == this->time()
         && ! M_audio_memory->defenseLine().empty()
         //&& M_audio_memory->defenseLine().front().x_ + 1.0 < new_line
         )
    {
        double heard_x = 0.0;
        for ( std::vector< AudioMemory::DefenseLine >::const_iterator it
                  = M_audio_memory->defenseLine().begin();
              it != M_audio_memory->defenseLine().end();
              ++it )
        {
            heard_x += it->x_;
        }
        heard_x /= static_cast< double >( M_audio_memory->defenseLine().size() );

        if ( heard_x + 1.0 < new_line )
        {
            dlog.addText( Logger::WORLD,
                          __FILE__": heard defense line is used. %.1f -> %.1f",
                          new_line, heard_x );

            new_line = heard_x;
        }
    }

    M_defense_line_x = new_line;

    dlog.addText( Logger::WORLD,
                  __FILE__": DefenseLine = %.2f",
                  new_line );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
WorldModel::checkGhost( const ViewArea & varea )
{
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //  NOTE: this method is called from updateAfterSee

    //////////////////////////////////////////////////////////////////
    // ball
    dlog.addText( Logger::WORLD,
                  "world.checkGhost. ball_conf= %d, rpos_conf= %d",
                  ball().posCount(), ball().rposCount() );
    if ( ball().posCount() > 0
         && ball().posValid() )
    {
        const double BALL_VIS_DIST2
            = square( ServerParam::i().visibleDistance()
                      - ( self().vel().r() / self().playerType().playerDecay() ) * 0.1
                      - ( ball().vel().r() / ServerParam::i().ballDecay() ) * 0.05
                      - 0.25 );

        Vector2D ballrel = ball().pos() - varea.origin_;
        dlog.addText( Logger::WORLD,
                      "world.checkGhost. ball. global_dist= %.2f."
                      "  visdist= %.2f.  ",
                      ballrel.r(), std::sqrt( BALL_VIS_DIST2 ) );
        // dir threshold is 5.0 // old version 8.0
        if ( varea.contains( ball().pos(), 5.0, BALL_VIS_DIST2 ) )
        {
            dlog.addText( Logger::WORLD,
                          "world.checkGhost. forget ball." );
            M_ball.setGhost( this->time() );
        }
    }

    const double VIS_DIST2
            = square( ServerParam::i().visibleDistance()
                      - ( self().vel().r() / self().playerType().playerDecay() ) * 0.1
                      - 0.25 );
    //////////////////////////////////////////////////////////////////
    // players

    {
        PlayerCont::iterator it = M_teammates.begin();
        while ( it != M_teammates.end() )
        {
            if ( it->posCount() > 0
                 && varea.contains( it->pos(), 5.0, VIS_DIST2 ) )
            {
                if ( it->unum() == Unum_Unknown
                     && it->posCount() >= 10
                     && it->ghostCount() >= 2 )
                {
                    dlog.addText( Logger::WORLD,
                                  __FILE__": checkGhost. erase teammate (%.1f %.1f)",
                                  it->pos().x, it->pos().y );

                    it = M_teammates.erase( it );
                    continue;
                }

                dlog.addText( Logger::WORLD,
                              __FILE__": checkGhost. setGhost to teammate %d (%.1f %.1f).",
                              it->unum(), it->pos().x, it->pos().y );
                it->setGhost();
            }

            ++it;
        }
    }

    {
        PlayerCont::iterator it = M_opponents.begin();
        while ( it != M_opponents.end() )
        {
            if ( it->posCount() > 0
                 && varea.contains( it->pos(), 5.0, VIS_DIST2 ) )
            {
                if ( it->posCount() >= 10
                     && it->ghostCount() >= 2 )
                {
                    dlog.addText( Logger::WORLD,
                                  __FILE__": checkGhost. erase opponent (%.1f %.1f)",
                                  it->pos().x, it->pos().y );

                    it = M_opponents.erase( it );
                    continue;
                }

                dlog.addText( Logger::WORLD,
                              __FILE__": checkGhost. setGhost to opponent %d (%.1f %.1f).",
                              it->unum(), it->pos().x, it->pos().y );
                it->setGhost();
            }

            ++it;
        }
    }

    {
        PlayerCont::iterator it = M_unknown_players.begin();
        while ( it != M_unknown_players.end() )
        {
            if ( it->posCount() > 0
                 && varea.contains( it->pos(), 5.0, VIS_DIST2 ) )
            {
                if ( it->distFromSelf() < 40.0 * 1.06
                     || it->isGhost() ) // detect twice
                {
                    dlog.addText( Logger::WORLD,
                                  __FILE__": checkGhost. erase unknown player (%.1f %.1f)",
                                  it->pos().x, it->pos().y );
                    it = M_unknown_players.erase( it );
                    continue;
                }

                dlog.addText( Logger::WORLD,
                              __FILE__": checkGhost. setGhost to unknown player (%.1f %.1f)",
                              it->pos().x, it->pos().y );
                it->setGhost();
            }

            ++it;
        }
    }

}

/*-------------------------------------------------------------------*/
/*!

*/
void
WorldModel::updateDirCount( const ViewArea & varea )
{
    const double dir_buf// = DIR_STEP * 0.5;
        = ( ( self().lastMove().valid()
              && self().lastMove().r() > 0.5 )
            ? DIR_STEP * 0.5 + 1.0
            : DIR_STEP * 0.5 );

    const AngleDeg left_limit = varea.angle_ - varea.width_half_ + dir_buf;
    const AngleDeg right_limit = varea.angle_ + varea.width_half_ - dir_buf;

    AngleDeg left_dir = varea.angle_ - varea.width_half_;
    int idx = static_cast< int >( ( left_dir.degree() - 0.5 + 180.0 ) / DIR_STEP );

    AngleDeg dir = -180.0 + DIR_STEP * idx;

    while ( dir.isLeftOf( left_limit ) )
    {
        dir += DIR_STEP;
        idx += 1;
        if ( idx > DIR_CONF_DIVS ) idx = 0;
    }

    dlog.addText( Logger::WORLD,
                  "%s: updateDirCount: left=%.1f right=%.1f dir buf=%.3f start_dir=%.1f start_idx=%d"
                  ,__FILE__,
                  left_limit.degree(), right_limit.degree(),
                  dir_buf, dir.degree(), idx );

    while ( dir.isLeftOf( right_limit ) )
    {
        idx = static_cast< int >( ( dir.degree() - 0.5 + 180.0 ) / DIR_STEP );
        if ( idx > DIR_CONF_DIVS - 1 )
        {
            std::cerr << teamName() << " : " << self().unum()
                      << " DIR_CONF over flow  " << idx << std::endl;
            idx = DIR_CONF_DIVS - 1;
        }
        else if ( idx < 0 )
        {
            std::cerr << teamName() << " : " << self().unum()
                      << " DIR_CONF down flow  " << idx << std::endl;
            idx = 0;
        }
        //#ifdef DEBUG
#if 0
        dlog.addText( Logger::WORLD,
                      "=== update dir. index=%d : angle=%.0f",
                      idx, dir.degree() );
#endif
        M_dir_count[idx] = 0;
        dir += DIR_STEP;
    }

    //#ifdef DEBUG
#if 0
    if ( dlog.isLogFlag( Logger::WORLD ) )
    {
        double d = -180.0;
        for ( int i = 0; i < DIR_CONF_DIVS; ++i, d += DIR_STEP )
        {
            dlog.addText( Logger::WORLD,
                          "__ dir count: %.0f - %d",
                          d, M_dir_count[i] );
        }
    }
#endif
}

/*-------------------------------------------------------------------*/
/*!

*/
int
WorldModel::dirRangeCount( const AngleDeg & angle,
                           const double & width,
                           int * max_count,
                           int * sum_count,
                           int * ave_count ) const
{
    if ( width <= 0.0 || 360.0 < width )
    {
        std::cerr << M_time << " " << self().unum() << ":"
                  << " invalid dir range"
                  << std::endl;
        return 1000;
    }

    int counter = 0;
    int tmp_sum_count = 0;
    int tmp_max_count = 0;

    AngleDeg tmp_angle = angle;
    if ( width > DIR_STEP ) tmp_angle -= width * 0.5;

    double add_dir = 0.0;
    while ( add_dir < width )
    {
        int c = dirCount( tmp_angle );

        tmp_sum_count += c;

        if ( c > tmp_max_count )
        {
            tmp_max_count = c;
        }

        add_dir += DIR_STEP;
        tmp_angle += DIR_STEP;
        ++counter;
    }

    if ( max_count )
    {
        *max_count = tmp_max_count;
    }

    if ( sum_count )
    {
        *sum_count = tmp_sum_count;
    }

    if ( ave_count )
    {
        *ave_count = tmp_sum_count / counter;
    }

    return counter;
}

/*-------------------------------------------------------------------*/
/*!

*/
AbstractPlayerCont
WorldModel::getPlayerCont( const PlayerPredicate * predicate ) const
{
    AbstractPlayerCont rval;

    if ( ! predicate ) return rval;

    const AbstractPlayerCont::const_iterator end = allPlayers().end();
    for( AbstractPlayerCont::const_iterator it = allPlayers().begin();
         it != end;
         ++it )
    {
        if ( (*predicate)( **it ) )
        {
            rval.push_back( *it );
        }
    }

    delete predicate;
    return rval;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
WorldModel::getPlayerCont( AbstractPlayerCont & cont,
                           const PlayerPredicate * predicate ) const
{
    if ( ! predicate ) return;

    const AbstractPlayerCont::const_iterator end = allPlayers().end();
    for( AbstractPlayerCont::const_iterator it = allPlayers().begin();
         it != end;
         ++it )
    {
        if ( (*predicate)( **it ) )
        {
            cont.push_back( *it );
        }
    }

    delete predicate;
}

/*-------------------------------------------------------------------*/
/*!

*/
const
PlayerObject *
WorldModel::getOpponentGoalie() const
{
    const PlayerCont::const_iterator end = M_opponents.end();
    for ( PlayerCont::const_iterator it = M_opponents.begin();
          it != end;
          ++it )
    {
        if ( it->goalie() )
        {
            return &(*it);
        }
    }

    // return &M_dummy_opponent;
    return static_cast< PlayerObject * >( 0 );
}

/*-------------------------------------------------------------------*/
/*!

*/
const
PlayerObject *
WorldModel::getTeammateNearestTo( const Vector2D & point,
                                  const int count_thr,
                                  double * dist_to_point ) const
{
    const PlayerObject * p = static_cast< PlayerObject * >( 0 );
    double min_dist2 = 40000.0;

    const PlayerPtrCont::const_iterator end = M_teammates_from_self.end();
    for ( PlayerPtrCont::const_iterator it = M_teammates_from_self.begin();
          it != end;
          ++it )
    {
        if ( (*it)->posCount() > count_thr )
        {
            continue;
        }
        double tmp = (*it)->pos().dist2(point);
        if ( tmp < min_dist2 )
        {
            p = *it;
            min_dist2 = tmp;
        }
    }

    if ( dist_to_point )
    {
        *dist_to_point = std::sqrt( min_dist2 );
    }
    return p;
}

/*-------------------------------------------------------------------*/
/*!

*/
const
PlayerObject *
WorldModel::getOpponentNearestTo( const Vector2D & point,
                                  const int count_thr,
                                  double * dist_to_point ) const
{
    const PlayerObject * p = static_cast< PlayerObject * >( 0 );
    double min_dist2 = 40000.0;

    const PlayerPtrCont::const_iterator end = M_opponents_from_self.end();
    for ( PlayerPtrCont::const_iterator it = M_opponents_from_self.begin();
          it != end;
          ++it )
    {
        if ( (*it)->posCount() > count_thr )
        {
            continue;
        }
        double tmp = (*it)->pos().dist2( point );
        if ( tmp < min_dist2 )
        {
            p = *it;
            min_dist2 = tmp;
        }
    }

    if ( dist_to_point )
    {
        *dist_to_point = std::sqrt( min_dist2 );
    }
    return p;
}

}
