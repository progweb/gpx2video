#ifndef ALGORITHM_H
#define ALGORITHM_H

//==============================================================================
//
//       Algorithm - some lat-lon calculations functions in the GPX library
//
//               Copyright (C) 2018  Dick van Oudheusden
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

namespace gpx
{
  ///
  /// Convert degrees in radians
  ///
  /// @param  deg        a value in degrees
  ///
  /// @return the value in radians
  ///
  DLL_API double deg2rad(double deg);

  ///
  /// Convert radians in degrees
  ///
  /// @param  rad        a value in radians
  ///
  /// @return the value in degrees
  ///
  DLL_API double rad2deg(double rad);

  ///
  /// Calculate the distance between two lat-lon points
  ///
  /// @param  lat1       the latitude of the start point in degrees
  /// @param  lon1       the longitude of the start point in degrees
  /// @param  lat2       the latitude of the end point in degrees
  /// @param  lon2       the longitude of the end point in degrees
  ///
  /// @return the distance between the two points in metres
  ///
  DLL_API double distance(double lat1, double lon1, double lat2, double lon2);

  ///
  /// Calculate the bearing between two lat-lon points
  ///
  /// @param  lat1       the latitude of the start point in degrees
  /// @param  lon1       the longitude of the start point in degrees
  /// @param  lat2       the latitude of the end point in degrees
  /// @param  lon2       the longitude of the end point in degrees
  ///
  /// @return the distance between the two points in metres
  ///
  DLL_API double bearing(double lat1, double lon1, double lat2, double lon2);

  ///
  /// Calculate the crosstrack distance between the lat-lon1 - lat-lon2 line and point lat-lon3
  ///
  /// @param  lat1       the latitude of the start point in degrees
  /// @param  lon1       the longitude of the start point in degrees
  /// @param  lat2       the latitude of the end point in degrees
  /// @param  lon2       the longitude of the end point in degrees
  /// @param  lat3       the latitude of the point3 in degrees
  /// @param  lon3       the longitude of the point3 in degrees
  ///
  /// @return the crosstrack distance between the line and the point in metres
  ///
  DLL_API double crosstrack(double lat1, double lon1, double lat2, double lon2, double lat3, double lon3);
}

#endif

