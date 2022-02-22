// -*-c++-*-

/*!
  \file soccer_agent.cpp
  \brief abstract soccer agent class Source File.
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

#include "soccer_agent.h"

#include "basic_client.h"

#include <rcsc/param/cmd_line_parser.h>
#include <iostream>
#include <cassert>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
SoccerAgent::SoccerAgent()
    : M_client( static_cast< BasicClient * >( 0 ) )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
SoccerAgent::~SoccerAgent()
{
    //std::cerr << "delete SoccerAgent" << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
SoccerAgent::init( BasicClient * client,
                   const int argc,
                   const char * const * argv )
{
    assert( client );

    M_client = client;

    CmdLineParser cmd_parser( argc, argv );
    return initImpl( cmd_parser );
}

}
