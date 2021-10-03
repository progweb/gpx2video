#ifndef WRITER_H
#define WRITER_H

//==============================================================================
//
//             Writer - the writer class in the gpx library
//
//               Copyright (C) 2016  Dick van Oudheusden
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free
// Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//==============================================================================
//
//  $Date$ $Revision$
//
//==============================================================================

#include <iostream>
#include <string>

#include "GPX.h"

namespace gpx
{
  ///
  /// @class Writer
  ///
  /// @brief The xml writer class.
  ///
  
  class DLL_API Writer
  {
  public:

    ///
    /// Constructor
    ///
    Writer();

    ///
    /// Deconstructor
    ///
    virtual ~Writer();

    // Properties

    // Methods

    ///
    /// Write a node to output stream
    ///
    /// @param  stream        the output stream
    /// @param  node          the node to be written
    /// @param  prettyPrinted should the output be pretty printed ?
    ///
    /// @return success
    ///
    bool write(std::ostream &stream, Node *node, bool prettyPrinted);

  private:

    static
    void indent(std::ostream &stream, int level);

    static
    std::string translate(const std::string &value);

    bool doWrite(std::ostream &stream, Node *node, int level);

    // Disable copy constructors
    Writer(const Writer &);
    Writer& operator=(const Writer &);
  };
}


#endif

