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

#ifndef BHV_OBAKE_RECEIVE_TEST_H
#define BHV_OBAKE_RECEIVE_TEST_H
#include <rcsc/player/soccer_action.h>

class Bhv_ObakeReceiveTest{
public:
    bool execute(rcsc::PlayerAgent * agent);
//    bool checkExistOpponentTest(rcsc::PlayerAgent * agent);
    void getBestMiddleTest(rcsc::PlayerAgent * agent);
    bool putNewVectorIntoSearchOrderTest(rcsc::PlayerAgent * agent);
};
#endif
