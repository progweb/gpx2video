#ifndef REPORT_CERR_H
#define REPORT_CERR_H

//==============================================================================
//
//              ReportCerr - the report on cerr class
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
#include "Report.h"

namespace layout
{
  ///
  /// @class ReportCerr
  ///
  /// @brief The report on cerr class for reporting warnings
  ///
  
  class DLL_API ReportCerr : public Report
  {
    public:

    ///
    /// Constructor
    ///

    ReportCerr();

    ///
    /// Deconstructor
    ///

    virtual ~ReportCerr();
    
    ///
    /// Report a warning for a node
    ///
    /// @param  node    the node for with the warning (can be 0)
    /// @param  warning the warning
    /// @param  extra   the extra information
    ///

    virtual void report(const Node *node, Report::Warning warning, const std::string &extra);
  };
}

#endif

