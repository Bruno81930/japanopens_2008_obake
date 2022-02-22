// -*-c++-*-

/*!
  \file player_predicate.h
  \brief player predicate classes Header File
*/

/*
 *Copyright:

 Copyright (C) Hiroki SHIMORA, Hidehisa AKIYAMA

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


#ifndef	RCSC_PLAYER_PLAYER_PREDICATE_H
#define	RCSC_PLAYER_PLAYER_PREDICATE_H

#include <rcsc/player/abstract_player_object.h>
#include <rcsc/player/world_model.h>
#include <rcsc/math_util.h>

#include <boost/shared_ptr.hpp>

#include <vector>
#include <algorithm>
#include <cmath>

namespace rcsc {

/*!
  \class PlayerPredicate
  \brief abstract predicate class for player matching
*/
class PlayerPredicate {
protected:
    /*!
      \brief protected constructor
     */
    PlayerPredicate()
      { }

public:
    /*!
      \brief virtual destructor
     */
    virtual
    ~PlayerPredicate()
      { }

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return true if target player is matched to the condition.
     */
    virtual
    bool operator()( const AbstractPlayerObject & p ) const = 0;
};

/*!
  \class SelfPlayerPredicate
  \brief check if target player is self or not
*/
class SelfPlayerPredicate
    : public PlayerPredicate {
private:
    //! const rererence to the WorldModel instance
    const WorldModel & M_world;

    // not used
    SelfPlayerPredicate();
public:
    /*!
      \brief construct with the WorldModel instance
      \param wm const reference to the WorldModel instance
     */
    explicit
    SelfPlayerPredicate( const WorldModel & wm )
        : M_world( wm )
      { }

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return true if target player is agent itself
     */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          return p.side() == M_world.self().side()
              && p.unum() == M_world.self().unum();
      }
};

/*!
  \class TeammateOrSelfPlayerPredicate
  \brief check if target player is teammate (include self) or not
 */
class TeammateOrSelfPlayerPredicate
    : public PlayerPredicate {
private:
    //! const rererence to the WorldModel instance
    const WorldModel & M_world;

    // not used
    TeammateOrSelfPlayerPredicate();
public:
    /*!
      \brief construct with the WorldModel instance
      \param wm const reference to the WorldModel instance
     */
    explicit
    TeammateOrSelfPlayerPredicate( const WorldModel & wm )
        : M_world( wm )
      { }

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return true if target player is teammate (include self)
     */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          return p.side() == M_world.self().side();
      }
};

/*!
  \class TeammatePlayerPredicate
  \brief check if target player is teammate (not include self) or not
 */
class TeammatePlayerPredicate
    : public PlayerPredicate {
private:
    //! const rererence to the WorldModel instance
    const WorldModel & M_world;

    // not used
    TeammatePlayerPredicate();
public:
    /*!
      \brief construct with the WorldModel instance
      \param wm const reference to the WorldModel instance
     */
    explicit
    TeammatePlayerPredicate( const WorldModel & wm )
        : M_world( wm )
      { }

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return true if target player is teammate (not include self)
     */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          return p.side() == M_world.self().side()
              && p.unum() != M_world.self().unum();
      }
};

/*!
  \class OpponentPlayerPredicate
  \brief check if target player is opponent (not include unknown player) or not

  XXX: OpponentTeamPlayerPredicate?
*/
class OpponentPlayerPredicate
    : public PlayerPredicate {
private:
    //! const rererence to the WorldModel instance
    const WorldModel & M_world;

    // not used
    OpponentPlayerPredicate();
public:
    /*!
      \brief construct with the WorldModel instance
      \param wm const reference to the WorldModel instance
     */
    explicit
    OpponentPlayerPredicate( const WorldModel & wm )
        : M_world( wm )
      { }

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return true if target player is opponent (not include unknown player)
     */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          return p.side() != M_world.self().side()
              && p.side() != NEUTRAL;
      }
};

