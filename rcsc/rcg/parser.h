// -*-c++-*-

/*!
  \file parser.h
  \brief abstract rcg parser class Header File.
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

#ifndef RCSC_RCG_PARSER_H
#define RCSC_RCG_PARSER_H

#include <boost/shared_ptr.hpp>

#include <istream>

namespace rcsc {
namespace rcg {

class Handler;

/////////////////////////////////////////////////////////////////////

/*!
  \class Parser
  \brief rcg stream parser interface class
*/
class Parser {
protected:

    /*!
      \brief constructor is accessible only from the derived classes.
     */
    Parser()
      { }

    /*!
      \brief virtual destructor
    */
    virtual
    ~Parser()
      { }

public:

    /*!
      \brief (pure virtual) get log version
      \return version number
     */
    virtual
    int version() const = 0;

    /*!
      \brief (pure virtual) analyze log data from input stream
      \param is reference to the imput stream (usually ifstream/gzifstream).
      \param handler reference to the rcg data handler.
      \retval true, if successfuly parsed.
      \retval false, if incorrect format is detected.
     */
    virtual
    bool parse( std::istream & is,
                Handler & handler ) const = 0;
};


//! typedef of the rcg parser pointer.
typedef boost::shared_ptr< Parser > ParserPtr;

} // end of namespace
} // end of namespace

#endif
