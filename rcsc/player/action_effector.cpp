// -*-c++-*-

/*!
  \file action_effector.cpp
  \brief Effector Source File
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

#include "action_effector.h"

#include "player_agent.h"
#include "world_model.h"
#include "body_sensor.h"
#include "player_command.h"
#include "say_message_builder.h"

#include <rcsc/math_util.h>
#include <rcsc/common/server_param.h>
#include <rcsc/common/logger.h>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
ActionEffector::ActionEffector( const PlayerAgent & agent )
    : M_agent( agent )
    , M_command_body( static_cast< PlayerBodyCommand * >( 0 ) )
    , M_command_turn_neck( static_cast< PlayerTurnNeckCommand * >( 0 ) )
    , M_command_change_view( static_cast< PlayerChangeViewCommand * >( 0 ) )
    , M_command_say( static_cast< PlayerSayCommand * >( 0 ) )
    , M_command_pointto( static_cast< PlayerPointtoCommand * >( 0 ) )
    , M_command_attentionto( static_cast< PlayerAttentiontoCommand * >( 0 ) )
    , M_last_action_time( 0, 0 )
    , M_last_body_command_type( PlayerCommand::ILLEGAL )
    , M_done_turn_neck( false )
    , M_kick_accel( 0.0, 0.0 )
    , M_kick_accel_error( 0.0, 0.0 )
    , M_turn_actual( 0.0 )
    , M_turn_error( 0.0 )
    , M_dash_accel( 0.0, 0.0 )
    //, M_dash_accel_error(0.0, 0.0)
    , M_dash_power( 0.0 )
    , M_move_pos( 0.0, 0.0 )
    , M_catch_time( 0, 0 )
    , M_tackle_power( 0.0 )
    , M_tackle_dir( 0.0 )
    , M_turn_neck_moment( 0.0 )
    , M_say_message( "" )
    , M_pointto_pos( 0.0, 0.0 )
{
    for ( int i = PlayerCommand::INIT;
          i <= PlayerCommand::ILLEGAL;
          ++i )
    {
        M_command_counter[i] = 0;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
ActionEffector::~ActionEffector()
{
    if ( M_command_body )
    {
        delete M_command_body;
        M_command_body = static_cast< PlayerBodyCommand * >( 0 );
    }

    if ( M_command_turn_neck )
    {
        delete M_command_turn_neck;
        M_command_turn_neck = static_cast< PlayerTurnNeckCommand * >( 0 );
    }

    if ( M_command_change_view )
    {
        delete M_command_change_view;
        M_command_change_view = static_cast< PlayerChangeViewCommand * >( 0 );
    }

    if ( M_command_say )
    {
        delete M_command_say;
        M_command_say = static_cast< PlayerSayCommand * >( 0 );;
    }

    if ( M_command_pointto )
    {
        delete M_command_pointto;
        M_command_pointto = static_cast< PlayerPointtoCommand * >( 0 );;
    }

    if ( M_command_attentionto )
    {
        delete M_command_attentionto;
        M_command_attentionto = static_cast< PlayerAttentiontoCommand * >( 0 );;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionEffector::reset()
{
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //  CAUTION: this function must be called after WorldModel::update()
    //             and before action decision.
    //  Do NOT update change_viwe info for adjustment of see arrival timing

    M_last_body_command_type = PlayerCommand::ILLEGAL;
    M_done_turn_neck = false;
    M_say_message = "";

    // it is not necesarry to reset these value,
    // because value is selected by last command type specifier in updator function.

    //M_kick_accel.assign(0.0, 0.0);
    //M_kick_accel_error.assign(0.0, 0.0);
    //M_turn_actual =  M_turn_error = 0.0;
    //M_dash_accel.assign(0.0, 0.0);
    //M_dash_accel_error.assign(0.0, 0.0);
    //M_dash_power = 0.0;
    //M_move_pos.assign(0.0, 0.0);
    //M_catch_time
    //M_tackle_power = 0.0;
    //M_tackle_dir = 0.0;
    //M_turnneck_moment = 0.0;
    //M_pointto_pos.assign(0.0, 0.0);
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionEffector::incCommandCount( const PlayerCommand::Type type )
{
    if ( type < PlayerCommand::INIT
         || PlayerCommand::ILLEGAL <= type )
    {
        std::cerr << "ActionEffector::incCommandCount()"
                  << "  illegal command type ID "
                  << type
                  << std::endl;
        return;
    }

    M_command_counter[type] += 1;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionEffector::checkCommandCount( const BodySensor & sense )
{
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //  CAUTION: this function must be called after catching sense_body.

    // if action count does not match the internal model,
    //  reset that action effect.

    if ( sense.kickCount() != M_command_counter[PlayerCommand::KICK] )
    {
        std::cout << M_agent.world().self().unum()
                  << ": lost kick? at " << M_last_action_time
                  << " sense=" << sense.kickCount()
                  << " internal=" << M_command_counter[PlayerCommand::KICK]
                  << std::endl;
        dlog.addText( Logger::SYSTEM,
                      "lost kick? sense= %d internal= %d",
                      sense.kickCount(),
                      M_command_counter[PlayerCommand::KICK] );
        M_last_body_command_type = PlayerCommand::ILLEGAL;
        M_kick_accel.assign( 0.0, 0.0 );
        M_kick_accel_error.assign( 0.0, 0.0 );
        M_command_counter[PlayerCommand::KICK] = sense.kickCount();
    }

    if ( sense.turnCount() != M_command_counter[PlayerCommand::TURN] )
    {
        std::cout << M_agent.world().self().unum()
                  << ": lost turn? at " << M_last_action_time
                  << " sense=" << sense.turnCount()
                  << " internal=" << M_command_counter[PlayerCommand::TURN]
                  << std::endl;
        dlog.addText( Logger::SYSTEM,
                      "lost turn? sense= %d internal= %d",
                      sense.turnCount(),
                      M_command_counter[PlayerCommand::TURN] );
        M_last_body_command_type = PlayerCommand::ILLEGAL;
        M_turn_actual = 0.0;
        M_turn_error = 0.0;
        M_command_counter[PlayerCommand::TURN] = sense.turnCount();
    }

    if ( sense.dashCount() != M_command_counter[PlayerCommand::DASH] )
    {
        std::cout << M_agent.world().self().unum()
                  << ": lost dash? at " << M_last_action_time
                  << " sense=" << sense.dashCount()
                  << " internal=" << M_command_counter[PlayerCommand::DASH]
                  << std::endl;
        dlog.addText( Logger::SYSTEM,
                      "lost dash? sense= %d internal= %d",
                      sense.dashCount(),
                      M_command_counter[PlayerCommand::DASH] );
        M_last_body_command_type = PlayerCommand::ILLEGAL;
        M_dash_accel.assign( 0.0, 0.0 );
        //M_dash_accel_error.assign( 0.0, 0.0 );
        M_dash_power = 0.0;
        M_command_counter[PlayerCommand::DASH] = sense.dashCount();
    }

    if ( sense.moveCount() != M_command_counter[PlayerCommand::MOVE] )
    {
        std::cout << M_agent.world().self().unum()
                  << ": lost move? at " << M_last_action_time
                  << " sense=" << sense.moveCount()
                  << " internal=" << M_command_counter[PlayerCommand::MOVE]
                  << std::endl;
        dlog.addText( Logger::SYSTEM,
                      "lost move? sense= %d internal= %d",
                      sense.moveCount(),
                      M_command_counter[PlayerCommand::MOVE] );
        M_last_body_command_type = PlayerCommand::ILLEGAL;
        M_move_pos.invalidate();
        M_command_counter[PlayerCommand::MOVE] = sense.moveCount();
    }

    if ( sense.catchCount() != M_command_counter[PlayerCommand::CATCH] )
    {
        std::cout << M_agent.world().self().unum()
                  << ": lost catch? at " << M_last_action_time
                  << " sense=" << sense.catchCount()
                  << " internal=" << M_command_counter[PlayerCommand::CATCH]
                  << std::endl;
        dlog.addText( Logger::SYSTEM,
                      "lost catch? sense= %d internal= %d",
                      sense.catchCount(),
                      M_command_counter[PlayerCommand::CATCH] );
        M_last_body_command_type = PlayerCommand::ILLEGAL;
        M_catch_time.assign( 0, 0 );
        M_command_counter[PlayerCommand::CATCH] = sense.catchCount();
    }

    if ( sense.tackleCount() != M_command_counter[PlayerCommand::TACKLE] )
    {
        std::cout << M_agent.world().self().unum()
                  << ": lost tackle? at " << M_last_action_time
                  << " sense=" << sense.tackleCount()
                  << " internal=" << M_command_counter[PlayerCommand::TACKLE]
                  << std::endl;
        dlog.addText( Logger::SYSTEM,
                      "lost tackle? sense= %d internal= %d",
                      sense.tackleCount(),
                      M_command_counter[PlayerCommand::TACKLE] );
        M_last_body_command_type = PlayerCommand::ILLEGAL;
        M_tackle_power = 0.0;
        M_tackle_dir = 0.0;
        M_command_counter[PlayerCommand::TACKLE] = sense.tackleCount();
    }

    if ( sense.turnNeckCount() != M_command_counter[PlayerCommand::TURN_NECK] )
    {
        std::cout << M_agent.world().self().unum()
                  << ": lost turn_neck? at " << M_last_action_time
                  << " sense=" << sense.turnNeckCount()
                  << " internal=" << M_command_counter[PlayerCommand::TURN_NECK]
                  << std::endl;
        dlog.addText( Logger::SYSTEM,
                      "lost turn_neck? sense= %d internal= %d",
                      sense.turnNeckCount(),
                      M_command_counter[PlayerCommand::TURN_NECK] );
        M_done_turn_neck = false;
        M_turn_neck_moment = 0.0;
        M_command_counter[PlayerCommand::TURN_NECK] = sense.turnNeckCount();
    }

    if ( sense.changeViewCount() != M_command_counter[PlayerCommand::CHANGE_VIEW] )
    {
        std::cout << M_agent.world().self().unum()
                  << ": lost change_view? at " << M_last_action_time
                  << " sense=" << sense.changeViewCount()
                  << " internal=" << M_command_counter[PlayerCommand::CHANGE_VIEW]
                  << std::endl;
        dlog.addText( Logger::SYSTEM,
                      "lost change_view? sense= %d internal= %d",
                      sense.changeViewCount(),
                      M_command_counter[PlayerCommand::CHANGE_VIEW] );
        M_command_counter[PlayerCommand::CHANGE_VIEW] = sense.changeViewCount();
    }

    if ( sense.sayCount() != M_command_counter[PlayerCommand::SAY] )
    {
        std::cout << M_agent.world().self().unum()
                  << ": lost say? at " << M_last_action_time
                  << " sense=" << sense.sayCount()
                  << " internal=" << M_command_counter[PlayerCommand::SAY]
                  << std::endl;
        dlog.addText( Logger::SYSTEM,
                      "lost say? sense= %d internal= %d",
                      sense.sayCount(),
                      M_command_counter[PlayerCommand::SAY] );
        M_command_counter[PlayerCommand::SAY] = sense.sayCount();
    }

    if ( sense.pointtoCount() != M_command_counter[PlayerCommand::POINTTO] )
    {
        std::cout << M_agent.world().self().unum()
                  << ": lost pointto? at " << M_last_action_time
                  << " sense=" << sense.pointtoCount()
                  << " internal=" << M_command_counter[PlayerCommand::POINTTO]
                  << std::endl;
        dlog.addText( Logger::SYSTEM,
                      "lost pointto? sense= %d internal= %d",
                      sense.pointtoCount(),
                      M_command_counter[PlayerCommand::POINTTO] );
        M_command_counter[PlayerCommand::POINTTO] = sense.pointtoCount();
    }

    if ( sense.attentiontoCount() != M_command_counter[PlayerCommand::ATTENTIONTO] )
    {
        std::cout << M_agent.world().self().unum()
                  << ": lost attentionto? at " << M_last_action_time
                  << " sense=" << sense.attentiontoCount()
                  << " internal=" << M_command_counter[PlayerCommand::ATTENTIONTO]
                  << std::endl;
        dlog.addText( Logger::SYSTEM,
                      "lost attentionto? sense= %d internal= %d",
                      sense.attentiontoCount(),
                      M_command_counter[PlayerCommand::ATTENTIONTO] );
        M_command_counter[PlayerCommand::ATTENTIONTO] = sense.attentiontoCount();
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
ActionEffector::makeCommand( std::ostream & to )
{
    M_last_action_time = M_agent.world().time();

    if ( M_command_body )
    {
        M_last_body_command_type = M_command_body->type();
        if ( M_last_body_command_type == PlayerCommand::CATCH )
        {
            M_catch_time = M_agent.world().time();
        }
        M_command_body->toStr( to );
        incCommandCount( M_command_body->type() );
        delete M_command_body;
        M_command_body = static_cast< PlayerBodyCommand * >( 0 );
    }
    else
    {
        if ( ! M_agent.world().self().isFreezed() )
        {
            dlog.addText( Logger::SYSTEM,
                          "  WARNING. no body command." );
            std::cerr << M_agent.world().teamName() << ' '
                      << M_agent.world().self().unum()<< ": "
                      << M_agent.world().time()
                      << "  WARNING. no body command." << std::endl;
            // register dummy command
            PlayerTurnCommand turn( 0 );
            turn.toStr( to );
            incCommandCount( PlayerCommand::TURN );
        }
    }

    if ( M_command_turn_neck )
    {
        M_done_turn_neck = true;
        M_command_turn_neck->toStr( to );
        incCommandCount( PlayerCommand::TURN_NECK );
        delete M_command_turn_neck;
        M_command_turn_neck = static_cast< PlayerTurnNeckCommand * >( 0 );
    }

    if ( M_command_change_view )
    {
        M_command_change_view->toStr( to );
        incCommandCount( PlayerCommand::CHANGE_VIEW );
        delete M_command_change_view;
        M_command_change_view = static_cast< PlayerChangeViewCommand * >( 0 );
    }

    if ( M_command_pointto )
    {
        M_command_pointto->toStr( to );
        incCommandCount( PlayerCommand::POINTTO );
        delete M_command_pointto;
        M_command_pointto = static_cast< PlayerPointtoCommand * >( 0 );
    }

    if ( M_command_attentionto )
    {
        M_command_attentionto->toStr( to );
        incCommandCount( PlayerCommand::ATTENTIONTO );
        delete M_command_attentionto;
        M_command_attentionto = static_cast< PlayerAttentiontoCommand * >( 0 );
    }

    if ( ServerParam::i().synchMode() )
    {
        PlayerDoneCommand done_com;
        done_com.toStr( to );
    }

    makeSayCommand();
    if ( M_command_say )
    {
        M_command_say->toStr( to );
        incCommandCount( PlayerCommand::SAY );
        delete M_command_say;
        M_command_say = static_cast< PlayerSayCommand * >( 0 );
    }
    clearSayMessages();

    return to;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionEffector::setKick( const double & power,
                         const AngleDeg & rel_dir )
{
    dlog.addText( Logger::ACTION,
                  "register kick. power= %.1f, rel_dir= %.1f",
                  power, rel_dir.degree() );

    double command_power = power;
    if ( command_power > ServerParam::i().maxPower() + 0.01 )
    {
        std::cerr << M_agent.world().teamName() << ' '
                  << M_agent.world().self().unum() << ": "
                  << M_agent.world().time()
                  << " kick power is over max. com=" << command_power
                  << " > sparam=" << ServerParam::i().maxPower() << std::endl;
        dlog.addText( Logger::ACTION,
                      "setKick. power over. %.10f",
                      command_power);
        command_power = ServerParam::i().maxPower();
    }

    if ( command_power < 0.0 )
    {
        std::cerr << M_agent.world().teamName() << ' '
                  << M_agent.world().self().unum() << ": "
                  << M_agent.world().time()
                  << " kick power is negative " << command_power
                  << std::endl;
        dlog.addText( Logger::ACTION,
                      "Effector::setKick. power is negative. %.1f",
                      command_power );
        command_power = 0.0;
    }

    command_power = rint( command_power );

    dlog.addText( Logger::ACTION,
                  "Effector::setKick. Power= %.1f  Dir= %.1f  KickRate= %.4f  Accel= %.2f",
                  command_power, rel_dir.degree(), M_agent.world().self().kickRate(),
                  M_agent.world().self().kickRate() * command_power );

    //////////////////////////////////////////////////
    // create command object
    if ( M_command_body )
    {
        delete M_command_body;
        M_command_body = static_cast< PlayerBodyCommand * >( 0 );
    }
    M_command_body = new PlayerKickCommand( command_power, rel_dir.degree() );

    // set estimated action effect
    M_kick_accel.setPolar( command_power * M_agent.world().self().kickRate(),
                           M_agent.world().self().body() + rel_dir );

    // rcssserver/src/player.cc
    //  add noise to kick
    //  Value maxrnd = kick_rand * power / ServerParam::instance().maxp;
    //  PVector kick_noise(drand(-maxrnd,maxrnd),drand(-maxrnd,maxrnd));
    //  ball->push(kick_noise);

    // double mincos, maxcos, minsin, maxsin;
    // get_con_min_max(body_dir, body_err, &mincos, &maxcos);
    // get_sin_min_max(body_dir, body_err, &minsin, &maxsin);
    // M_kick_accel_error.assign((maxcos - mincos) * 0.5, (maxsin - minsin) * 0.5);
    // M_kick_accel_error *= M_kick_accel.r();
    // M_kick_accel_error.add(maxrnd, maxrnd);

    double maxrnd = ( M_agent.world().self().playerType().kickRand()
                      * command_power
                      / ServerParam::i().maxPower() );
    M_kick_accel_error.assign( maxrnd, maxrnd );

    dlog.addText( Logger::SYSTEM,
                  "effector.setKick. accel=(%f, %f) err=(%f, %f)",
                  M_kick_accel.x, M_kick_accel.y,
                  M_kick_accel_error.x, M_kick_accel_error.y );
}


namespace {
/*-------------------------------------------------------------------*/
/*!
  conserve dash power
  used only from thie file
*/
double
conserve_dash_power( const WorldModel & world,
                     double power )
{
    const bool back_dash = ( power < 0.0 ? true : false );
    const double required_stamina = ( back_dash
                                      ? power * -2.0
                                      : power );
    if ( required_stamina < 0.0 )
    {
        std::cerr << world.teamName() << ' '
                  << world.self().unum() << ": "
                  << world.time()
                  << " dash power should be positive now"
                  << std::endl;
        dlog.addText( Logger::ACTION,
                      " dash power should be positive now" );
    }

    const double available_stamina
        = world.self().stamina()
        + world.self().playerType().extraStamina();
    // insufficient stamina
    if ( available_stamina < required_stamina )
    {
        dlog.addText( Logger::ACTION,
                      " conserve_dash_power: no stamina. power = %.1f. stamina = %.1f",
                      power, available_stamina );
        power = available_stamina;
        if ( back_dash )
        {
            power *= -0.5;
        }
    }

    const double dash_rate = world.self().dashRate();

    // if can keep max speed without max power,
    // conserve dash power
    double accel_radius = std::fabs( power ) * dash_rate;
    AngleDeg accel_angle = world.self().body();
    if ( back_dash )
    {
        accel_angle += 180.0;
    }

    // reduce waste accel radius
    world.self().playerType().normalizeAccel( world.self().vel(),
                                              accel_angle, &accel_radius );

    power = rint( accel_radius / dash_rate );
    if ( back_dash )
    {
        power *= -1.0;
    }
    dlog.addText( Logger::ACTION,
                  " conserved power = %.1f",
                  power );
    power = ServerParam::i().normalizePower( power );

    return power;
}

}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionEffector::setDash( const double & power )
{
    dlog.addText( Logger::ACTION,
                  "register dash. power= %.1f",
                  power );

    double command_power = power;
    if ( command_power > ServerParam::i().maxPower() + 0.01 )
    {
        dlog.addText( Logger::ACTION,
                      "dash power is too high" );
        std::cerr << M_agent.world().teamName() << ' '
                  << M_agent.world().self().unum() << ": "
                  << M_agent.world().time()
                  << " dash power is too high: " << power << std::endl;
        command_power = ServerParam::i().maxPower();
    }
    if ( command_power < ServerParam::i().minPower() - 0.01 )
    {
        dlog.addText( Logger::ACTION,
                      "dash power is too low" );
        std::cerr << M_agent.world().teamName() << ' '
                  << M_agent.world().self().unum() << ": "
                  << M_agent.world().time()
                  << " dash power is too low;: " << power << std::endl;
        command_power = ServerParam::i().minPower();
    }

    command_power = conserve_dash_power( M_agent.world(), command_power );

    command_power = rint( command_power );

    //////////////////////////////////////////////////
    // create command object
    if ( M_command_body )
    {
        delete M_command_body;
        M_command_body = static_cast< PlayerBodyCommand * >( 0 );
    }
    M_command_body = new PlayerDashCommand( command_power );

    // set estimated command effect
    const double accel_mag
        = ( M_agent.world().self().effort()
            * command_power
            * M_agent.world().self().playerType().dashPowerRate() );
    /*
     * accel_mag = std::min( effort * command_power * mydprate,
     *                       ServerParam::playerAccelMax() );
     */

    M_dash_power = command_power;
    M_dash_accel.setPolar( accel_mag,
                           M_agent.world().self().body() );

#if 0
    double mincos, maxcos, minsin, maxsin;
    M_agent.world().self().body().getCosMinMax( M_agent.world().self().faceError(), &mincos, &maxcos );
    M_agent.world().self().body().getSinMinMax( M_agent.world().self().faceError(), &minsin, &maxsin );

    // set only dir info, radius is 1.0
    M_dash_accel_error.assign( ( maxcos - mincos ) * 0.5,
                               ( maxsin - minsin ) * 0.5 );
    // set radius
    M_dash_accel_error *= accel_mag;

    // rcssserver/src/object.C
    // PVector MPObject::noise()
    //  {
    //    Value maxrnd = randp * vel.r() ;
    //    return PVector(drand(-maxrnd,maxrnd),drand(-maxrnd,maxrnd)) ;
    //  }
    //
    // vel += noise();
    //

    // player_rand : default value is 0.05
    M_dash_accel_error.add( accel_mag * ServerParam::i().playerRand(),
                            accel_mag * ServerParam::i().playerRand() );
#endif
    dlog.addText( Logger::SYSTEM,
                  "effector.setDash. accel=(%f, %f)",
                  M_dash_accel.x, M_dash_accel.y );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionEffector::setTurn( const AngleDeg & moment )
{
    dlog.addText( Logger::ACTION,
                  "register turn. moment = %.1f",
                  moment.degree() );

    const double my_speed = M_agent.world().self().vel().r();
    double command_moment = moment.degree();
    // required turn param
    command_moment
        *= ( 1.0
             + ( my_speed
                 * M_agent.world().self().playerType().inertiaMoment() ) );

    dlog.addText( Logger::ACTION,
                  "register turn. moment= %.1f, cmd_param= %.1f, my_inertia= %.1f",
                  moment.degree(), command_moment,
                  M_agent.world().self().playerType().inertiaMoment() );

    // check parameter range
    if ( command_moment > ServerParam::i().maxMoment() )
    {
        dlog.addText( Logger::ACTION,
                      "Effector::setTurn. over max moment. moment= %.1f, command= %.1f",
                      moment.degree(), command_moment );
        command_moment = ServerParam::i().maxMoment();
    }
    if ( command_moment < ServerParam::i().minMoment() )
    {
        dlog.addText( Logger::ACTION,
                      "Effector::setTurn. under min moment. moment= %.1f, command= %.1f",
                      moment.degree(), command_moment );
        command_moment = ServerParam::i().minMoment();
    }

    command_moment = rint( command_moment );

    //////////////////////////////////////////////////
    // create command object
    if ( M_command_body )
    {
        delete M_command_body;
        M_command_body = static_cast< PlayerBodyCommand * >( 0 );
    }

    // moment is a command param, not a real moment.
    M_command_body = new PlayerTurnCommand( command_moment );

    // set estimated action effect
    /*
      turn noise algorithm
      drand(h,l) returns the random value within [h,l].

      player_rand : default value is 0.05

      double r1 = vel.mod();
      double actual_moment
      = ((1.0 + drand(-player_rand, player_rand)) * moment/(1.0 + HP_inertia_moment * r1));
      angle_body = normalize_angle(angle_body + moment);
    */
    // convert to real moment
    M_turn_actual = command_moment
        / ( 1.0 + my_speed * M_agent.world().self().playerType().inertiaMoment() );
    M_turn_error = std::fabs( ServerParam::i().playerRand()
                              * M_turn_actual );

    dlog.addText( Logger::SYSTEM,
                  "effector.setTurn. command_moment= %.2f. actual_turn= %.2f. error= %.2f",
                  command_moment, M_turn_actual, M_turn_error );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionEffector::setMove( const double & x,
                         const double & y )

{
    dlog.addText( Logger::ACTION,
                  "register move. (%.1f, %.1f)",
                  x, y );

    double command_x = x;
    double command_y = y;

    // check move point.
    // move point must be in pitch.
    if ( std::fabs( command_y ) > ServerParam::i().pitchHalfWidth()
         || std::fabs( command_x ) > ServerParam::i().pitchHalfLength() )
    {
        std::cerr << M_agent.world().teamName() << ' '
                  << M_agent.world().self().unum() << ": "
                  << M_agent.world().time()
                  << " Must move to a place in the pitch ("
                  << command_x << ", " << command_y << ")" << std::endl;
        dlog.addText( Logger::ACTION,
                      "Effector::setMove. (%.1f, %.1f). must move to a place in pitch",
                      command_x, command_y );
        command_x = min_max( - ServerParam::i().pitchHalfLength(),
                             command_x,
                             ServerParam::i().pitchHalfLength() );
        command_y = min_max( - ServerParam::i().pitchHalfWidth(),
                             command_y,
                             ServerParam::i().pitchHalfWidth() );
    }

    // when kickoff & kickoff offside is enabled,
    // move point must be in our side
    if ( ServerParam::i().kickoffOffside()
         && command_x > 0.0 )
    {
        std::cerr << M_agent.world().teamName() << ' '
                  << M_agent.world().self().unum() << ": "
                  << M_agent.world().time()
                  << " Must move to a place in our half ("
                  << command_x << ", " << command_y << ")" << std::endl;
        dlog.addText( Logger::ACTION,
                      "Effector::setMove. (%.1f, %.1f). must move to a place in our half",
                      command_x, command_y );
        command_x = -0.1;
    }

    // when goalie catch mode,
    // move point must be in our penalty area
    if ( M_agent.world().gameMode().type() == GameMode::GoalieCatch_
         && M_agent.world().gameMode().side() == M_agent.world().ourSide() )
    {
        if ( command_x < -ServerParam::i().pitchHalfLength() + 1.0 )
        {
            std::cerr << M_agent.world().teamName() << ' '
                      << M_agent.world().self().unum() << ": "
                      << M_agent.world().time()
                      << " Must move to a place within penalty area(1) ("
                      << command_x << ", " << command_y << ")" << std::endl;
            dlog.addText( Logger::ACTION,
                          "Effector::setMove. (%.1f, %.1f). must move to a place in penalty area(1)",
                          command_x, command_y );
            command_x = - ServerParam::i().pitchHalfLength() + 1.0;
        }
        if ( command_x > - ServerParam::i().ourPenaltyAreaLineX() - 1.0 )
        {
            std::cerr << M_agent.world().teamName() << ' '
                      << M_agent.world().self().unum() << ": "
                      << M_agent.world().time()
                      << " Must move to a place within penalty area(2) ("
                      << command_x << ", " << command_y << ")"<< std::endl;
            dlog.addText( Logger::ACTION,
                          "Effector::setMove. (%.1f, %.1f). must move to a place in penalty area(2)",
                          command_x, command_y );
            command_x = - ServerParam::i().ourPenaltyAreaLineX() - 1.0;
        }
        if ( command_y > ServerParam::i().penaltyAreaHalfWidth() - 1.0 )
        {
            std::cerr << M_agent.world().teamName() << ' '
                      << M_agent.world().self().unum() << ": "
                      << M_agent.world().time()
                      << " Must move to a place within penalty area(3) ("
                      << command_x << ", " << command_y << ")"<< std::endl;
            dlog.addText( Logger::ACTION,
                          "Effector::setMove. (%.1f, %.1f). must move to a place in penalty area(3)",
                          command_x, command_y );
            command_y = ServerParam::i().penaltyAreaHalfWidth() - 1.0;
        }
        if ( command_y < - ServerParam::i().penaltyAreaHalfWidth() + 1.0 )
        {
            std::cerr << M_agent.world().teamName() << ' '
                      << M_agent.world().self().unum() << ": "
                      << M_agent.world().time()
                      << " Must move to a place within penalty area(4) ("
                      << command_x << ", " << command_y << ")"<< std::endl;
            dlog.addText( Logger::ACTION,
                          "Effector::setMove. (%.1f, %.1f). must move to a place in penalty area(4)",
                          command_x, command_y );
            command_y = - ServerParam::i().penaltyAreaHalfWidth() + 1.0;
        }
    }

    //////////////////////////////////////////////////
    // create command object
    if ( M_command_body )
    {
        delete M_command_body;
        M_command_body = static_cast< PlayerBodyCommand * >( 0 );
    }
    M_command_body = new PlayerMoveCommand( command_x, command_y );

    M_move_pos.assign( command_x, command_y );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionEffector::setCatch()
{
    dlog.addText( Logger::ACTION,
                  "register catch" );

    // catch area is rectangle.
    // "real" catchable length is the length of diagonal line.
    const double diagonal_angle
        = AngleDeg::atan2_deg( ServerParam::i().catchAreaWidth() * 0.5,
                               ServerParam::i().catchAreaLength() );

    // relative angle
    AngleDeg catch_angle = M_agent.world().ball().angleFromSelf() - M_agent.world().self().body();
    dlog.addText( Logger::ACTION,
                  "Effector::setCatch. ball_rel_angle= %.1f  diagonal_angle= %.1f",
                  catch_angle.degree(), diagonal_angle );

    // add diagonal angle
    catch_angle += diagonal_angle;

    //////////////////////////////////////////////////
    // create command object
    if ( M_command_body )
    {
        delete M_command_body;
        M_command_body = static_cast< PlayerBodyCommand * >( 0 );
    }
    M_command_body = new PlayerCatchCommand( catch_angle.degree() );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionEffector::setTackle( const double & power_or_dir )
{
    dlog.addText( Logger::ACTION,
                  "register tackle. power_or_dir=%.1f",
                  power_or_dir );

    double command_arg = power_or_dir;

    if ( M_agent.config().version() >= 12.0 )
    {
        if ( command_arg < -180.0 || 180.0 < command_arg )
        {
            std::cerr << M_agent.world().teamName() << ' '
                      << M_agent.world().self().unum() << ": "
                      << M_agent.world().time()
                      << "tackle dir over the range. dir=" << command_arg
                      << std::endl;
            dlog.addText( Logger::ACTION,
                          "Effector::setTackle. dir over. %f",
                          command_arg );
        }
        command_arg = AngleDeg::normalize_angle( power_or_dir );
    }
    else
    {
        if ( command_arg > ServerParam::i().maxTacklePower() + 0.01 )
        {
            std::cerr << M_agent.world().teamName() << ' '
                      << M_agent.world().self().unum() << ": "
                      << M_agent.world().time()
                      << "tackle power overflow. com=" << command_arg
                      << " > sparam=" << ServerParam::i().maxTacklePower()
                      << std::endl;
            dlog.addText( Logger::ACTION,
                          "Effector::setTackle. power over. %f",
                          command_arg );
            command_arg = ServerParam::i().maxTacklePower();
        }

        if ( command_arg < - ServerParam::i().maxBackTacklePower() - 0.01 )
        {
            std::cerr << M_agent.world().teamName() << ' '
                      << M_agent.world().self().unum() << ": "
                      << M_agent.world().time()
                      << "tackle power underflow " << command_arg
                      << std::endl;
            dlog.addText( Logger::ACTION,
                          "Effector::setKick. power underflow. %f",
                          command_arg );
            command_arg = ServerParam::i().minPower();
        }
    }

    command_arg = rint( command_arg );

    //////////////////////////////////////////////////
    // create command object
    if ( M_command_body )
    {
        delete M_command_body;
        M_command_body = static_cast< PlayerBodyCommand * >( 0 );
    }
    M_command_body = new PlayerTackleCommand( command_arg );

    // set estimated command effect
    M_tackle_power = command_arg;
    if ( M_agent.config().version() >= 12.0 )
    {
        M_tackle_power = ServerParam::i().maxTacklePower();
        M_tackle_dir = command_arg;
    }
    else
    {
        M_tackle_dir = ( command_arg > 0.0
                         ? M_agent.world().self().body().degree()
                         : ( M_agent.world().self().body() + 180.0 ).degree() );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionEffector::setTurnNeck( const AngleDeg & moment )
{
    dlog.addText( Logger::ACTION,
                  "register turn_neck. moment= %.1f",
                  moment.degree() );

    double command_moment = moment.degree();

    // adjust for neck moment range
    if ( command_moment > ServerParam::i().maxNeckMoment() + 0.01 )
    {
        std::cerr << M_agent.world().teamName() << ' '
                  << M_agent.world().self().unum() << ": "
                  << M_agent.world().time()
                  << "  doTurnNeck. over max moment. "
                  << command_moment << std::endl;
        dlog.addText( Logger::ACTION,
                      "Effector::setTurnNeck. over max moment. %.1f",
                      command_moment );
        command_moment = ServerParam::i().maxNeckMoment();
    }
    if ( command_moment < ServerParam::i().minNeckMoment() - 0.01 )
    {
        std::cerr << M_agent.world().teamName() << ' '
                  << M_agent.world().self().unum() << ": "
                  << M_agent.world().time()
                  << "  doTurnNeck. under min moment. "
                  << command_moment << std::endl;
        dlog.addText( Logger::ACTION,
                      "Effector::setTurnNeck. under min moment. %.1f",
                      command_moment );
        command_moment = ServerParam::i().minNeckMoment();
    }

    command_moment = rint( command_moment );

    // adjust for neck angle range
    AngleDeg next_neck_angle = M_agent.world().self().neck();
    next_neck_angle += command_moment;

    if ( next_neck_angle.degree() > ServerParam::i().maxNeckAngle() )
    {
        command_moment = rint( ServerParam::i().maxNeckAngle()
                               - M_agent.world().self().neck().degree() );
        dlog.addText( Logger::ACTION,
                      "Effector::setTurnNeck. next_neck= %.1f. over max. new-moment= %.1f",
                      next_neck_angle.degree(), command_moment );
    }

    if ( next_neck_angle.degree() < ServerParam::i().minNeckAngle() )
    {
        command_moment = rint( ServerParam::i().minNeckAngle()
                               - M_agent.world().self().neck().degree() );
        dlog.addText( Logger::ACTION,
                      "Effector::setTurnNeck. next_neck= %.1f. under min. new-momment= %.1f",
                      next_neck_angle.degree(), command_moment );
    }

    //////////////////////////////////////////////////
    // create command object
    if ( M_command_turn_neck )
    {
        delete M_command_turn_neck;
        M_command_turn_neck = static_cast< PlayerTurnNeckCommand * >( 0 );
    }
    M_command_turn_neck = new PlayerTurnNeckCommand( command_moment );

    // set estimated command effect
    M_turn_neck_moment = command_moment;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionEffector::setChangeView( const ViewWidth & width )
{
    dlog.addText( Logger::ACTION,
                  "register change_view. width= %d",
                  width.type() );

    //////////////////////////////////////////////////
    // create command object
    if ( M_command_change_view )
    {
        delete M_command_change_view;
        M_command_change_view = static_cast< PlayerChangeViewCommand * >( 0 );
    }

    M_command_change_view = new PlayerChangeViewCommand( width,
                                                         ViewQuality::HIGH,
                                                         M_agent.config().version() );
}

/*-------------------------------------------------------------------*/
/*!

*/
// void
// ActionEffector::setSay( const std::string & msg,
//                         const double & version )
// {
//     dlog.addText( Logger::ACTION,
//                   "register say. [%s]",
//                   msg.c_str() );

//     M_protocl_version = version;

//     //////////////////////////////////////////////////
//     // create command object
//     if ( M_command_say )
//     {
//         //delete M_command_say;
//         //M_command_say = static_cast< PlayerSayCommand * >( 0 );
//         M_command_say->assign( msg );
//     }
//     else
//     {
//         M_command_say = new PlayerSayCommand( msg, version );
//     }

//     M_say_message = msg;
// }

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionEffector::addSayMessage( const SayMessage * message )
{
    dlog.addText( Logger::ACTION,
                  "add new say message." );

    M_say_messages.push_back( message );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
ActionEffector::removeSayMessage( const char header )
{
    dlog.addText( Logger::ACTION,
                  __FILE__": remove say message. header = %c", header );

    bool removed = false;

    std::vector< const SayMessage * >::iterator it = M_say_messages.begin();

    while ( it != M_say_messages.end() )
    {
        if ( (*it)->header() == header )
        {
            it = M_say_messages.erase( it );
            removed = true;
            dlog.addText( Logger::ACTION,
                          __FILE__": removed" );
        }
        else
        {
            ++it;
        }
    }

    return removed;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionEffector::setPointto( const double & x,
                            const double & y )
{
    dlog.addText( Logger::ACTION,
                  "register pointto. (%.2f, %.2f)",
                  x, y );

    Vector2D target_pos( x, y );
    Vector2D target_rel = target_pos - M_agent.world().self().pos();
    target_rel.rotate( - M_agent.world().self().face() );

    //////////////////////////////////////////////////
    // create command object
    if ( M_command_pointto )
    {
        delete M_command_pointto;
        M_command_pointto = static_cast< PlayerPointtoCommand * >( 0 );
    }
    M_command_pointto = new PlayerPointtoCommand( target_rel.r(),
                                                  target_rel.th().degree() );

    // set estimated commadn effect
    M_pointto_pos = target_pos;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionEffector::setPointtoOff()
{
    dlog.addText( Logger::ACTION,
                  "register pointto off" );

    //////////////////////////////////////////////////
    // create command object
    if ( M_command_pointto )
    {
        delete M_command_pointto;
        M_command_pointto = static_cast< PlayerPointtoCommand * >( 0 );
    }
    M_command_pointto = new PlayerPointtoCommand();

    // set estimated command effect
    M_pointto_pos.invalidate();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionEffector::setAttentionto( const SideID side,
                                const int unum )
{
    dlog.addText( Logger::ACTION,
                  "register attentionto. side= %d, unum= %d",
                  side, unum );

    //////////////////////////////////////////////////
    // create command object
    if ( M_command_attentionto )
    {
        delete M_command_attentionto;
        M_command_attentionto = static_cast< PlayerAttentiontoCommand * >( 0 );
    }

    M_command_attentionto
        = new PlayerAttentiontoCommand( ( M_agent.world().ourSide() == side
                                          ? PlayerAttentiontoCommand::OUR
                                          : PlayerAttentiontoCommand::OPP ),
                                        unum );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionEffector::setAttentiontoOff()
{
    dlog.addText( Logger::ACTION,
                  "register attentionto off" );

    //////////////////////////////////////////////////
    // create command object
    if ( M_command_attentionto )
    {
        delete M_command_attentionto;
        M_command_attentionto = static_cast< PlayerAttentiontoCommand * >( 0 );
    }
    M_command_attentionto = new PlayerAttentiontoCommand();
}

/*-------------------------------------------------------------------*/
/*!

*/
int
ActionEffector::getSayMessageLength() const
{
    int len = 0;

    const std::vector< const SayMessage * >::const_iterator end = M_say_messages.end();
    for ( std::vector< const SayMessage * >::const_iterator it = M_say_messages.begin();
          it != end;
          ++it )
    {
        len += (*it)->length();
    }

    return len;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionEffector::makeSayCommand()
{
    if ( M_command_say )
    {
        delete M_command_say;
        M_command_say = static_cast< PlayerSayCommand * >( 0 );
    }

    M_say_message.erase();

    // std::sort( M_say_messages.begin(), M_say_messages.end(),
    //            SayMessagePtrSorter() );

    const std::vector< const SayMessage * >::const_iterator end = M_say_messages.end();
    for ( std::vector< const SayMessage * >::const_iterator it = M_say_messages.begin();
          it != end;
          ++it )
    {
        (*it)->toStr( M_say_message );
    }

    if ( M_say_message.empty() )
    {
        return;
    }

    M_command_say = new PlayerSayCommand( M_say_message,
                                          M_agent.config().version() );

    dlog.addText( rcsc::Logger::ACTION,
                  __FILE__": say message [%s]",
                  M_say_message.c_str() );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionEffector::clearSayMessages()
{
    const std::vector< const SayMessage * >::const_iterator end = M_say_messages.end();
    for ( std::vector< const SayMessage * >::const_iterator it = M_say_messages.begin();
          it != end;
          ++it )
    {
        delete *it;
    }

    M_say_messages.clear();
}

////////////////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------*/
/*!

*/
AngleDeg
ActionEffector::queuedNextMyBody() const
{
    AngleDeg next_angle = M_agent.world().self().body();
    if ( M_command_body
         && M_command_body->type() == PlayerCommand::TURN )
    {
        double moment = 0.0;
        getTurnInfo( &moment, NULL );
        next_angle += moment;
        dlog.addText( Logger::SYSTEM,
                      "effector.queuedNextMyBody. cur_body= %f + moment= %f ->  %f",
                      M_agent.world().self().body().degree(), moment, next_angle.degree() );
    }

    return next_angle;
}

/*-------------------------------------------------------------------*/
/*!

*/
Vector2D
ActionEffector::queuedNextMyPos() const
{
    Vector2D vel = M_agent.world().self().vel();
    if ( M_command_body
         && M_command_body->type() == PlayerCommand::DASH )
    {
        Vector2D accel( 0.0, 0.0 );
        getDashInfo( &accel, NULL );
        vel += accel;

        double tmp = vel.r();
        if ( tmp > M_agent.world().self().playerType().playerSpeedMax() )
        {
            vel *= M_agent.world().self().playerType().playerSpeedMax() / tmp;
        }
    }

    return M_agent.world().self().pos() + vel;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
ActionEffector::queuedNextBallKickable() const
{
    if ( M_agent.world().ball().rposCount() >= 3 )
    {
        return false;
    }

    Vector2D my_next = queuedNextMyPos();
    Vector2D ball_next = queuedNextBallPos();

    return my_next.dist( ball_next ) < M_agent.world().self().kickableArea() - 0.06;
}

/*-------------------------------------------------------------------*/
/*!

*/
Vector2D
ActionEffector::queuedNextBallPos() const
{
    if ( ! M_agent.world().ball().posValid() )
    {
        return Vector2D::INVALIDATED;
    }

    Vector2D vel( 0.0, 0.0 ), accel( 0.0, 0.0 );

    if ( M_agent.world().ball().velValid() )
    {
        vel = M_agent.world().ball().vel();
    }

    if ( M_command_body
         && M_command_body->type() == PlayerCommand::KICK )
    {
        getKickInfo( &accel, NULL );
    }

    vel += accel;

    return M_agent.world().ball().pos() + vel;
}

/*-------------------------------------------------------------------*/
/*!

*/
Vector2D
ActionEffector::queuedNextBallVel() const
{
    Vector2D vel( 0.0, 0.0 ), accel( 0.0, 0.0 );

    if ( M_agent.world().ball().velValid() )
    {
        vel = M_agent.world().ball().vel();
    }

    if ( M_command_body
         && M_command_body->type() == PlayerCommand::KICK )
    {
        getKickInfo( &accel, NULL );
    }

    vel += accel;
    vel *= ServerParam::i().ballDecay();
    return vel;
}

/*-------------------------------------------------------------------*/
/*!

*/
AngleDeg
ActionEffector::queuedNextAngleFromBody( const Vector2D & target ) const
{
    Vector2D next_rpos = target - queuedNextMyPos();

    return next_rpos.th() - queuedNextMyBody();
}

/*-------------------------------------------------------------------*/
/*!

*/
ViewWidth
ActionEffector::queuedNextViewWidth() const
{
    if ( M_command_change_view )
    {
        return M_command_change_view->width();
    }

    return M_agent.world().self().viewWidth();
}

}
