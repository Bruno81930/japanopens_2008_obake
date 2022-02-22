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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "obake_strategy.h"

ObakeStrategy::Area
ObakeStrategy::getArea(const rcsc::Vector2D &search_point)
{
    if(search_point.x >= 39.0)
    {
        if(search_point.absY() <= 14.0)
        {
            return ShootChance;
        }
        else
        {
            return Cross;
        }
    }
    else if(search_point.x >= 25.0
            && search_point.absY() <= 11.0)
    {
        return StrongAttack;
    }
    return Attack;
}

