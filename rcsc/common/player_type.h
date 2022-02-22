// -*-c++-*-

/*!
  \file player_type.h
  \brief heterogenious player parametor Header File
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

#ifndef RCSC_PARAM_PLAYER_TYPE_H
#define RCSC_PARAM_PLAYER_TYPE_H

#include <rcsc/geom/vector_2d.h>
#include <rcsc/rcg/types.h>
#include <rcsc/soccer_math.h>
#include <rcsc/types.h>

#include <map>
#include <vector>
#include <iostream>

namespace rcsc {

class ServerParam;

/*!
  \class PlayerType
  \brief heterogeneous player parametor class
 */
class PlayerType {
private:
    int M_id;
    double M_player_speed_max;
    double M_stamina_inc_max;
    double M_player_decay;
    double M_inertia_moment;
    double M_dash_power_rate;
    double M_player_size;
    double M_kickable_margin;
    double M_kick_rand;
    double M_extra_stamina;
    double M_effort_max;
    double M_effort_min;

    // additional parameters

    double M_kickable_area;

    // if player's dprate & effort is not enough,
    // player never reach player_speed_max
    double M_real_speed_max;

    double M_player_speed_max2; // squared value
    double M_real_speed_max2;   // squared value

    //! dash cycles to reach max speed
    int M_cycles_to_reach_max_speed;

    //! distance table by continuous dashes from the velocity 0.
    std::vector< double > M_dash_distance_table;

    // stamina cconsumption table by continuous dashes
    //std::vector< double > M_stamina_table;

public:
    /*!
      \brief default constructo

      just set Hetero_Unknown to Id
     */
    PlayerType()
        : M_id( Hetero_Unknown )
      { }

    /*!
      \brief constructor with all parameters
      \param sparam server parameter
      \param id hetero Id
      \param pspdmax player_speed_max parameter
      \param sincmax stamina_inc_max parameter
      \param pdecay player_decay parameter
      \param imoment inertia_moment parameter
      \param dprate dash_power_rate parameter
      \param psize player_size parameter
      \param kmargin kickable_margin parameter
      \param krand kick_rand parameter
      \param estamina extra_stamina parameter
      \param effmax effort_max parameter
      \param effmin effort_min parameter
    */
    PlayerType( const ServerParam & sparam,
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
                const double & effmin );

    /*!
      \brief construct default type parameter using ServerParam
      \param sparam const reference to the ServerParam
    */
    explicit
    PlayerType( const ServerParam & sparam );

    /*!
      \brief construct with message from rcssserver
      \param sparam const reference to the ServerParam
      \param msg raw message from rcssserver
      \param version client version that determins message protocol
    */
    PlayerType( const ServerParam & sparam,
                const char * msg,
                const double & version );

    /*!
      \brief construct with monitor protocol
      \param sparam const reference to the ServerParam
      \param from monitor protocol data
     */
    PlayerType( const ServerParam & sparam,
                const rcg::player_type_t & from );

    /*!
      \brief conver to the monitor protocol format
      \param to reference to the data variable.
     */
    void convertTo( rcg::player_type_t & to ) const;

    /*!
      \brief convert to the rcss parameter message
      \return parameter message string
     */
    std::string toStr() const;

private:

    /*!
      \brief analyze version 8 protocol server message
      \param msg raw message string from rcssserver
     */
    void parseV8( const char * msg );

    /*!
      \brief analyze version 7 protocol server message
      \param msg raw message string from rcssserver
     */
    void parseV7( const char * msg );

    /*!
      \brief set additional parameters value
      \param sparam const reference to the ServerParam
     */
    void initAdditionalParams( const ServerParam & sparam );

public:

