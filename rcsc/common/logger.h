// -*-c++-*-

/*!
  \file logger.h
  \brief Logger class Header File
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

#ifndef RCSC_PLAYER_LOGGER_H
#define RCSC_PLAYER_LOGGER_H

#include <rcsc/geom/vector_2d.h>
#include <rcsc/geom/circle_2d.h>
#include <rcsc/geom/rect_2d.h>
#include <rcsc/geom/triangle_2d.h>

#include <boost/cstdint.hpp>

#include <cstdio>

namespace rcsc {

class GameTime;

/*!
  \class Logger
  \brief log output manager
*/
class Logger {
public:

    static const boost::int32_t LEVEL_00 = 0x00000000;
    static const boost::int32_t LEVEL_01 = 0x00000001;
    static const boost::int32_t LEVEL_02 = 0x00000002;
    static const boost::int32_t LEVEL_03 = 0x00000004;
    static const boost::int32_t LEVEL_04 = 0x00000008;
    static const boost::int32_t LEVEL_05 = 0x00000010;
    static const boost::int32_t LEVEL_06 = 0x00000020;
    static const boost::int32_t LEVEL_07 = 0x00000040;
    static const boost::int32_t LEVEL_08 = 0x00000080;
    static const boost::int32_t LEVEL_09 = 0x00000100;
    static const boost::int32_t LEVEL_10 = 0x00000200;
    static const boost::int32_t LEVEL_11 = 0x00000400;
    static const boost::int32_t LEVEL_12 = 0x00000800;
    static const boost::int32_t LEVEL_13 = 0x00001000;
    static const boost::int32_t LEVEL_14 = 0x00002000;
    static const boost::int32_t LEVEL_15 = 0x00004000;
    static const boost::int32_t LEVEL_16 = 0x00008000;
    static const boost::int32_t LEVEL_17 = 0x00010000;
    static const boost::int32_t LEVEL_18 = 0x00020000;
    static const boost::int32_t LEVEL_19 = 0x00040000;
    static const boost::int32_t LEVEL_20 = 0x00080000;
    static const boost::int32_t LEVEL_21 = 0x00100000;
    static const boost::int32_t LEVEL_22 = 0x00200000;
    static const boost::int32_t LEVEL_23 = 0x00400000;
    static const boost::int32_t LEVEL_24 = 0x00800000;
    static const boost::int32_t LEVEL_25 = 0x01000000;
    static const boost::int32_t LEVEL_26 = 0x02000000;
    static const boost::int32_t LEVEL_27 = 0x04000000;
    static const boost::int32_t LEVEL_28 = 0x08000000;
    static const boost::int32_t LEVEL_29 = 0x10000000;
    static const boost::int32_t LEVEL_30 = 0x20000000;
    static const boost::int32_t LEVEL_31 = 0x40000000;
    static const boost::int32_t LEVEL_32 = 0x80000000;
    static const boost::int32_t LEVEL_ANY = 0xffffffff;

    /*************************************************
    Log Message Line Format:
    Line := <Time> <Level> <Type> <Content>
    Time := integer value
    Level := integer value
    Type :=  T | p | l | c | t | r | m
    Text := <Str>
    Point := <x:Real> <y:Real>[ <Color>]
    Line := <x1:Real> <y1:Real> <x2:Real> <y2:Real>[ <Color>]
    Circle := <x:Real> <y:Real> <r:Real>[ <Color>]
    Triangle := <x1:Real> <y1:Real> <x2:Real> <y2:Real> <x3:Real> <y3:Real>[ <Color>]
    Rectangle := <leftX:Real> <topY:Real> <width:Real> <height:Real>[ <Color>]
    Message := <x:Real> <y:Real>[ (c <Color>)] <Str>
    **************************************************/

    static const boost::int32_t SYSTEM    = LEVEL_01;
    static const boost::int32_t SENSOR    = LEVEL_02;
    static const boost::int32_t WORLD     = LEVEL_03;
    static const boost::int32_t ACTION    = LEVEL_04;
    static const boost::int32_t INTERCEPT = LEVEL_05;
    static const boost::int32_t KICK      = LEVEL_06;
    static const boost::int32_t DRIBBLE   = LEVEL_07;
    static const boost::int32_t PASS      = LEVEL_08;
    static const boost::int32_t CROSS     = LEVEL_09;
    static const boost::int32_t SHOOT     = LEVEL_10;
    static const boost::int32_t CLEAR     = LEVEL_11;
    static const boost::int32_t TEAM      = LEVEL_12;
    static const boost::int32_t ROLE      = LEVEL_13;

private:

    //! const pointer to GameTime instance
    const GameTime * M_time;

    //! output file stream
    FILE* M_fout;

    //! log level flag
    boost::int32_t M_flags;

public:
    /*!
      \brief allocate message buffer memory
     */
    Logger();

    /*!
      \brief if file is opened, flush buffer and close file.
     */
    ~Logger();

