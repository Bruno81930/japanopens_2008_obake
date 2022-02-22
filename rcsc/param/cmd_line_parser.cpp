// -*-c++-*-

/*!
  \file cmd_line_parser.cpp
  \brief command line argument parser Source File
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

#include "cmd_line_parser.h"

#include "param_map.h"

#include <iterator>
#include <algorithm>
#include <functional>
#include <cstdio>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
CmdLineParser::CmdLineParser( const int argc,
                              const char * const * argv,
                              const std::string & name_space )
    : M_namespace( name_space )
{
    //std::copy( argv + 1, argv + argc, std::back_inserter( M_args ) );
    for ( int i = 1; i < argc; ++i )
    {
        std::string arg( argv[i] );
        if ( arg == "=" )
        {
            continue;
        }

        std::string::size_type eq_pos = arg.find( '=' );
        if ( eq_pos == std::string::npos // no '='
             || eq_pos == 0 // 1st char is '='
             || eq_pos == arg.length() - 1 // last char is '='
             || arg[0] != '-' )
        {
            M_args.push_back( arg );
            continue;
        }

        std::string::size_type q_pos = arg.find_first_of( "\"\'" );
        if ( q_pos == std::string::npos
             || eq_pos < q_pos )
        {
            M_args.push_back( arg.substr( 0, eq_pos ) );
            M_args.push_back( arg.substr( eq_pos + 1, arg.length() - eq_pos - 1 ) );
            continue;
        }

        M_args.push_back( arg );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
CmdLineParser::CmdLineParser( const std::list< std::string > & args,
                              const std::string & name_space )
    : M_args( args )
    , M_namespace( name_space )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
bool
CmdLineParser::parse( ParamMap & param_map )
{
    const std::string name_space = ( M_namespace.empty()
                                     ? ""
                                     : M_namespace + "::" );
    const std::string::size_type len = name_space.length();

    std::list< std::string >::iterator it = M_args.begin();
    while ( it != M_args.end() )
    {
        // get parameter name stiring & parameter string
        std::string name_str;
        bool is_long_name = false;

        if ( len != 0
             && ! it->compare( 0, len, name_space ) )
        {
            is_long_name = true;
            name_str = it->substr( len );
        }
        else if ( ! it->compare( 0, 2, "--" ) )
        {
            is_long_name = true;
            name_str = it->substr( 2 );
        }
        else if ( it->length() > 1
                  && it->at( 0 ) == '-' )
        {
            name_str = it->substr( 1 );
        }
        else
        {
            ++it;
            continue;
        }

        if ( name_str.empty() )
        {
            std::cerr << "***ERROR*** CmdLineParser. Empty parameter name"
                      << std::endl;
            ++it;
            continue;
        }


        ParamPtr param_ptr = ( is_long_name
                               ? param_map.findLongName( name_str )
                               : param_map.findShortName( name_str ) );

        if ( ! param_ptr )
        {
            //std::cerr << "***ERROR*** CmdLineParser. Unknown option name ["
            //          << long_name << "]" << std::endl;
            ++it;
            continue;
        }

        // if parameter is switch type, always set true.
        if ( param_ptr->isSwitch() )
        {
            param_ptr->analyze( "" );
            it = M_args.erase( it );
            continue;
        }

        // No value argument
        std::list< std::string >::iterator value_it = it;
        ++value_it;
        if ( value_it == M_args.end() )
        {
            std::cerr << "***ERROR*** CmdLineParser. Value must be specified for [--"
                      << name_str << "]" << std::endl;
            ++it;
            continue;
        }

        // analyze value string
        if ( ! param_ptr->analyze( *value_it ) )
        {
            std::cerr << "***ERROR*** CmdLineParser. Invalid option. name=["
                      << name_str << "] value=[" << *value_it << "]"
                      << std::endl;
            ++it;
            continue;
        }

        ++value_it; // because the list::erase erases [first, last).
        it = M_args.erase( it, value_it );
    }

    parsePositional();

    return M_args.empty();
}


/*-------------------------------------------------------------------*/
/*!

*/
void
CmdLineParser::parsePositional()
{
    M_positional_options.clear();

    std::list< std::string >::iterator it = M_args.begin();
    while ( it != M_args.end() )
    {
        if ( ! it->compare( 0, 2, "--" )
             || ( it->length() > 1
                  && it->at( 0 ) == '-' )
             )
        {
            // This argument is a option name.
            // If argument string contains only '-', it becomes a positional argument.
        }
        else
        {
            M_positional_options.push_back( *it );
        }

        ++it;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
CmdLineParser::print( std::ostream & os ) const
{
    for ( std::list< std::string >::const_iterator it = M_args.begin();
          it != M_args.end();
          ++it )
    {
        os << *it << ' ';
    }

    return os << std::flush;
}

}
