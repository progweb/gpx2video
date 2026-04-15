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
    _shape(this, "shape",   Node::ELEMENT, true),
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

	_border(this, "border", Node::ELEMENT, false),
	_bordercolor(this, "border-color", Node::ELEMENT, false),
	_bgcolor(this, "background-color", Node::ELEMENT, false),

	_font(this, "font", Node::ELEMENT, false),
	_text(this, "text", Node::ELEMENT, false),

	_label_fontfamily(this, "label-font-family", Node::ELEMENT, false),
	_label_fontsize(this, "label-font-size", Node::ELEMENT, false),
	_label_fontstyle(this, "label-font-style", Node::ELEMENT, false),
	_label_fontweight(this, "label-font-weight", Node::ELEMENT, false),
	_label_align(this, "label-align", Node::ELEMENT, false),
	_label_color(this, "label-color", Node::ELEMENT, false),
	_label_shadowopacity(this, "label-shadow-opactiy", Node::ELEMENT, false),
	_label_shadowdistance(this, "label-shadow-distance", Node::ELEMENT, false),
	_label_borderwidth(this, "label-border-width", Node::ELEMENT, false),
	_label_bordercolor(this, "label-border-color", Node::ELEMENT, false),

	_value_fontfamily(this, "value-font-family", Node::ELEMENT, false),
	_value_fontsize(this, "value-font-size", Node::ELEMENT, false),
	_value_fontstyle(this, "value-font-style", Node::ELEMENT, false),
	_value_fontweight(this, "value-font-weight", Node::ELEMENT, false),
	_value_align(this, "value-align", Node::ELEMENT, false),
	_value_color(this, "value-color", Node::ELEMENT, false),
	_value_shadowopacity(this, "value-shadow-opactiy", Node::ELEMENT, false),
	_value_shadowdistance(this, "value-shadow-distance", Node::ELEMENT, false),
	_value_borderwidth(this, "value-border-width", Node::ELEMENT, false),
	_value_bordercolor(this, "value-border-color", Node::ELEMENT, false),

	_needle_type(this, "needle-type", Node::ELEMENT, false),
	_needle_primary_color(this, "needle-primary-color", Node::ELEMENT, false),
	_needle_secondary_color(this, "needle-secondary-color", Node::ELEMENT, false),

    _unit(this, "unit",   Node::ELEMENT, false),
    _zoom(this, "zoom",   Node::ELEMENT, false),
    _format(this, "format",   Node::ELEMENT, false),
    _source(this, "source",   Node::ELEMENT, false),
	_nbr_lap(this, "nbr-lap", Node::ELEMENT, false),

	_with_label(this, "with-label", Node::ELEMENT, false),
	_with_value(this, "with-value", Node::ELEMENT, false),
	_with_icon(this, "with-icon", Node::ELEMENT, false),
	_with_unit(this, "with-unit", Node::ELEMENT, false),
	_with_tick(this, "with-tick", Node::ELEMENT, false),
	_with_tick_label(this, "with-tick-label", Node::ELEMENT, false),
	_with_needle(this, "with-needle", Node::ELEMENT, false)
  {
    getInterfaces().push_back(&_name);
    getInterfaces().push_back(&_type);
    getInterfaces().push_back(&_shape);
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

    getInterfaces().push_back(&_border);
    getInterfaces().push_back(&_bordercolor);
    getInterfaces().push_back(&_bgcolor);

    getInterfaces().push_back(&_font);
    getInterfaces().push_back(&_text);

    getInterfaces().push_back(&_label_fontfamily);
    getInterfaces().push_back(&_label_fontsize);
    getInterfaces().push_back(&_label_fontstyle);
    getInterfaces().push_back(&_label_fontweight);
    getInterfaces().push_back(&_label_align);
    getInterfaces().push_back(&_label_color);
    getInterfaces().push_back(&_label_shadowopacity);
    getInterfaces().push_back(&_label_shadowdistance);
    getInterfaces().push_back(&_label_borderwidth);
    getInterfaces().push_back(&_label_bordercolor);

    getInterfaces().push_back(&_value_fontfamily);
    getInterfaces().push_back(&_value_fontsize);
    getInterfaces().push_back(&_value_fontstyle);
    getInterfaces().push_back(&_value_fontweight);
    getInterfaces().push_back(&_value_align);
    getInterfaces().push_back(&_value_color);
    getInterfaces().push_back(&_value_shadowopacity);
    getInterfaces().push_back(&_value_shadowdistance);
    getInterfaces().push_back(&_value_borderwidth);
    getInterfaces().push_back(&_value_bordercolor);

    getInterfaces().push_back(&_needle_type);
    getInterfaces().push_back(&_needle_primary_color);
    getInterfaces().push_back(&_needle_secondary_color);

    getInterfaces().push_back(&_unit);
    getInterfaces().push_back(&_zoom);
    getInterfaces().push_back(&_format);
    getInterfaces().push_back(&_source);
    getInterfaces().push_back(&_nbr_lap);

    getInterfaces().push_back(&_with_label);
    getInterfaces().push_back(&_with_value);
    getInterfaces().push_back(&_with_icon);
    getInterfaces().push_back(&_with_unit);
    getInterfaces().push_back(&_with_tick);
    getInterfaces().push_back(&_with_tick_label);
    getInterfaces().push_back(&_with_needle);

	_shape.setValue("text");

	_display.setValue("true");

	_margin_left.setValue("-1");
	_margin_right.setValue("-1");
	_margin_top.setValue("-1");
	_margin_bottom.setValue("-1");

	_padding_left.setValue("-1");
	_padding_right.setValue("-1");
	_padding_top.setValue("-1");
	_padding_bottom.setValue("-1");

	_label_fontsize.setValue("-1");
	_label_shadowopacity.setValue("-1");
	_label_shadowdistance.setValue("-1");
	_label_borderwidth.setValue("-1");

	_value_fontsize.setValue("-1");
	_value_shadowopacity.setValue("-1");
	_value_shadowdistance.setValue("-1");
	_value_borderwidth.setValue("-1");

	_with_label.setValue("true");
	_with_value.setValue("true");
	_with_icon.setValue("true");
	_with_unit.setValue("true");
	_with_tick.setValue("true");
	_with_tick_label.setValue("true");
	_with_needle.setValue("true");
  }

  Widget::~Widget()
  {
  }
}

