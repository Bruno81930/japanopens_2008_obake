// -*-c++-*-

/*!
  \file world_model.h
  \brief world model Header File
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

#ifndef RCSC_PLAYER_WORLD_MODEL_H
#define RCSC_PLAYER_WORLD_MODEL_H

#include <rcsc/player/self_object.h>
#include <rcsc/player/ball_object.h>
#include <rcsc/player/player_object.h>

#include <rcsc/geom/vector_2d.h>
#include <rcsc/game_mode.h>
#include <rcsc/game_time.h>
#include <rcsc/types.h>

#include <boost/shared_ptr.hpp>

#include <list>
#include <vector>
#include <string>

namespace rcsc {

class AudioMemory;
class ActionEffector;
class BodySensor;
class FullstateSensor;
class InterceptTable;
class Localization;
class PlayerPredicate;
class PlayerType;
class PenaltyKickState;
class VisualSensor;
struct ViewArea;

/*!
  \class WorldModel
  \brief player's internal field status
*/
class WorldModel {
public:

    enum {
        DIR_CONF_DIVS = 72
    };

    static const double DIR_STEP;

private:

    Localization * M_localize; //!< localization module
    InterceptTable * M_intercept_table; //!< interception info table
    boost::shared_ptr< AudioMemory > M_audio_memory; //!< heard info memory
    PenaltyKickState * M_penalty_kick_state; //!< penalty kick mode status

    //////////////////////////////////////////////////
    std::string M_teamname; //!< our teamname
    SideID M_our_side; //!< our side ID

    std::string M_opponent_teamname; //!< opponent teamname

    //////////////////////////////////////////////////
    // timer & mode

    GameTime M_time; //!< updated time
    GameTime M_sense_body_time; //!< sense_body updated time
    GameTime M_see_time; //!< see updated time
    GameTime M_fullstate_time; //!< fullstate update time

    GameTime M_last_set_play_start_time; //!< SetPlay started time
    long M_setplay_count; //!< setplay counter

    GameMode M_game_mode; //!< playmode and scores

    //////////////////////////////////////////////////
    // objects
    SelfObject M_self; //!< self object
    BallObject M_ball; //!< ball object
    // instance of each group
    PlayerCont M_teammates; //!< unum known teammmates
    PlayerCont M_opponents; //!< unum known opponents
    PlayerCont M_unknown_players; //!< unknown players

    //////////////////////////////////////////////////
    // objects reference (pointers to each object)
    //   these container is updated just before decide action
    //     or update_after_see
    //   pointers to each object
    //     updated in  void refreshPlayerMatrix();
    //! sorted by distance from self
    PlayerPtrCont M_teammates_from_self;
    //! sorted by distance from ball. include unknown players
    PlayerPtrCont M_opponents_from_self;
    //! sorted by distance from self
    PlayerPtrCont M_teammates_from_ball;
    //! sorted by distance from ball. include unknown players
    PlayerPtrCont M_opponents_from_ball;

    //! uniform number of opponent goalie
    int M_opponent_goalie_unum;

    AbstractPlayerCont M_all_players; //!< all players including self
    AbstractPlayerCont M_all_teammates; //!< all teammates including self
    AbstractPlayerCont M_all_opponents; //!< all opponents

    AbstractPlayerObject * M_known_teammates[12]; //!< unum known teammates (includes self)
    AbstractPlayerObject * M_known_opponents[12]; //!< unum known opponents (excludes unknown player)

    //////////////////////////////////////////////////
    // strategic value

    // updated in  updateOffsideLine();
    double M_offside_line_x; //!< offside line x value
    int M_offside_line_count; //!< accuracy count for the offside line

    double M_defense_line_x; //!< our defense line x value(their offside line)

    bool M_exist_kickable_teammate; //!< true if exist kickable teammate
    bool M_exist_kickable_opponent; //!< true if exist kickable opponent

