// -*-c++-*-

/*!
  \file say_message_parser.h
  \brief player's say message parser Header File
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

#ifndef RCSC_COMMON_SAY_MESSAGE_PARSER_H
#define RCSC_COMMON_SAY_MESSAGE_PARSER_H

#include <rcsc/types.h>

#include <boost/shared_ptr.hpp>
#include <string>

namespace rcsc {

class AudioMemory;
class GameTime;

/*-------------------------------------------------------------------*/
/*!
  \class SayMessageParser
  \brief abstract player's say message parser
 */
class SayMessageParser {
private:

    // not used
    SayMessageParser( const SayMessageParser & );
    SayMessageParser & operator=( const SayMessageParser & );

protected:

    /*!
      \brief protected constructer
     */
    SayMessageParser()
      { }

public:

    /*!
      \brief virtual destruct. do nothing.
     */
    virtual
    ~SayMessageParser()
      { }

    /*!
      \brief pure virtual method that returns header character.
      \return header character.
     */
    virtual
    char header() const = 0;

    /*!
      \brief virtual method which analyzes audio messages.
      \param sender sender's uniform number
      \param dir sender's direction
      \param msg raw audio message
      \param current current game time
      \retval bytes read if success
      \retval 0 message ID is not match. other parser should be tried.
      \retval -1 failed to parse
    */
    virtual
    int parse( const int sender,
               const double & dir,
               const char * msg,
               const GameTime & current ) = 0;

};

/*-------------------------------------------------------------------*/
/*!
  \class BallMessageParser
  \brief ball info message parser

  format:
  "b<pos_vel:5>"
  the length of message == 6
 */
