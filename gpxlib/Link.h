#ifndef LINK_H
#define LINK_H

//==============================================================================
//
//                Link - the Link class in the GPX library
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
#include "URI.h"

namespace gpx
{
  ///
  /// @class Link
  ///
  /// @brief The link to an external resource class.
  ///
  
  class Link : public Node
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
    Link(Node *parent, const char *name, Type type, bool mandatory = false);

    ///
    /// Deconstructor
    ///
    virtual ~Link();
    
    ///
    /// Validate the Link object
    ///
    /// @param  report  the optional report stream
    ///
    /// @return is validation succesfull
    ///
    
    virtual bool validate(Report *report = nullptr) const;
    
    ///
    /// Get href
    ///
    /// @return the href attribute
    ///
    URI  &href() { return _href; }
  
    ///
    /// Get text
    ///
    /// @return the text element
    ///
    String  &text() { return _text; }
  
    ///
    /// Get type
    ///
    /// @return the type element
    ///
    String  &type() { return _type; }
  
    private:
    
    // Members
    URI    _href;
    String _text;
    String _type;
    
    // Disable copy constructors
    Link(const Link &);
    Link& operator=(const Link &);  
  };
}

#endif

