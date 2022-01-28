#ifndef DECIMAL_H
#define DECIMAL_H

//==============================================================================
//
//           Decimal - the decimal simple type in the GPX library
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
  /// @class Decimal
  ///
  /// @brief The decimal class.
  ///
  
  class Decimal : public Node
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
    Decimal(Node *parent, const char *name, Type type, bool mandatory = false);

    ///
    /// Deconstructor
    ///
    virtual ~Decimal();
    
    ///
    /// Validate the Decimal object
    ///
    /// @param  report  the optional report stream
    ///
    /// @return is validation succesfull
    ///
    
    virtual bool validate(Report *report = nullptr) const;
    
	operator int() const { 
		if (this->getValue().empty())
			return 0;

		return std::stoi(this->getValue());
	}
    
	operator double() const { 
		if (this->getValue().empty())
			return 0.0;

		return std::stod(this->getValue());
	}
    
  protected:

    ///
    /// Convert to a double value of the Decimal object
    ///
    /// @param  value  the double value
    ///
    /// @return is the value correct
    ///

    bool convert(double &value) const;

  private:
    
    // Members
    float          _value;
    
    // Disable copy constructors
    Decimal(const Decimal &);
    Decimal& operator=(const Decimal &);  
  };
}

#endif

