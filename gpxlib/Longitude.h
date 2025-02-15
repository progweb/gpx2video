#ifndef LONGITUDE_H
#define LONGITUDE_H

//==============================================================================
//
//             Longitude - the Longitude type in the GPX library
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

#include "Decimal.h"


namespace gpx
{
  ///
  /// @class Longitude
  ///
  /// @brief The longitude of a point class in decimal degrees.
  ///
  
  class Longitude : public Decimal
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
    Longitude(Node *parent, const char *name, Type type, bool mandatory = false);

    ///
    /// Deconstructor
    ///
    virtual ~Longitude();
    
    ///
    /// Validate the Longitude object
    ///
    /// @param  report  the optional report stream
    ///
    /// @return is validation succesfull
    ///
    
    virtual bool validate(Report *report = nullptr) const;
    
	operator double() const { 
      double value = 0.0;

      convert(value);

	  return value;
	}
    
    private:
    
    // Members
    
    // Disable copy constructors
    Longitude(const Longitude &);
    Longitude& operator=(const Longitude &);  
  };
}

#endif

