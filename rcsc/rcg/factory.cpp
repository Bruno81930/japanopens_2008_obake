// -*-c++-*-

/*!
  \file factory.cpp
  \brief parser factory utility Source File.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "factory.h"

#include "serializer_v1.h"
#include "serializer_v2.h"
#include "serializer_v3.h"
#include "serializer_v4.h"

#include "parser_v1.h"
#include "parser_v2.h"
#include "parser_v3.h"
#include "parser_v4.h"

#include "types.h"

namespace rcsc {
namespace rcg {

/*-------------------------------------------------------------------*/
/*!

*/
ParserPtr
make_parser( std::istream & is )
{
    char header[5];
    int version = REC_OLD_VERSION;
    ParserPtr p;

    is.read( header, 4 ); // read 'U', 'L', 'G', <version>

    if ( is.gcount() != 4 )
    {
        return p;
    }

    if ( header[0] == 'U'
         && header[1] == 'L'
         && header[2] == 'G' )
    {
        version = static_cast< int >( header[3] );
    }

    if ( version == REC_OLD_VERSION )
    {
        p = ParserPtr( new ParserV1 );
    }
    else if ( version == REC_VERSION_2 )
    {
        p = ParserPtr( new ParserV2 );
    }
    else if ( version == REC_VERSION_3 )
    {
        p = ParserPtr( new ParserV3 );
    }
    else if ( version == static_cast< int >( '0' ) + REC_VERSION_4 )
    {
        p = ParserPtr( new ParserV4 );
    }

    return p;
}

/*-------------------------------------------------------------------*/
/*!

*/
SerializerPtr
make_serializer( const int version )
{
    SerializerPtr ptr;

    if ( version == REC_OLD_VERSION )
    {
        ptr = SerializerPtr( new SerializerV1 );
    }
    else if ( version == REC_VERSION_2 )
    {
        ptr = SerializerPtr( new SerializerV2 );
    }
    else if ( version == REC_VERSION_3 )
    {
        ptr = SerializerPtr( new SerializerV3 );
    }
    else if ( version == REC_VERSION_4 )
    {
        ptr = SerializerPtr( new SerializerV4 );
    }

    return ptr;
}

} // end of namespace
} // end of namespace
