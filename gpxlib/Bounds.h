#ifndef BOUNDS_H
#define BOUNDS_H

//==============================================================================
//
//              Bounds - the Bounds class in the GPX library
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

namespace gpx
{
  ///
  /// @class Bounds
  ///
  /// @brief The bounds defining the extent of an element class.
  ///
  
  class Bounds : public Node
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
    Bounds(Node *parent, const char *name, Type type, bool mandatory = false);

    ///
    /// Deconstructor
    ///
    virtual ~Bounds();
    
    // Properties
    
    ///
    /// Get minlat
    ///
    /// @return the minlat attribute
    ///
    Latitude  &minlat() { return _minlat; }
    
    ///
    /// Get minlon
    ///
    /// @return  the minlon attribute
    ///
    Longitude &minlon() { return _minlon; }
    
    ///
    /// Get maxlat
    ///
    /// @return  the maxlat attribute
    ///
    Latitude  &maxlat() { return _maxlat; }
    
    ///
    /// Get maxlon
    ///
    /// @return  the maxlon attribute
    ///
    Longitude &maxlon() { return _maxlon; }
    
    private:
    
    // Members
    Latitude  _minlat;
    Longitude _minlon;
    Latitude  _maxlat;
    Longitude _maxlon;

    
    // Disable copy constructors
    Bounds(const Bounds &);
    Bounds& operator=(const Bounds &);  
  };
}

#endif

