#ifndef __TCX__ACTIVITIES_H__
#define __TCX__ACTIVITIES_H__

//==============================================================================
//
//                   Activities - the Activities class
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
#include "List.h"
#include "Activity.h"

namespace tcx
{
  ///
  /// @class Activities
  ///
  /// @brief The Activities class.
  ///

  class DLL_API Activities : public Node
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
    Activities(Node *parent, const char *name, Type type, bool mandatory = false);

    ///
    /// Deconstructor
    ///
    virtual ~Activities();
    
    ///
    /// Get activities
    ///
    /// @return the list of lap elements
    ///
    List<Activity> &activities() { return _activities;}

  private:

    // Members
    List<Activity>    _activities;

    // Disable copy constructors
    Activities(const Activities &);
    Activities& operator=(const Activities &);  
  };
}

#endif