class BallMessageParser
    : public SayMessageParser {
private:

    //! pointer to the audio memory
    boost::shared_ptr< AudioMemory > M_memory;

public:

    /*!
      \brief construct with audio memory
      \param memory pointer to the memory
     */
    explicit
    BallMessageParser( boost::shared_ptr< AudioMemory > memory );

    /*!
      \brief get the header character.
      \return header character.
     */
    static
    char sheader()
      {
          return 'b';
      }

    /*!
      \brief get the header character.
      \return header character.
     */
    char header() const
      {
          return sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    std::size_t slength()
      {
          return 6;
      }

    /*!
      \brief virtual method which analyzes audio messages.
      \param sender sender's uniform number
      \param dir sender's direction
      \param msg raw audio message
      \param current current game time
      \retval bytes read if success
      \retval 0 message ID is not match. other parser should be tried.
      \retval -1 failed to parse
    */
    int parse( const int sender,
               const double & dir,
               const char * msg,
               const GameTime & current );

};

/*-------------------------------------------------------------------*/
/*!
  \class PassMessageParser
  \brief pass info message parser

  format:
  "p<unum_pos:4><pos_vel:5>"
  the length of message == 10
 */
class PassMessageParser
    : public SayMessageParser {
private:

    //! pointer to the audio memory
    boost::shared_ptr< AudioMemory > M_memory;

public:

    /*!
      \brief construct with audio memory
      \param memory pointer to the memory
     */
    explicit
    PassMessageParser( boost::shared_ptr< AudioMemory > memory );

    /*!
      \brief get the header character.
      \return header character.
     */
    static
    char sheader()
      {
          return 'p';
      }

    /*!
      \brief get the header character.
      \return header character.
     */
    char header() const
      {
          return sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    std::size_t slength()
      {
          return 10;
      }

    /*!
      \brief virtual method which analyzes audio messages.
      \param sender sender's uniform number
      \param dir sender's direction
      \param msg raw audio message
      \param current current game time
      \retval bytes read if success
      \retval 0 message ID is not match. other parser should be tried.
      \retval -1 failed to parse
    */
    int parse( const int sender,
               const double & dir,
               const char * msg,
               const GameTime & current );

};

/*-------------------------------------------------------------------*/
/*!
  \class InterceptMessageParser
  \brief intercept info message parser

  format:
  "i<unum:1><cycle:1>"
  the length of message == 3  format:
 */
class InterceptMessageParser
    : public SayMessageParser {
private:

    //! pointer to the audio memory
    boost::shared_ptr< AudioMemory > M_memory;

public:

    /*!
      \brief construct with audio memory
      \param memory pointer to the memory
     */
    explicit
    InterceptMessageParser( boost::shared_ptr< AudioMemory > memory );

    /*!
      \brief get the header character.
      \return header character.
     */
    static
    char sheader()
      {
          return 'i';
      }

    /*!
      \brief get the header character.
      \return header character.
     */
    char header() const
      {
          return sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    std::size_t slength()
      {
          return 3;
      }

    /*!
      \brief virtual method which analyzes audio messages.
      \param sender sender's uniform number
      \param dir sender's direction
      \param msg raw audio message
      \param current current game time
      \retval bytes read if success
      \retval 0 message ID is not match. other parser should be tried.
      \retval -1 failed to parse
    */
    int parse( const int sender,
               const double & dir,
               const char * msg,
               const GameTime & current );

};

/*-------------------------------------------------------------------*/
/*!
  \class GoalieMessageParser
  \brief goalie info message parser

  format:
  "g<unum_pos:4>"
  the length of message == 5
 */
class GoalieMessageParser
    : public SayMessageParser {
private:

    //! pointer to the audio memory
    boost::shared_ptr< AudioMemory > M_memory;

public:

    /*!
      \brief construct with audio memory
      \param memory pointer to the memory
     */
    explicit
    GoalieMessageParser( boost::shared_ptr< AudioMemory > memory );

    /*!
      \brief get the header character.
      \return header character.
     */
    static
    char sheader()
      {
          return 'g';
      }

    /*!
      \brief get the header character.
      \return header character.
     */
    char header() const
      {
          return sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    std::size_t slength()
      {
          return 5;
      }

    /*!
      \brief virtual method which analyzes audio messages.
      \param sender sender's uniform number
      \param dir sender's direction
      \param msg raw audio message
      \param current current game time
      \retval bytes read if success
      \retval 0 message ID is not match. other parser should be tried.
      \retval -1 failed to parse
    */
    int parse( const int sender,
               const double & dir,
               const char * msg,
               const GameTime & current );

};

/*-------------------------------------------------------------------*/
/*!
  \class OffsideLineMessageParser
  \brief offside line info message parser

  format:
  "o<x_rate:1>"
  the length of message == 2
 */
class OffsideLineMessageParser
    : public SayMessageParser {
private:

    //! pointer to the audio memory
    boost::shared_ptr< AudioMemory > M_memory;

public:

    /*!
      \brief construct with audio memory
      \param memory pointer to the memory
     */
    explicit
    OffsideLineMessageParser( boost::shared_ptr< AudioMemory > memory );

    /*!
      \brief get the header character.
      \return header character.
     */
    static
    char sheader()
      {
          return 'o';
      }

    /*!
      \brief get the header character.
      \return header character.
     */
    char header() const
      {
          return sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    std::size_t slength()
      {
          return 2;
      }

    /*!
      \brief virtual method which analyzes audio messages.
      \param sender sender's uniform number
      \param dir sender's direction
      \param msg raw audio message
      \param current current game time
      \retval bytes read if success
      \retval 0 message ID is not match. other parser should be tried.
      \retval -1 failed to parse
    */
    int parse( const int sender,
               const double & dir,
               const char * msg,
               const GameTime & current );

};

/*-------------------------------------------------------------------*/
/*!
  \class DefenseLineMessageParser
  \brief offside line info message parser

  format:
  "o<x_rate:1>"
  the length of message == 2
 */
class DefenseLineMessageParser
    : public SayMessageParser {
private:

    //! pointer to the audio memory
    boost::shared_ptr< AudioMemory > M_memory;

public:

    /*!
      \brief construct with audio memory
      \param memory pointer to the memory
     */
    explicit
    DefenseLineMessageParser( boost::shared_ptr< AudioMemory > memory );

    /*!
      \brief get the header character.
      \return header character.
     */
    static
    char sheader()
      {
          return 'd';
      }

    /*!
      \brief get the header character.
      \return header character.
     */
    char header() const
      {
          return sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    std::size_t slength()
      {
          return 2;
      }

    /*!
      \brief virtual method which analyzes audio messages.
      \param sender sender's uniform number
      \param dir sender's direction
      \param msg raw audio message
      \param current current game time
      \retval bytes read if success
      \retval 0 message ID is not match. other parser should be tried.
      \retval -1 failed to parse
    */
    int parse( const int sender,
               const double & dir,
               const char * msg,
               const GameTime & current );

};

/*-------------------------------------------------------------------*/
/*!
  \class WaitRequestMessageParser
  \brief wait request message parser

  format:
  "w"
  the length of message == 1
 */
class WaitRequestMessageParser
    : public SayMessageParser {
private:

    //! pointer to the audio memory
    boost::shared_ptr< AudioMemory > M_memory;

public:

    /*!
      \brief construct with audio memory
      \param memory pointer to the memory
     */
    explicit
    WaitRequestMessageParser( boost::shared_ptr< AudioMemory > memory );

    /*!
      \brief get the header character.
      \return header character.
     */
    static
    char sheader()
      {
          return 'w';
      }

    /*!
      \brief get the header character.
      \return header character.
     */
    char header() const
      {
          return sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    std::size_t slength()
      {
          return 1;
      }

    /*!
      \brief virtual method which analyzes audio messages.
      \param sender sender's uniform number
      \param dir sender's direction
      \param msg raw audio message
      \param current current game time
      \retval bytes read if success
      \retval 0 message ID is not match. other parser should be tried.
      \retval -1 failed to parse
    */
    int parse( const int sender,
               const double & dir,
               const char * msg,
               const GameTime & current );

};

/*-------------------------------------------------------------------*/
/*!
  \class PassRequestMessageParser
  \brief pass request (hey pass) message parser

  format:
  "h<pos:3>"
  the length of message == 4
 */
class PassRequestMessageParser
    : public SayMessageParser {
private:

    //! pointer to the audio memory
    boost::shared_ptr< AudioMemory > M_memory;

public:

    /*!
      \brief construct with audio memory
      \param memory pointer to the memory
     */
    explicit
    PassRequestMessageParser( boost::shared_ptr< AudioMemory > memory );

    /*!
      \brief get the header character.
      \return header character.
     */
    static
    char sheader()
      {
          return 'h';
      }

    /*!
      \brief get the header character.
      \return header character.
     */
    char header() const
      {
          return sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    std::size_t slength()
      {
          return 4;
      }

    /*!
      \brief virtual method which analyzes audio messages.
      \param sender sender's uniform number
      \param dir sender's direction
      \param msg raw audio message
      \param current current game time
      \retval bytes read if success
      \retval 0 message ID is not match. other parser should be tried.
      \retval -1 failed to parse
    */
    int parse( const int sender,
               const double & dir,
               const char * msg,
               const GameTime & current );

};

/*-------------------------------------------------------------------*/
/*!
  \class StaminaMessageParser
  \brief stamina rate value message parser

  format:
  "s<rate:1>"
  the length of message == 2
 */
class StaminaMessageParser
    : public SayMessageParser {
private:

    //! pointer to the audio memory
    boost::shared_ptr< AudioMemory > M_memory;

public:

    /*!
      \brief construct with audio memory
      \param memory pointer to the memory
     */
    explicit
    StaminaMessageParser( boost::shared_ptr< AudioMemory > memory );

    /*!
      \brief get the header character.
      \return header character.
     */
    static
    char sheader()
      {
          return 's';
      }

    /*!
      \brief get the header character.
      \return header character.
     */
    char header() const
      {
          return sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    std::size_t slength()
      {
          return 2;
      }

    /*!
      \brief virtual method which analyzes audio messages.
      \param sender sender's uniform number
      \param dir sender's direction
      \param msg raw audio message
      \param current current game time
      \retval bytes read if success
      \retval 0 message ID is not match. other parser should be tried.
      \retval -1 failed to parse
    */
    int parse( const int sender,
               const double & dir,
               const char * msg,
               const GameTime & current );

};

/*-------------------------------------------------------------------*/
/*!
  \class RecoveryMessageParser
  \brief recovery rate value message parser

  format:
  "r<rate:1>"
  the length of message == 2
 */
class RecoveryMessageParser
    : public SayMessageParser {
private:

    //! pointer to the audio memory
    boost::shared_ptr< AudioMemory > M_memory;

public:

    /*!
      \brief construct with audio memory
      \param memory pointer to the memory
     */
    explicit
    RecoveryMessageParser( boost::shared_ptr< AudioMemory > memory );

    /*!
      \brief get the header character.
      \return header character.
     */
    static
    char sheader()
      {
          return 'r';
      }

    /*!
      \brief get the header character.
      \return header character.
     */
    char header() const
      {
          return sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    std::size_t slength()
      {
          return 2;
      }

    /*!
      \brief virtual method which analyzes audio messages.
      \param sender sender's uniform number
      \param dir sender's direction
      \param msg raw audio message
      \param current current game time
      \retval bytes read if success
      \retval 0 message ID is not match. other parser should be tried.
      \retval -1 failed to parse
    */
    int parse( const int sender,
               const double & dir,
               const char * msg,
               const GameTime & current );

};

/*-------------------------------------------------------------------*/
/*!
  \class DribbleMessageParser
  \brief dribble target point message parser

  format:
  "D<count_pos:3>"
  the length of message == 4
 */
class DribbleMessageParser
    : public SayMessageParser {
private:

    //! pointer to the audio memory
    boost::shared_ptr< AudioMemory > M_memory;

public:

    /*!
      \brief construct with audio memory
      \param memory pointer to the memory
     */
    explicit
    DribbleMessageParser( boost::shared_ptr< AudioMemory > memory );

    /*!
      \brief get the header character.
      \return header character.
     */
    static
    char sheader()
      {
          return 'D';
      }

    /*!
      \brief get the header character.
      \return header character.
     */
    char header() const
      {
          return sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    std::size_t slength()
      {
          return 4;
      }

    /*!
      \brief virtual method which analyzes audio messages.
      \param sender sender's uniform number
      \param dir sender's direction
      \param msg raw audio message
      \param current current game time
      \retval bytes read if success
      \retval 0 message ID is not match. other parser should be tried.
      \retval -1 failed to parse
    */
    int parse( const int sender,
               const double & dir,
               const char * msg,
               const GameTime & current );

};

/*-------------------------------------------------------------------*/
/*!
  \class BallGoalieMessageParser
  \brief ball & goalie info message parser

  format:
  "G<bpos_bvel_gpos_gbody:9>"
  the length of message == 10
 */
class BallGoalieMessageParser
    : public SayMessageParser {
private:

    //! pointer to the audio memory
    boost::shared_ptr< AudioMemory > M_memory;

public:

    /*!
      \brief construct with audio memory
      \param memory pointer to the memory
     */
    explicit
    BallGoalieMessageParser( boost::shared_ptr< AudioMemory > memory );

    /*!
      \brief get the header character.
      \return header character.
     */
    static
    char sheader()
      {
          return 'G';
      }

    /*!
      \brief get the header character.
      \return header character.
     */
    char header() const
      {
          return sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    std::size_t slength()
      {
          return 10;
      }

    /*!
      \brief virtual method which analyzes audio messages.
      \param sender sender's uniform number
      \param dir sender's direction
      \param msg raw audio message
      \param current current game time
      \retval bytes read if success
      \retval 0 message ID is not match. other parser should be tried.
      \retval -1 failed to parse
    */
    int parse( const int sender,
               const double & dir,
               const char * msg,
               const GameTime & current );

};

/*-------------------------------------------------------------------*/
/*!
  \class OnePlayerMessageParser
  \brief player info message parser

  format:
  "1<unum_pos_body:>"
  the length of message == 5
 */
class OnePlayerMessageParser
    : public SayMessageParser {
private:

    //! pointer to the audio memory
    boost::shared_ptr< AudioMemory > M_memory;

public:

    /*!
      \brief construct with audio memory
      \param memory pointer to the memory
     */
    explicit
    OnePlayerMessageParser( boost::shared_ptr< AudioMemory > memory );

    /*!
      \brief get the header character.
      \return header character.
     */
    static
    char sheader()
      {
          return 'P';
      }

    /*!
      \brief get the header character.
      \return header character.
     */
    char header() const
      {
          return sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    std::size_t slength()
      {
          return 5;
      }

    /*!
      \brief virtual method which analyzes audio messages.
      \param sender sender's uniform number
      \param dir sender's direction
      \param msg raw audio message
      \param current current game time
      \retval bytes read if success
      \retval 0 message ID is not match. other parser should be tried.
      \retval -1 failed to parse
    */
    int parse( const int sender,
               const double & dir,
               const char * msg,
               const GameTime & current );

};

/*-------------------------------------------------------------------*/
/*!
  \class BallPlayerMessageParser
  \brief ball & player info message parser

  format:
  "B<pos_vel_body:>"
  the length of message == ??
 */
class BallPlayerMessageParser
    : public SayMessageParser {
private:

    //! pointer to the audio memory
    boost::shared_ptr< AudioMemory > M_memory;

public:

    /*!
      \brief construct with audio memory
      \param memory pointer to the memory
     */
    explicit
    BallPlayerMessageParser( boost::shared_ptr< AudioMemory > memory );

    /*!
      \brief get the header character.
      \return header character.
     */
    static
    char sheader()
      {
          return 'B';
      }

    /*!
      \brief get the header character.
      \return header character.
     */
    char header() const
      {
          return sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    std::size_t slength()
      {
          return 10;
      }

    /*!
      \brief virtual method which analyzes audio messages.
      \param sender sender's uniform number
      \param dir sender's direction
      \param msg raw audio message
      \param current current game time
      \retval bytes read if success
      \retval 0 message ID is not match. other parser should be tried.
      \retval -1 failed to parse
    */
    int parse( const int sender,
               const double & dir,
               const char * msg,
               const GameTime & current );

};

}

#endif
