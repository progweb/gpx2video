#ifndef __TCX__HEARTRATE_H__
#define __TCX__HEARTRATE_H__

//==============================================================================
//
//                   HeartRate - the HeartRate class
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
#include "Unsigned.h"

namespace tcx
{
  ///
  /// @class HeartRate
  ///
  /// @brief The HeartRate class.
  ///

  class DLL_API HeartRate : public Node
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
    HeartRate(Node *parent, const char *name, Type type, bool mandatory = false);

    ///
    /// Deconstructor
    ///
    virtual ~HeartRate();
    
    // Properties

    ///
    /// Get value
    ///
    /// @return the value elements
    ///
    Unsigned  &value() { return _value; }

  private:

    // Members
    Unsigned      _value;

    // Disable copy constructors
    HeartRate(const HeartRate &);
    HeartRate& operator=(const HeartRate &);  
  };
}

#endif

