#ifndef __LAYOUT__REPORT_H__
#define __LAYOUT__REPORT_H__

//==============================================================================
//
//                Report - the report interface
//
//               Copyright (C) 2016 Dick van Oudheusden
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
#include <string>

namespace layout
{
  class Node;

  ///
  /// @interface Report
  ///
  /// @brief The report interface for reporting warnings
  ///
  
  class DLL_API Report
  {
    public:

    enum Warning
    {
      SUCCESS,
      ADD_ALREADY_PRESENT_NODE,
      ADD_UNKNOWN_NODE,
      MISSING_MANDATORY_NODE,
      REMOVE_UNKNOWN_CHILD,
      INCORRECT_VALUE,
      DOUBLE_LAYOUT,
      MISSING_LAYOUT,
      MISFORMED_LAYOUT,
      INSERT_BEFORE_NODE_NOT_FOUND
    };

    ///
    /// Constructor
    ///

    Report();

    ///
    /// Deconstructor
    ///

    virtual ~Report();
    
    ///
    /// Report a warning for a node
    ///
    /// @param  node    the node for with the warning (can be 0)
    /// @param  warning the warning
    /// @param  extra   the extra information
    ///

    virtual void report(const Node *node, Warning warning, const std::string &extra) = 0;

    ///
    /// Convert a warning to a text
    ///
    /// @param  warning the warning
    ///
    /// @return the text
    ///

    static std::string text(Warning warning);
  };
}

#endif

