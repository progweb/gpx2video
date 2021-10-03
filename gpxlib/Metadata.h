#ifndef META_DATA_H
#define META_DATA_H

//==============================================================================
//
//             Metadata - the Metadata class in the gpx library
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

#include "export.h"
#include "Node.h"

#include "String_.h"
#include "Person.h"
#include "Copyright.h"
#include "Link.h"
#include "DateTime.h"
#include "Bounds.h"
#include "Extensions.h"
#include "List.h"

namespace gpx
{
  ///
  /// @class Metadata
  ///
  /// @brief The metadata from another schema's class.
  ///
  
  class DLL_API Metadata : public Node
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
    Metadata(Node *parent, const char *name, Type type, bool mandatory = false);

    ///
    /// Deconstructor
    ///
    virtual ~Metadata();
    
    ///
    /// Get name
    ///
    /// @return the name element
    ///
    String  &name() { return _name; }
    
    ///
    /// Get desc
    ///
    /// @return the desc element
    ///
    String  &desc() { return _desc; }
    
    ///
    /// Get author
    ///
    /// @return the author element
    ///
    Person  &author() { return _author; }
    
    ///
    /// Get copyright
    ///
    /// @return the copyright element
    ///
    Copyright  &copyright() { return _copyright; }
    
    ///
    /// Get link
    ///
    /// @return the link element
    ///
    List<Link> &links() {return _links;}
    
    ///
    /// Get time
    ///
    /// @return the time element
    ///
    DateTime  &time() { return _time; }
    
    ///
    /// Get keywords
    ///
    /// @return the keywords element
    ///
    String  &keywords() { return _keywords; }
    
    ///
    /// Get bounds
    ///
    /// @return the bounds element
    ///
    Bounds  &bounds() { return _bounds; }
    
    ///
    /// Get extensions
    ///
    /// @return the extensions element
    ///
    Extensions  &extensions() { return _extensions; }
    
    private:
    
    // Members
    String     _name;
    String     _desc;
    Person     _author;
    Copyright  _copyright;
    DateTime   _time;
    String     _keywords;
    Bounds     _bounds;
    Extensions _extensions;
    List<Link> _links;
    
    // Disable copy constructors
    Metadata(const Metadata &);
    Metadata& operator=(const Metadata &);  
  };
}

#endif
