// -*-c++-*-

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA

 This code is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2, or (at your option)
 any later version.

 This code is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this code; see the file COPYING.  If not, write to
 the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

 *EndCopyright:
 */

/////////////////////////////////////////////////////////////////////

#ifndef AGENT2D_BHV_BASIC_TACKLE_H
#define AGENT2D_BHV_BASIC_TACKLE_H

#include <rcsc/player/soccer_action.h>

class Bhv_BasicTackle
    : public rcsc::SoccerBehavior {
private:
    const double M_min_prob;
    const double M_body_thr;
public:
    Bhv_BasicTackle( const double & min_prob,
                     const double & body_thr )
        : M_min_prob( min_prob )
        , M_body_thr( body_thr )
      { }

    bool execute( rcsc::PlayerAgent * agent );
};

#endif
