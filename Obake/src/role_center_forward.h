// -*-c++-*-

/*
*Copyright:

Copyright Copyright (C) Hidehisa AKIYAMA

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

/*
In this file, the code which is between 
"begin of the added code" and "end of the added code" 
has been added by Shogo TAKAGI
*/

/*
*Copyright:

Copyright (C) Shogo TAKAGI

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*EndCopyright:
*/

/////////////////////////////////////////////////////////////////////

#ifndef AGENT2D_ROLE_CENTER_FORWARD_H
#define AGENT2D_ROLE_CENTER_FORWARD_H

#include "soccer_role.h"

class RoleCenterForward
    : public SoccerRole {
private:
    //begin of the added code
    /********************************************/
    static int  S_mark_number;
    static int  S_mark_number_count;
    static int  S_pass_cycle;
    static int  S_previous_ball_keeper;
    static int  S_current_ball_keeper;
    static int  S_current_ball_keeper_count;
    static int  S_current_ball_keeper_total;
    static rcsc::Vector2D S_mark_position;
    /********************************************/
    //end of the added code
public:
    explicit
    RoleCenterForward( boost::shared_ptr< const rcsc::Formation > f )
        : SoccerRole( f )
      { }

    ~RoleCenterForward()
      { }

    virtual
    void execute( rcsc::PlayerAgent * agent );

    static
    std::string name()
      {
          return std::string( "CenterForward" );
      }
    static
    SoccerRole * create( boost::shared_ptr< const rcsc::Formation > f )
      {
          return ( new RoleCenterForward( f ) );
      }
private:

    void doKick( rcsc::PlayerAgent * agent );

    void doMove( rcsc::PlayerAgent * agent );
};

#endif