/*!
  \class OpponentOrUnknownPlayerPredicate
  \brief check if target player is opponent (include unknown player) or not
*/
class OpponentOrUnknownPlayerPredicate
    : public PlayerPredicate {
private:
    //! const rererence to the WorldModel instance
    const WorldModel & M_world;

    // not used
    OpponentOrUnknownPlayerPredicate();
public:
    /*!
      \brief construct with the WorldModel instance
      \param wm const reference to the WorldModel instance
     */
    explicit
    OpponentOrUnknownPlayerPredicate( const WorldModel & wm )
        : M_world( wm )
      { }

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return true if target player is opponent (include unknown player)
     */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          return p.side() != M_world.self().side();
      }
};

/*!
  \class GoaliePlayerPredicate
  \brief check if target player is goalie or not
*/
class GoaliePlayerPredicate
    : public PlayerPredicate {
public:

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return true if target player is goalie
     */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          return p.goalie();
      }
};

/*!
  \class FieldPlayerPredicate
  \brief check if target player is field player or not
*/
class FieldPlayerPlayerPredicate
    : public PlayerPredicate {
public:

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return true if target player is not goalie
     */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          return ! p.goalie();
      }
};

/*!
  \class CoordinateAccuratePlayerPredicate
  \brief check if target player's positional information has enough accuracy.
*/
class CoordinateAccuratePlayerPredicate
    : public PlayerPredicate {
private:
    //! threshold accuracy value
    const int M_threshold;

    // not used
    CoordinateAccuratePlayerPredicate();
public:
    /*!
      \brief construct with threshold value
      \param threshold accuracy threshold value
     */
    explicit
    CoordinateAccuratePlayerPredicate( const int threshold )
        : M_threshold( threshold )
      { }

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return true if target player's posCount() is less than equal threshold
     */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          return p.posCount() <= M_threshold;
      }
};

/*!
  \class XCoordinateForwardPlayerPredicate
  \brief check if target player's x coordinate is greater(forwarder) than threshold value
 */
class XCoordinateForwardPlayerPredicate
    : public PlayerPredicate {
private:
    //! threshold x-coordinate value
    const double M_threshold;

    // not used
    XCoordinateForwardPlayerPredicate();
public:
    /*!
      \brief construct with threshold value
      \param threshold x-coordinate threshold value
     */
    explicit
    XCoordinateForwardPlayerPredicate( const double & threshold )
        : M_threshold( threshold )
      { }

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return true if target player's pos(.x) is greater(forwarder) than equal threshold
     */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          return p.pos().x >= M_threshold;
      }
};

/*!
  \class XCoordinateBackwardPlayerPredicate
  \brief check if target player's x coordinate is less(backwarder) than threshold value
 */
class XCoordinateBackwardPlayerPredicate
    : public PlayerPredicate {
private:
    //! threshold x-coordinate value
    const double M_threshold;

    // not used
    XCoordinateBackwardPlayerPredicate();
public:
    /*!
      \brief construct with threshold value
      \param threshold x-coordinate threshold value
     */
    explicit
    XCoordinateBackwardPlayerPredicate( const double & threshold )
        : M_threshold( threshold )
      { }

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return true if target player's pos().x is less(backwarder) than equal threshold
     */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          return p.pos().x <= M_threshold;
      }
};

/*!
  \class YCoordinatePlusPlayerPredicate
  \brief check if target player's y coordinate is more right than threshold value
 */
class YCoordinatePlusPlayerPredicate
    : public PlayerPredicate {
private:
    //! threshold y-coordinate value
    const double M_threshold;

    // not used
    YCoordinatePlusPlayerPredicate();
public:
    /*!
      \brief construct with threshold value
      \param threshold y-coordinate threshold value
     */
    explicit
    YCoordinatePlusPlayerPredicate( const double & threshold )
        : M_threshold( threshold )
      { }

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return true if target player's pos().y is more right than equal threshold
     */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          return p.pos().y >= M_threshold;
      }
};

/*!
  \class YCoordinateMinusPlayerPredicate
  \brief check if target player's y coordinate is more left than threshold value
 */
