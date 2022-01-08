#ifndef __LAYOUT__UNSIGNED_H__
#define __LAYOUT__UNSIGNED_H__

//==============================================================================
//
//           Unsigned - the unsigned simple type in the LAYOUT library
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


namespace layout
{
  ///
  /// @class Unsigned
  ///
  /// @brief The unsigned class.
  ///
  
  class Unsigned : public Node
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
    Unsigned(Node *parent, const char *name, Type type, bool mandatory = false);

    ///
    /// Deconstructor
    ///
    virtual ~Unsigned();
    
    ///
    /// Validate the Unsigned object
    ///
    /// @param  report  the optional report stream
    ///
    /// @return is validation succesfull
    ///
    
    virtual bool validate(Report *report = nullptr) const;

	operator unsigned int() const { 
		return std::stoi(this->getValue());
	}
    
    private:
    
    // Members
    
    // Disable copy constructors
    Unsigned(const Unsigned &);
    Unsigned& operator=(const Unsigned &);  
  };
}

#endif