    int id() const
      {
          return M_id;
      }
    const
    double & playerSpeedMax() const
      {
          return M_player_speed_max;
      }
    const
    double & staminaIncMax() const
      {
          return M_stamina_inc_max;
      }
    const
    double & playerDecay() const
      {
          return M_player_decay;
      }
    const
    double & inertiaMoment() const
      {
          return M_inertia_moment;
      }
    const
    double & dashPowerRate() const
      {
          return M_dash_power_rate;
      }
    const
    double & playerSize() const
      {
          return M_player_size;
      }
    const
    double & kickableMargin() const
      {
          return M_kickable_margin;
      }
    const
    double & kickRand() const
      {
          return M_kick_rand;
      }
    const
    double & extraStamina() const
      {
          return M_extra_stamina;
      }
    const
    double & effortMax() const
      {
          return M_effort_max;
      }
    const
    double & effortMin() const
      {
          return M_effort_min;
      }

    ////////////////////////////////////////////////
    // additional parameters

    const
    double & kickableArea() const
      {
          return M_kickable_area;
      }
    const
    double & realSpeedMax() const
      {
          return M_real_speed_max;
      }
    const
    double & playerSpeedMax2() const
      {
          return M_player_speed_max2;
      }
    const
    double & realSpeedMax2() const
      {
          return M_real_speed_max2;
      }

    /*!
      \brief get dash reachable distance table
      \return const reference to the distance table container
     */
    const
    std::vector< double > & dashDistanceTable() const
      {
          return M_dash_distance_table;
      }

    ////////////////////////////////////////////////
    /*!
      \brief calculate enable cycles to keep to dash using max power
      \param sparam server parameter
      \param dash_power used dash power
      \param current_stamina current agent's stamina
      \param current_recovery current agent's recovery
      \return max cycles to keep same dash power
    */
    int getMaxDashCyclesSavingStamina( const ServerParam & sparam,
                                       const double & dash_power,
                                       const double & current_stamina,
                                       const double & current_recovery ) const;

    /*
      \brief estimate the number of available dashes with max power
      \param stamina available stamina
      \return estimated cycle
     */
    //int maxDashCyclesWith( const double & stamina ) const;

    /*
      \brief get the consumed stamina value after nr dashes with the max power
      from the velocity 0.
      \param n_dash dash count
      \return consumed stamina value

      this method can be used when player's recover is not decayed.
     */
    //double consumedStaminaAfterNrDash( const int n_dash ) const;

    ////////////////////////////////////////////////
    /*!
      \brief estimate cycles to reach max speed from zero.
      \param dash_power used dash power
      \return estimated cycles to reach.
    */
    int cyclesToReachMaxSpeed( const double & dash_power ) const;

    /*!
      \brief estimate cycles to reach max speed from zero using max dash power.
      \return estimated cycles to reach.

      returned value is calculated by initAdditionalParams()
    */
    int cyclesToReachMaxSpeed() const
      {
          return M_cycles_to_reach_max_speed;
      }
    ////////////////////////////////////////////////
    /*!
      \brief estimate cycles to reach the specific distance with start speed 0.
      \param dash_dist distance to reach
      \return estimated cycles to reach
    */
    int cyclesToReachDistance( const double & dash_dist ) const;
    ////////////////////////////////////////////////
    /*!
      \brief check if this type player can over player_speed_max
      \param dash_power used dash_power
      \param effort current effort value
      \return true if player has the potential to go over the max speed
     */
    bool canOverSpeedMax( const double & dash_power,
                          const double & effort ) const
      {
          return ( std::fabs( dash_power ) * dashPowerRate() * effort
                   > playerSpeedMax() * ( 1.0 - playerDecay() ) );
      }
    ////////////////////////////////////////////////
    /*!
      \brief estimate dash power to keep max speed
      \param sparam server parameter
      \param effort current agent's effort
      \return conserved dash power
    */
    double getDashPowerToKeepMaxSpeed( const ServerParam & sparam,
                                       const double & effort ) const;

    /*!
      \brief estimate dash power to keep max speed with max effort value
      \param sparam server parameter
      \return conserved dash power
    */
    double getDashPowerToKeepMaxSpeed( const ServerParam & sparam ) const
      {
          return getDashPowerToKeepMaxSpeed( sparam, effortMax() );
      }