class YCoordinateMinusPlayerPredicate
    : public PlayerPredicate {
private:
    //! threshold y-coordinate value
    const double M_threshold;

    // not used
    YCoordinateMinusPlayerPredicate();
public:
    /*!
      \brief construct with threshold value
      \param threshold y-coordinate threshold value
     */
    explicit
    YCoordinateMinusPlayerPredicate( const double & threshold )
        : M_threshold( threshold )
      { }

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return true if target player's pos().y is more left than equal threshold
     */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          return p.pos().y <= M_threshold;
      }
};

/*!
  \class PointFarPlayerPredicate
  \brief check if target player's distance from tha base point is greater than threshold distance
 */
class PointFarPlayerPredicate
    : public PlayerPredicate {
private:
    //! base point
    const Vector2D M_base_point;
    //! squared threshold distance
    const double M_threshold2;

    // not used
    PointFarPlayerPredicate();
public:
    /*!
      \brief construct with base point and threshold distance
      \param base_point base point
      \param threshold distance threshold value
     */
    PointFarPlayerPredicate( const Vector2D & base_point,
                             const double & threshold )
        : M_base_point( base_point )
        , M_threshold2( threshold * threshold )
      { }

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return true if target player's distance from base_point is greater than equal threshold
     */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          return ( p.pos() - M_base_point ).r2() >= M_threshold2;
      }
};

/*!
  \class PointNearPlayerPredicate
  \brief check if target player's distance from tha base point is less than threshold distance
 */
class PointNearPlayerPredicate
    : public PlayerPredicate {
private:
    //! base point
    const Vector2D M_base_point;
    //! squared threshold distance
    const double M_threshold2;

    // not used
    PointNearPlayerPredicate();
public:
    /*!
      \brief construct with base point and threshold distance
      \param base_point base point
      \param threshold distance threshold value
     */
    PointNearPlayerPredicate( const Vector2D & base_point,
                              const double & threshold )
        : M_base_point( base_point )
        , M_threshold2( threshold * threshold )
      { }

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return true if target player's distance from base_point is less than equal threshold
     */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          return ( p.pos() - M_base_point ).r2() <= M_threshold2;
      }
};

/*!
  \class AbsAngleDiffLessPlayerPredicate
  \brief check if target player's absolute angle difference from base angle is less than threshold angle
 */
class AbsAngleDiffLessPlayerPredicate
    : public PlayerPredicate {
private:
    //! base point
    const Vector2D M_base_point;
    //! compared angle
    const AngleDeg M_base_angle;
    //! angle threshold value (degree)
    const double M_threshold;

    // not used
    AbsAngleDiffLessPlayerPredicate();
public:
    /*!
      \brief construct with base point and threshold distance
      \param base_point base point
      \param base_angle compared angle
      \param degree_threshold angle threshold value (degree)
     */
    AbsAngleDiffLessPlayerPredicate( const Vector2D & base_point,
                                     const AngleDeg & base_angle,
                                     const double & degree_threshold )
        : M_base_point( base_point )
        , M_base_angle( base_angle ),
          M_threshold( std::fabs( degree_threshold ) )
      { }

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return true if target player's absolute angle difference from base_angle is less than equal threshold
     */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          return ( ( p.pos() - M_base_point ).th() - M_base_angle ).abs() <= M_threshold;
      }
};

/*!
  \class AbsAngleDiffGreaterPlayerPredicate
  \brief check if target player's absolute angle difference from base angle is greater than threshold angle
 */
class AbsAngleDiffGreaterPlayerPredicate
    : public PlayerPredicate {
private:
    //! base point
    const Vector2D M_base_point;
    //! compared angle
    const AngleDeg M_base_angle;
    //! angle threshold value (degree)
    const double M_threshold;

    // not used
    AbsAngleDiffGreaterPlayerPredicate();
public:
    /*!
      \brief construct with base point and threshold distance
      \param base_point base point
      \param base_angle compared angle
      \param threshold angle threshold value (degree)
     */
    AbsAngleDiffGreaterPlayerPredicate( const Vector2D & base_point,
                                        const AngleDeg & base_angle,
                                        const double & threshold )
        : M_base_point( base_point )
        , M_base_angle( base_angle )
        , M_threshold( std::fabs( threshold ) )
      { }

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return true if target player's absolute angle difference from base_angle is greater than equal threshold
     */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          return ( ( p.pos() - M_base_point ).th() - M_base_angle ).abs() >= M_threshold;
      }
};

