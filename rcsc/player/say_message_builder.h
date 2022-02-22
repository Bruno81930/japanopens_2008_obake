// -*-c++-*-

/*!
  \file say_message_builder.h
  \brief player's say message builder Header File
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

#ifndef RCSC_PLAYER_PLAYER_MESSAGE_BUILDER_H
#define RCSC_PLAYER_PLAYER_MESSAGE_BUILDER_H

#include <rcsc/geom/vector_2d.h>
#include <rcsc/common/say_message_parser.h>

#include <string>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!
  \class SayMessage
  \brief abstract player's say message encoder
*/
class SayMessage {
private:

    // not used
    SayMessage( const SayMessage & );
    SayMessage & operator=( const SayMessage & );

protected:

    /*!
      \brief protected constructer
    */
    SayMessage()
      { }

public:

    /*!
      \brief virtual destruct. do nothing.
    */
    virtual
    ~SayMessage()
      { }

    /*!
      \brief pure virtual method. get the header character of this message
      \return header character of this message
     */
    virtual
    char header() const = 0;

    /*!
      \brief pure virtual method. get the length of this message
      \return the length of encoded message
    */
    virtual
    std::size_t length() const = 0;

    /*!
      \brief append the audio message to be sent
      \param to reference to the message string instance
      \return result status of encoding
    */
    virtual
    bool toStr( std::string & to ) const = 0;

};