    /*!
      \brief estimate dash power to keep the specified speed
      \param speed the desired speed
      \param effort current effort value
      \return estimated dash power, but not normalized
     */
    double getDashPowerToKeepSpeed( const double & speed,
                                    const double & effort ) const
      {
          return speed * ( ( 1.0 - playerDecay() )
                           / (dashPowerRate() * effort ) );
      }

    /*!
      \brief estimate one cycle stamina comsumption to keep mas speed
      \param sparam server parameter
      \return get the comsumed stamina value
     */
    double getOneStepStaminaComsumption( const ServerParam & sparam ) const
      {
          return getDashPowerToKeepMaxSpeed( sparam, effortMax() ) - staminaIncMax();
      }


    ////////////////////////////////////////////////
    /*!
      \brief calculate kick rate
      \param sparam server parameter
      \param ball_dist ball distance from agent
      \param dir_diff ball angle difference from agent body angle
      \return kick rate value
    */
    double kickRate( const ServerParam & sparam,
                     const double & ball_dist,
                     const double & dir_diff ) const;

    /*!
      \brief calculate dash rate
      \param effort current effort value
      \return dash power rate multiplied by effort
     */
    double dashRate( const double & effort ) const
      {
          return effort * dashPowerRate();
      }

    /*!
      \brief calculate effective turn angle
      \param command_moment turn command argument
      \param speed current speed
      \return estimated result turn angle
     */
    double effectiveTurn( const double & command_moment,
                          const double & speed ) const
      {
          return command_moment / ( 1.0 + inertiaMoment() * speed );
      }

    /*!
      \brief calculate final reachable speed
      \param dash_power used dash power
      \param effort current effort
      \return maximal speed value with the specified params
     */
    double finalSpeed( const double & dash_power,
                       const double & effort ) const
      {
          return std::min( playerSpeedMax(),
                           ( ( std::fabs(dash_power) * dashPowerRate() * effort ) // == accel
                             / ( 1.0 - playerDecay() ) ) ); // sum inf geom series
      }
    ////////////////////////////////////////////////
    /*!
      \brief calculate inertia movement vector
      \param initial_vel initial velocity vector
      \param n_step cycles to be estimated
      \return total travel vector
     */
    Vector2D inertiaTravel( const Vector2D & initial_vel,
                            const int n_step ) const
      {
          return inertia_n_step_travel( initial_vel, n_step, playerDecay() );
      }

    /*!
      \brief calculate reach point
      \param initial_pos initial point
      \param initial_vel initial velocity vector
      \param n_step cycles to be estimated
      \rturn the reached point
     */
    Vector2D inertiaPoint( const Vector2D& initial_pos,
                           const Vector2D& initial_vel,
                           const int n_step ) const
      {
          return inertia_n_step_point( initial_pos,
                                       initial_vel,
                                       n_step,
                                       playerDecay() );
      }

    /*!
      \brief calculate total ineartia movement vector
      \param initial_vel initial velocity vector
      \return total travel vector when plyer stops
     */
    Vector2D inertiaFinalTravel( const Vector2D & initial_vel ) const
      {
          return inertia_final_travel( initial_vel, playerDecay() );
      }

    /*!
      \brief calculate final reach point
      \param initial_pos initial position
      \param initial_vel initial velocity vector
      \return the reached point when player stops
     */
    Vector2D inertiaFinalPoint( const Vector2D & initial_pos,
                                const Vector2D & initial_vel ) const
      {
          return inertia_final_point( initial_pos, initial_vel, playerDecay() );
      }

    ////////////////////////////////////////////////
    /*!
      \brief normalize accel range when try to new dash(accel_mag, accel_angle)
      \param velocity current agent's velocity
      \param accel_angle accel angle -> agent's body angle or reversed body angle.
      \param accel_mag pointer to accel magnitude variable
      \return true if normalized, false otherwise.
    */
    bool normalizeAccel( const Vector2D & velocity,
                         const AngleDeg & accel_angle,
                         double * accel_mag ) const;