/*!
  \class OffsidePositionPlayerPredicate
  \brief check if target player is in offside area
 */
class OffsidePositionPlayerPredicate
    : public PlayerPredicate {
private:
    //! const reference to the WorldModel instance
    const WorldModel & M_world;

    // not used
    OffsidePositionPlayerPredicate();
public:
    /*!
      \brief construct with the WorldModel instance
      \param wm const reference to the WorldModel instance
     */
    OffsidePositionPlayerPredicate( const WorldModel & wm )
        : M_world( wm )
      { }

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return true if target player is in offside area. if target player is unknown, false is always returned.
     */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          if ( p.side() == M_world.self().side() )
          {
              return p.pos().x > M_world.offsideLineX();
          }
          else if ( p.side() == NEUTRAL )
          {
              return false;
          }
          else
          {
              return p.pos().x < bound( M_world.ball().pos().x, M_world.defenseLineX(), 0.0 );
          }
      }
};

/*!
  \class AndPlayerPredicate
  \brief composite logical "and" predicate
*/
class AndPlayerPredicate
    : public PlayerPredicate {
private:
    //! the set of predicate
    std::vector< boost::shared_ptr< const PlayerPredicate > > M_predicates;

    // not used
    AndPlayerPredicate();
public:
    // XXX: implicit_shared_ptr

    /*!
      \brief construct with 2 predicates. all arguments must be a dynamically allocated object.
      \param p1 1st predicate
      \param p2 2nd predicate
     */
    AndPlayerPredicate( const PlayerPredicate * p1 ,
                        const PlayerPredicate * p2 )
        : M_predicates()
      {
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p1 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p2 ) );
      }

    /*!
      \brief construct with 3 predicates. all arguments must be a dynamically allocated object.
      \param p1 1st predicate
      \param p2 2nd predicate
      \param p3 3rd predicate
     */
    AndPlayerPredicate( const PlayerPredicate * p1 ,
                        const PlayerPredicate * p2 ,
                        const PlayerPredicate * p3 )
        : M_predicates()
      {
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p1 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p2 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p3 ) );
      }

    /*!
      \brief construct with 4 predicates. all arguments must be a dynamically allocated object.
      \param p1 1st predicate
      \param p2 2nd predicate
      \param p3 3rd predicate
      \param p4 4th predicate
     */
    AndPlayerPredicate( const PlayerPredicate * p1 ,
                        const PlayerPredicate * p2 ,
                        const PlayerPredicate * p3 ,
                        const PlayerPredicate * p4 )
        : M_predicates()
      {
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p1 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p2 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p3 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p4 ) );
      }

    /*!
      \brief construct with 5 predicates. all arguments must be a dynamically allocated object.
      \param p1 1st predicate
      \param p2 2nd predicate
      \param p3 3rd predicate
      \param p4 4th predicate
      \param p5 5th predicate
     */
    AndPlayerPredicate( const PlayerPredicate * p1 ,
                        const PlayerPredicate * p2 ,
                        const PlayerPredicate * p3 ,
                        const PlayerPredicate * p4 ,
                        const PlayerPredicate * p5 )
        : M_predicates()
      {
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p1 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p2 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p3 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p4 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p5 ) );
      }

    /*!
      \brief construct with 6 predicates. all arguments must be a dynamically allocated object.
      \param p1 1st predicate
      \param p2 2nd predicate
      \param p3 3rd predicate
      \param p4 4th predicate
      \param p5 5th predicate
      \param p6 6th predicate
     */
    AndPlayerPredicate( const PlayerPredicate * p1 ,
                        const PlayerPredicate * p2 ,
                        const PlayerPredicate * p3 ,
                        const PlayerPredicate * p4 ,
                        const PlayerPredicate * p5 ,
                        const PlayerPredicate * p6 )
        : M_predicates()
      {
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p1 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p2 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p3 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p4 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p5 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p6 ) );
      }

    /*!
      \brief construct with 7 predicates. all arguments must be a dynamically allocated object.
      \param p1 1st predicate
      \param p2 2nd predicate
      \param p3 3rd predicate
      \param p4 4th predicate
      \param p5 5th predicate
      \param p6 6th predicate
      \param p7 7th predicate
     */
    AndPlayerPredicate( const PlayerPredicate * p1 ,
                        const PlayerPredicate * p2 ,
                        const PlayerPredicate * p3 ,
                        const PlayerPredicate * p4 ,
                        const PlayerPredicate * p5 ,
                        const PlayerPredicate * p6 ,
                        const PlayerPredicate * p7 )
        : M_predicates()
      {
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p1 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p2 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p3 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p4 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p5 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p6 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p7 ) );
      }

    /*!
      \brief construct with 8 predicates. all arguments must be a dynamically allocated object.
      \param p1 1st predicate
      \param p2 2nd predicate
      \param p3 3rd predicate
      \param p4 4th predicate
      \param p5 5th predicate
      \param p6 6th predicate
      \param p7 7th predicate
      \param p8 8th predicate
     */
    AndPlayerPredicate( const PlayerPredicate * p1 ,
                        const PlayerPredicate * p2 ,
                        const PlayerPredicate * p3 ,
                        const PlayerPredicate * p4 ,
                        const PlayerPredicate * p5 ,
                        const PlayerPredicate * p6 ,
                        const PlayerPredicate * p7 ,
                        const PlayerPredicate * p8 )
        : M_predicates()
      {
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p1 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p2 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p3 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p4 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p5 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p6 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p7 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p8 ) );
      }

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return the result of "and" operation of all predicates
     */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          const std::vector< boost::shared_ptr< const PlayerPredicate > >::const_iterator
              p_end = M_predicates.end();

          for ( std::vector< boost::shared_ptr< const PlayerPredicate > >::const_iterator
                    it = M_predicates.begin();
                it != p_end;
                ++it )
          {
              if ( ! (**it)( p ) )
              {
                  return false;
              }
          }

          return true;
      }
};

