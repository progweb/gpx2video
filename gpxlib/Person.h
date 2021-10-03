#ifndef PERSON_H
#define PERSON_H

//==============================================================================
//
//             Person - the Person class in the GPX library
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
#include "EMail.h"
#include "Link.h"


namespace gpx
{
  ///
  /// @class Person
  ///
  /// @brief The person or organisation class.
  ///
  
  class Person : public Node
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
    Person(Node *parent, const char *name, Type type, bool mandatory = false);

    ///
    /// Deconstructor
    ///
    virtual ~Person();

    ///
    /// Get name
    ///
    /// @return the name element
    ///
    String  &name() { return _name; }
    
    ///
    /// Get email
    ///
    /// @return the email element
    ///
    EMail  &email() { return _email; }
    
    ///
    /// Get link
    ///
    /// @return the link element
    ///
    Link  &link() { return _link; }
    
    private:
    
    // Members
    String    _name;
    EMail     _email;
    Link      _link;
    
    // Disable copy constructors
    Person(const Person &);
    Person& operator=(const Person &);  
  };
}

#endif

