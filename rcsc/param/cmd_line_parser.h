// -*-c++-*-

/*!
  \file cmd_line_parser.h
  \brief command line argument parser Header File
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

#ifndef RCSC_PARAM_CMD_LINE_PARSER_H
#define RCSC_PARAM_CMD_LINE_PARSER_H

#include <rcsc/param/param_parser.h>

#include <list>
#include <vector>
#include <string>
#include <ostream>

namespace rcsc {

/*!
  \class CmdLineParser
  \brief command line parser
 */
class CmdLineParser
    : public ParamParser {
private:

    //! copy of command line arguments without program name
    std::list< std::string > M_args;

    //! namespace string
    const std::string M_namespace;

    //! container of the parsed positional options
    std::vector< std::string > M_positional_options;

    //! not used
    CmdLineParser();
public:

    /*!
      \brief construct with original command line arguments
      \param argc number of argument
      \param argv const double array of char
      \param name_space namespace string
     */
    CmdLineParser( const int argc,
                   const char * const * argv,
                   const std::string & name_space = "" );

    /*!
      \brief construct with original command line arguments
      \param args argument string container
      \param name_space namespace string
     */
    CmdLineParser( const std::list< std::string > & args,
                   const std::string & name_space = "" );

    /*!
      \brief analyze arguments and results are stored to parameter map
      \param param_map reference to the parameter container
      \return true if successfully parserd
     */
    bool parse( ParamMap & param_map );

private:

    /*!
      \brief analyze positional options and store them to M_positional_args
     */
    void parsePositional();

public:

    /*!
      \brief get the stored arguments
      \return const reference to the container
     */
    const
    std::list< std::string > & args() const
      {
          return M_args;
      }

    /*!
      \brief check if all arguments are successfully parsed.
      \return status of parsing result.
     */
    bool failed() const
      {
          return M_args.size() != M_positional_options.size();
      }

    /*!
      \brief get the positional arguments
      \return const reference to the container of positional argument string
     */
    const
    std::vector< std::string > & positionalOptions() const
      {
          return M_positional_options;
      }

    /*!
      \brief put the stored arguments to the output stream
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & print( std::ostream & os ) const;

};

}

#endif