/*!
  \class OrPlayerPredicate
  \brief composite logical "or" predicate
*/
class OrPlayerPredicate
    : public PlayerPredicate {
private:
    //! the set of predicate
    std::vector< boost::shared_ptr< const PlayerPredicate > > M_predicates;

    // not used
    OrPlayerPredicate();
public:
    // XXX: implicit_shared_ptr

    /*!
      \brief construct with 2 predicates. all arguments must be a dynamically allocated object.
      \param p1 1st predicate
      \param p2 2nd predicate
     */
    OrPlayerPredicate( const PlayerPredicate * p1 ,
                       const PlayerPredicate * p2 )
        : M_predicates()
      {
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p1 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p2 ) );
      }

    /*!
      \brief construct with 3 predicates. all arguments must be a dynamically allocated object.
      \param p1 1st predicate
      \param p2 2nd predicate
      \param p3 3rd predicate
     */
    OrPlayerPredicate( const PlayerPredicate * p1 ,
                       const PlayerPredicate * p2 ,
                       const PlayerPredicate * p3 )
        : M_predicates()
      {
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p1 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p2 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p3 ) );
      }

    /*!
      \brief construct with 4 predicates. all arguments must be a dynamically allocated object.
      \param p1 1st predicate
      \param p2 2nd predicate
      \param p3 3rd predicate
      \param p4 4th predicate
     */
    OrPlayerPredicate( const PlayerPredicate * p1 ,
                       const PlayerPredicate * p2 ,
                       const PlayerPredicate * p3 ,
                       const PlayerPredicate * p4 )
        : M_predicates()
      {
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p1 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p2 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p3 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p4 ) );
      }

    /*!
      \brief construct with 5 predicates. all arguments must be a dynamically allocated object.
      \param p1 1st predicate
      \param p2 2nd predicate
      \param p3 3rd predicate
      \param p4 4th predicate
      \param p5 5th predicate
     */
    OrPlayerPredicate( const PlayerPredicate * p1 ,
                       const PlayerPredicate * p2 ,
                       const PlayerPredicate * p3 ,
                       const PlayerPredicate * p4 ,
                       const PlayerPredicate * p5 )
        : M_predicates()
      {
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p1 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p2 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p3 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p4 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p5 ) );
      }

    /*!
      \brief construct with 6 predicates. all arguments must be a dynamically allocated object.
      \param p1 1st predicate
      \param p2 2nd predicate
      \param p3 3rd predicate
      \param p4 4th predicate
      \param p5 5th predicate
      \param p6 6th predicate
     */
    OrPlayerPredicate( const PlayerPredicate * p1 ,
                       const PlayerPredicate * p2 ,
                       const PlayerPredicate * p3 ,
                       const PlayerPredicate * p4 ,
                       const PlayerPredicate * p5 ,
                       const PlayerPredicate * p6 )
        : M_predicates()
      {
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p1 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p2 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p3 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p4 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p5 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p6 ) );
      }

    /*!
      \brief construct with 7 predicates. all arguments must be a dynamically allocated object.
      \param p1 1st predicate
      \param p2 2nd predicate
      \param p3 3rd predicate
      \param p4 4th predicate
      \param p5 5th predicate
      \param p6 6th predicate
      \param p7 7th predicate
     */
    OrPlayerPredicate( const PlayerPredicate * p1 ,
                       const PlayerPredicate * p2 ,
                       const PlayerPredicate * p3 ,
                       const PlayerPredicate * p4 ,
                       const PlayerPredicate * p5 ,
                       const PlayerPredicate * p6 ,
                       const PlayerPredicate * p7 )
        : M_predicates()
      {
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p1 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p2 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p3 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p4 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p5 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p6 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p7 ) );
      }

    /*!
      \brief construct with 8 predicates. all arguments must be a dynamically allocated object.
      \param p1 1st predicate
      \param p2 2nd predicate
      \param p3 3rd predicate
      \param p4 4th predicate
      \param p5 5th predicate
      \param p6 6th predicate
      \param p7 7th predicate
      \param p8 8th predicate
     */
    OrPlayerPredicate( const PlayerPredicate * p1 ,
                       const PlayerPredicate * p2 ,
                       const PlayerPredicate * p3 ,
                       const PlayerPredicate * p4 ,
                       const PlayerPredicate * p5 ,
                       const PlayerPredicate * p6 ,
                       const PlayerPredicate * p7 ,
                       const PlayerPredicate * p8 )
        : M_predicates()
      {
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p1 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p2 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p3 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p4 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p5 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p6 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p7 ) );
          M_predicates.push_back( boost::shared_ptr< const PlayerPredicate >( p8 ) );
      }

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return the result of "or" operation of all predicates
     */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          const std::vector< boost::shared_ptr< const PlayerPredicate > >::const_iterator
              p_end = M_predicates.end();

          for ( std::vector< boost::shared_ptr< const PlayerPredicate > >::const_iterator
                    it = M_predicates.begin();
                it != p_end;
                ++it )
          {
              if ( (**it)( p ) )
              {
                  return true;
              }
          }

          return false;
      }
};

/*!
  \class NotPlayerPredicate
  \brief logical "not" predicate
*/
class NotPlayerPredicate
    : public PlayerPredicate {
private:
    //! predicate instance
    boost::shared_ptr< const PlayerPredicate > M_predicate;

    // not used
    NotPlayerPredicate();
public:
    // XXX: implicit_shared_ptr

    /*!
      \brief construct with the predicate. argument must be a dynamically allocated object.
      \param predicate pointer to the predicate instance
     */
    explicit
    NotPlayerPredicate( const PlayerPredicate * predicate )
        : M_predicate( predicate )
      {
      }

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return the locigal "not" result of M_predicate
     */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          return ! (*M_predicate)( p );
      }
};

}

#endif
