#ifndef __TCX__TRACKPOINT_H__
#define __TCX__TRACKPOINT_H__

//==============================================================================
//
//                   TrackPoint - the TrackPoint class
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
#include "Decimal.h"
#include "DateTime.h"
#include "Position.h"
#include "HeartRate.h"
#include "Extensions.h"

namespace tcx
{
  ///
  /// @class TrackPoint
  ///
  /// @brief The TrackPoint class.
  ///

  class DLL_API TrackPoint : public Node
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
    TrackPoint(Node *parent, const char *name, Type type, bool mandatory = false);

    ///
    /// Deconstructor
    ///
    virtual ~TrackPoint();

    // Properties

    ///
    /// Get time
    ///
    /// @return the time element
    ///
    DateTime  &time() { return _time; }
    
    ///
    /// Get altitude meters
    ///
    /// @return the altitude meters element
    ///
    Decimal  &altitude() { return _altitude; }

    ///
    /// Get position
    ///
    /// @return the position element
    ///
    Position  &position() { return _position; }
    
    ///
    /// Get heartrate
    ///
    /// @return the heartrate element
    ///
    HeartRate  &heartrate() { return _heartrate; }
    
    ///
    /// Get cadence meters
    ///
    /// @return the cadence element
    ///
    Unsigned  &cadence() { return _cadence; }

    ///
    /// Get extensions
    ///
    /// @return the extensions element
    ///
    Extensions  &extensions() { return _extensions; }

  private:

    // Members
    DateTime      _time;
	Decimal       _altitude;
	Position      _position;
	HeartRate     _heartrate;
	Unsigned      _cadence;
    Extensions    _extensions;

    // Disable copy constructors
    TrackPoint(const TrackPoint &);
    TrackPoint& operator=(const TrackPoint &);  
  };
}

#endif

