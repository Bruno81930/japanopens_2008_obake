// -*-c++-*-

/*!
  \file param_map.cpp
  \brief parameter registry map Source File
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

#include "param_map.h"

#include <functional>

namespace rcsc {

/*!
  \struct LongNamePredicate
  \brief function object to check if the parameter's long name is same
  or not.
 */
struct LongNamePredicate
    : public std::unary_function< ParamPtr, bool > {

    std::string name_; //!< name string to be compared

    /*!
      \brief construct with the compared long name string
      \param long_name the compared long name
     */
    explicit
    LongNamePredicate( const std::string & long_name )
        : name_( long_name )
      { }

    /*!
      \brief predicate operator
      \param arg compared parameter
      \return compared result
     */
    result_type operator()( const argument_type & arg ) const
      {
          return arg->longName() == name_;
      }
};

namespace {

/*-------------------------------------------------------------------*/
/*!

*/
bool
is_true( const std::string & value_str )
{
    return ( value_str == "true"
             || value_str == "on"
             || value_str == "1"
             || value_str == "yes" );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
is_false( const std::string & value_str )
{
    return ( value_str == "false"
             || value_str == "off"
             || value_str == "0"
             || value_str == "no" );
}

}

/*-------------------------------------------------------------------*/
/*!

*/
bool
ParamGeneric< bool >::analyze( const std::string & value_str )
{
    if ( value_str.empty() )
    {
        return false;
    }

    if ( is_true( value_str ) )
    {
        *M_value_ptr = true;
    }
    else if ( is_false( value_str ) )
    {
        *M_value_ptr = false;
    }
    else
    {
        std::cerr << __FILE__ << ':' << __LINE__
                  << " ***ERROR*** Unexpected value string: type bool. ["
                  << value_str << "]"
                  << std::endl;
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
ParamGeneric< bool >::printValue( std::ostream & os ) const
{
    return os << std::boolalpha << *M_value_ptr;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
ParamSwitch::analyze( const std::string & value_str )
{
    if ( value_str.empty()
         || is_true( value_str ) )
    {
        *M_value_ptr = true;
    }
    else if ( is_false( value_str ) )
    {
        *M_value_ptr = false;
    }
    else
    {
        std::cerr << __FILE__ << ':' << __LINE__
                  << " ***ERROR*** Unexpected value string: type switch. ["
                  << value_str << "]"
                  << std::endl;
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
ParamSwitch::printValue( std::ostream & os ) const
{
    os << ( *M_value_ptr ? "on" : "off" );
    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
ParamMap::Registrar &
ParamMap::Registrar::operator()( const std::string & long_name,
                                 const std::string & short_name,
                                 const BoolSwitch & value,
                                 const char * description )
{
    if ( value.ptr_ == static_cast< bool * >( 0 ) )
    {
        std::cerr << "***ERROR*** detected null pointer for the option "
                  << long_name << std::endl;
        return *this;
    }

    ParamPtr ptr ( new ParamSwitch( long_name,
                                    short_name,
                                    value.ptr_,
                                    description ) );
    M_param_map.add( ptr );

    return *this;
}

/*-------------------------------------------------------------------*/
/*!

*/
ParamMap &
ParamMap::add( ParamMap & param_map )
{
    if ( this == &param_map )
    {
        return *this;
    }

    for ( std::vector< ParamPtr >::iterator it = param_map.M_parameters.begin();
          it != param_map.M_parameters.end();
          ++it )
    {
        add( *it );
    }

    return *this;
}

/*-------------------------------------------------------------------*/
/*!

*/
ParamMap::Registrar &
ParamMap::add( ParamPtr param )
{
    if ( ! param )
    {
        std::cerr << __FILE__ << ':' << __LINE__
                  << " ***ERROR*** ParamMap::add(). "
                  << "detected null ParamPtr."
                  << std::endl;
        return M_registrar;
    }

    if ( param->longName().empty() )
    {
        std::cerr << __FILE__ << ':' << __LINE__
                  << " ***ERROR*** ParamMap::add(). "
                  << "Empty parameter name! parameter was not registered."
                  << std::endl;
        return M_registrar;
    }

    if ( param->longName().find( '=' ) != std::string::npos
         || param->shortName().find( '=' ) != std::string::npos )
    {
        std::cerr << " ***ERROR*** "
                  << " the option name [" << param->longName()
                  << "] or [" << param->shortName()
                  << "] contains an illegal character \'=\'."
                  << std::endl;
        return M_registrar;
    }

    if ( M_long_name_map.find( param->longName() ) != M_long_name_map.end()
         || M_short_name_map.find( param->shortName() ) != M_short_name_map.end() )
    {
        std::cerr << " ***ERROR*** "
                  << " the option name [" << param->longName()
                  << "] or [" << param->shortName()
                  << "] has already been registered."
                  << std::endl;
        return M_registrar;
    }

    M_parameters.push_back( param );

    M_long_name_map[ param->longName() ] = param;

    if ( ! param->shortName().empty() )
    {
        M_short_name_map[ param->shortName() ] = param;
    }

    return M_registrar;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ParamMap::remove( const std::string & long_name )
{
    M_parameters.erase( std::remove_if( M_parameters.begin(),
                                        M_parameters.end(),
                                        LongNamePredicate( long_name ) ),
                        M_parameters.end() );

    std::map< std::string, ParamPtr >::iterator it_long
        = M_long_name_map.find( long_name );
    if ( it_long != M_long_name_map.end() )
    {
        if ( ! it_long->second->shortName().empty() )
        {
            std::map< std::string, ParamPtr >::iterator it_short
                = M_short_name_map.find( it_long->second->shortName() );
            M_short_name_map.erase( it_short );
        }

        M_long_name_map.erase( it_long );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
ParamPtr
ParamMap::findLongName( const std::string & long_name )
{
    std::map< std::string, ParamPtr >::iterator it
        = M_long_name_map.find( long_name );

    if ( it != M_long_name_map.end() )
    {
        return it->second;
    }

    // return NULL
    return ParamPtr( static_cast< ParamEntity * >( 0 ) );
}

/*-------------------------------------------------------------------*/
/*!

*/
ParamPtr
ParamMap::findShortName( const std::string & short_name )
{
    std::map< std::string, ParamPtr >::iterator it
        = M_short_name_map.find( short_name );

    if ( it != M_short_name_map.end() )
    {
        return it->second;
    }

    // return NULL
    return ParamPtr( static_cast< ParamEntity * >( 0 ) );
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
ParamMap::printHelp( std::ostream & os,
                     const bool with_default ) const
{
    const std::string indent_nl = "\n        ";

    if ( ! M_option_name.empty() )
    {
        os << M_option_name << ":\n";
    }

    const std::vector< ParamPtr >::const_iterator end = M_parameters.end();
    for ( std::vector< ParamPtr >::const_iterator it = M_parameters.begin();
          it != end;
          ++it )
    {
        os << "    --" << (*it)->longName();

        if ( ! (*it)->shortName().empty() )
        {
            os << " [ -" << (*it)->shortName() << " ]";
        }

        if ( ! (*it)->isSwitch() )
        {
            os << " <Value>";
        }

        if ( with_default )
        {
            os << " : (DefaultValue=\"";
            (*it)->printValue( os ) << "\")";
        }

        if ( ! (*it)->description().empty() )
        {
            // format description message
            const std::string & desc = (*it)->description();
            std::string::size_type nl_pos = 0;
            for ( std::string::size_type pos = desc.find( ' ' );
                  pos != std::string::npos;
                  pos = desc.find( ' ', pos + 1 ) )
            {
                if ( pos > nl_pos + 72 - 6 )
                {
                    os << indent_nl << desc.substr( nl_pos, pos - nl_pos );
                    nl_pos = pos + 1;
                }
            }
            os << indent_nl << desc.substr( nl_pos );
        }

        os << "\n";
    }
    return os << std::flush;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
ParamMap::printValues( std::ostream & os ) const
{
    const std::vector< ParamPtr >::const_iterator end = M_parameters.end();
    for ( std::vector<  ParamPtr >::const_iterator it = M_parameters.begin();
          it != end;
          ++it )
    {
        os << (*it)->longName() << '\t';
        (*it)->printValue( os );
        os << '\n';
    }
    return os << std::flush;
}

}
