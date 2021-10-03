#ifndef DATETIME_H
#define DATETIME_H

//==============================================================================
//
//           DateTime - the date time simple type in the GPX library
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
  /// @class DateTime
  ///
  /// @brief The date time class.
  ///
  
  class DateTime : public Node
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
    DateTime(Node *parent, const char *name, Type type, bool mandatory = false);

    ///
    /// Deconstructor
    ///
    virtual ~DateTime();
    
    ///
    /// Validate the DateTime object
    ///
    /// @param  report  the optional report stream
    ///
    /// @return is validation succesfull
    ///
    
    virtual bool validate(Report *report = nullptr) const;
    
    
    private:
    
    // Members
    
    // Disable copy constructors
    DateTime(const DateTime &);
    DateTime& operator=(const DateTime &);  
  };
}

#endif

