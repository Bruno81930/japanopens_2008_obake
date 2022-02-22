// -*-c++-*-

/*!
  \file stamina_model.cpp
  \brief player's stamina model Source File
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

#include "stamina_model.h"

#include <rcsc/common/server_param.h>
#include <rcsc/common/player_type.h>
#include <rcsc/game_time.h>

#include <algorithm>

namespace rcsc {

const double StaminaModel::DEFAULT_RECOVERY_MAX = 1.0;

/*-------------------------------------------------------------------*/
/*!

*/
StaminaModel:: StaminaModel()
    : M_stamina( 4000.0 )
    , M_recovery( 1.0 )
    , M_effort( 1.0 )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
void
StaminaModel::init( const double & stamina_max,
                    const double & effort_max )
{
    M_stamina = stamina_max;
    M_recovery = DEFAULT_RECOVERY_MAX;
    M_effort = effort_max;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
StaminaModel::update( const PlayerType & player_type,
                      const double & dashpower )
{
    // substitute dash power from stamina value
    if ( dashpower >= 0.0 )
    {
        M_stamina -= dashpower;
    }
    else
    {
        M_stamina -= dashpower * -2.0;
    }

    // update recovery value
    if ( M_stamina <= ServerParam::i().recoverDecThrValue() )
    {
        if ( M_recovery > ServerParam::i().recoverMin() )
        {
            M_recovery = std::max( ServerParam::i().recoverMin(),
                                   M_recovery - ServerParam::i().recoverDec() );
            std::cerr << "recover dec " << M_recovery << std::endl;
        }
    }

    // update effort value
    // !!! using HETERO PLAYER PARAMS !!!
    if ( M_stamina <= ServerParam::i().effortDecThrValue() )
    {
        if ( M_effort > player_type.effortMin() )
        {
            M_effort = std::max( player_type.effortMin(),
                                 M_effort - ServerParam::i().effortDec() );
        }
    }
    else if ( M_stamina >= ServerParam::i().effortIncThrValue() )
    {
        if ( M_effort < player_type.effortMax() )
        {
            M_effort = std::min( player_type.effortMax(),
                                 M_effort + ServerParam::i().effortInc() );
        }
    }

    // recover stamina value
    M_stamina = std::min( ServerParam::i().staminaMax(),
                          M_stamina + M_recovery * player_type.staminaIncMax() );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
StaminaModel::updateAfterSense( const PlayerType & player_type,
                                const double & sensed_stamina,
                                const double & sensed_effort,
                                const GameTime & current )
{
    // set directly
    M_stamina = sensed_stamina;
    M_effort = sensed_effort;

    // reset recover value, when new harf start
    if ( ServerParam::i().halfTime() > 0 // server setting is normal game mode
         && current.cycle() % ServerParam::i().halfTime() == 1 ) // just after start
    {
        int half_count = (current.cycle() / ServerParam::i().halfTime()) % 2;
        if ( half_count < 2 )
        {
            M_recovery = DEFAULT_RECOVERY_MAX;
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
StaminaModel::updateAfterFullstate( const double & stamina,
                                    const double & effort,
                                    const double & recovery )
{
    M_stamina = stamina;
    M_effort = effort;
    M_recovery = recovery;
}

}
