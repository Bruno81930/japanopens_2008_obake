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

#ifndef OBAKE_FUZZY_GRADE_MODEL_H
#define OBAKE_FUZZY_GRADE_MODEL_H

class Obake_FuzzyGradeModel{
public:
    double straightDown(const double &x,
			const double &x1,
			const double &x2);
    double straightUp(const int &x,
		      const int &x1,
		      const int &x2);
    double straightUp(const double &x, 
		      const double &x1,
		      const double &x2);
    double triangle(const double &x,
		    const double &x1,
		    const double &x2);
    double trapezoid(const double &x,
		     const double &x1, 
		     const double &x2,
		     const double &x3,
		     const double &x4);
};

#endif
