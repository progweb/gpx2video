//==============================================================================
//
//               Widget - the track class in the LAYOUT library
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

#include "Widget.h"

using namespace std;

namespace layout
{
  Widget::Widget(Node *parent, const char *name, Node::Type type, bool mandatory) :
    Node(parent, name, type, mandatory),
    _name(this, "name", Node::ELEMENT, true),
    _type(this, "type",   Node::ELEMENT, true),
	_display(this, "display", Node::ATTRIBUTE, false),
    _position(this, "position",   Node::ATTRIBUTE, false),
    _align(this, "align",   Node::ATTRIBUTE, false),
    _unit(this, "unit",   Node::ELEMENT, false),
    _zoom(this, "zoom",   Node::ELEMENT, false),
    _format(this, "format",   Node::ELEMENT, false),
    _source(this, "source",   Node::ELEMENT, false),
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
	_font(this, "font", Node::ELEMENT, false),
	_text(this, "text", Node::ELEMENT, false),
	_txtcolor(this, "text-color", Node::ELEMENT, false),
	_txtshadow(this, "text-shadow", Node::ELEMENT, false),
	_border(this, "border", Node::ELEMENT, false),
	_bordercolor(this, "border-color", Node::ELEMENT, false),
	_bgcolor(this, "background-color", Node::ELEMENT, false),
	_nbr_lap(this, "nbr-lap", Node::ELEMENT, false),
	_with_label(this, "with-label", Node::ELEMENT, false),
	_with_value(this, "with-value", Node::ELEMENT, false),
	_with_picto(this, "with-picto", Node::ELEMENT, false),
	_with_unit(this, "with-unit", Node::ELEMENT, false)
  {
    getInterfaces().push_back(&_name);
    getInterfaces().push_back(&_type);
    getInterfaces().push_back(&_display);
    getInterfaces().push_back(&_position);
    getInterfaces().push_back(&_align);
    getInterfaces().push_back(&_unit);
    getInterfaces().push_back(&_zoom);
    getInterfaces().push_back(&_format);
    getInterfaces().push_back(&_source);
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
    getInterfaces().push_back(&_font);
    getInterfaces().push_back(&_text);
    getInterfaces().push_back(&_txtcolor);
    getInterfaces().push_back(&_txtshadow);
    getInterfaces().push_back(&_border);
    getInterfaces().push_back(&_bordercolor);
    getInterfaces().push_back(&_bgcolor);
    getInterfaces().push_back(&_nbr_lap);
    getInterfaces().push_back(&_with_label);
    getInterfaces().push_back(&_with_value);
    getInterfaces().push_back(&_with_picto);
    getInterfaces().push_back(&_with_unit);

	_display.setValue("true");

	_margin_left.setValue("-1");
	_margin_right.setValue("-1");
	_margin_top.setValue("-1");
	_margin_bottom.setValue("-1");

	_padding_left.setValue("-1");
	_padding_right.setValue("-1");
	_padding_top.setValue("-1");
	_padding_bottom.setValue("-1");

	_with_label.setValue("true");
	_with_value.setValue("true");
	_with_picto.setValue("true");
	_with_unit.setValue("true");
  }

  Widget::~Widget()
  {
  }
}

