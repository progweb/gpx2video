//==============================================================================
//
//               Map - the track class in the LAYOUT library
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

#include "Map.h"

using namespace std;

namespace layout
{
  Map::Map(Node *parent, const char *name, Node::Type type, bool mandatory) :
    Node(parent, name, type, mandatory),
    _source(this, "source",   Node::ELEMENT, true),
	_display(this, "display", Node::ATTRIBUTE, false),
    _position(this, "position",   Node::ATTRIBUTE, false),
    _align(this, "align",   Node::ATTRIBUTE, false),
	_at(this, "at", Node::ATTRIBUTE, false),
	_duration(this, "duration", Node::ATTRIBUTE, false),
    _x(this, "x", Node::ATTRIBUTE, false),
    _y(this, "y", Node::ATTRIBUTE, false),
    _width(this, "width", Node::ATTRIBUTE, false),
    _height(this, "height", Node::ATTRIBUTE, false),
    _margin(this, "margin", Node::ELEMENT, false),
    _margin_left(this, "margin-left", Node::ELEMENT, false),
    _margin_right(this, "margin-right", Node::ELEMENT, false),
    _margin_top(this, "margin-top", Node::ELEMENT, false),
    _margin_bottom(this, "margin-bottom", Node::ELEMENT, false),
    _zoom(this, "zoom", Node::ELEMENT, false),
    _factor(this, "factor", Node::ELEMENT, false),
    _marker(this, "marker", Node::ELEMENT, false),
	_border(this, "border", Node::ELEMENT, false),
	_bordercolor(this, "border-color", Node::ELEMENT, false)
  {
    getInterfaces().push_back(&_source);
	getInterfaces().push_back(&_display);
    getInterfaces().push_back(&_position);
    getInterfaces().push_back(&_align);
    getInterfaces().push_back(&_at);
    getInterfaces().push_back(&_duration);
    getInterfaces().push_back(&_x);
    getInterfaces().push_back(&_y);
    getInterfaces().push_back(&_width);
    getInterfaces().push_back(&_height);
    getInterfaces().push_back(&_margin);
    getInterfaces().push_back(&_margin_left);
    getInterfaces().push_back(&_margin_right);
    getInterfaces().push_back(&_margin_top);
    getInterfaces().push_back(&_margin_bottom);
    getInterfaces().push_back(&_zoom);
    getInterfaces().push_back(&_factor);
    getInterfaces().push_back(&_marker);
    getInterfaces().push_back(&_border);
    getInterfaces().push_back(&_bordercolor);

	_display.setValue("true");

	_margin_left.setValue("-1");
	_margin_right.setValue("-1");
	_margin_top.setValue("-1");
	_margin_bottom.setValue("-1");

	_zoom.setValue("12");
	_factor.setValue("1.0");
  }

  Map::~Map()
  {
  }
}

