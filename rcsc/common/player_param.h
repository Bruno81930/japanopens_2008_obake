// -*-c++-*-

/*!
  \file player_param.h
  \brief player_param for rcssserver Header File
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

#ifndef RCSC_PARAM_PLAYER_PARAM_H
#define RCSC_PARAM_PLAYER_PARAM_H

#include <rcsc/rcg/types.h>

#include <boost/scoped_ptr.hpp>

namespace rcsc {

namespace rcg {
struct player_params_t;
}
class ParamMap;

/*!
  \class PlayerParam
  \brief trade-off parameters to generate PlayerType
 */
class PlayerParam {
private:

    //! parameter map implementation
    boost::scoped_ptr< ParamMap > M_param_map;

    static const int DEFAULT_PLAYER_TYPES; //!< default number of player types
    static const int DEFAULT_SUBS_MAX; //!< default max number of substitution oppotunity
    static const int DEFAULT_PT_MAX; //!< default max number of same player type at the same time

    static const bool DEFAULT_ALLOW_MULT_DEFAULT_TYPE; //!< default value for M_allow_mult_default_type

    static const double DEFAULT_PLAYER_SPEED_MAX_DELTA_MIN; //!< default tradeoff parameter. player speed max
    static const double DEFAULT_PLAYER_SPEED_MAX_DELTA_MAX; //!< default tradeoff parameter. player speed max
    static const double DEFAULT_STAMINA_INC_MAX_DELTA_FACTOR; //!< default tradeoff parameter. stamina inc max

    static const double DEFAULT_PLAYER_DECAY_DELTA_MIN; //!< default tradeoff parameter. player decay
    static const double DEFAULT_PLAYER_DECAY_DELTA_MAX; //!< default tradeoff parameter. player decay
    static const double DEFAULT_INERTIA_MOMENT_DELTA_FACTOR; //!< default tradeoff parameter. inertia moment

    static const double DEFAULT_DASH_POWER_RATE_DELTA_MIN; //!< default tradeoff parameter. dash power rate
    static const double DEFAULT_DASH_POWER_RATE_DELTA_MAX; //!< default tradeoff parameter. dash power rate
    static const double DEFAULT_PLAYER_SIZE_DELTA_FACTOR; //!< default tradeoff parameter. player size

    static const double DEFAULT_KICKABLE_MARGIN_DELTA_MIN; //!< default tradeoff parameter. kickable margin
    static const double DEFAULT_KICKABLE_MARGIN_DELTA_MAX; //!< default tradeoff parameter. kickable margin
    static const double DEFAULT_KICK_RAND_DELTA_FACTOR; //!< default tradeoff parameter. kick rand

    static const double DEFAULT_EXTRA_STAMINA_DELTA_MIN; //!< default tradeoff parameter. extra stamina
    static const double DEFAULT_EXTRA_STAMINA_DELTA_MAX; //!< default tradeoff parameter. extra stamina
    static const double DEFAULT_EFFORT_MAX_DELTA_FACTOR; //!< default tradeoff parameter. effort max
    static const double DEFAULT_EFFORT_MIN_DELTA_FACTOR; //!< default tradeoff parameter. effort min

    static const int    DEFAULT_RANDOM_SEED; //!< default random seed value (should be negative value)

    static const double DEFAULT_NEW_DASH_POWER_RATE_DELTA_MIN; //!< default tradeoff parameter. dash power rate
    static const double DEFAULT_NEW_DASH_POWER_RATE_DELTA_MAX; //!< default tradeoff parameter. dash power rate
    static const double DEFAULT_NEW_STAMINA_INC_MAX_DELTA_FACTOR; //!< default tradeoff parameter. stamina inc max

    int M_player_types; //!< the number of player types
    int M_subs_max; //!< max number of player substitution oppotunity
    int M_pt_max; //!< max number of same player type at the same time

    bool M_allow_mult_default_type; //!< if true, the number of default type player is not restricted.

    double M_player_speed_max_delta_min; //!< (obsolete) tradeoff parameter for player speed max
    double M_player_speed_max_delta_max; //!< (obsolete) tradeoff parameter for player speed max
    double M_stamina_inc_max_delta_factor; //!< (obsolete) tradeoff parameter for

    double M_player_decay_delta_min; //!< tradeoff parameter for player decay
    double M_player_decay_delta_max; //!< tradeoff parameter for player decay
    double M_inertia_moment_delta_factor; //!< tradeoff parameter for inertia moment

    double M_dash_power_rate_delta_min; //!< (obsolete) tradeoff parameter for dash power rate
    double M_dash_power_rate_delta_max; //!< (obsolete) tradeoff parameter for dash power rate
    double M_player_size_delta_factor; //!< (obsolete) tradeoff parameter for player size

    double M_kickable_margin_delta_min; //!< tradeoff parameter for kickable margin
    double M_kickable_margin_delta_max; //!< tradeoff parameter for kickable margin
    double M_kick_rand_delta_factor; //!< tradeoff parameter for kick rand

