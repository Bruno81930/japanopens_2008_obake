// -*-c++-*-

/*!
  \file formation_factory.h
  \brief formation factory method Header File.
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

#ifndef RCSC_FORMATION_FACTORY_H
#define RCSC_FORMATION_FACTORY_H

#include <rcsc/formation/formation.h>
#include <string>

namespace rcsc {

/*!
  \brief make the suitable type formation instance depending on the input file
  \param is input stream
  \return smart pointer to the formation instance
*/
FormationPtr
make_formation( std::istream & is );

/*!
  \brief make the suitable type formation instance depending on the input file
  \param type method type name
  \return smart pointer to the formation instance
*/
FormationPtr
make_formation( const std::string & type );

}

#endif