/*-------------------------------------------------------------------*/
/*!
  \class BallMessage
  \brief ball info message encoder

  format:
  "b<pos_vel:5>"
  the length of message == 6
*/
class BallMessage
    : public SayMessage {
private:

    Vector2D M_ball_pos; //!< ball position to be encoded
    Vector2D M_ball_vel; //!< ball velocity to be encoded

public:

    /*!
      \brief construct with raw information
      \param ball_pos ball position to be encoded
      \param ball_pos ball velocity to be encoded
    */
    BallMessage( const Vector2D & ball_pos,
                 const Vector2D & ball_vel )
        : M_ball_pos( ball_pos )
        , M_ball_vel( ball_vel )
      { }

    /*!
      \brief get the header character of this message
      \return header character of this message
     */
    char header() const
      {
          return BallMessageParser::sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    std::size_t slength()
      {
          return BallMessageParser::slength();
      }

    /*!
      \brief get the length of this message
      \return the length of encoded message
    */
    std::size_t length() const
      {
          return slength();
      }

    /*!
      \brief append this info to the audio message
      \param to reference to the message string instance
      \return result status of encoding
    */
    bool toStr( std::string & to ) const;

};

/*-------------------------------------------------------------------*/
/*!
  \class PassMessage
  \brief pass info message encoder

  format:
  "p<unum_pos:4><pos_vel:5>"
  the length of message == 10
*/
class PassMessage
    : public SayMessage {
private:

    int M_receiver_unum; //!< pass receiver's uniform number
    Vector2D M_receive_point; //!< desired pass receive point

    Vector2D M_ball_pos; //!< ball first pos
    Vector2D M_ball_vel; //!< ball first vel

public:

    /*!
      \brief construct with raw information
      \param receiver_number pass receiver's uniform number
      \param receive_point desired pass receive point
    */
    PassMessage( const int receiver_unum,
                 const Vector2D & receive_point,
                 const Vector2D & ball_pos,
                 const Vector2D & ball_vel )
        : M_receiver_unum( receiver_unum )
        , M_receive_point( receive_point )
        , M_ball_pos( ball_pos )
        , M_ball_vel( ball_vel )
      { }

    /*!
      \brief get the header character of this message
      \return header character of this message
     */
    char header() const
      {
          return PassMessageParser::sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    std::size_t slength()
      {
          return PassMessageParser::slength();
      }

    /*!
      \brief get the length of this message
      \return the length of encoded message
    */
    std::size_t length() const
      {
          return slength();
      }

    /*!
      \brief append this info to the audio message
      \param to reference to the message string instance
      \return result status of encoding
    */
    bool toStr( std::string & to ) const;

};

/*-------------------------------------------------------------------*/
/*!
  \class InterceptMessage
  \brief intercept info message encoder

  format:
  "i<unum:1><cycle:1>"
  the length of message == 3
*/
class InterceptMessage
    : public SayMessage {
private:

    bool M_our; //!< flag of interceptor's side
    int M_unum; //!< interceptor's uniform number
    int M_cycle; //!< interception cycle

public:

    /*!
      \brief construct with raw information
      \param our flag of intereptor side
      \param unum interceptor's uniform number
      \param cycle interception cycle
    */
    InterceptMessage( const bool our,
                      const int unum,
                      const int cycle )
        : M_our( our )
        , M_unum( unum )
        , M_cycle( cycle )
      { }

    /*!
      \brief get the header character of this message
      \return header character of this message
     */
    char header() const
      {
          return InterceptMessageParser::sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    std::size_t slength()
      {
          return InterceptMessageParser::slength();
      }

    /*!
      \brief get the length of this message
      \return the length of encoded message
    */
    std::size_t length() const
      {
          return slength();
      }

    /*!
      \brief append this info to the audio message
      \param to reference to the message string instance
      \return result status of encoding
    */
    bool toStr( std::string & to ) const;
};

/*-------------------------------------------------------------------*/
/*!
  \class GoalieMessage
  \brief goalie info message encoder

  format:
  "g<pos_body:4>"
  the length of message == 5
*/
class GoalieMessage
    : public SayMessage {
private:

    int M_goalie_unum; //!< goalie's uniform number
    Vector2D M_goalie_pos; //!< goalie's position
    AngleDeg M_goalie_body; //!< goalie's body angle

public:

    /*!
      \brief construct with raw information
      \param goalie_unum goalie's uniform number
      \param goalie_pos goalie's global position
    */
    GoalieMessage( const int goalie_unum,
                   const Vector2D & goalie_pos,
                   const AngleDeg & goalie_body )
        : M_goalie_unum( goalie_unum )
        , M_goalie_pos( goalie_pos )
        , M_goalie_body( goalie_body )
      { }

    /*!
      \brief get the header character of this message
      \return header character of this message
     */
    char header() const
      {
          return GoalieMessageParser::sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    std::size_t slength()
      {
          return GoalieMessageParser::slength();
      }

    /*!
      \brief get the length of this message
      \return the length of encoded message
    */
    std::size_t length() const
      {
          return slength();
      }

    /*!
      \brief append this info to the audio message
      \param to reference to the message string instance
      \return result status of encoding
    */
    bool toStr( std::string & to ) const;
};

/*-------------------------------------------------------------------*/
/*!
  \class OffsideLineMessage
  \brief offside line info message encoder

  format:
  "o<x_rate:1>"
  the length of message == 2
*/
class OffsideLineMessage
    : public SayMessage {
private:

    double M_offside_line_x; //!< raw offside line x coordinate value

public:

    /*!
      \brief construct with raw information
      \param offside_line_x raw offside line x coordinate value
    */
    explicit
    OffsideLineMessage( const double & offside_line_x )
        : M_offside_line_x( offside_line_x )
      { }

    /*!
      \brief get the header character of this message
      \return header character of this message
     */
    char header() const
      {
          return OffsideLineMessageParser::sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    std::size_t slength()
      {
          return OffsideLineMessageParser::slength();
      }

    /*!
      \brief get the length of this message
      \return the length of encoded message
    */
    std::size_t length() const
      {
          return slength();
      }

    /*!
      \brief append this info to the audio message
      \param to reference to the message string instance
      \return result status of encoding
    */
    bool toStr( std::string & to ) const;
};

/*-------------------------------------------------------------------*/
/*!
  \class DefenseLineMessage
  \brief defense line info message encoder

  format:
  "d<x_rate:1>"
  the length of message == 2
*/
class DefenseLineMessage
    : public SayMessage {
private:

    double M_defense_line_x; //!< raw defense line x coordinate value

public:

    /*!
      \brief construct with raw information
      \param defense_line_x raw defense line x coordinate value
    */
    explicit
    DefenseLineMessage( const double & defense_line_x )
        : M_defense_line_x( defense_line_x )
      { }

    /*!
      \brief get the header character of this message
      \return header character of this message
     */
    char header() const
      {
          return DefenseLineMessageParser::sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    std::size_t slength()
      {
          return DefenseLineMessageParser::slength();
      }

    /*!
      \brief get the length of this message
      \return the length of encoded message
    */
    std::size_t length() const
      {
          return slength();
      }

    /*!
      \brief append this info to the audio message
      \param to reference to the message string instance
      \return result status of encoding
    */
    bool toStr( std::string & to ) const;
};

/*-------------------------------------------------------------------*/
/*!
  \class WaitRequestMessage
  \brief wait request message encoder

  format:
  "w"
  the length of message == 1
*/
class WaitRequestMessage
    : public SayMessage {
private:

public:

    /*!
      \brief construct with raw information
    */
    WaitRequestMessage()
      { }

    /*!
      \brief get the header character of this message
      \return header character of this message
     */
    char header() const
      {
          return WaitRequestMessageParser::sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    std::size_t slength()
      {
          return WaitRequestMessageParser::slength();
      }

    /*!
      \brief get the length of this message
      \return the length of encoded message
    */
    std::size_t length() const
      {
          return slength();
      }

    /*!
      \brief append this info to the audio message
      \param to reference to the message string instance
      \return result status of encoding
    */
    bool toStr( std::string & to ) const;
};

/*-------------------------------------------------------------------*/
/*!
  \class PassRequestMessage
  \brief pass request info message encoder

  format:
  "h<pos:3>"
  the length of message == 4
*/
class PassRequestMessage
    : public SayMessage {
private:

    Vector2D M_target_point; //!< dash target point

public:

    /*!
      \brief construct with raw information
      \param target_point dash target point
    */
    explicit
    PassRequestMessage( const Vector2D & target_point )
        : M_target_point( target_point )
      { }

    /*!
      \brief get the header character of this message
      \return header character of this message
     */
    char header() const
      {
          return PassRequestMessageParser::sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    std::size_t slength()
      {
          return PassRequestMessageParser::slength();
      }

    /*!
      \brief get the length of this message
      \return the length of encoded message
    */
    std::size_t length() const
      {
          return slength();
      }

    /*!
      \brief append this info to the audio message
      \param to reference to the message string instance
      \return result status of encoding
    */
    bool toStr( std::string & to ) const;
};

/*-------------------------------------------------------------------*/
/*!
  \class StaminaMessage
  \brief stamina info message encoder

  format:
  "s<rate:1>"
  the length of message == 2
*/
class StaminaMessage
    : public SayMessage {
private:

    double M_stamina; //!< raw stamina value

public:

    /*!
      \brief construct with raw information
      \param stamina raw stamina value
    */
    explicit
    StaminaMessage( const double & stamina )
        : M_stamina( stamina )
      { }

    /*!
      \brief get the header character of this message
      \return header character of this message
     */
    char header() const
      {
          return StaminaMessageParser::sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    std::size_t slength()
      {
          return StaminaMessageParser::slength();
      }

    /*!
      \brief get the length of this message
      \return the length of encoded message
    */
    std::size_t length() const
      {
          return slength();
      }

    /*!
      \brief append this info to the audio message
      \param to reference to the message string instance
      \return result status of encoding
    */
    bool toStr( std::string & to ) const;
};

/*-------------------------------------------------------------------*/
/*!
  \class RecoveryMessage
  \brief recovery info message encoder

  format:
  "r<rate:1>"
  the length of message == 2
*/
class RecoveryMessage
    : public SayMessage {
private:

    double M_recovery; //!< raw recovery value

public:

    /*!
      \brief construct with raw information
      \param recovery raw recovery value
    */
    explicit
    RecoveryMessage( const double & recovery )
        : M_recovery( recovery )
      { }

    /*!
      \brief get the header character of this message
      \return header character of this message
     */
    char header() const
      {
          return RecoveryMessageParser::sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    std::size_t slength()
      {
          return RecoveryMessageParser::slength();
      }

    /*!
      \brief get the length of this message
      \return the length of encoded message
    */
    std::size_t length() const
      {
          return slength();
      }

    /*!
      \brief append this info to the audio message
      \param to reference to the message string instance
      \return result status of encoding
    */
    bool toStr( std::string & to ) const;
};

/*-------------------------------------------------------------------*/
/*!
  \class DribbleMessage
  \brief dribble info message encoder

  format:
  "D<count_pos:3>"
  the length of message == 4
*/
class DribbleMessage
    : public SayMessage {
private:

    Vector2D M_target_point; //!< dribble target point
    int M_queue_count; //!< dribble queue count

public:

    /*!
      \brief construct with raw information
      \param target_point dribble target point
      \param queue_count dribble action queue count
    */
    DribbleMessage( const Vector2D & target_point,
                    const int queue_count )
        : M_target_point( target_point )
        , M_queue_count( queue_count )
      { }

    /*!
      \brief get the header character of this message
      \return header character of this message
     */
    char header() const
      {
          return DribbleMessageParser::sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    std::size_t slength()
      {
          return DribbleMessageParser::slength();
      }

    /*!
      \brief get the length of this message
      \return the length of encoded message
    */
    std::size_t length() const
      {
          return slength();
      }

    /*!
      \brief append this info to the audio message
      \param to reference to the message string instance
      \return result status of encoding
    */
    bool toStr( std::string & to ) const;
};

/*-------------------------------------------------------------------*/
/*!
  \class BallGoalieMessage
  \brief ball & goalie info message encoder

  format:
  "G<bpos_bvel_gpos_gbody:9>"
  the length of message == 5
*/
class BallGoalieMessage
    : public SayMessage {
private:

    Vector2D M_ball_pos; //!< ball position
    Vector2D M_ball_vel; //!< ball vel
    Vector2D M_goalie_pos; //!< goalie's position
    AngleDeg M_goalie_body; //!< goalie's body angle

public:

    /*!
      \brief construct with raw information
      \param ball_pos ball position
      \param ball_vel ball velocity
      \param goalie_pos goalie's global position
      \param goalie_body goalie's body angle
    */
    BallGoalieMessage( const Vector2D & ball_pos,
                       const Vector2D & ball_vel,
                       const Vector2D & goalie_pos,
                       const AngleDeg & goalie_body )
        : M_ball_pos( ball_pos )
        , M_ball_vel( ball_vel )
        , M_goalie_pos( goalie_pos )
        , M_goalie_body( goalie_body )
      { }

    /*!
      \brief get the header character of this message
      \return header character of this message
     */
    char header() const
      {
          return BallGoalieMessageParser::sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    std::size_t slength()
      {
          return BallGoalieMessageParser::slength();
      }

    /*!
      \brief get the length of this message
      \return the length of encoded message
    */
    std::size_t length() const
      {
          return slength();
      }

    /*!
      \brief append this info to the audio message
      \param to reference to the message string instance
      \return result status of encoding
    */
    bool toStr( std::string & to ) const;
};

/*-------------------------------------------------------------------*/
/*!
  \class OnePlayerMessage
  \brief one player info message encoder

  format:
  "1<unum_pos_body:4>"
  the length of message == 5
*/
class OnePlayerMessage
    : public SayMessage {
private:

    int M_unum; //!< player's unum [1-22]. if opponent, unum > 11
    Vector2D M_player_pos; //!< player's position
    AngleDeg M_player_body; //!< player's body angle

public:

    /*!
      \brief construct with raw information
      \param unum player's unum
      \param player_pos goalie's global position
      \param player_body goalie's body angle
    */
    OnePlayerMessage( const int unum,
                      const Vector2D & player_pos,
                      const AngleDeg & player_body )
        : M_unum( unum )
        , M_player_pos( player_pos )
        , M_player_body( player_body )
      { }

    /*!
      \brief get the header character of this message
      \return header character of this message
     */
    char header() const
      {
          return OnePlayerMessageParser::sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    std::size_t slength()
      {
          return OnePlayerMessageParser::slength();
      }

    /*!
      \brief get the length of this message
      \return the length of encoded message
    */
    std::size_t length() const
      {
          return slength();
      }

    /*!
      \brief append this info to the audio message
      \param to reference to the message string instance
      \return result status of encoding
    */
    bool toStr( std::string & to ) const;
};

/*-------------------------------------------------------------------*/
/*!
  \class BallPlayerMessage
  \brief ball & player info message encoder

  format:
  "1<bpos_bvel_unum_ppos_pbody:9>"
  the length of message == 10
*/
class BallPlayerMessage
    : public SayMessage {
private:

    Vector2D M_ball_pos; //!< ball position
    Vector2D M_ball_vel; //!< ball velocity

    int M_unum; //!< player's unum [1-22]. if opponent, unum > 11
    Vector2D M_player_pos; //!< player's position
    AngleDeg M_player_body; //!< player's body angle

public:

    /*!
      \brief construct with raw information
      \param ball_pos ball position
      \param ball_vel ball velocity
      \param unum player's unum
      \param player_pos goalie's global position
      \param player_body goalie's body angle
    */
    BallPlayerMessage( const Vector2D & ball_pos,
                       const Vector2D & ball_vel,
                       const int unum,
                       const Vector2D & player_pos,
                       const AngleDeg & player_body )
        : M_ball_pos( ball_pos )
        , M_ball_vel( ball_vel )
        , M_unum( unum )
        , M_player_pos( player_pos )
        , M_player_body( player_body )
      { }

    /*!
      \brief get the header character of this message
      \return header character of this message
     */
    char header() const
      {
          return BallPlayerMessageParser::sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    std::size_t slength()
      {
          return BallPlayerMessageParser::slength();
      }

    /*!
      \brief get the length of this message
      \return the length of encoded message
    */
    std::size_t length() const
      {
          return slength();
      }

    /*!
      \brief append this info to the audio message
      \param to reference to the message string instance
      \return result status of encoding
    */
    bool toStr( std::string & to ) const;
};

}

#endif