    //////////////////////////////////////////////////
    // player type management

    int M_teammate_types[11]; //!< teammate type reference
    int M_opponent_types[11]; //!< opponent type flag

    //////////////////////////////////////////////////
    // visual info

    //! array of direction confidence count
    int M_dir_count[DIR_CONF_DIVS];

    //////////////////////////////////////////////////

    //! not used
    WorldModel( const WorldModel & );
    //! not used
    WorldModel & operator=( const WorldModel & );

public:
    /*!
      \brief initialize member variables
    */
    WorldModel();

    /*!
      \brief delete dynamic allocated memory
    */
    ~WorldModel();

    /*!
      \brief get intercept table
      \return const pointer to the intercept table instance
    */
    const
    InterceptTable * interceptTable() const;

    /*!
      \brief get penalty kick state
      \return const pointer to the penalty kick state instance
    */
    const
    PenaltyKickState * penaltyKickState() const;

    /*!
      \brief get audio memory
      \return const reference to the audio memory instance
     */
    const
    AudioMemory & audioMemory() const
      {
          return *M_audio_memory;
      }

    /*!
      \brief init team info
      \param teamname our team name string
      \param ourside our side ID
      \param my_unum my uniform number
      \param my_goalie true if I am goalie
      \return true if successfully initialized, false otherwise

      This method is called just after receive init reply
    */
    bool initTeamInfo( const std::string & teamname,
                       const SideID ourside,
                       const int my_unum,
                       const bool my_goalie );

    /*!
      \brief set new audio memory
      \param memory pointer to the memory instance. This must be
      a dynamically allocated object.
     */
    void setAudioMemory( boost::shared_ptr< AudioMemory > memory );

    /*!
      \brief set teammate player type
      \param unum uniform number of changed teammate
      \param id player type ID
    */
    void setTeammatePlayerType( const int unum,
                                const int id );

    /*!
      \brief set opponent player type
      \param unum uniform number of changed opponent
      \param id player type ID
    */
    void setOpponentPlayerType( const int unum,
                                const int id );

    // update stuff
private:
    /*!
      \brief internal update
      \param act action effector
      \param current current game time

      This method updates world status using recorded action effect only
    */
    void update( const ActionEffector & act,
                 const GameTime & current );
public:
    /*!
      \brief update by sense_body.
      \param sense sense_body info
      \param act action effector
      \param current current game time

      This method is called just after sense_body message receive
    */
    void updateAfterSense( const BodySensor & sense,
                           const ActionEffector & act,
                           const GameTime & current );

    /*!
      \brief update by see info
      \param see analyzed see info
      \param sense analyzed sense_body info
      \param act action effector
      \param current current game time

      This method is called just after see message receive
    */
    void updateAfterSee( const VisualSensor & see,
                         const BodySensor & sense,
                         const ActionEffector & act,
                         const GameTime & current );

    /*!
      \brief update by fullstate info
      \param fullstate analyzed fullstate info
      \param act action effector
      \param current current game time

      This method is called just after fullstate message receive
     */
    void updateAfterFullstate( const FullstateSensor & fullstate,
                               const ActionEffector & act,
                               const GameTime & current );

    /*!
      \brief update current playmode
      \param game_mode playmode info
      \param current current game time

      This method is called after heared referee message
    */
    void updateGameMode( const GameMode & game_mode,
                         const GameTime & current );
    /*!
      \brief update self view move
      \param w new view width
      \param q new view quality
    */
    void setViewMode( const ViewWidth & w,
                      const ViewQuality & q )
      {
          M_self.setViewMode( w, q );
      }

    /*!
      \brief internal update for action decision
      \param act action effector
      \param current current game time

      This method is called just before action decision to update and
      adjust world model.
    */
    void updateJustBeforeDecision( const ActionEffector & act,
                                   const GameTime & current );

