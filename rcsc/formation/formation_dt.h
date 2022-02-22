// -*-c++-*-

/*!
	\file formation_dt.h
	\brief formation data classes using Delaunay Triangulation Header File.
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

#ifndef RCSC_FORMATION_FORMATION_DT_H
#define RCSC_FORMATION_FORMATION_DT_H

#include <rcsc/formation/formation.h>
#include <rcsc/geom/delaunay_triangulation.h>
#include <iostream>

namespace rcsc {

/*!
  \class FormationDT
  \brief formation which utilizes Delaunay Triangulation
*/
class FormationDT
    : public Formation {
public:

    /*!
      \struct Param
      \brief training data object
     */
    struct Param {
        Vector2D ball_; //!< ball position = focus point
        std::vector< Vector2D > players_; //!< all players' desired position

        /*!
          \brief just allocate memory
        */
        Param()
            : ball_( 0.0, 0.0 )
            , players_( 11 )
          { }

        /*!
          \brief initialize with training data snapshot
        */
        Param( const Snapshot & data );

        /*!
          \brief initialize with specified data
          \param ball ball position
          \param players players' position container
        */
        Param( const Vector2D & ball,
               const std::vector< Vector2D > & players )
            : ball_( ball )
            , players_( players )
          { }

        /*!
          \brief initialize with specified data
          \param ball ball position
          \param players players' position container
          \return const reference to itself
        */
        const
        Param & assign( const Vector2D & ball,
                        const std::vector< Vector2D > & players )
          {
              ball_ = ball;
              players_ = players;
              return *this;
          }

        /*!
          \brief get the specified player's position
          \param unum player number
          \return position value
        */
        Vector2D getPosition( const int unum ) const;
    };

private:

    //! player's role names
    std::string M_role_name[11];

    //! set of desired positins used by delaunay triangulation & linear interpolation
    std::vector< Param > M_param;

    //! delaunay triangulation
    DelaunayTriangulation M_triangulation;

public:

    /*!
      \brief just call the base class constructor to initialize formation method name
    */
    FormationDT();

    /*!
      \brief simple accessor.
      \return const reference to the param instance
     */
    const
    std::vector< Param > & params() const
      {
          return M_param;
      }

    /*!
      \brief static method. get formation method name
      \return method name string
    */
    static
    std::string name()
      {
          return std::string( "DelaunayTriangulation" );
      }

    /*!
      \brief static factory method. create new object
      \return new object
    */
    static
    Formation * create()
      {
          return ( new FormationDT );
      }

    /*!
      \brief get the delaunay triangulation
      \return const reference to the triangulation instance
     */
    const
    DelaunayTriangulation & triangulation() const
      {
          return M_triangulation;
      }

    //--------------------------------------------------------------

    /*!
      \brief get the name of this formation
      \return name string
    */
    virtual
    std::string methodName() const
      {
          return FormationDT::name();
      }

    /*!
      \brief create default formation. assign role and initial positions.
      \return snapshow variable for the initial state(ball pos=(0,0)).
    */
    virtual
    Snapshot createDefaultParam();

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
      \brief put data to the output stream.
      \param os reference to the output stream
      \return reference to the output stream
    */
    virtual
    std::ostream & print( std::ostream & os ) const;

private:

    Vector2D interpolate( const int unum,
                          const Vector2D & focus_point,
                          const DelaunayTriangulation::TrianglePtr tri ) const;

    /*!
      \brief restore role assignment from the input stream
      \param is reference to the input stream
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
