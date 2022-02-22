// -*-c++-*-

/*!
  \file stamina_model.h
  \brief player's stamina model Header File
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

#ifndef RCSC_PLAYER_STAMINA_MODEL_H
#define RCSC_PLAYER_STAMINA_MODEL_H

namespace rcsc {

class PlayerType;
class GameTime;

/*!
  \class StaminaModel
  \brief stamina management class
*/
class StaminaModel {
private:
    //! simulator's default max recovery value
    static const double DEFAULT_RECOVERY_MAX;

    //! stamina value
    double M_stamina;
    //! recover rate
    double M_recovery;
    //! effort rate
    double M_effort;
public:
    /*!
      \brief init members by built-in values
    */
    StaminaModel();

    /*!
      \brief initialize internal variables with server parameters
      \param stamina_max maximal stamina parameter
      \param effort_max maxmal effort parameter

      initial recovery parameter is always DEFAULT_RECOVERY_MAX
    */
    void init( const double & stamina_max,
               const double & effort_max );

    /*!
      \brief get current stamina value
      \return const reference to the current stamina value
    */
    const
    double & stamina() const
      {
          return M_stamina;
      }

    /*!
      \brief get current recovery value
      \return const reference to the current recovery value
    */
    const
    double & recovery() const
      {
          return M_recovery;
      }

    /*!
      \brief get current effort value
      \return const reference to the current effort value
    */
    const
    double & effort() const
      {
          return M_effort;
      }

    /*!
      \brief update stamina related variables
      \param player_type PlayerType parameter
      \param dashpower previous dash command parameter

      update without sense_body message.
      this means that this method try to update variables
      only with internal values and estimated previous action effect,
      but we can update the stamina information very safety.
    */
    void update( const PlayerType & player_type,
                 const double & dashpower );

    /*!
      \brief update with sense_body message
      \param player_type PlayerType parameter
      \param sensed_stamina stamina value included in sense_body
      \param sensed_effort effort value included in sense_body
      \param current game time that sense_body was received
    */
    void updateAfterSense( const PlayerType & player_type,
                           const double & sensed_stamina,
                           const double & sensed_effort,
                           const GameTime & current );

    /*!
      \brief update with fullstate message
      \param stamina stamina value included in fullstate
      \param effort effort value included in fullstate
      \param recovery recovery value included in fullstate
     */
    void updateAfterFullstate( const double & stamina,
                               const double & effort,
                               const double & recovery );

};

}

#endif
