//==============================================================================
//
//          Report - the basic report class in the TCX library
//
//               Copyright (C) 2016  Dick van Oudheusden
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
//  $Date:$ $Revision:$
//
//==============================================================================

#include "Report.h"


namespace tcx
{
  Report::Report()
  {
  }

  Report::~Report()
  {
  }

  std::string Report::text(Warning warning)
  {
    switch(warning)
    {
      case Report::SUCCESS                      : return "Ok";
      case Report::ADD_ALREADY_PRESENT_NODE     : return "Trying to add an already present node, ignored";
      case Report::ADD_UNKNOWN_NODE             : return "Unknown child node added";
      case Report::MISSING_MANDATORY_NODE       : return "Mandatory node is missing";
      case Report::REMOVE_UNKNOWN_CHILD         : return "Trying to remove an unknown child node, ignored";
      case Report::INCORRECT_VALUE              : return "Node has an incorrect value";
      case Report::DOUBLE_TCX                   : return "TCX source contains a double tcx element";
      case Report::MISSING_TCX                  : return "TCX source contains no tcx element";
      case Report::MISFORMED_TCX                : return "TCX source has a misformed layout";
      case Report::INSERT_BEFORE_NODE_NOT_FOUND : return "Insert before node not found, added";
      default                                   : return "Unknown warning";
    }
  }
}


