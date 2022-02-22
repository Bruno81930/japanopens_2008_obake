// -*-c++-*-

/*!
  \file timer.h
  \brief milli second timer Header File
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

#ifndef RCSC_TIMER_H
#define RCSC_TIMER_H

#include <iostream>

namespace rcsc {

/*!
  \class TimeStamp
  \brief time stamp object
*/
class TimeStamp {
private:
    long M_sec; //!< tv_sec in struct timeval
    long M_usec; //!< tv_usec in struct timeval

public:
    /*!
      \brief set 0 to all time values
     */
    TimeStamp()
        : M_sec( 0 )
        , M_usec( 0 )
      { }

    /*!
      \brief set current time using ::gettimeofday
     */
    void setCurrent();

    /*!
      \brief accessor method
      \return second value
     */
    long sec() const
      {
          return M_sec;
      }

    /*!
      \brief accessor method
      \return micro second value
     */
    long usec() const
      {
          return M_usec;
      }

    /*!
      \brief calculate milli seconds difference from new_time to this
      \param new_time compared time stamp
      \return elapsed milli seconds from new_time to this
     */
    long getMSecDiffFrom( const TimeStamp & new_time ) const;

    /*!
      \brief calculate milli seconds difference from new_time to this
      \param new_time compared time stamp
      \return elapsed milli seconds by floating point number from new_time to this
     */
    double getRealMSecDiffFrom( const TimeStamp & new_time ) const;

    /*!
      \brief static utility that calculate milli seconds difference
      \param old_time compared time stamp
      \param new_time compared time stamp
      \param sec_diff reference to the solution variable
      \param usec_diff reference to the solution variable
     */
    static
    void calc_time_diff( const TimeStamp & old_time,
                         const TimeStamp & new_time,
                         long & sec_diff,
                         long & usec_diff );
};

/////////////////////////////////////////////////////////////////////

/*!
  \class MSecTimer
  \brief milli second stop watch
 */
class MSecTimer {
private:
    //! started time
    TimeStamp M_start_time;

public:
    /*!
      \brief set started time
     */
    MSecTimer()
      {
          M_start_time.setCurrent();
      }

    /*!
      \brief reset started time
     */
    void restart()
      {
          M_start_time.setCurrent();
      }

    /*!
      \brief retuned elapsed time
      \return elapsed milli seconde by long integer
     */
    long elapsed() const;

    /*!
      \brief retuned elapsed time
      \return elapsed milli seconde by floating point number
     */
    double elapsedReal() const;

    /*!
      \brief output elapsed time
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & print( std::ostream & os ) const;
};

}

/*-------------------------------------------------------------------*/
/*!
  \brief stream operator for MSecTimer
  \param os reference to the output stream
  \param t const reference to the MSecTimer object
  \return reference to the output stream
*/
inline
std::ostream &
operator<<( std::ostream & os,
            const rcsc::MSecTimer & t )
{
    return t.print( os );
}

#endif