    /*!
      \brief internal update using internal info and stored command info.
      \param act ActionEffector object

      called just before command send
    */
    void setCommandEffect( const ActionEffector & act );

private:
    /*!
      \brief self localization
      \param see analyzed see info
      \param current current game time
    */
    void localizeSelf( const VisualSensor & see,
                       const GameTime & current );

    /*!
      \brief ball localization
      \param see analyzed see info
      \param act action effector
      \param current current game time
    */
    void localizeBall( const VisualSensor & see,
                       const ActionEffector & act,
                       const GameTime & current );

    /*!
      \brief estimate ball velocity using position difference
      \param see analyzed see info
      \param act action effector
      \param current current game time
      \param current seen relative pos
      \param current seen relative pos error
      \param vel reference to the velocity variable
      \param vel_error reference to the velocity error variable
      \param vel_count reference to the velocity count variable
    */
    void estimateBallVelByPosDiff( const VisualSensor & see,
                                   const ActionEffector & act,
                                   const GameTime & current,
                                   const Vector2D & rpos,
                                   const Vector2D & rpos_error,
                                   Vector2D & vel,
                                   Vector2D & vel_error,
                                   int & vel_count );

    /*!
      \brief players localization
      \param see analyzed see info
      \param current current game time
    */
    void localizePlayers( const VisualSensor & see );

    /*!
      \brief check player that has team info
      \param side seen side info
      \param player localized info
      \param seen_dist see info data
      \param old_known_players old team known players
      \param old_unknown_players previous unknown players
      \param new_known_players new team known players
    */
    void checkTeamPlayer( const SideID side,
                          const Localization::PlayerT & player,
                          const double & seen_dist,
                          PlayerCont & old_known_players,
                          PlayerCont & old_unknown_players,
                          PlayerCont & new_known_players );

    /*!
      \brief check player that has no identifier. matching to unknown players
      \param player localized info
      \param seen_dist see info data
      \param old_teammates previous seen teammates
      \param old_opponents previous seen opponents
      \param old_unknown_players previous seen unknown player
      \param new_teammates current seen teammates
      \param new_opponents current seen opponents
      \param new_unknown_players current seen unknown players
    */
    void checkUnknownPlayer( const Localization::PlayerT & player,
                             const double & seen_dist,
                             PlayerCont & old_teammates,
                             PlayerCont & old_opponent,
                             PlayerCont & old_unknown_players,
                             PlayerCont & new_teammates,
                             PlayerCont & new_opponents,
                             PlayerCont & new_unknown_players );

    /*!
      \brief check collision.
    */
    void updateCollision();

    /*!
      \brief check ghost object
      \param varea current view area info
    */
    void checkGhost( const ViewArea & varea );

    /*!
      \brief update seen direction accuracy
      \param varea seen view area info
    */
    void updateDirCount( const ViewArea & varea );

    /*!
      \brief update opponent goalie by heard info
    */
    void updateGoalieByHear();

    /*!
      \brief update opponent goalie by heard info
    */
    void updatePlayerByHear();

    /*!
      \brief update player type id of the recognized players
     */
    void updatePlayerType();

    /*!
      \brief update object relation

      This method makes special player container
    */
    void updateObjectRelation();

    /*!
      \brief update player info. relation with ball and self.
     */
    void updatePlayerMatrix();

    /*!
      \brief update offside line
    */
    void updateOffsideLine();

    /*!
      \brief update defense line
    */
    void updateDefenseLine();

public:

    /*!
      \brief get our teamname
      \return const reference to the team name string
    */
    const
    std::string & teamName() const
      {
          return M_teamname;
      }

    /*!
      \brief get our team side Id
      \return side Id
    */
    SideID ourSide() const
      {
          return M_our_side;
      }

    /*!
      \brief get opponent teamname
      \return const reference to the team name string
    */
    const
    std::string & opponentTeamName() const
      {
          return M_opponent_teamname;
      }

