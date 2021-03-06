//==============================================================================
//
//                LAYOUT - the root node in the LAYOUT library
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

#include "Layout.h"


using namespace std;

namespace layout
{

  Layout::Layout() :
    Node(nullptr, "layout", Node::ELEMENT, true),
    _version(this, "version", Node::ATTRIBUTE, true),
    _creator(this, "creator", Node::ATTRIBUTE, true),
    _widgets(this, "widget", Node::ELEMENT, false),
    _tracks(this, "track", Node::ELEMENT, false),
    _maps(this, "map", Node::ELEMENT, false)
  {
    getInterfaces().push_back(&_version);
    getInterfaces().push_back(&_creator);

    getInterfaces().push_back(&_widgets);
    getInterfaces().push_back(&_tracks);
    getInterfaces().push_back(&_maps);
  }

  Layout::~Layout()
  {
  }
}
