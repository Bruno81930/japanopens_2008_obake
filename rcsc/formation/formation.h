// -*-c++-*-

/*!
	\file formation.h
	\brief formation data classes Header File.
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

#ifndef RCSC_FORMATION_FORMATION_H
#define RCSC_FORMATION_FORMATION_H

#include <rcsc/geom/vector_2d.h>

#include <boost/shared_ptr.hpp>

#include <string>
#include <list>
#include <vector>
#include <iostream>

namespace rcsc {

/*!
  \class Formation
  \brief abstarct formation class
*/
class Formation {
public:

    enum SideType {
        SIDE = -1, //!< original type, consider all region
        SYNMETRY = 1, //!< synmetry type, this type refers SIDE
        CENTER = 0, //!< original type, consider half region
    };

    /*!
      \brief this struct is used to record the field status
      as a training data
    */
    struct Snapshot {
        Vector2D ball_; //!< ball position
        std::vector< Vector2D > players_; //!< players' position

        Snapshot()
          {
              players_.reserve( 11 );
          }

        Snapshot( const Vector2D & ball,
                  const std::vector< Vector2D > & players )
            : ball_( ball )
            , players_( players )
          { }
    };

protected:

    /*!
      synmetry number.holder
      <0 negative number means this player is original SIDE type.
      >0 positive numver means that this player is SYNMETRY type and
      referes other player.
      ==0 Zero means that this player is CENTER type.
    */
    int M_synmetry_number[11];


public:

    /*!
      \brief initialize synmetry number matrix by -1
    */
    Formation();


    virtual
    ~Formation()
      { }

    /*!
      \brief create default formation. assign role and initial positions.
      \return snapshot variable for the initial state(ball pos=(0,0)).
    */
    virtual
    Snapshot createDefaultParam() = 0;

    /*!
      \brief get the name of this formation
      \return name string
    */
    virtual
    std::string methodName() const = 0;

    /*!
      \brief check if player is SIDE type or not
      \param unum player's number
      \return true or false
    */
    bool isSideType( const int unum ) const
      {
          if ( unum < 1 || 11 < unum ) return false;
          return ( M_synmetry_number[unum - 1] < 0 );
      }

    /*!
      \brief check if player is center type or not
      \param unum player's number
      \return true or false
    */
    bool isCenterType( const int unum ) const
      {
          if ( unum < 1 || 11 < unum ) return false;
          return ( M_synmetry_number[unum - 1] == 0 );
      }

    /*!
      \brief check if player is right side type or not
      \param unum player's number
      \return true or false
    */
    bool isSynmetryType( const int unum ) const
      {
          if ( unum < 1 || 11 < unum ) return false;
          return ( M_synmetry_number[unum - 1] > 0 );
      }

    /*!
      \brief get symmetry reference number of the specified player.
      \param unum target player's number
      \retrun number that player refers, if player is SYNMETRY type.
      otherwise 0 or -1.
    */
    int getSynmetryNumber( const int unum ) const
      {
          if ( unum < 1 || 11 < unum ) return 0;
          return M_synmetry_number[unum - 1];
      }

    //--------------------------------------------------------------

    /*!
      \brief set player's role data. if necessary, new parameter is created.
      \param unum status changed player's uniform number
      \param synmetry_unum 0 means type is CENTER, <0 means type is SIDE,
      >0 means type is SYNMETRY
      \param role_name new role name string
      \return result of the registration
    */
    bool updateRole( const int unum,
                     const int synmetry_unum,
                     const std::string & role_name );

protected:

    /*!
      \brief create new role parameter.
      \param unum target player's number
      \param role_name new role name
      \param type side type of this parameter
    */
    virtual
    void createNewRole( const int unum,
                        const std::string & role_name,
                        const SideType type ) = 0;

    /*!
      \brief set the role name of the specified player
      \param unum target player's number
      \param name role name string.
    */
    virtual
    void setRoleName( const int unum,
                      const std::string & name ) = 0;


    /*!
      \brief set the specified player to the CENTER type
      \param unum player's number
    */
    void setCenterType( const int unum );

    /*!
      \brief set the specified player to the SIDE type
      \param unum player's number
    */
    void setSideType( const int unum );

    /*!
      \brief set symmetry player info
      \param unum changed player's number
      \param synmetry_unum refered player number
    */
    bool setSynmetryType( const int unum,
                          const int synmetry_unum );

public:

    /*!
      \brief get the role name of the specified player
      \param unum target player's number
      \return role name string. if empty string is returned,
      that means no role parameter is assigned for unum.
    */
    virtual
    std::string getRoleName( const int unum ) const = 0;

    /*!
      \brief get position for the current focus point
      \param unum player number
      \param focus_point current focus point, usually ball position.
    */
    virtual
    Vector2D getPosition( const int unum,
                          const Vector2D & focus_point ) const = 0;
    /*
      \brief get all positions for the current focus point
      \param focus_point current focus point, usually ball position
      \param positions contaner to store the result
     */
    virtual
    void getPositions( const Vector2D & focus_point,
                       std::vector< Vector2D > & positions ) const = 0;

    /*!
      \brief update formation paramter using training data set
      \param train_data training data container
    */
    virtual
    void train( const std::list< Snapshot > & train_data ) = 0;

    /*!
      \brief restore data from the input stream.
      \param is reference to the input stream.
      \return parsing result
    */
    virtual
    bool read( std::istream & is ) = 0;

    /*!
      \brief put data to the output stream.
      \param os reference to the output stream
      \return reference to the output stream
    */
    virtual
    std::ostream & print( std::ostream & os ) const = 0;


protected:

    /*!
      \brief check formation type name from the current stream pos.
      \param is reference to the input stream
      \return if success, the number of read lines will be returned.
      if failed, returned -1.
    */
    int readName( std::istream & is );

    /*!
      \brief put formation type name to the output stream
      \param os reference to the output stream
      \return reference to the output stream
    */
    std::ostream & printName( std::ostream & os ) const;

};

/*!
  \brief type of the formation pointer
 */
typedef boost::shared_ptr< Formation > FormationPtr;

}

#endif
