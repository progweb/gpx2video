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
    _orientation(this, "orientation",   Node::ATTRIBUTE, false),
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
    _padding(this, "padding", Node::ELEMENT, false),
    _padding_left(this, "padding-left", Node::ELEMENT, false),
    _padding_right(this, "padding-right", Node::ELEMENT, false),
    _padding_top(this, "padding-top", Node::ELEMENT, false),
    _padding_bottom(this, "padding-bottom", Node::ELEMENT, false),

    _view(this, "view", Node::ELEMENT, false),
    _zoom(this, "zoom", Node::ELEMENT, false),
    _factor(this, "factor", Node::ELEMENT, false),
	_border(this, "border", Node::ELEMENT, false),
	_bordercolor(this, "border-color", Node::ELEMENT, false),
	_bgcolor(this, "background-color", Node::ELEMENT, false),
	_path_smooth(this, "path-smooth", Node::ELEMENT, false),
	_path_thick(this, "path-thick", Node::ELEMENT, false),
	_path_border(this, "path-border", Node::ELEMENT, false),
	_path_border_color(this, "path-border-color", Node::ELEMENT, false),
	_path_primary_color(this, "path-primary-color", Node::ELEMENT, false),
	_path_secondary_color(this, "path-secondary-color", Node::ELEMENT, false),

	_follow_course(this, "follow-course", Node::ELEMENT, false),

    _icon_end_name(this, "icon-end-name",   Node::ELEMENT, false),
    _icon_start_name(this, "icon-start-name",   Node::ELEMENT, false),
    _icon_position_name(this, "icon-position-name",   Node::ELEMENT, false),
	_icon_end_color(this, "icon-end-color", Node::ELEMENT, false),
	_icon_start_color(this, "icon-start-color", Node::ELEMENT, false),
	_icon_position_color(this, "icon-position-color", Node::ELEMENT, false),
    _icon_end_size(this, "icon-end-size", Node::ELEMENT, false),
    _icon_start_size(this, "icon-start-size", Node::ELEMENT, false),
    _icon_position_size(this, "icon-position-size", Node::ELEMENT, false),
	
	_with_icon_end(this, "with-maker-end", Node::ELEMENT, false),
	_with_icon_start(this, "with-maker-start", Node::ELEMENT, false),
	_with_icon_position(this, "with-maker-position", Node::ELEMENT, false)
  {
    getInterfaces().push_back(&_source);
	getInterfaces().push_back(&_display);
    getInterfaces().push_back(&_position);
    getInterfaces().push_back(&_orientation);
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
    getInterfaces().push_back(&_padding);
    getInterfaces().push_back(&_padding_left);
    getInterfaces().push_back(&_padding_right);
    getInterfaces().push_back(&_padding_top);
    getInterfaces().push_back(&_padding_bottom);

    getInterfaces().push_back(&_view);
    getInterfaces().push_back(&_zoom);
    getInterfaces().push_back(&_factor);
    getInterfaces().push_back(&_border);
    getInterfaces().push_back(&_bordercolor);
    getInterfaces().push_back(&_bgcolor);
    getInterfaces().push_back(&_path_smooth);
    getInterfaces().push_back(&_path_thick);
    getInterfaces().push_back(&_path_border);
    getInterfaces().push_back(&_path_border_color);
    getInterfaces().push_back(&_path_primary_color);
    getInterfaces().push_back(&_path_secondary_color);

	getInterfaces().push_back(&_follow_course);

    getInterfaces().push_back(&_icon_end_name);
    getInterfaces().push_back(&_icon_start_name);
    getInterfaces().push_back(&_icon_position_name);
    getInterfaces().push_back(&_icon_end_color);
    getInterfaces().push_back(&_icon_start_color);
    getInterfaces().push_back(&_icon_position_color);
    getInterfaces().push_back(&_icon_end_size);
    getInterfaces().push_back(&_icon_start_size);
    getInterfaces().push_back(&_icon_position_size);

    getInterfaces().push_back(&_with_icon_end);
    getInterfaces().push_back(&_with_icon_start);
    getInterfaces().push_back(&_with_icon_position);

	_display.setValue("true");

	_margin_left.setValue("-1");
	_margin_right.setValue("-1");
	_margin_top.setValue("-1");
	_margin_bottom.setValue("-1");

	_padding_left.setValue("-1");
	_padding_right.setValue("-1");
	_padding_top.setValue("-1");
	_padding_bottom.setValue("-1");

	_zoom.setValue("12");
	_factor.setValue("1.0");

	_path_smooth.setValue("1");
	_path_thick.setValue("3.0");
	_path_border.setValue("1.4");
	_path_border_color.setValue("#000000ff");
	_path_primary_color.setValue("#669df6ff");
	_path_secondary_color.setValue("#ffffffff");

	_follow_course.setValue("false");

	_icon_end_color.setValue("#00000000");
	_icon_start_color.setValue("#00000000");
	_icon_position_color.setValue("#00000000");

	_icon_end_size.setValue("1.0");
	_icon_start_size.setValue("1.0");
	_icon_position_size.setValue("1.0");

	_with_icon_end.setValue("true");
	_with_icon_start.setValue("true");
	_with_icon_position.setValue("true");
  }

  Map::~Map()
  {
  }
}

