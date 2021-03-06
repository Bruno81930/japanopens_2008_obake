// -*-c++-*-

/*!
  \file tcp_socket.cpp
  \brief TCP connection socket class Source File.
*/

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA, Hiroki SHIMORA

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

#include "tcp_socket.h"

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
TCPSocket::TCPSocket( const char * hostname,
                      const int port )
    : BasicSocket()
{
    if ( open( BasicSocket::STREAM_TYPE )
         && bind()
         && setAddr( hostname, port )
         && connectToPresetAddr() != -1 )
    {
        return;
    }

    this->close();
}

/*-------------------------------------------------------------------*/
/*!

*/
TCPSocket::~TCPSocket()
{
}

/*-------------------------------------------------------------------*/
/*!

*/
int
TCPSocket::connect()
{
    return BasicSocket::connectToPresetAddr();
}

/*-------------------------------------------------------------------*/
/*!

*/
int
TCPSocket::send( const char * data,
                 const std::size_t len )
{
    return BasicSocket::writeToStream( data, len );
}

/*-------------------------------------------------------------------*/
/*!

*/
int
TCPSocket::receive( char * buf,
                    std::size_t len )
{
    return BasicSocket::readFromStream( buf, len );
}

} // end namespace
