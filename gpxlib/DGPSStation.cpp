//==============================================================================
//
//           DGPS Station - the DGPS station type in the GPX library
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
//  $Date: 2013-03-10 12:02:27 +0100 (Sun, 10 Mar 2013) $ $Revision: 5 $
//
//==============================================================================

#include <stdlib.h>

#include "DGPSStation.h"

using namespace std;

namespace gpx
{
  
  DGPSStation::DGPSStation(Node *parent, const char *name, Node::Type type, bool mandatory) :
    Node(parent, name, type, mandatory)
  {
  }

  DGPSStation::~DGPSStation()
  {
  }
    
  bool DGPSStation::validate(Report *report) const
  {
    bool ok = Node::validate(report);
    
    if (ok)
    {
      size_t length = getValue().length();
      size_t i      = 0;
      int    value  = 0;

      while ((i < length) && (isspace(getValue().at(i))))
      {
        i++;
      }

      while ((i < length) && (isdigit(getValue().at(i))))
      {
        value = value * 10 + int(getValue().at(i) - '0');

        i++;
      }

      while ((i < length) && (isspace(getValue().at(i))))
      {
        i++;
      }

      if (i != length)
      {
        if (report != nullptr)
        {
          report->report(this, Report::INCORRECT_VALUE, this->getValue());
        }

        ok = false;
      }
      else if (value > 1023) // Negative values are excluded
      {
        if (report != nullptr)
        {
          report->report(this, Report::INCORRECT_VALUE, this->getValue());
        }
        ok = false;
      }
    }
    
    return ok;
  }
    
}


