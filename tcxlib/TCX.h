#ifndef __TCX__TCX_H__
#define __TCX__TCX_H__

//==============================================================================
//
//                   TCX - the TCX class
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
#include "Activities.h"

///
/// @mainpage
///
/// This is the documentation of the tcxlib library.
///
/// See Related Pages for the ToDo list.
///
/// See Namespaces for the tcx namespace documentation.
///
/// See Classes for the Class list, Class Index, Class Hierarchy and Class Members documentation.
///
/// Enjoy.
///

///
/// @namespace tcx
///
/// @brief The namespace of the tcx library
///

namespace tcx
{
  ///
  /// @class TCX
  ///
  /// @brief The root node of a TCX document
  ///

  class DLL_API TCX : public Node
  {
  public:

    ///
    /// Constructor
    ///
    TCX();

    ///
    /// Deconstructor
    ///
    virtual ~TCX();
    
    ///
    /// Get Activities
    ///
    /// @return the list of activity elements
    ///
    Activities &activities() { return _activities;}

  private:

    // Members
    Activities   _activities;

    // Disable copy constructors
    TCX(const TCX &);
    TCX& operator=(const TCX &);  
  };
}


#endif

