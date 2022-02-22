// -*-c++-*-

/*!
  \file debug_client.h
  \brief Interface for Soccer Viewer & soccerwindow2 Header File
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

#ifndef RCSC_PLAYER_DEBUG_CLIENT_H
#define RCSC_PLAYER_DEBUG_CLIENT_H

#include <rcsc/geom/vector_2d.h>
#include <rcsc/geom/triangle_2d.h>
#include <rcsc/geom/rect_2d.h>
#include <rcsc/geom/circle_2d.h>

#include <boost/shared_ptr.hpp>

#include <fstream>
#include <vector>
#include <string>
#include <utility>

namespace rcsc {

class UDPSocket;
class WorldModel;

/*!
  \class DebugClient
  \brief Debug Server Interface class.

  Current supported debug servers:
  - Soccer_Viewer
  - soccerwindow2.
*/
class DebugClient {
public:
    enum ServerType {
        SoccerViewer,
        SoccerWindow2,
    };

    static const std::size_t MAX_LINE = 50;
    static const std::size_t MAX_TRIANGLE = 50;
    static const std::size_t MAX_RECT = 50;
    static const std::size_t MAX_CIRCLE = 50;

private:

    //! if false, all debug info are not created.
    bool M_on;

    //! flag to check connection.
    bool M_connected;

    //! connection to Soccer Viewer
    boost::shared_ptr< UDPSocket > M_socket;

    //! flag to check write mode
    bool M_write_mode;

    //! main buffer to output all
    std::string M_main_buffer;
    //! target number shown in display
    int M_target_unum;
    //! target point shown in display
    Vector2D M_target_point;
    //! message shown in display
    std::string M_message;

    //! lines shown in display
    std::vector< std::pair< Vector2D, Vector2D > > M_lines;
    //! triangle info to be drawn
    std::vector< Triangle2D > M_triangles;
    //! rectanble info to be drawn
    std::vector< Rect2D > M_rectangles;
    //! circle info to be drawn
    std::vector< Circle2D > M_circles;


public:
    /*!
      \brief init member variables
    */
    DebugClient();

    /*!
      \brief close connection
    */
    ~DebugClient();

    /*!
      \brief connect to the debug server
      \param hostname host name string thatdebug server is running
      \param port port number for debug server connection
    */
    void connect( const std::string & hostname,
                  const int port );

    /*!
      \brief set write mode
    */
    void setWriteMode( const bool on );

    /*!
      \brief output to stream or socket
      \param world const reference to the world mode object
    */
    void writeAll( const WorldModel & world );

private:
    /*!
      \brief close file and connection
    */
    void close();

    /*!
      \brief make world model message
      \param world const reference to the world mode object

      convert world model to Soccer Viewer log format
    */
    void toStr( const WorldModel & world );

    /*!
      \brief send debug message to the debug server
    */
    void send();

    /*!
      \brief write debug message to disk
      \param cycle current game cycle
    */
    void write( const long & cycle );

public:
    /*!
      \brief clear all data
    */
    void clear();

    /*!
      \brief add formated string to buffer
    */
    void addMessage( char * msg, ... );

    /*!
      \brief set target player
      \param unum target player's uniform number
    */
    void setTarget( const int unum )
      {
          M_target_unum = unum;
      }

    /*!
      \brief set target point
      \param p target point
    */
    void setTarget( const Vector2D & p )
      {
          M_target_point = p;
      }

    /*!
      \brief set line info to be drawn
      \param from line start point
      \param to line end point
    */
    void addLine( const Vector2D & from,
                  const Vector2D & to );

    /*!
      \brief set triangle info to be drawn
      \param v1 vertex 1
      \param v2 vertex 2
      \param v3 vertex 3
    */
    void addTriangle( const Vector2D & v1,
                      const Vector2D & v2,
                      const Vector2D & v3 )
      {
          if ( M_on )
          {
              addTriangle( Triangle2D( v1, v2, v3 ) );
          }
      }

    /*!
      \brief set triangle info to be drawn
      \param tri triangle object
    */
    void addTriangle( const Triangle2D & tri );

    /*!
      \brief set rectangle info to be drawn
      \param rect rectanble object
    */
    void addRectangle( const Rect2D & rect );

    /*!
      \brief set circle info to be drawn
      \param center center coordinate
      \param radius radius value
     */
    void addCircle( const Vector2D & center,
                    const double & radius )
      {
          addCircle( Circle2D( center, radius ) );
      }

    /*!
      \brief set circle info to be drawn
      \param circle circle object
     */
    void addCircle( const Circle2D & circle );
};

}

#endif
