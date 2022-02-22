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

#include "obake_fuzzy_grade_model.h"

double
Obake_FuzzyGradeModel::straightDown(const double &x,
				    const double &x1,
				    const double &x2)
{
    const double k = -1.0 / (x2 - x1);
    double grade = 0.0;
    if(x <= x1)
    {
        grade = 1;
    }
    else if(x > x1 && x < x2)
    {
        grade = 1 + k * (x - x1);
    }
  
    return grade;
}

double
Obake_FuzzyGradeModel::straightUp(const double &x,
				  const double &x1,
				  const double &x2)
{
    const double k = 1.0 / (x2 - x1);
    double grade = 1.0;
    if(x <= x1)
    {
        grade = 0.0;
    }
    else if(x > x1 && x < x2)
    {
        grade = k * (x - x1);
    }
    
    return grade;
}

double
Obake_FuzzyGradeModel::straightUp(const int &x,
				  const int &x1,
				  const int &x2)
{
    const double k = 1.0 / (x2 - x1);
    double grade = 1.0;
    if(x <= x1)
    {
        grade = 0.0;
    }
    else if(x > x1 && x < x2)
    {
        grade = k * (x - x1);
    }
    
    return grade;
}


double
Obake_FuzzyGradeModel::triangle(const double &x,
				const double &x1,
				const double &x2)
{
    double grade, k;
    grade = 0.0;
    if(x <= x1)
    {
        grade = 0.0;
    }
    else if(x > x1 && x <= x1 + (x2 - x1) / 2)
    {
        k = 1.0 / ((x2 - x1) / 2);
        grade =  k * (x - x1);
    }
    else if(x > x1 + (x2 - x1) / 2 && x < x2)
    {
        k = -1.0 / ((x2 - x1) / 2);
        grade = 1 + k * (x - x1 - (x2 - x1) / 2);
    }

    return grade;
}

double
Obake_FuzzyGradeModel::trapezoid(const double &x,
				 const double &x1,
				 const double &x2,
				 const double &x3,
				 const double &x4)
{
    double grade, k;
    grade = 0.0;
    if(x > x1 && x < x2)
    {
        k = 1.0 / (x2 - x1);
        grade = k * (x - x1);
    }
    else if(x >= x2 && x <= x3)
    {
        grade = 1.0;
    }
    else if(x > x3 && x < x4)
    {
        k = -1.0 / (x4 - x3);
        grade = 1.0 + k * (x - x3);
    }    
    
    return grade;
}










