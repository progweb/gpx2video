#ifndef EMAIL_H
#define EMAIL_H

//==============================================================================
//
//                EMail - the Email class in the GPX library
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

namespace gpx
{
  ///
  /// @class EMail
  ///
  /// @brief The email address class.
  ///
  
  class EMail : public Node
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
    EMail(Node *parent, const char *name, Type type, bool mandatory = false);

    ///
    /// Deconstructor
    ///
    virtual ~EMail();
    
    ///
    /// Get id
    ///
    /// @return the id attribute
    ///
    String  &id() { return _id; }

    ///
    /// Get domain
    ///
    /// @return the domain attribute
    ///
    String  &domain() { return _domain; }

    private:
    
    // Members
    String   _id;
    String   _domain;
    
    // Disable copy constructors
    EMail(const EMail &);
    EMail& operator=(const EMail &);  
  };
}


#endif

