//==============================================================================
//
//    Algorithm - some lat-lon calculations functions in the GPX library
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
//  $Date: 2013-03-10 12:02:27 +0100 (Sun, 10 Mar 2013) $ $Revision: 5 $
//
//==============================================================================
#include <cmath>

#include "Algorithm.h"

namespace gpx
{
  double deg2rad(double deg)
  {
    return (deg * M_PI) / 180.0;
  }

  double rad2deg(double rad)
  {
    return (rad * 180.0) / M_PI;
  }

// http://www.movable-type.co.uk/scripts/latlong.html

// distance in metres
  double distance(double lat1, double lon1, double lat2, double lon2)
  {
    const double R = 6371E3; // m

    double lat1rad = deg2rad(lat1);
    double lon1rad = deg2rad(lon1);
    double lat2rad = deg2rad(lat2);
    double lon2rad = deg2rad(lon2);

    double dlat    = lat2rad - lat1rad;
    double dlon    = lon2rad - lon1rad;

    double a       = sin(dlat / 2.0) * sin(dlat / 2.0) + cos(lat1rad) * cos(lat2rad) * sin(dlon / 2.0) * sin(dlon / 2.0);
    double c       = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));

    return c * R;
  }
// std::cout << "Distance:  " << GpxSim::calcDistance(50.06639, -5.71472, 58.64389, -3.07000) << std::endl; // 968853.52

// bearing in rads
  double bearing(double lat1, double lon1, double lat2, double lon2)
  {
    double lat1rad = deg2rad(lat1);
    double lon1rad = deg2rad(lon1);
    double lat2rad = deg2rad(lat2);
    double lon2rad = deg2rad(lon2);

    double dlon    = lon2rad - lon1rad;

    double y       = sin(dlon) * cos(lat2rad);
    double x       = cos(lat1rad) * sin(lat2rad) - sin(lat1rad) * cos(lat2rad) * cos(dlon);

    return atan2(y, x);
  }
// std::cout << "Bearing:   " << GpxSim::calcBearing(50.06639, -5.71472, 58.64389, -3.07000) << std::endl; // 0.16


// crosstrack distance in metres from point3 to the point1-point2 line
  double crosstrack(double lat1, double lon1, double lat2, double lon2, double lat3, double lon3)
  {
    const double R = 6371E3; // m

    double distance13 = distance(lat1, lon1, lat3, lon3) / R;
    double bearing13  = bearing(lat1, lon1, lat3, lon3);
    double bearing12  = bearing(lat1, lon1, lat2, lon2);

    return asin(sin(distance13) * sin(bearing13 - bearing12)) * R;
  }
// std::cout << "Crosstrack:" << GpxSim::calcCrosstrack(53.3206, -1.7297, 53.1887, 0.1334, 53.2611, -0.7972) << std::endl; // -307.55

}
