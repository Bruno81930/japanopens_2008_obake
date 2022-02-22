// -*-c++-*-

/*!
  \file timer.cpp
  \brief milli second timer Source File
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

#include <rcsc/timer.h>

#include <iostream>
#include <cstdio> // perror
#include <sys/time.h> // struct timeval, gettimeofday()

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

 */
void
TimeStamp::setCurrent()
{
    // in <sys/time.h>
    // struct timeval {
    //   long tv_sec;  // seconds
    //   long tv_usec; // microseconds
    // };

    struct timeval current; // has long tv_sec and long tv_usec

    if ( ::gettimeofday( &current, NULL ) == -1 )
    {
        std::perror( "TimeStamp::SetCurrent" );
        M_sec = M_usec = 0;
    }
    else
    {
        M_sec = current.tv_sec;
        M_usec = current.tv_usec;
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
long
TimeStamp::getMSecDiffFrom( const TimeStamp & old_time ) const
{
    long sec_diff = 0, usec_diff = 0;
    calc_time_diff( *this, old_time,
                    sec_diff, usec_diff );

    return sec_diff * 1000 + usec_diff / 1000;
}

/*-------------------------------------------------------------------*/
/*!

 */
double
TimeStamp::getRealMSecDiffFrom( const TimeStamp & old_time ) const
{
    long sec_diff = 0, usec_diff = 0;
    calc_time_diff( *this, old_time,
                    sec_diff, usec_diff );

    return static_cast< double >( sec_diff * 1000 )
        + static_cast< double >( usec_diff ) * 0.001;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
TimeStamp::calc_time_diff( const TimeStamp & new_time,
                           const TimeStamp & old_time,
                           long & sec_diff,
                           long & usec_diff )
{
    sec_diff = new_time.sec() - old_time.sec();
    usec_diff = new_time.usec() - old_time.usec();

    while ( usec_diff >= (1000 * 1000) )
    {
        usec_diff -= 1000 * 1000;
        ++sec_diff;
    }

    while ( usec_diff < 0 && sec_diff > 0 )
    {
        usec_diff += (1000 * 1000);
        --sec_diff;
    }

    // error check
    if ( usec_diff < 0 )
    {
        std::cerr << "TimeStmap::calc_time_diff usec must be >=0 "
                  << "  old_sec=" << old_time.sec()
                  << "  old_usec=" << old_time.usec()
                  << "  new_sec=" << new_time.sec()
                  << "  new_usec=" << new_time.usec()
                  << std::endl;
        sec_diff = usec_diff = 0;
    }
}

/////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------*/
/*!

*/
long
MSecTimer::elapsed() const
{
    TimeStamp end_time;
    end_time.setCurrent();
    return end_time.getMSecDiffFrom( M_start_time );
}

/*-------------------------------------------------------------------*/
/*!

*/
double
MSecTimer::elapsedReal() const
{
    TimeStamp end_time;
    end_time.setCurrent();
    return end_time.getRealMSecDiffFrom( M_start_time );
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
MSecTimer::print( std::ostream & os ) const
{
    os << "elapsed: " << elapsedReal() << " [msec]"
       << std::endl;
    return os;
}

}