    /*!
      \brief get opponent team side Id
      \return side Id
    */
    SideID theirSide() const
      {
          return M_our_side == LEFT ? RIGHT : LEFT;
      }

    /*!
      \brief check if our team is left or not
      \return true if our team is left side
    */
    bool isOurLeft() const
      {
          return M_our_side == LEFT;
      }

    /*!
      \brief check if our team is right or not
      \return true if our team is right side
    */
    bool isOurRight() const
      {
          return M_our_side == RIGHT;
      }

    /*!
      \brief get last updated time (== current game time)
      \return const reference to the game time object
    */
    const
    GameTime & time() const
      {
          return M_time;
      }

    /*!
      \brief get last time updated by see
      \return const reference to the game time object
    */
    const
    GameTime & seeTime() const
      {
          return M_see_time;
      }

    /*!
       \brief get last time updated by fullstate
       \return const reference to the game time object
     */
    const
    GameTime & fullstateTime() const
      {
          return M_fullstate_time;
      }

    /*!
      \brief get last time updated by fullstate
     */

    /*!
      \brief get last setplay type playmode start time
      \return const reference to the game time object
    */
    const
    GameTime & lastSetPlayStartTime() const
      {
          return M_last_set_play_start_time;
      }

    /*!
      \brief get cycle count that setplay type playmode is keeped
      \return counted long integer
    */
    const
    long & setplayCount() const
      {
          return M_setplay_count;
      }

    /*!
      \brief get current playmode info
      \return const reference to the GameMode object
    */
    const
    GameMode & gameMode() const
      {
          return M_game_mode;
      }

    /*!
      \brief get self info
      \return const reference to the SelfObject
    */
    const
    SelfObject & self() const
      {
          return M_self;
      }

    /*!
      \brief get ball info
      \return const reference to the BallObject
    */
    const
    BallObject & ball() const
      {
          return M_ball;
      }

    /*!
      \brief get teammate info
      \return const reference to the PlayerObject container
    */
    const
    PlayerCont & teammates() const
      {
          return M_teammates;
      }

    /*!
      \brief get opponent info
      \return const reference to the PlayerObject container
    */
    const
    PlayerCont & opponents() const
      {
          return M_opponents;
      }

    /*!
      \brief get unknown player info
      \return const reference to the PlayerObject container
    */
    const
    PlayerCont & unknownPlayers() const
      {
          return M_unknown_players;
      }

    // reference to the sorted players

    /*!
      \brief get teammates sorted by distance from self
      \return const reference to the PlayerObject pointer container
    */
    const
    PlayerPtrCont & teammatesFromSelf() const
      {
          return M_teammates_from_self;
      }

    /*!
      \brief get opponents sorted by distance from self
      \return const reference to the PlayerObject pointer container
    */
    const
    PlayerPtrCont & opponentsFromSelf() const
      {
          return M_opponents_from_self;
      }

    /*!
      \brief get teammates sorted by distance from ball
      \return const reference to the PlayerObject pointer container
    */
    const
    PlayerPtrCont & teammatesFromBall() const
      {
          return M_teammates_from_ball;
      }

    /*!
      \brief get opponents sorted by distance from ball
      \return const reference to the PlayerObject pointer container
    */
    const
    PlayerPtrCont & opponentsFromBall() const
      {
          return M_opponents_from_ball;
      }

    /*!
      \brief get the uniform number of opponent goalie
      \return uniform number value or Unum_Unknown
     */
    int opponentGoalieUnum() const
      {
          return M_opponent_goalie_unum;
      }

    /*!
      \brief get all players including self.
      \return const rerefence to the AbstractPlayerObject pointer container..
     */
    const
    AbstractPlayerCont & allPlayers() const
      {
          return M_all_players;
      }

    /*!
      \brief get all teammates including self.
      \return const rerefence to the AbstractPlayerObject pointer container..
     */
    const
    AbstractPlayerCont & allTeammates() const
      {
          return M_all_teammates;
      }