    /*!
      \brief normalize accel range when try to new dash(accel)
      \param velocity current agent's velocity
      \param accel new accel
      \return true if normalized, false otherwise
    */
    bool normalizeAccel( const Vector2D & velocity,
                         Vector2D * accel ) const;
    ////////////////////////////////////////////////
    /*!
      \brief predict agent's stamina and effort
      \param sparam server parameter
      \param n_wait number of wait cycles
      \param stamina pointer to stamina variable
      \param effort pointer to effort variable
      \param recovery current agent's recovery
    */
    void predictStaminaAfterWait( const ServerParam & sparam,
                                  const int n_wait,
                                  double * stamina,
                                  double * effort,
                                  const double & recovery ) const;

    /*!
      \brief predict agent's stamina related values after one dash
      \param sparam server parameter
      \param dash_power used dash power
      \param stamina pointer to stamina variable
      \param effort pointer to effort variable
      \param recovery pointer to recovery variable
    */
    void predictStaminaAfterDash( const ServerParam & sparam,
                                  const double & dash_power,
                                  double * stamina,
                                  double * effort,
                                  double * recovery ) const;

    /*!
      \brief predict stamina related values after nr dashes
      \param sparam server parameter
      \param dash_power used dash power
      \param n_dash number of dash cycles
      \param stamina pointer to stamina variable
      \param effort pointer to effort variable
      \param recovery pointer to recovery variable
     */
    void predictStaminaAfterNrDash( const ServerParam & sparam,
                                    const double & dash_power,
                                    const int n_dash,
                                    double * stamina,
                                    double * effort,
                                    double * recovery ) const;

    /*!
      \brief output parameters to stream
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & print( std::ostream & os ) const;
};


///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

/*!
  \class PlayerTypeSet
  \brief PlayerType parameter holder
*/
class PlayerTypeSet {
public:
    //! typedef of the player type contaier. key: id, value: player type
    typedef std::map< int, PlayerType > PlayerTypeMap;
private:
    //! map for hetero player id and parameter
    PlayerTypeMap M_player_type_map;

    //! dummy player type parameter
    PlayerType M_dummy_type;

    /*!
      \brief create dummy type. private access for singleton.
     */
    PlayerTypeSet();

public:
    /*!
      \brief destcut members
     */
    ~PlayerTypeSet();

    /*!
      \brief singleton interface. get player type set instance
      \return reference to the player type set instance
     */
    static
    PlayerTypeSet & instance();

    /*!
      \brief singleton interface. get player type set instance
      \return const reference to the player type set instance
     */
    inline
    static
    const
    PlayerTypeSet & i()
      {
          return instance();
      }

    /*!
      \brief regenerate default player type parameter using server param
      \param sparam const reference to the server parameter instance
     */
    void resetDefaultType( const ServerParam & sparam );

    /*!
      \brief add new player type parameter
      \param param const reference to the new parameter object
     */
    void insert( const PlayerType & param );

private:
    /*!
      \brief regenerate dummy player type parameter
      using the most effective parameter in existing parameters

      Generated player type parameter will be the fastest type
     */
    void createDummyType();

public:

    /*!
      \brief get player type map
      \return const reference to the player type map object
     */
    const
    PlayerTypeMap & playerTypeMap() const
      {
          return M_player_type_map;
      }

    /*!
      \brief get dummy type parameter
      \return const reference to the dummy player type paramaeter
     */
    const
    PlayerType & dummyType() const
      {
          return M_dummy_type;
      }

    /*!
      \brief get player type parameter that Id is id
      \param id wanted player type Id
      \return const pointer to the player type parameter object
     */
    const
    PlayerType * get( const int id ) const;

    /*!
      \brief put parameters to the output stream
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & print( std::ostream & os ) const;
};

}

#endif
