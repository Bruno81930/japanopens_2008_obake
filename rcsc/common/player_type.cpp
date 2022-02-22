// -*-c++-*-

/*!
  \file player_type.cpp
  \brief heterogenious player parametor Source File
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

#include "player_type.h"
#include "player_param.h"
#include "server_param.h"

#include <rcsc/rcg/util.h>

#include <sstream>
#include <algorithm>
#include <string>
#include <cstdio>


namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
PlayerType::PlayerType( const ServerParam & sparam,
                        const int id,
                        const double & pspdmax,
                        const double & sincmax,
                        const double & pdecay,
                        const double & imoment,
                        const double & dprate,
                        const double & psize,
                        const double & kmargin,
                        const double & krand,
                        const double & estamina,
                        const double & effmax,
                        const double & effmin )
    : M_id( id )
    , M_player_speed_max( pspdmax )
    , M_stamina_inc_max( sincmax )
    , M_player_decay( pdecay )
    , M_inertia_moment( imoment )
    , M_dash_power_rate( dprate )
    , M_player_size( psize )
    , M_kickable_margin( kmargin )
    , M_kick_rand( krand )
    , M_extra_stamina( estamina )
    , M_effort_max( effmax )
    , M_effort_min( effmin )
    , M_cycles_to_reach_max_speed( 6 )
{
    if ( id < Hetero_Unknown
         || PlayerParam::i().playerTypes() <= id )
    {
        std::cerr << "PlayerType. Invalid player type id " << id
                  << std::endl;
    }

    initAdditionalParams( sparam );
}

/*-------------------------------------------------------------------*/
/*!

*/
PlayerType::PlayerType( const ServerParam & sparam )
    : M_id( Hetero_Default )
    , M_player_speed_max( sparam.defaultPlayerSpeedMax() )
    , M_stamina_inc_max( sparam.defaultStaminaIncMax() )
    , M_player_decay( sparam.defaultPlayerDecay() )
    , M_inertia_moment( sparam.defaultInertiaMoment() )
    , M_dash_power_rate( sparam.defaultDashPowerRate() )
    , M_player_size( sparam.defaultPlayerSize() )
    , M_kickable_margin( sparam.defaultKickableMargin() )
    , M_kick_rand( sparam.defaultKickRand() )
    , M_extra_stamina( sparam.defaultExtraStamina() )
    , M_effort_max( sparam.defaultEffortMax() )
    , M_effort_min( sparam.defaultEffortMin() )
{
    initAdditionalParams( sparam );
}

/*-------------------------------------------------------------------*/
/*!

*/
PlayerType::PlayerType( const ServerParam & sparam,
                        const char * msg,
                        const double & version )
{
    if ( version >= 8.0 )
    {
        parseV8( msg );
    }
    else
    {
        parseV7( msg );
    }

    initAdditionalParams( sparam );
}

