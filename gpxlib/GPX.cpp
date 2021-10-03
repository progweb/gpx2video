//==============================================================================
//
//                GPX - the root node in the GPX library
//
//          Copyright (C) 2013-2016 Dick van Oudheusden
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

#include "GPX.h"


using namespace std;

namespace gpx
{

  GPX::GPX() :
    Node(nullptr, "gpx", Node::ELEMENT, true),
    _version(this, "version", Node::ATTRIBUTE, true),
    _creator(this, "creator", Node::ATTRIBUTE, true),
    _metadata(this, "metadata", Node::ELEMENT, false),
    _extensions(this, "extensions", Node::ELEMENT, false),
    _wpts(this, "wpt", Node::ELEMENT, false),
    _rtes(this, "rte", Node::ELEMENT, false),
    _trks(this, "trk", Node::ELEMENT, false)

  {
    getInterfaces().push_back(&_version);
    getInterfaces().push_back(&_creator);

    getInterfaces().push_back(&_metadata);
    getInterfaces().push_back(&_extensions);
    getInterfaces().push_back(&_wpts);
    getInterfaces().push_back(&_rtes);
    getInterfaces().push_back(&_trks);
  }

  GPX::~GPX()
  {
  }
}