    double M_extra_stamina_delta_min; //!< tradeoff parameter for extra stamina
    double M_extra_stamina_delta_max; //!< tradeoff parameter for extra stamina
    double M_effort_max_delta_factor; //!< tradeoff parameter for effort max
    double M_effort_min_delta_factor; //!< tradeoff parameter for effort min

    int M_random_seed; //!< seed value to generate hetero parameters

    double M_new_dash_power_rate_delta_min; //!< tradeoff parameter for dash power rate
    double M_new_dash_power_rate_delta_max; //!< tradeoff parameter for dash power rate
    double M_new_stamina_inc_max_delta_factor; //!< tradeoff parameter for

    /*!
      \brief default constructor.

      set default value to variables
      and create parameter map for param parser.
     */
    PlayerParam();

public:
    /*!
      \brief destructor
     */
    ~PlayerParam();

    //! singleton interface
    static
    PlayerParam & instance();

    //! const singleton interface
    inline
    static const
    PlayerParam & i()
      {
          return instance();
      }

private:
    /*!
      \brief set default value to variables
     */
    void setDefaultParam();

    /*!
      \brief create parameter map for param parser
     */
    void createMap();

public:
    /*!
      \brief analyze server message
      \param msg raw message string from server
      \param version client version that defines message protocol
      \return result of parse status
     */
    bool parse( const char * msg,
                const double & version );

private:
    /*!
      \brief analyze version 7 protocol message
      \param msg raw message string from rcssserver
      \return result of parse status
     */
    bool parseV7( const char * msg );


public:
    /*!
      \brief convert parameters from monitor protcol data
      \param from monitor protocol data structure
     */
    void convertFrom( const rcg::player_params_t & from );

    /*!
      \brief convert parameters to monitor protcol data
      \param to reference to the data structure variable
     */
    void convertTo( rcg::player_params_t & to ) const;

    /*!
      \brief convert to the rcss parameter message
      \return parameter message string
     */
    std::string toStr() const;


    /*!
      \brief get the number of player types
      \return number of player types
     */
    int playerTypes() const
      {
          return M_player_types;
      }

    /*!
      \brief get the max number of substituion while play_on
      \return max number of substituion
     */
    int subsMax() const
      {
          return M_subs_max;
      }

    /*!
      \brief get the max number of same player types at the same time
      \return max number of same player type at the same time
     */
    int ptMax() const
      {
          return M_pt_max;
      }

    bool allowMultDefaultType() const
      {
          return M_allow_mult_default_type;
      }

    const
    double & playerSpeedMaxDeltaMin() const
      {
          return M_player_speed_max_delta_min;
      }
    const
    double & playerSpeedMaxDeltaMax() const
      {
          return M_player_speed_max_delta_max;
      }
    const
    double & staminaIncMaxDeltaFactor() const
      {
          return M_stamina_inc_max_delta_factor;
      }

    const
    double & playerDecayDeltaMin() const
      {
          return M_player_decay_delta_min;
      }
    const
    double & playerDecayDeltaMax() const
      {
          return M_player_decay_delta_max;
      }
    const
    double & inertiaMomentDeltaFactor() const
      {
          return M_inertia_moment_delta_factor;
      }

    const
    double & dashPowerRateDeltaMin() const
      {
          return M_dash_power_rate_delta_min;
      }
    const
    double & dashPowerRateDeltaMax() const
      {
          return M_dash_power_rate_delta_max;
      }
    const
    double & playerSizeDeltaFactor() const
      {
          return M_player_size_delta_factor;
      }

    const
    double & kickableMarginDeltaMin() const
      {
          return M_kickable_margin_delta_min;
      }
    const
    double & kickableMarginDeltaMax() const
      {
          return M_kickable_margin_delta_max;
      }
    const
    double & kickRandDeltaFactor() const
      {
          return M_kick_rand_delta_factor;
      }

    const
    double & extraStaminaDeltaMin() const
      {
          return M_extra_stamina_delta_min;
      }
    const
    double & extraStaminaDeltaMax() const
      {
          return M_extra_stamina_delta_max;
      }
    const
    double & effortMaxDeltaFactor() const
      {
          return M_effort_max_delta_factor;
      }
    const
    double & effortMinDeltaFactor() const
      {
          return M_effort_min_delta_factor;
      }

    int randomSeed() const
      {
          return M_random_seed;
      }

    const
    double & newDashPowerRateDeltaMin() const
      {
          return M_new_dash_power_rate_delta_min;
      }
    const
    double & newDashPowerRateDeltaMax() const
      {
          return M_new_dash_power_rate_delta_max;
      }
    const
    double & newStaminaIncMaxDeltaFactor() const
      {
          return M_new_stamina_inc_max_delta_factor;
      }
};

}

#endif
