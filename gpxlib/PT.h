#ifndef PT_H
#define PT_H

//==============================================================================
//
//                PT - the Point class in the GPX library
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

#include "Latitude.h"
#include "Longitude.h"
#include "Decimal.h"
#include "DateTime.h"


namespace gpx
{
  ///
  /// @class PT
  ///
  /// @brief The geographic point class with optional elevation and time.
  ///
  
  class PT : public Node
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
    PT(Node *parent, const char *name, Type type, bool mandatory = false);

    ///
    /// Deconstructor
    ///
    virtual ~PT();
    
    ///
    /// Get lat
    ///
    /// @return the lat attribute
    ///
    Latitude  &lat() { return _lat; }

    ///
    /// Get lon
    ///
    /// @return the lon attribute
    ///
    Longitude  &lon() { return _lon; }

    ///
    /// Get ele
    ///
    /// @return the ele element
    ///
    Decimal  &ele() { return _ele; }

    ///
    /// Get time
    ///
    /// @return the time element
    ///
    DateTime  &time() { return _time; }

    private:
    
    // Members
    Latitude   _lat;
    Longitude  _lon;
    Decimal    _ele;
    DateTime   _time;
    
    // Disable copy constructors
    PT(const PT &);
    PT& operator=(const PT &);  
  };
}

#endif

