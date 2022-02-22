// -*-c++-*-

/*!
  \file factory.h
  \brief parser factory utility Header File.
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

#ifndef RCSC_RCG_FACTORY_H
#define RCSC_RCG_FACTORY_H

#include <rcsc/rcg/parser.h>
#include <rcsc/rcg/serializer.h>

namespace rcsc {
namespace rcg {

/*!
  \brief make the suitable version parser instance depending on input
  \param ism reference to the imput stream.
  \return smart pointer to the rcg parser instance

  after checking, the pionted index of istreambuf becomes 4.
*/
ParserPtr
make_parser( std::istream & is );

/*!
  \brief make the suitable version serializer instance
  \param version required rcg format version
  \return smart pointer to the rcg serializer instance
 */
SerializerPtr
make_serializer( const int version );

} // end of namespace
} // end of namespace

#endif
