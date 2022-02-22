// -*-c++-*-

/*!
  \file formation_factory.cpp
  \brief formation factory method Source File.
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

#include "formation_factory.h"

#include "formation_static.h"
#include "formation_bpn.h"
#include "formation_dt.h"
#include "formation_knn.h"
#include "formation_ngnet.h"
#include "formation_rbf.h"
#include "formation_sbsp.h"
#include "formation_uva.h"

#include <fstream>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
FormationPtr
make_formation( std::istream & is )
{
    std::string temp, type;
    is >> temp >> type;

    return make_formation( type );
}

/*-------------------------------------------------------------------*/
/*!

*/
FormationPtr
make_formation( const std::string & type )
{
    FormationPtr ptr;

    if ( type == FormationStatic::name() )
    {
        ptr = FormationPtr( FormationStatic::create() );
    }
    else if ( type == FormationBPN::name() )
    {
        ptr = FormationPtr( FormationBPN::create() );
    }
    else if ( type == FormationDT::name() )
    {
        ptr = FormationPtr( FormationDT::create() );
    }
    else if ( type == FormationKNN::name() )
    {
        ptr = FormationPtr( FormationKNN::create() );
    }
    else if ( type == FormationNGNet::name() )
    {
        ptr = FormationPtr( FormationNGNet::create() );
    }
    else if ( type == FormationRBF::name() )
    {
        ptr = FormationPtr( FormationRBF::create() );
    }
    else if ( type == FormationSBSP::name() )
    {
        ptr = FormationPtr( FormationSBSP::create() );
    }
    else if ( type == FormationUvA::name() )
    {
        ptr = FormationPtr( FormationUvA::create() );
    }

    return ptr;
}

}
