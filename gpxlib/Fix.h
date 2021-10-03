#ifndef FIX_H
#define FIX_H

//==============================================================================
//
//                 Fix - the fix type  in the GPX library
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


namespace gpx
{
  ///
  /// @class Fix
  ///
  /// @brief The type of gpx fix class.
  ///
  
  class Fix : public Node
  {
    public:

    ///
    /// @enum Fixes
    ///
    /// @brief the values for a gpx fix
    ///
    
    enum Fixes
    {
      NONE, TWO_D, THREE_D, DGPS, PPS
    };
    
    ///
    /// Constructor
    ///
    /// @param  parent     the parent node
    /// @param  name       the name of the attribute or element
    /// @param  type       the node type (ATTRIBUTE or ELEMENT)
    /// @param  mandatory  is the attribute or element mandatory ?
    ///
    Fix(Node *parent, const char *name, Type type, bool mandatory = false);

    ///
    /// Deconstructor
    ///
    virtual ~Fix();
    
    ///
    /// Validate the Fix object
    ///
    /// @param  report  the optional report stream
    ///
    /// @return is validation succesfull
    ///
    
    virtual bool validate(Report *report = nullptr) const;
  
    private:
    
    // Members
    
    // Disable copy constructors
    Fix(const Fix &);
    Fix& operator=(const Fix &);  
  };
}

#endif

