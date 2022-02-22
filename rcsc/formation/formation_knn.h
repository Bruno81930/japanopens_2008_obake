// -*-c++-*-

/*!
	\file formation_knn.h
	\brief k-nearest neighbor formation method class Header File.
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

#ifndef RCSC_FORMATION_FORMATION_KNN_H
#define RCSC_FORMATION_FORMATION_KNN_H

#include <iostream>
#include <map>

#include <rcsc/geom/vector_2d.h>
#include <rcsc/formation/formation.h>

namespace rcsc {

/*!
  \class FormationKNN
  \brief k-nearest neighbor type formation
 */
class FormationKNN
    : public Formation {
public:

    /*!
      \struct Data
      \brief training data
     */
    struct Data {
        Vector2D ball_; //!< ball position = focus point
        std::vector< Vector2D > players_; //!< all players' desired position

        /*!
          \brief just allocate memory
        */
        Data();

        /*!
          \brief initialize with training data snapshot
        */
        Data( const Snapshot & snapshot );

        /*!
          \brief initialize with specified data
          \param ball ball position
          \param players players' position container
        */
        Data( const Vector2D & ball,
              const std::vector< Vector2D > & players );
        /*!
          \brief initialize with specified data
          \param ball ball position
          \param players players' position container
          \return const reference to itself
        */
        const
        Data & assign( const Vector2D & ball,
                       const std::vector< Vector2D > & players );

        /*!
          \brief get the specified player's position
          \param unum player number
          \return position value
        */
        Vector2D getPosition( const int unum ) const;

    };


private:

    //! parameter for k-nearest neighbor
    size_t M_k;

    //! players' role names
    std::string M_role_name[11];

    //! data instance container
    std::vector< Data > M_data;

    //! data pointer container
    mutable std::vector< Data * > M_data_ptr;

public:
    /*!
      \brief just call the base class constructor
     */
    FormationKNN();

    /*!
      \brief static method. get the type name of this formation
      \return type name string
     */
    static
    std::string name()
      {
          return std::string( "k-NN" );
      }

    /*!
      \brief static factory method. create this class.
      \return pointer to the new instance
     */
    static
    Formation * create()
      {
          return ( new FormationKNN );
      }

    //--------------------------------------------------------------

    /*!
      \brief get the name of this formation
      \return name string
     */
    virtual
    std::string methodName() const
      {
          return FormationKNN::name();
      }

    /*!
      \brief create default formation. assign role and initial positions.
      \return snapshow variable for the initial stat(ball pos=(0,0)).
     */
    virtual
    Snapshot createDefaultParam();

    /*!
      \brief get the parameter container
      \return const reference to the parameter container
     */
    const
    std::vector< Data > & data() const
      {
          return M_data;
      }

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
                        const SideType type );

    /*!
      \brief set the role name of the specified player
      \param unum target player's number
      \param name role name string.
     */
    virtual
    void setRoleName( const int unum,
                      const std::string & name );

public:

    /*!
      \brief get the role name of the specified player
      \param unum target player's number
      \return role name string. if empty string is returned,
      that means no role parameter is assigned for unum.
     */
    virtual
    std::string getRoleName( const int unum ) const;

    /*!
      \brief get position for the current focus point
      \param unum player number
      \param forcus_point current focus point, usually ball position.
     */
    virtual
    Vector2D getPosition( const int unum,
                          const Vector2D & focus_point ) const;

    /*
      \brief get all positions for the current focus point
      \param focus_point current focus point, usually ball position
      \param positions contaner to store the result
     */
    virtual
    void getPositions( const Vector2D & focus_point,
                       std::vector< Vector2D > & positions ) const;

    /*!
      \brief update formation paramter using training data set
      \param train_data training data container
     */
    virtual
    void train( const std::list< Snapshot > & train_data );

    /*!
      \brief restore data from the input stream.
      \param is reference to the input stream.
      \return parsing result
     */
    virtual
    bool read( std::istream & is );

    /*!
      \brief put all variables to output stream
      \param os reference to the output stream
      \return  reference to the output stream
     */
    virtual
    std::ostream & print( std::ostream & os ) const;

private:


    /*!
      \brief restore players from the input stream
      \param is reference to the input stream.
      \return parsing result
     */
    bool readPlayers( std::istream & is );

    /*!
      \brief restore roles from the input stream
      \param is reference to the input stream.
      \return parsing result
     */
    bool readRoles( std::istream & is );

    /*!
      \brief restore kernel point and sample data from the input stream
      \param is reference to the input stream
      \return parsing result
    */
    bool readSamples( std::istream & is );

};

}

#endif