    /*!
      \brief set new flag Id
      \param time const pointer to the game time instance
      \param id new flag Id
      \param on if true, set flag for id
     */
    void setLogFlag( const GameTime * time,
                     const boost::int32_t id,
                     const bool on = true );

    /*!
      \brief check if level is included
      \param id checked flag Id
      \return true if id is included in flags
     */
    bool isLogFlag( const boost::int32_t id ) const
      {
          return ( M_flags & id );
      }

    /*!
      \brief open file to record
      \param file_path file path to open
     */
    void open( const char * file_path );

    /*!
      \brief check if file is opend
      \return true if file is opened
     */
    bool isOpen()
      {
          return ( M_fout != NULL );
      }

    /*!
      \brief put message to file directry. no flush. Do NOT use this method by yourself.
      \param msg message
     */
    void print( const char * msg );

    /*!
      \brief flush stored message
    */
    void flush();

    /*!
      \brief clear buffer without flush
    */
    void clear();


    /*!
      \brief add free message to buffer with cycle, level & message tag 'T'
      \param id debug flag id
      \param msg message
     */
    void addText( const boost::int32_t id,
                  char * msg, ... );

    /*!
      \brief add point info to buffer with cycle, level & message tag 'p'
      \param x point coordinate x
      \param y point coordinate y
      \param color color name string
     */
    void addPoint( const boost::int32_t id,
                   const double & x,
                   const double & y,
                   const char * color = NULL );

    void addPoint( const boost::int32_t id,
                   const Vector2D & pos,
                   const char * color = NULL )
      {
          addPoint( id, pos.x, pos.y, color );
      }

    /*!
      \brief add point info to buffer with cycle, level & message tag 'p'
      \param x point coordinate x
      \param y point coordinate y
      \param r red value
      \param g green value
      \param b blue value
     */
    void addPoint( const boost::int32_t id,
                   const double & x,
                   const double & y,
                   const char r, const char g, const char b );

    void addPoint( const boost::int32_t id,
                   const Vector2D & pos,
                   const char r, const char g, const char b )
      {
          addPoint( id, pos.x, pos.y, r, g, b );
      }

    /*!
      \brief add line info to buffer with cycle, level & message tag 'l'
      \param x1 line start point coordinate x
      \param y1 line start point coordinate y
      \param x2 line end point coordinate x
      \param y2 line end point coordinate y
      \param color color name string
     */
    void addLine( const boost::int32_t id,
                  const double & x1,
                  const double & y1,
                  const double & x2,
                  const double & y2,
                  const char * color = NULL );

    void addLine( const boost::int32_t id,
                  const Vector2D & start,
                  const Vector2D & end,
                  const char * color = NULL )
      {
          addLine( id, start.x, start.y, end.x, end.y, color );
      }

    /*!
      \brief add line info to buffer with cycle, level & message tag 'l'
      \param x1 line start point coordinate x
      \param y1 line start point coordinate y
      \param x2 line end point coordinate x
      \param y2 line end point coordinate y
      \param r red value
      \param g green value
      \param b blue value
     */
    void addLine( const boost::int32_t id,
                  const double & x1,
                  const double & y1,
                  const double & x2,
                  const double & y2,
                  const char r, const char g, const char b );

    void addLine( const boost::int32_t id,
                  const Vector2D & start,
                  const Vector2D & end,
                  const char r, const char g, const char b )
      {
          addLine( id, start.x, start.y, end.x, end.y, r, g, b );
      }

    /*!
      \brief add circle info to buffer with cycle, level & message tag 'c'
      \param x circle center point coordinate x
      \param y circle center point coordinate y
      \param radius circle radius
      \param color color name string
     */
    void addCircle( const boost::int32_t id,
                    const double & x,
                    const double & y,
                    const double & radius,
                    const char * color = NULL );

    void addCircle( const boost::int32_t id,
                    const Vector2D & center,
                    const double & radius,
                    const char * color = NULL )
      {
          addCircle( id, center.x, center.y, radius, color );
      }

    void addCircle( const boost::int32_t id,
                    const Circle2D & circle,
                    const char * color = NULL )
      {
          addCircle( id, circle.center().x, circle.center().y, circle.radius(), color );
      }

    /*!
      \brief add circle info to buffer with cycle, level & message tag 'c'
      \param x circle center point coordinate x
      \param y circle center point coordinate y
      \param radius circle radius
      \param r red value
      \param g green value
      \param b blue value
     */
    void addCircle( const boost::int32_t id,
                    const double & x,
                    const double & y,
                    const double & radius,
                    const char r, const char g, const char b );

    void addCircle( const boost::int32_t id,
                    const Vector2D & center,
                    const double & radius,
                    const char r, const char g, const char b )
      {
          addCircle( id, center.x, center.y, radius, r, g, b );
      }

    void addCircle( const boost::int32_t id,
                    const Circle2D & circle,
                    const char r, const char g, const char b )
      {
          addCircle( id,
                     circle.center().x, circle.center().y, circle.radius(),
                     r, g, b );
      }

