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
//  $Date: 2013-03-10 12:02:27 +0100 (Sun, 10 Mar 2013) $ $Revision: 5 $
//
//==============================================================================

#include "WPT.h"


using namespace std;

namespace gpx
{
  WPT::WPT(Node *parent, const char *name, Node::Type type, bool mandatory) :
    Node(parent, name, type, mandatory),
    _lat(this, "lat", Node::ATTRIBUTE, true),
    _lon(this, "lon", Node::ATTRIBUTE, true),
    _ele(this, "ele", Node::ELEMENT, false),
    _time(this, "time", Node::ELEMENT, false),
    _magvar(this, "magvar", Node::ELEMENT, false),
    _geoidheight(this, "geoidheight", Node::ELEMENT, false),
    _name(this, "name", Node::ELEMENT, false),
    _cmt(this, "cmt", Node::ELEMENT, false),
    _desc(this, "desc", Node::ELEMENT, false),
    _src(this, "src", Node::ELEMENT, false),
    _sym(this, "sym", Node::ELEMENT, false),
    _type(this, "type", Node::ELEMENT, false),
    _fix(this, "fix", Node::ELEMENT, false),
    _sat(this, "sat", Node::ELEMENT, false),
    _hdop(this, "hdop", Node::ELEMENT, false),
    _vdop(this, "vdop", Node::ELEMENT, false),
    _pdop(this, "pdop", Node::ELEMENT, false),
    _ageofdgpsdata(this, "ageofdgpsdata", Node::ELEMENT, false),
    _dgpsid(this, "dgpsid", Node::ELEMENT, false),
    _links(this, "link", Node::ELEMENT, false),
    _extensions(this, "extensions", Node::ELEMENT, false)
  {
    getInterfaces().push_back(&_lat);
    getInterfaces().push_back(&_lon);
    
    getInterfaces().push_back(&_ele);
    getInterfaces().push_back(&_time);
    getInterfaces().push_back(&_magvar);
    getInterfaces().push_back(&_geoidheight);
    getInterfaces().push_back(&_name);
    getInterfaces().push_back(&_cmt);
    getInterfaces().push_back(&_desc);
    getInterfaces().push_back(&_src);
    getInterfaces().push_back(&_sym);
    getInterfaces().push_back(&_type);
    getInterfaces().push_back(&_fix);
    getInterfaces().push_back(&_sat);
    getInterfaces().push_back(&_hdop);
    getInterfaces().push_back(&_vdop);
    getInterfaces().push_back(&_pdop);
    getInterfaces().push_back(&_ageofdgpsdata);
    getInterfaces().push_back(&_dgpsid);
    getInterfaces().push_back(&_links);
    getInterfaces().push_back(&_extensions);
  }
  
  WPT::~WPT()
  {
  }
}