    /*!
      \brief get all opponents including self.
      \return const rerefence to the AbstractPlayerObject pointer container..
     */
    const
    AbstractPlayerCont & allOpponents() const
      {
          return M_all_opponents;
      }

    //////////////////////////////////////////////////////////

    /*!
      \brief get number specified teammate player object pointer
      \param unum wanted player's uniform number
      \return const pointer to the PlayerObject instance or NULL
    */
//     const
//     PlayerObject * getTeammate( const int unum ) const
//       {
//           const PlayerPtrCont::const_iterator end = teammatesFromSelf().end();
//           for ( PlayerPtrCont::const_iterator it = teammatesFromSelf().begin();
//                 it != end;
//                 ++it )
//           {
//               if ( (*it)->unum() == unum )
//               {
//                   return *it;
//               }
//           }
//           return static_cast< PlayerObject * >( 0 );
//       }
    const
    AbstractPlayerObject * teammate( const int unum ) const
      {
          if ( unum <= 0 || 11 < unum ) return M_known_teammates[0];
          return M_known_teammates[unum];
      }

    /*!
      \brief get number specified opponent player object pointer
      \param unum wanted player's uniform number
      \return const pointer to the PlayerObject instance or NULL
    */
//     const
//     PlayerObject * getOpponent( const int unum ) const
//       {
//           const PlayerPtrCont::const_iterator end = opponentsFromSelf().end();
//           for ( PlayerPtrCont::const_iterator it = opponentsFromSelf().begin();
//                 it != end;
//                 ++it )
//           {
//               if ( (*it)->unum() == unum )
//               {
//                   return *it;
//               }
//           }
//           return static_cast< PlayerObject * >( 0 );
//       }
    const
    AbstractPlayerObject * opponent( const int unum ) const
      {
          if ( unum <= 0 || 11 < unum ) return M_known_opponents[0];
          return M_known_opponents[unum];
      }

    /*!
      \brief get fist PlayerObject in [first, last] that satisfies confidence
      count threshold
      \param first first iterator of PlayerObject pointer container
      \param last last iterator of PlayerObject pointer container
      \param count_thr accuracy count threshold
      \return if found, const pointer to the PlayerOjbect. else NULL
    */
    const
    PlayerObject * getFirstPlayer( PlayerPtrCont::const_iterator first,
                                   PlayerPtrCont::const_iterator last,
                                   const int count_thr ) const
      {
          while ( first != last )
          {
              if ( ! (*first)->isGhost()
                   && (*first)->posCount() <= count_thr )
              {
                  return *first;
              }
              ++first;
          }
          return static_cast< PlayerObject * >( 0 );
      }

    /*!
      \brief get teammate nearest to self with confidence count check
      \param count_thr accuracy count threshold
      \return if found, const pointer to the PlayerOjbect. else NULL
    */
    const
    PlayerObject * getTeammateNearestToSelf( const int count_thr ) const
      {
          return getFirstPlayer( teammatesFromSelf().begin(),
                                 teammatesFromSelf().end(),
                                 count_thr );
      }

    /*!
      \brief get opponent nearest to self with accuracy count check
      \param count_thr accuracy count threshold
      \return if found, const pointer to the PlayerOjbect. else NULL
    */
    const
    PlayerObject * getOpponentNearestToSelf( const int count_thr ) const
      {
          return getFirstPlayer( opponentsFromSelf().begin(),
                                 opponentsFromSelf().end(),
                                 count_thr );
      }

    /*!
      \brief get the distance to teammate nearest to self wtth accuracy count
      \param count_thr accuracy count threshold
      \return distance to the matched opponent. if not found, a big value is returned.
     */
    double getDistTeammateNearestToSelf( const int count_thr ) const
      {
          const PlayerObject * p = getTeammateNearestToSelf( count_thr );
          return ( p ? p->distFromSelf() : 65535.0 );
      }