    /*!
      \brief add triangle info to buffer with cycle, level & message tag 't'
      \param x1 line 1st point coordinate x
      \param y1 line 1st point coordinate y
      \param x2 line 2nd point coordinate x
      \param y2 line 2nd point coordinate y
      \param x3 line 3rd point coordinate x
      \param y3 line 3rd point coordinate y
      \param color color name string
     */
    void addTriangle( const boost::int32_t id,
                      const double & x1,
                      const double & y1,
                      const double & x2,
                      const double & y2,
                      const double & x3,
                      const double & y3,
                      const char * color = NULL );

    void addTriangle( const boost::int32_t id,
                      const Vector2D & p1,
                      const Vector2D & p2,
                      const Vector2D & p3,
                      const char * color = NULL )
      {
          addTriangle( id,
                       p1.x, p1.y,
                       p2.x, p2.y,
                       p3.x, p3.y,
                       color );
      }

    void addTriangle( const boost::int32_t id,
                      const Triangle2D & tri,
                      const char * color = NULL )
      {
          addTriangle( id,
                       tri.a().x, tri.a().y,
                       tri.b().x, tri.b().y,
                       tri.c().x, tri.c().y,
                       color );
      }

    /*!
      \brief add triangle info to buffer with cycle, level & message tag 't'
      \param x1 line 1st point coordinate x
      \param y1 line 1st point coordinate y
      \param x2 line 2nd point coordinate x
      \param y2 line 2nd point coordinate y
      \param x3 line 3rd point coordinate x
      \param y3 line 3rd point coordinate y
      \param r red value
      \param g green value
      \param b blue value
     */
    void addTriangle( const boost::int32_t id,
                      const double & x1,
                      const double & y1,
                      const double & x2,
                      const double & y2,
                      const double & x3,
                      const double & y3,
                      const char r, const char g, const char b );

    void addTriangle( const boost::int32_t id,
                      const Vector2D & p1,
                      const Vector2D & p2,
                      const Vector2D & p3,
                      const char r, const char g, const char b )
      {
          addTriangle( id,
                       p1.x, p1.y,
                       p2.x, p2.y,
                       p3.x, p3.y,
                       r, g, b );
      }

    void addTriangle( const boost::int32_t id,
                      const Triangle2D & tri,
                      const char r, const char g, const char b )
      {
          addTriangle( id,
                       tri.a().x, tri.a().y,
                       tri.b().x, tri.b().y,
                       tri.c().x, tri.c().y,
                       r, g, b );
      }

    /*!
      \brief add rect info to buffer with cycle, level & message tag 'r'
      \param left top left point coordinate x
      \param top top left point coordinate y
      \param length x range of the rectangle
      \param width y range of the rectangle
      \param color color name string
     */
    void addRect( const boost::int32_t id,
                  const double & left,
                  const double & top,
                  const double & length,
                  const double & width,
                  const char * color = NULL );

    void addRect( const boost::int32_t id,
                  const Rect2D & rect,
                  const char * color = NULL )
      {
          addRect( id,
                   rect.left(), rect.top(),
                   rect.size().length(), rect.size().width(),
                   color );
      }

    /*!
      \brief add rect info to buffer with cycle, level & message tag 'r'
      \param left top left point coordinate x
      \param top top left point coordinate y
      \param length x range of the rectangle
      \param width y range of the rectangle
      \param r red value
      \param g green value
      \param b blue value
     */
    void addRect( const boost::int32_t id,
                  const double & left,
                  const double & top,
                  const double & length,
                  const double & width,
                  const char r, const char g, const char b );

    void addRect( const boost::int32_t id,
                  const Rect2D & rect,
                  const char r, const char g, const char b )
      {
          addRect( id,
                   rect.left(), rect.top(),
                   rect.size().length(), rect.size().width(),
                   r, g, b );
      }

    /*!
      \brief add message info to buffer with cycle, level & message tag 'm'
      \param x text drawed point coordinate x
      \param y text drawed point coordinate y
      \param msg drawd text
      \param color color name string
     */
    void addMessage( const boost::int32_t id,
                     const double & x,
                     const double & y,
                     const char * msg,
                     const char * color = NULL );

    void addMessage( const boost::int32_t id,
                     const Vector2D & pos,
                     const char * msg,
                     const char * color = NULL )
      {
          addMessage( id,
                      pos.x, pos.y, msg,
                      color );
      }

    /*!
      \brief add message info to buffer with cycle, level & message tag 'm'
      \param x text drawed point coordinate x
      \param y text drawed point coordinate y
      \param msg drawd text
      \param r red value
      \param g green value
      \param b blue value
     */
    void addMessage( const boost::int32_t id,
                     const double & x,
                     const double & y,
                     const char * msg,
                     const char r, const char g, const char b );

    void addMessage( const boost::int32_t id,
                     const Vector2D & pos,
                     const char * msg,
                     const char r, const char g, const char b )
      {
          addMessage( id,
                      pos.x, pos.y, msg,
                      r, g, b );
      }

};

//! global variable
extern Logger dlog;

}

#endif
