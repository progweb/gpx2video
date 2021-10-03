#ifndef COPYRIGHT_H
#define COPYRIGHT_H

//==============================================================================
//
//             Copyright - the Copyright class in the GPX library
//
//               Copyright (C) 2013  Dick van Oudheusden
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

#include "Node.h"

#include "String_.h"
#include "Unsigned.h"
#include "URI.h"


namespace gpx
{
  ///
  /// @class Copyright
  ///
  /// @brief The copyright holder and license class.
  ///
  
  class Copyright : public Node
  {
    public:

    ///
    /// Constructor
    ///
    /// @param  parent     the parent node
    /// @param  name       the name of the attribute or element
    /// @param  type       the node type (ATTRIBUTE or ELEMENT)
    /// @param  mandatory  is the attribute or element mandatory ?
    ///
    Copyright(Node *parent, const char *name, Type type, bool mandatory = false);

    ///
    /// Deconstructor
    ///
    virtual ~Copyright();
    
    ///
    /// Get author
    ///
    /// @return the author attribute
    ///
    String  &author() { return _author; }

    ///
    /// Get year
    ///
    /// @return the year element
    ///
    Unsigned  &year() { return _year; }

    ///
    /// Get license
    ///
    /// @return the license element
    ///
    URI  &license() { return _license; }

    private:
    
    // Members
    String   _author;
    Unsigned _year;
    URI      _license;

    // Disable copy constructors
    Copyright(const Copyright &);
    Copyright& operator=(const Copyright &);  
  };
}

#endif