    /*!
      \brief get the distance to opponent nearest to self wtth accuracy count
      \param count_thr accuracy count threshold
      \return distance to the matched opponent. if not found, a big value is returned.
     */
    double getDistOpponentNearestToSelf( const int count_thr ) const
      {
          const PlayerObject * p = getOpponentNearestToSelf( count_thr );
          return ( p ? p->distFromSelf() : 65535.0 );
      }

    /*!
      \brief get teammate nearest to with with confidence count check
      \param count_thr accuracy count threshold
      \return if found, const pointer to the PlayerOjbect. else NULL
    */
    const
    PlayerObject * getTeammateNearestToBall( const int count_thr ) const
      {
          return getFirstPlayer( teammatesFromBall().begin(),
                                 teammatesFromBall().end(),
                                 count_thr );
      }

    /*!
      \brief get opponent nearest to ball with confidence count check
      \param count_thr accuracy count threshold
      \return if found, const pointer to the PlayerOjbect. else NULL
    */
    const
    PlayerObject * getOpponentNearestToBall( const int count_thr ) const
      {
          return getFirstPlayer( opponentsFromBall().begin(),
                                 opponentsFromBall().end(),
                                 count_thr );
      }

    /*!
      \brief get the distance to teammate nearest to ball wtth accuracy count
      \param count_thr accuracy count threshold
      \return distance to the matched opponent. if not found, a big value is returned.
     */
    double getDistTeammateNearestToBall( const int count_thr ) const
      {
          const PlayerObject * p = getTeammateNearestToSelf( count_thr );
          return ( p ? p->distFromBall() : 65535.0 );
      }

    /*!
      \brief get the distance to opponent nearest to ball wtth accuracy count
      \param count_thr accuracy count threshold
      \return distance to the matched opponent. if not found, a big value is returned.
     */
    double getDistOpponentNearestToBall( const int count_thr ) const
      {
          const PlayerObject * p = getOpponentNearestToBall( count_thr );
          return ( p ? p->distFromBall() : 65535.0 );
      }

    // tactical info

    /*!
      \brief get estimated offside line x coordinate
      \return real value
    */
    const
    double & offsideLineX() const
      {
          return M_offside_line_x;
      }

    /*!
      \brief get the accuracy count for the offside line
      \return accuracy count
     */
    int offsideLineCount() const
      {
          return M_offside_line_count;
      }

    /*!
      \brief get estimated defside line(offside line for opponent) x coordinate
      \return real value
    */
    const
    double & defenseLineX() const
      {
          return M_defense_line_x;
      }

    /*!
      \brief check if exist kickable teammate
      \return true if agent estimated that kickable teammate exists
    */
    bool existKickableTeammate() const
      {
          return M_exist_kickable_teammate;
      }

    /*!
      \brief check if exist kickable opponent
      \return true if agent estimated that kickable opponent exists
    */
    bool existKickableOpponent() const
      {
          return M_exist_kickable_opponent;
      }

    /*!
      \brief get player type Id of teammate
      \param unum target teammate uniform number
      \return player type Id. if unum is illegal, Default Id is returned.
    */
    int teammateHeteroID( const int unum ) const
      {
          if ( unum < 1 || 11 < unum )
          {
              std::cerr << "WorldModel::teammateHeteroID. Illegal unum "
                        << unum << std::endl;
              return Hetero_Default;
          }
          return M_teammate_types[ unum - 1 ];
      }

    /*!
      \brief get player type of the specified teammate
      \param unum target teammate uniform number
      \return const pointer to the player type object instance
     */
    const
    PlayerType * teammatePlayerType( const int unum ) const;

    /*!
      \brief get player type Id of opponent
      \param unum target teammate uniform number
      \return player type Id. if unum is illegal, Unknown is returned.
    */
    int opponentHeteroID( const int unum ) const
      {
          if ( unum < 1 || 11 < unum )
          {
              std::cerr << "WorldModel::opponentHeteroID. Illegal unum "
                        << unum << std::endl;
              return Hetero_Unknown;
          }
          return M_opponent_types[ unum - 1 ];
      }