/*-------------------------------------------------------------------*/
/*!

*/
PlayerType::PlayerType( const ServerParam & sparam,
                        const rcg::player_type_t & from )
{
    M_id = rcg::nstohi( from.id );
    M_player_speed_max = rcg::nltohd( from.player_speed_max );
    M_stamina_inc_max = rcg::nltohd( from.stamina_inc_max );
    M_player_decay = rcg::nltohd( from.player_decay );
    M_inertia_moment = rcg::nltohd( from.inertia_moment );
    M_dash_power_rate = rcg::nltohd( from.dash_power_rate );
    M_player_size = rcg::nltohd( from.player_size );
    M_kickable_margin = rcg::nltohd( from.kickable_margin );
    M_kick_rand = rcg::nltohd( from.kick_rand );
    M_extra_stamina = rcg::nltohd( from.extra_stamina );
    M_effort_max = rcg::nltohd( from.effort_max );
    M_effort_min = rcg::nltohd( from.effort_min );

    /*
      long sparelong1;
      long sparelong2;
      long sparelong3;
      long sparelong4;
      long sparelong5;
      long sparelong6;
      long sparelong7;
      long sparelong8;
      long sparelong9;
      long sparelong10;
    */

    initAdditionalParams( sparam );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerType::convertTo( rcg::player_type_t & to ) const
{
    to.id = rcg::hitons( M_id );
    to.player_speed_max = rcg::hdtonl( M_player_speed_max );
    to.stamina_inc_max = rcg::hdtonl( M_stamina_inc_max );
    to.player_decay = rcg::hdtonl( M_player_decay );
    to.inertia_moment = rcg::hdtonl( M_inertia_moment );
    to.dash_power_rate = rcg::hdtonl( M_dash_power_rate );
    to.player_size = rcg::hdtonl( M_player_size );
    to.kickable_margin = rcg::hdtonl( M_kickable_margin );
    to.kick_rand = rcg::hdtonl( M_kick_rand );
    to.extra_stamina = rcg::hdtonl( M_extra_stamina );
    to.effort_max = rcg::hdtonl( M_effort_max );
    to.effort_min = rcg::hdtonl( M_effort_min );

    /*
      long sparelong1;
      long sparelong2;
      long sparelong3;
      long sparelong4;
      long sparelong5;
      long sparelong6;
      long sparelong7;
      long sparelong8;
      long sparelong9;
      long sparelong10;
    */
}

/*-------------------------------------------------------------------*/
/*!

*/
std::string
PlayerType::toStr() const
{
    std::ostringstream os;

    os << "(player_type "
       << "(id " << M_id << ')'
       << "(player_speed_max " << M_player_speed_max << ')'
       << "(stamina_inc_max " << M_stamina_inc_max << ')'
       << "(player_decay " << M_player_decay << ')'
       << "(inertia_moment " << M_inertia_moment << ')'
       << "(dash_power_rate " << M_dash_power_rate << ')'
       << "(player_size " << M_player_size << ')'
       << "(kickable_margin " << M_kickable_margin << ')'
       << "(kick_rand " << M_kick_rand << ')'
       << "(extra_stamina " << M_extra_stamina << ')'
       << "(effort_max " << M_effort_max << ')'
       << "(effort_min " << M_effort_min << ')'
       << ')';

    return os.str();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerType::parseV8( const char * msg )
{
    // read v8 protocol
    /*
      "(player_type (id 0) (player_speed_max 1.2) (stamina_inc_max 45)
      (player_decay 0.4) (inertia_moment 5)
      (dash_power_rate 0.006) (player_size 0.3)
      (kickable_margin 0.7) (kick_rand 0)
      (extra_stamina 0) (effort_max 1) (effort_min 0.6))"
      "(player_type (id 3) (player_speed_max 1.2) (stamina_inc_max 40.68)
      (player_decay 0.5308) (inertia_moment 8.27)
      (dash_power_rate 0.006432) (player_size 0.3)
      (kickable_margin 0.7002) (kick_rand 0.0001)
      (extra_stamina 98) (effort_max 0.804) (effort_min 0.404))";
    */

    int n_read = 0;

    char name[32];
    int id = 0;
    if ( std::sscanf( msg, " ( player_type ( %s %d ) %n ",
                      name, &id, &n_read ) != 2
         || n_read == 0
         || id < 0 )
    {
        std::cerr << "PlayerType id read error. "
                  << msg
                  << std::endl;
        return;
    }
    msg += n_read;

    M_id = id;

    int n_param = 0;
    while ( *msg != '\0' && *msg != ')' )
    {
        double val = 0.0;
        if ( std::sscanf( msg, " ( %s %lf ) %n ",
                          name, &val, &n_read ) != 2
             || n_read == 0 )
        {
            std::cerr << "PlayerType parameter read error: "
                      << msg << std::endl;
            break;
        }
        msg += n_read;

        if ( ! std::strcmp( name, "player_speed_max" ) )
        {
            M_player_speed_max = val;
        }
        else if ( ! std::strcmp( name, "stamina_inc_max" ) )
        {
            M_stamina_inc_max = val;
        }
        else if ( ! std::strcmp( name, "player_decay" ) )
        {
            M_player_decay = val;
        }
        else if ( ! std::strcmp( name, "inertia_moment" ) )
        {
            M_inertia_moment = val;
        }
        else if ( ! std::strcmp( name, "dash_power_rate" ) )
        {
            M_dash_power_rate = val;
        }
        else if ( ! std::strcmp( name, "player_size" ) )
        {
            M_player_size = val;
        }
        else if ( ! std::strcmp( name, "kickable_margin" ) )
        {
            M_kickable_margin = val;
        }
        else if ( ! std::strcmp( name, "kick_rand" ) )
        {
            M_kick_rand = val;
        }
        else if ( ! std::strcmp( name, "extra_stamina" ) )
        {
            M_extra_stamina = val;
        }
        else if ( ! std::strcmp( name, "effort_max" ) )
        {
            M_effort_max = val;
        }
        else if ( ! std::strcmp( name, "effort_min" ) )
        {
            M_effort_min = val;
        }
        else
        {
            std::cerr << "player_type: param name error " << msg << std::endl;
            break;
        }

        ++n_param;
    }

    //std::cerr << "  read " << n_param << " params" << endl;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerType::parseV7( const char * msg )
{
    // read v7 protocol
    // read param value only, no param name
    /*
      from rcssserver/src/hetroplayer.C
      std::ostream& toStr(std::ostream& o,
      const PlayerTypeSensor_v7::data_t& data)
      {
      return o << "(player_type " << data.M_id << " "
      << data.M_player_speed_max << " "
      << data.M_stamina_inc_max << " "
      << data.M_player_decay << " "
      << data.M_inertia_moment << " "
      << data.M_dash_power_rate << " "
      << data.M_player_size << " "
      << data.M_kickable_margin << " "
      << data.M_kick_rand << " "
      << data.M_extra_stamina << " "
      << data.M_effort_max << " "
      << data.M_effort_min
      << ")";
      }
    */

    std::istringstream msg_strm( msg );
    std::string tmp;

    msg_strm >> tmp // skip "(player_type"
             >> M_id
             >> M_player_speed_max
             >> M_stamina_inc_max
             >> M_player_decay
             >> M_inertia_moment
             >> M_dash_power_rate
             >> M_player_size
             >> M_kickable_margin
             >> M_kick_rand
             >> M_extra_stamina
             >> M_effort_max
             >> M_effort_min;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerType::initAdditionalParams( const ServerParam & sparam )
{
    M_kickable_area = playerSize() + kickableMargin() + sparam.ballSize();

    ///////////////////////////////////////////////////////////////////

    double accel
        = sparam.maxPower() * dashPowerRate() * effortMax();

    // see also soccer_math.h
    M_real_speed_max = accel / (1.0 - playerDecay()); // sum inf geom series
    if ( M_real_speed_max > playerSpeedMax() )
    {
        M_real_speed_max = playerSpeedMax();
    }

    ///////////////////////////////////////////////////////////////////
    M_player_speed_max2 = playerSpeedMax() * playerSpeedMax();
    M_real_speed_max2 = realSpeedMax() * realSpeedMax();

    /////////////////////////////////////////////////////////////////////
    double speed = 0.0;
    double dash_power = sparam.maxPower();
    double stamina = sparam.staminaMax();
    double effort = this->effortMax();
    double recover = sparam.recoverInit();

    double reach_dist = 0.0;

    M_cycles_to_reach_max_speed = -1;

    M_dash_distance_table.clear();
    M_dash_distance_table.reserve( 50 );

    for ( int counter = 1; counter <= 50; ++counter )
    {
        if ( speed + accel > playerSpeedMax() )
        {
            accel = playerSpeedMax() - speed;
            dash_power = std::min( sparam.maxPower(),
                                   accel / ( dashPowerRate() * effort ) );
        }

        speed += accel;

        reach_dist += speed;

        M_dash_distance_table.push_back( reach_dist );

        if ( M_cycles_to_reach_max_speed < 0
             && speed >= realSpeedMax() - 0.01 )
        {
            M_cycles_to_reach_max_speed = counter;
        }

        speed *= playerDecay();

        predictStaminaAfterDash( sparam,
                                 dash_power,
                                 &stamina, &effort, &recover );

        //M_stamina_table.push_back( sparam.staminaMax() - stamina );

        if ( stamina == 0.0 )
        {
            break;
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
int
PlayerType::getMaxDashCyclesSavingStamina( const ServerParam & sparam,
                                           const double & dash_power,
                                           const double & current_stamina,
                                           const double & current_recovery ) const
{
    double available_stamina
        = current_stamina
        - sparam.recoverDecThrValue()
        - 1.0;
    double used_stamina = ( dash_power > 0.0 ? dash_power : dash_power * -2.0 );

    available_stamina -= used_stamina; // buffer for last one dash
    if ( available_stamina < 0.0 )
    {
        return 0;
    }

    double one_cycle_consumpation
        = used_stamina - ( staminaIncMax() * current_recovery );
    return static_cast< int >
        ( std::floor( available_stamina / one_cycle_consumpation ) );
}

/*-------------------------------------------------------------------*/
/*!

*/
#if 0
int
PlayerType::maxDashCyclesWith( const double & stamina ) const
{
    if ( stamina <= 0.0 )
    {
        return 0;
    }

    std::vector< double >::const_iterator
        it = std::lower_bound( M_stamina_table.begin(),
                               M_stamina_table.end(),
                               stamina );

    if ( it != M_dash_distance_table.end() )
    {
        return static_cast< int >( it - M_stamina_table.begin() );
    }

    if ( stamina < M_stamina_table.front() )
    {
        return 0;
    }

    return M_stamina_table.size();
}
#endif

/*-------------------------------------------------------------------*/
/*!

*/
#if 0
double
PlayerType::consumedStaminaAfterNrDash( const int n_dash ) const
{
    if ( 0 <= n_dash && n_dash < (int)M_stamina_table.size() )
    {
        return M_stamina_table[n_dash];
    }

    return M_stamina_table.back();
}
#endif

/*-------------------------------------------------------------------*/
/*!

*/
int
PlayerType::cyclesToReachMaxSpeed( const double & dash_power ) const
{
    double accel = std::fabs( dash_power ) * dashPowerRate() * effortMax();
    double speed_max = accel / ( 1.0 - playerDecay() );
    if ( speed_max > playerSpeedMax() )
    {
        speed_max = playerSpeedMax();
    }

    double decn = 1.0 - ( ( speed_max - 0.01 ) * ( 1.0 - playerDecay() ) / accel );
    return static_cast< int >( std::ceil( std::log( decn )
                                          / std::log( playerDecay() ) ) );
}

/*-------------------------------------------------------------------*/
/*!

*/
int
PlayerType::cyclesToReachDistance( const double & dash_dist ) const
{
    std::vector< double >::const_iterator
        it = std::lower_bound( M_dash_distance_table.begin(),
                               M_dash_distance_table.end(),
                               dash_dist );

    if ( it != M_dash_distance_table.end() )
    {
        return ( static_cast< int >
                 ( it - M_dash_distance_table.begin() )
                 + 1 );
    }

    double rest_dist = dash_dist - M_dash_distance_table.back();
    int cycle = M_dash_distance_table.size();

    cycle += static_cast< int >( std::ceil( rest_dist / realSpeedMax() ) );

    return cycle;
}

/*-------------------------------------------------------------------*/
/*!

*/
double
PlayerType::getDashPowerToKeepMaxSpeed(  const ServerParam & sparam,
                                         const double & effort ) const
{
    // required accel in 1 step to keep max speed
    double required_power = playerSpeedMax() * ( 1.0 - playerDecay() );
    // required dash power to keep max speed
    required_power /= ( effort * dashPowerRate() );

    if ( required_power > sparam.maxPower() )
    {
        required_power = sparam.maxPower();
    }
    return required_power;
}

/*-------------------------------------------------------------------*/
/*!

*/
double
PlayerType::kickRate( const ServerParam & sparam,
                      const double & ball_dist,
                      const double & dir_diff ) const
{
    return (
            sparam.kickPowerRate()
            * ( 1.0
                - 0.25 * std::fabs( dir_diff ) / 180.0
                - ( 0.25 * ( ball_dist - sparam.ballSize() - playerSize() )
                    / kickableMargin() ) )
            );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
PlayerType::normalizeAccel( const Vector2D & velocity,
                            const AngleDeg & accel_angle,
                            double * accel_mag ) const
{
    // if ( *accel_mag > sparam.playerAccelMax() )
    //     *accel_mag = sparam.playerAccelMax();

    Vector2D dash_move
        = Vector2D::polar2vector( *accel_mag, accel_angle ); // set dash accel
    dash_move += velocity; // add current vel -> next move == player vel
    if ( dash_move.r2() > playerSpeedMax2() + 0.0001 )
    {
        Vector2D rel_vel = velocity.rotatedVector( -accel_angle );
        // sqr(rel_vel.y) + sqr(max_dash_x) == sqr(max_speed);
        // accel_mag = dash_x - rel_vel.x;
        double max_dash_x = std::sqrt( playerSpeedMax2()
                                       - rel_vel.y * rel_vel.y );
        *accel_mag = max_dash_x - rel_vel.x;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
PlayerType::normalizeAccel( const Vector2D & velocity,
                            Vector2D * accel ) const
{
    // double tmp = accel_mag->r();
    // if ( tmp > sparam.playerAccelMax() )
    //     *accel *= sparam.playerAccelMax() / tmp;

    if ( ( velocity + (*accel) ).r2() > playerSpeedMax2() + 0.0001 )
    {
        Vector2D rel_vel = velocity.rotatedVector( -accel->th() );
        // sqr(rel_vel.y) + sqr(max_dash_x) == sqr(max_speed);
        // accel_mag = dash_x - rel_vel.x;
        double max_dash_x = std::sqrt( playerSpeedMax2()
                                       - rel_vel.y * rel_vel.y );
        accel->setLength( max_dash_x - rel_vel.x );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerType::predictStaminaAfterWait( const ServerParam & sparam,
                                     const int n_wait,
                                     double * stamina,
                                     double * effort,
                                     const double & recovery ) const
{
    for ( int i = 0; i < n_wait; ++i )
    {
        // recovery check is skipped,
        // because recovery is never increased.

        // check effort
        if ( *stamina <= sparam.effortDecThrValue() )
        {
            if ( *effort > effortMin() )
            {
                *effort -= sparam.effortDec();
                if ( *effort < effortMin() )
                {
                    *effort = effortMin();
                }
            }
        }
        else if ( *stamina > sparam.effortIncThrValue() )
        {
            if ( *effort < effortMax() )
            {
                *effort += sparam.effortInc();
                if ( *effort > effortMax() )
                {
                    *effort = effortMax();
                }
            }
        }

        // recover stamina
        *stamina += staminaIncMax() * recovery;
        if ( *stamina > sparam.staminaMax() )
        {
            *stamina = sparam.staminaMax();
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerType::predictStaminaAfterDash( const ServerParam & sparam,
                                     const double & dash_power,
                                     double * stamina,
                                     double * effort,
                                     double * recovery ) const
{
    // decay stamina
    *stamina -= ( dash_power > 0.0 ? dash_power : (dash_power * -2.0) );
    if ( *stamina < 0.0 )
    {
        *stamina = 0.0;
    }

    // check recovery decay
    if ( *stamina <= sparam.recoverDecThrValue() )
    {
        if ( *recovery > sparam.recoverMin() )
        {
            *recovery -= sparam.recoverDec();
            *recovery = std::max( *recovery, sparam.recoverMin() );
        }
    }

    // check effort decay
    if ( *stamina <= sparam.effortDecThrValue() )
    {
        if ( *effort > effortMin() )
        {
            *effort -= sparam.effortDec();
            *effort = std::max( *effort, effortMin() );
        }
    }
    else if ( *stamina >= sparam.effortIncThrValue() )
    {
        if ( *effort < effortMax() )
        {
            *effort += sparam.effortInc();
            *effort = std::min( *effort, effortMax() );
        }
    }

    // increase stamina
    *stamina += staminaIncMax() * (*recovery);
    *stamina = std::min( *stamina, sparam.staminaMax() );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerType::predictStaminaAfterNrDash( const ServerParam & sparam,
                                       const double & dash_power,
                                       const int n_dash,
                                       double * stamina,
                                       double * effort,
                                       double * recovery ) const
{
    int n_safety_dash =
        getMaxDashCyclesSavingStamina( sparam, dash_power, *stamina, *recovery );
    if ( n_safety_dash > n_dash )
    {
        n_safety_dash = n_dash;
    }

    double one_cycle_consumpation
        = ( dash_power > 0.0 ? dash_power : dash_power * -2.0 )
        - ( staminaIncMax() * (*recovery) );
    *stamina -= one_cycle_consumpation * n_safety_dash;

    const int n_exhaust_dash = n_dash - n_safety_dash;
    for ( int i = 0; i < n_exhaust_dash; ++i )
    {
        predictStaminaAfterDash( sparam, dash_power, stamina, effort, recovery );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
PlayerType::print( std::ostream & os ) const
{
    os << "player_type id : " << id()
       << "\n  player_speed_max : " << playerSpeedMax()
       << "\n  stamina_inc_max :  " << staminaIncMax()
       << "\n  player_decay : "     << playerDecay()
       << "\n  inertia_moment : "   << inertiaMoment()
       << "\n  dash_power_rate : "  << dashPowerRate()
       << "\n  player_size : "      << playerSize()
       << "\n  kickable_margin : "  << kickableMargin()
       << "\n  kick_rand : "        << kickRand()
       << "\n  extra_stamina : "    << extraStamina()
       << "\n  effort_max : "       << effortMax()
       << "\n  effort_min : "       << effortMin()
       << std::endl;
    return os;
}



///////////////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------*/
/*!

*/
PlayerTypeSet &
PlayerTypeSet::instance()
{
    static PlayerTypeSet S_instance;
    return S_instance;
}

/*-------------------------------------------------------------------*/
/*!

*/
PlayerTypeSet::PlayerTypeSet()
    : M_dummy_type( ServerParam::i(),
                    Hetero_Unknown,
                    1.2, // player speed max
                    45.0, // stamina inc max
                    0.4, // decay
                    5.0, // inertia moment
                    0.006, // dash power rate
                    0.3, // size
                    0.7, // kickable margin
                    0.0, // kick rand
                    0.0, // extra stamina
                    1.0, // effort max
                    0.6 ) // effort min
{
    resetDefaultType( ServerParam::i() );
}

/*-------------------------------------------------------------------*/
/*!

*/
PlayerTypeSet::~PlayerTypeSet()
{

}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerTypeSet::resetDefaultType( const ServerParam & sparam )
{
    PlayerType default_type( sparam );
    insert( default_type );
    M_dummy_type = default_type;
}

/*-------------------------------------------------------------------*/
/*!
  return pointer to param
*/
void
PlayerTypeSet::insert( const PlayerType & param )
{
    if ( static_cast< int >( M_player_type_map.size() )
         >= PlayerParam::i().playerTypes() )
    {
        std::cerr << "player types over flow" << std::endl;
        return;
    }

    // insert new type
    M_player_type_map.insert( std::make_pair( param.id(), param ) );


    if ( static_cast< int >( M_player_type_map.size() )
         == PlayerParam::i().playerTypes() )
    {
        createDummyType();
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerTypeSet::createDummyType()
{
#if 0
    double player_speed_max = 0.0;
    double stamina_inc_max = 0.0;
    double player_decay = 1.0;
    double inertia_moment = 10.0;
    double dash_power_rate = 0.0;
    double player_size = 1.0;
    double kickable_margin = 0.0;
    double kick_rand = 1.0;
    double extra_stamina = 0.0;
    double effort_max = 0.0;
    double effort_min = 1.0;
    for ( PlayerTypeMap::iterator it = M_player_type_map.begin();
          it != M_player_type_map.end();
          ++it )
    {
        player_speed_max = std::max( player_speed_max, it->second.playerSpeedMax() );
        stamina_inc_max = std::max( stamina_inc_max, it->second.staminaIncMax() );
        player_decay = std::min( player_decay, it->second.playerDecay() );
        inertia_moment = std::min( inertia_moment, it->second.inertiaMoment() );
        dash_power_rate = std::max( dash_power_rate, it->second.dashPowerRate() );
        player_size = std::min( player_size, it->second.playerSize() );
        kickable_margin = std::max( kickable_margin, it->second.kickableMargin() );
        kick_rand = std::min( kick_rand, it->second.kickRand() );
        extra_stamina = std::max( extra_stamina, it->second.extraStamina() );
        effort_max = std::max( effort_max, it->second.effortMax() );
        effort_min = std::min( effort_min, it->second.effortMin() );
    }

    M_dummy_type = PlayerType( ServerParam::i(),
                               Hetero_Unknown,
                               player_speed_max,
                               stamina_inc_max,
                               player_decay,
                               inertia_moment,
                               dash_power_rate,
                               player_size,
                               kickable_margin,
                               kick_rand,
                               extra_stamina,
                               effort_max,
                               effort_min );
#endif
    for ( PlayerTypeMap::iterator it = M_player_type_map.begin();
          it != M_player_type_map.end();
          ++it )
    {
        if ( it->second.playerSpeedMax() > M_dummy_type.playerSpeedMax() )
        {
            M_dummy_type = it->second;
        }
        else if ( std::fabs( it->second.playerSpeedMax()
                             - M_dummy_type.playerSpeedMax() ) < 0.01
                  && it->second.cyclesToReachMaxSpeed() < M_dummy_type.cyclesToReachMaxSpeed() )
        {
            M_dummy_type = it->second;
        }
    }
}

/*-------------------------------------------------------------------*/
/*!
  return pointer to param
*/
const
PlayerType *
PlayerTypeSet::get( const int id ) const
{
    if ( id == Hetero_Unknown )
    {
        return &M_dummy_type;
    }
    else
    {
        PlayerTypeMap::const_iterator it = M_player_type_map.find( id );
        if ( it != M_player_type_map.end() )
        {
            return &( it->second );
        }
    }

    std::cerr << "PlayerTypeSet: get : player type id error " << id << std::endl;
    return static_cast< PlayerType * >( 0 );
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
PlayerTypeSet::print( std::ostream & os ) const
{
    os << "All Player Types: n";

    PlayerTypeMap::const_iterator it = M_player_type_map.begin();
    while ( it != M_player_type_map.end() )
    {
        it->second.print( os );
        ++it;
    }
    return os;
}

}
