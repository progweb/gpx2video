#ifndef WPT_H
#define WPT_H

//==============================================================================
//
//                WPT - the Waypoint class in the GPX library
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
#include "Unsigned.h"
#include "DateTime.h"
#include "Latitude.h"
#include "Longitude.h"
#include "Degrees.h"
#include "Fix.h"
#include "Link.h"
#include "DGPSStation.h"
#include "Extensions.h"
#include "List.h"


namespace gpx
{
  ///
  /// @class WPT
  ///
  /// @brief The waypoint class.
  ///
  
  class DLL_API WPT : public Node
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
    WPT(Node *parent, const char *name, Node::Type type, bool mandatory = false);

    ///
    /// Deconstructor
    ///
    virtual ~WPT();

    // Properties
    
    ///
    /// Get lat
    ///
    /// @return the lat attribute
    ///
    Latitude  &lat() { return _lat; }

    ///
    /// Get lon
    ///
    /// @return the lon attribute
    ///
    Longitude  &lon() { return _lon; }

    ///
    /// Get ele
    ///
    /// @return the ele element
    ///
    Decimal  &ele() { return _ele; }

    ///
    /// Get time
    ///
    /// @return the time element
    ///
    DateTime  &time() { return _time; }

    ///
    /// Get magvar
    ///
    /// @return the magvar element
    ///
    Degrees  &magvar() { return _magvar; }

    ///
    /// Get geoidheight
    ///
    /// @return the geoidheight element
    ///
    Decimal  &geoidheight() { return _geoidheight; }

    ///
    /// Get name
    ///
    /// @return the name element
    ///
    String  &name() { return _name; }

    ///
    /// Get &cmt
    ///
    /// @return the &cmt element
    ///
    String  &cmt() { return _cmt; }

    ///
    /// Get desc
    ///
    /// @return the desc element
    ///
    String  &desc() { return _desc; }

    ///
    /// Get src
    ///
    /// @return the src element
    ///
    String  &src() { return _src; }

    ///
    /// Get sym
    ///
    /// @return the sym element
    ///
    String  &sym() { return _sym; }

    ///
    /// Get type
    ///
    /// @return the type element
    ///
    String  &type() { return _type; }

    ///
    /// Get fix
    ///
    /// @return the fix element
    ///
    Fix  &fix() { return _fix; }

    ///
    /// Get sat
    ///
    /// @return the sat element
    ///
    Unsigned  &sat() { return _sat; }

    ///
    /// Get hdop
    ///
    /// @return the hdop element
    ///
    Decimal  &hdop() { return _hdop; }

    ///
    /// Get vdop
    ///
    /// @return the vdop element
    ///
    Decimal  &vdop() { return _vdop; }

    ///
    /// Get pdop
    ///
    /// @return the pdop element
    ///
    Decimal  &pdop() { return _pdop; }

    ///
    /// Get ageofdgpsdata
    ///
    /// @return the ageofdgpsdata element
    ///
    Decimal  &ageofdgpsdata() { return _ageofdgpsdata; }

    ///
    /// Get dgpsid
    ///
    /// @return the dgpsid element
    ///
    DGPSStation  &dgpsid() { return _dgpsid; }

    ///
    /// Get link
    ///
    /// @return the link elements
    ///
    List<Link> &links() {return _links;}

    ///
    /// Get extensions
    ///
    /// @return the extensions element
    ///
    Extensions  &extensions() { return _extensions; }

    // Methods
    
    private:
    
    // Members
    Latitude      _lat;
    Longitude     _lon;
    Decimal       _ele;
    DateTime      _time;
    Degrees       _magvar;
    Decimal       _geoidheight;
    String        _name;
    String        _cmt;
    String        _desc;
    String        _src;
    String        _sym;
    String        _type;
    Fix           _fix;
    Unsigned      _sat;
    Decimal       _hdop;
    Decimal       _vdop;
    Decimal       _pdop;
    Decimal       _ageofdgpsdata;
    DGPSStation   _dgpsid;
    List<Link>    _links;
    Extensions    _extensions;
    
    // Disable copy constructors
    WPT(const WPT &);
    WPT& operator=(const WPT &);  
  };
}

#endif

