// -*-c++-*-

/*!
  \file udp_socket.h
  \brief UDP connection socket class Header File.
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

#ifndef RCSC_NET_UDP_SOCKET_H
#define RCSC_NET_UDP_SOCKET_H

#include <rcsc/net/basic_socket.h>

#include <boost/scoped_ptr.hpp>

#include <cstddef>

namespace rcsc {

/*!
  \class UDPSocket
  \brief UDP/IP connection socket class
*/
class UDPSocket
    : public BasicSocket {
private:
    //! not used
    UDPSocket();
public:
    /*!
      \brief constructor for server socket
      \param port port number to receive packet.
    */
    explicit
    UDPSocket( const int port );

    /*!
      \brief constructor for client socket
      \param hostname remote host name (or IP address)
      \param port port number to send packet
     */
    UDPSocket( const char * hostname,
               const int port );

    /*!
      \brief destructor. close socket automatically
     */
    ~UDPSocket();

public:
     /*!
      \brief send diagram data to the connected host.
      \param data the pointer to the data to be sent.
      \param len the length of data.
      \return the length of sent data if successfuly sent, otherwise -1.
     */
    int send( const char * data,
              const std::size_t len );

    /*!
      \brief receive diagram data from the connected remote host.
      \param buf buffer to receive data
      \param len maximal length of buffer buf
      \param overwrite_dist_addr if this value is true,
      set distination address to sender address of this packet.
      \retval 0 error occured and errno is EWOULDBLOCK
      \retval -1 error occured
      \return the length of received data.
     */
    int receive( char * buf,
                 const std::size_t len );

};

} // end namespace

#endif
