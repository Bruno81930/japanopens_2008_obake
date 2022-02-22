// -*-c++-*-

/*!
  \file soccer_action.h
  \brief abstract player action Header File
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

#ifndef RCSC_PLAYER_SOCCER_ACTION_H
#define RCSC_PLAYER_SOCCER_ACTION_H

namespace rcsc {

class PlayerAgent;

/*!
  \class BodyAction
  \brief abstract body action
*/
class BodyAction {
private:

    //! not used
    BodyAction( const BodyAction & );
    //! not used
    BodyAction & operator=( const BodyAction & );

protected:
    /*!
      \brief nothing to do. but accessible only from derived
      classes.
     */
    BodyAction()
      { }

public:
    /*!
      \brief nothing to do, but should be virtual.
     */
    virtual
    ~BodyAction()
      { }

    /*!
      \brief pure virtual. set command to the action effector
      \retval true if action is performed
      \retval false if action is failed or not needed.
     */
    virtual
    bool execute( PlayerAgent * agent ) = 0;
};

/////////////////////////////////////////////////////////////////////

/*!
  \class NeckAction
  \brief abstract turn neck action
*/
class NeckAction {
private:

    //! not used
    NeckAction( const NeckAction & );
    //! not used
    NeckAction & operator=( const NeckAction & );

protected:
    /*!
      \brief nothing to do. but accessible only from derived
      classes.
     */
    NeckAction()
      { }

public:
    /*!
      \brief nothing to do, but should be virtual.
     */
    virtual
    ~NeckAction()
      { }

    /*!
      \brief pure virtual. set command to the action effector
      \retval true if action is performed
      \retval false if action is failed or not needed.
     */
    virtual
    bool execute( PlayerAgent * agent ) = 0;

    /*!
      \brief create cloned action object
      \return pointer to the cloned object instance.
     */
    virtual
    NeckAction * clone() const = 0;
};

/////////////////////////////////////////////////////////////////////

/*!
  \class ViewAction
  \brief abstract change view action
*/
class ViewAction {
private:

    //! not used
    ViewAction( const ViewAction & );
    //! not used
    ViewAction & operator=( const ViewAction & );

protected:
    /*!
      \brief nothing to do. but accessible only from derived
      classes.
     */
    ViewAction()
      { }

public:
    /*!
      \brief nothing to do, but should be virtual.
     */
    virtual
    ~ViewAction()
      { }

    /*!
      \brief pure virtual. set command to the action effector
      \retval true if action is performed
      \retval false if action is failed or not needed.
     */
    virtual
    bool execute( PlayerAgent * agent ) = 0;

    /*!
      \brief create cloned action object
      \return pointer to the cloned object instance.
     */
    virtual
    ViewAction * clone() const = 0;
};

/////////////////////////////////////////////////////////////////////

/*!
  \class ArmAction
  \brief abstract pointto action
*/
class ArmAction {
private:

    //! not used
    ArmAction( const ArmAction & );
    //! not used
    ArmAction & operator=( const ArmAction & );

protected:
    /*!
      \brief nothing to do. but accessible only from derived
      classes.
     */
    ArmAction()
      { }

public:
    /*!
      \brief nothing to do, but should be virtual.
     */
    virtual
    ~ArmAction()
      { }

    /*!
      \brief pure virtual. set command to the action effector
      \retval true if action is performed
      \retval false if action is failed or not needed.
     */
    virtual
    bool execute( PlayerAgent * agent ) = 0;

    /*!
      \brief create cloned action object
      \return pointer to the cloned object instance.
     */
    virtual
    ArmAction * clone() const = 0;
};

/////////////////////////////////////////////////////////////////////

/*!
  \class SoccerBehavior
  \brief abstract player behavior.
*/
class SoccerBehavior {
private:
    //! not used
    SoccerBehavior( const SoccerBehavior & );
    //! not used
    SoccerBehavior & operator=( const SoccerBehavior & );

protected
    /*!
      \brief nothing to do. but accessible only from derived
      classes.
     */:
    SoccerBehavior()
      { }

public:
    /*!
      \brief nothing to do, but should be virtual.
     */
    virtual
    ~SoccerBehavior()
      { }

    /*!
      \brief pure virtual. set command to the action effector
      \retval true if action is performed
      \retval false if action is failed or not needed.
     */
    virtual
    bool execute( PlayerAgent * agent ) = 0;
};

}

#endif