    /*!
      \brief get player type of the specified opponent
      \param unum target opponent uniform number
      \return const pointer to the player type object instance
     */
    const
    PlayerType * opponentPlayerType( const int unum ) const;

    // visual memory info

    /*!
      \brief get direction confidence count
      \param angle target direction
      \return confidence count value
    */
    int dirCount( const AngleDeg & angle ) const
      {
          int idx = static_cast< int >( ( angle.degree() - 0.5 + 180.0 )
                                        / DIR_STEP );
          if ( idx < 0 || DIR_CONF_DIVS - 1 < idx )
          {
              std::cerr << "WorldModel::getDirConf. index over flow"
                        << std::endl;
              idx = 0;
          }
          return M_dir_count[idx];
      }

    /*!
      \brief get max count, sum of count and average count of angle range
      \param angle center of target angle range
      \param width angle range
      \param max_count pointer to variable of max accuracy count
      \param sum_count pointer to variable of sum of accuracy count
      \param ave_count pointer to variable of average accuracy count
      \return steps in the range
    */
    int dirRangeCount( const AngleDeg & angle,
                       const double & width,
                       int * max_count,
                       int * sum_count,
                       int * ave_count ) const;

    //
    // interfaces to player objects
    //

    /*!
      \brief get the new container of AbstractPlayer matched with the predicate.
      \param predicate predicate object for the player condition matching. This have to be a dynamically allocated object.
      \return container of AbstractPlayer pointer.
     */
    AbstractPlayerCont getPlayerCont( const PlayerPredicate * predicate ) const;

    /*!
      \brief get the new container of AbstractPlayer matched with the predicate.
      \param cont reference to the result variable
      \param predicate predicate object for the player condition matching. This have to be a dynamically allocated object.
     */
    void getPlayerCont( AbstractPlayerCont & cont,
                        const PlayerPredicate * predicate ) const;


    /*!
      \brief get opponent goalie pointer
      \return if found pointer to goalie object, othewise NULL
    */
    const
    PlayerObject * getOpponentGoalie() const;

    /*!
      \brief get teammate pointer nearest to point
      \param point considered point
      \param count_thr cinfidence count threshold
      \param dist_to_point variable pointer to store the distance
      from retuned player to point
      \return if found, pointer to player object, othewise NULL
    */
    const
    PlayerObject * getTeammateNearestTo( const Vector2D & point,
                                         const int count_thr,
                                         double * dist_to_point ) const;

    /*!
      \brief get teammate pointer nearest to the specified player
      \param p pointer to the player
      \param count_thr accuracy count threshold
      \param dist_to_point variable pointer to store the distance
      from retuned player to point
      \return if found, pointer to player object, othewise NULL
     */
    const
    PlayerObject * getTeammateNearestTo( const PlayerObject * p,
                                         const int count_thr,
                                         double * dist_to_point ) const
      {
          if ( ! p ) return static_cast< const PlayerObject * >( 0 );
          return getTeammateNearestTo( p->pos(), count_thr, dist_to_point );
      }

    /*!
      \brief get opponent pointer nearest to point
      \param point considered point
      \param count_thr accuracy count threshold
      \param dist_to_point variable pointer to store the distance
      from retuned player to point
      \return if found pointer to player object, othewise NULL
    */
    const
    PlayerObject * getOpponentNearestTo( const Vector2D & point,
                                         const int count_thr,
                                         double * dist_to_point ) const;


    /*!
      \brief get teammate pointer nearest to the specified player
      \param p pointer to the player
      \param count_thr accuracy count threshold
      \param dist_to_point variable pointer to store the distance
      from retuned player to point
      \return if found, pointer to player object, othewise NULL
     */
    const
    PlayerObject * getOpponentNearestTo( const PlayerObject * p,
                                         const int count_thr,
                                         double * dist_to_point ) const
      {
          if ( ! p ) return static_cast< const PlayerObject * >( 0 );
          return getOpponentNearestTo( p->pos(), count_thr, dist_to_point );
      }

