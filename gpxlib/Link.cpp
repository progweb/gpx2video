//==============================================================================
//
//                Link - the Link class in the GPX library
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

#include "Link.h"



using namespace std;

namespace gpx
{
  Link::Link(Node *parent, const char *name, Node::Type type, bool mandatory) :
    Node(parent, name, type, mandatory),
    _href(this, "href", Node::ATTRIBUTE, true),
    _text(this, "text", Node::ELEMENT, false),
    _type(this, "type", Node::ELEMENT, false)
  {
    getInterfaces().push_back(&_href);
    getInterfaces().push_back(&_text);
    getInterfaces().push_back(&_type);
  }
  
  Link::~Link()
  {
  }

  bool Link::validate(Report *report) const
  {
    bool ok = Node::validate(report);

    /// @todo: Link::validate

    return ok;
  }
}


