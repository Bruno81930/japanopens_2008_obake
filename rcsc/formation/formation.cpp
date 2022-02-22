// -*-c++-*-

/*!
	\file formation.cpp
	\brief formation data classes Source File.
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

#include "formation.h"

#include <sstream>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

 */
Formation::Formation()
{
    for ( int i = 0; i < 11; ++i )
    {
        M_synmetry_number[i] = -1;
    }
}


/*-------------------------------------------------------------------*/
/*!

 */
void
Formation::setCenterType( const int unum )
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " *** ERROR *** invalid unum " << unum
                  << std::endl;
        return;
    }

    M_synmetry_number[unum - 1] = 0;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Formation::setSideType( const int unum )
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " *** ERROR *** invalid unum " << unum
                  << std::endl;
        return;
    }

    M_synmetry_number[unum - 1] = -1;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
Formation::setSynmetryType( const int unum,
                            const int synmetry_unum )
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " *** ERROR *** invalid unum " << unum
                  << std::endl;
        return false;
    }
    if ( synmetry_unum < 1 || 11 < synmetry_unum )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " *** ERROR *** invalid synmetry_unum " << unum
                  << std::endl;
        return false;
    }
    if ( synmetry_unum == unum )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " *** ERROR *** Never synmetry itself. unum = " << unum
                  << "  mirro = " << synmetry_unum
                  << std::endl;
        return false;
    }
    if ( M_synmetry_number[synmetry_unum - 1] > 0 )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " *** ERROR *** " << synmetry_unum << " is already synmetryed player. "
                  << std::endl;
        return false;
    }
    if ( M_synmetry_number[synmetry_unum - 1] == 0 )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " *** ERROR *** " << synmetry_unum << " is center type player. "
                  << std::endl;
        return false;
    }


    // check if unum is already assigned as original side type player.
    for ( int i = 0; i < 11; ++i )
    {
        if ( i + 1 == unum ) continue;
        if ( M_synmetry_number[i] == synmetry_unum )
        {
            // refered by other player.
            std::cerr << __FILE__ << ":" << __LINE__
                      << " *** ERROR *** player " << unum
                      << " has already refered by player " << i + 1
                      << std::endl;
            return false;
        }
    }

    M_synmetry_number[unum - 1] = synmetry_unum;

    setRoleName( unum, getRoleName( synmetry_unum ) );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
Formation::updateRole( const int unum,
                       const int synmetry_unum,
                       const std::string & role_name )
{
    if ( getSynmetryNumber( unum ) != synmetry_unum )
    {
        if ( synmetry_unum == 0 )
        {
            createNewRole( unum, role_name, Formation::CENTER );
            return true;
        }

        if ( synmetry_unum < 0 )
        {
            createNewRole( unum, role_name, Formation::SIDE );
            return true;
        }

        // ( synmetry_unum > 0 )

        if ( ! isSideType( synmetry_unum ) )
        {
            std::cerr << __FILE__ << ":" << __LINE__
                      << " You cannot use the player number " << synmetry_unum
                      << " as a synmetry number."
                      << std::endl;
            return false;
        }

        for ( int i = 1; i <= 11; ++i )
        {
            if ( i == unum || i == synmetry_unum ) continue;
            if ( getSynmetryNumber( i ) == synmetry_unum )
            {
                std::cerr << __FILE__ << ":" << __LINE__
                          << " player number " << synmetry_unum
                          << " has already refered by player " << i
                          << std::endl;
                return false;
            }
        }

        setSynmetryType( unum, synmetry_unum );
        return true;
    }

    if ( ! isSynmetryType( unum )
         && getRoleName( unum ) != role_name )
    {
        setRoleName( unum, role_name );
        return true;
    }

    return false;
}

/*-------------------------------------------------------------------*/
/*!

 */
int
Formation::readName( std::istream & is )
{
    int n_line = 0;
    std::string line_buf;

    while ( std::getline( is, line_buf ) )
    {
        ++n_line;
        std::istringstream istr( line_buf );

        std::string tag;
        istr >> tag;
        if ( tag.empty()
             || tag[0] == '#'
             || ! tag.compare( 0, 2, "//" ) )
        {
            continue;
        }

        if ( tag != "Formation" )
        {
            std::cerr << __FILE__ << ":" << __LINE__
                      << " *** ERROR *** Found invalid tag ["
                      << tag << "]"
                      << std::endl;
            return -1;
        }

        std::string name_str;
        istr >> name_str;
        if ( name_str != methodName() )
        {
            std::cerr << __FILE__ << ":" << __LINE__
                      << " *** ERROR *** Invalid formation type name. name must be "
                      << methodName()
                      << std::endl;
            return -1;
        }

        return n_line;
    }

    return -1;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
Formation::printName( std::ostream & os ) const
{
    os << "Formation " << methodName() << '\n';
    return os;
}

}