    /*!
      \brief template utility. check if teammate exist in the specified region.
      \param region template parameter. region to be checked
      \param count_thr confdence count threshold for players
      \param with_goalie if true, goalie player is cheked.
     */
    template < typename REGION >
    bool existTeammateIn( const REGION & region,
                          const int count_thr,
                          const bool with_goalie ) const
      {
          const PlayerPtrCont::const_iterator end = teammatesFromSelf().end();
          for ( PlayerPtrCont::const_iterator it = teammatesFromSelf().begin();
                it != end;
                ++it )
          {
              if ( (*it)->posCount() > count_thr
                   || (*it)->isGhost() )
              {
                  continue;
              }
              if ( (*it)->goalie() && ! with_goalie )
              {
                  continue;
              }
              if ( region.contains( (*it)->pos() ) )
              {
                  return true;
              }
          }
          return false;
      }

    /*!
      \brief template utility. check if opponent exist in the specified region.
      \param region template parameter. region to be checked
      \param count_thr confdence count threshold for players
      \param with_goalie if true, goalie player is cheked.
      \return true if some opponent exist
     */
    template < typename REGION >
    bool existOpponentIn( const REGION & region,
                          const int count_thr,
                          const bool with_goalie ) const
      {
          const PlayerPtrCont::const_iterator end = opponentsFromSelf().end();
          for ( PlayerPtrCont::const_iterator it = opponentsFromSelf().begin();
                it != end;
                ++it )
          {
              if ( (*it)->posCount() > count_thr
                   || (*it)->isGhost() )
              {
                  continue;
              }
              if ( (*it)->goalie() && ! with_goalie )
              {
                  continue;
              }
              if ( region.contains( (*it)->pos() ) )
              {
                  return true;
              }
          }
          return false;
      }

    /*!
      \brief count the number of teammate exist in the specified region
      \param region template parameter. region to be checked
      \param count_thr confdence count threshold for players
      \param with_goalie if true, goalie player is cheked.
      \return total count of teammate existed in the region
     */
    template < typename REGION >
    int countTeammatesIn( const REGION & region,
                          const int count_thr,
                          const bool with_goalie ) const
      {
          int count = 0;
          const PlayerPtrCont::const_iterator end = teammatesFromSelf().end();
          for ( PlayerPtrCont::const_iterator it = teammatesFromSelf().begin();
                it != end;
                ++it )
          {
              if ( (*it)->posCount() > count_thr
                   || (*it)->isGhost()
                   || ( (*it)->goalie() && ! with_goalie )
                   )
              {
                  continue;
              }
              if ( region.contains( (*it)->pos() ) )
              {
                  ++count;
              }
          }
          return count;
      }

    /*!
      \brief count the number of opponent exist in the specified region
      \param region template parameter. region to be checked
      \param count_thr confdence count threshold for players
      \param with_goalie if true, goalie player is cheked.
      \return total count of opponent existed in the region
     */
    template < typename REGION >
    int countOpponentsIn( const REGION & region,
                          const int count_thr,
                          const bool with_goalie ) const
      {
          int count = 0;
          const PlayerPtrCont::const_iterator end = opponentsFromSelf().end();
          for ( PlayerPtrCont::const_iterator it = opponentsFromSelf().begin();
                it != end;
                ++it )
          {
              if ( (*it)->posCount() > count_thr
                   || (*it)->isGhost()
                   || ( (*it)->goalie() && ! with_goalie )
                   )
              {
                  continue;
              }
              if ( region.contains( (*it)->pos() ) )
              {
                  ++count;
              }
          }
          return count;
      }

};

}

#endif
