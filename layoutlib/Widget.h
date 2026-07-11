#ifndef __LAYOUT__WIDGET_H__
#define __LAYOUT__WIDGET_H__

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
//  $Date$ $Revision$
//
//==============================================================================

#include "export.h"
#include "Node.h"

#include "Boolean.h"
#include "Decimal.h"
#include "String_.h"
#include "Unsigned.h"
#include "List.h"


namespace layout
{
  ///
  /// @class Widget
  ///
  /// @brief The track class.
  ///
  
  class DLL_API Widget : public Node
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
    Widget(Node *parent, const char *name, Type type, bool mandatory = false);

    ///
    /// Deconstructor
    ///
    virtual ~Widget();
    
    ///
    /// Get name
    ///
    /// @return the name element
    ///
    String &name() { return _name; }
    
    ///
    /// Get type
    ///
    /// @return the type element
    ///
    String &type() { return _type; }

    ///
    /// Get shape
    ///
    /// @return the shape element
    ///
    String &shape() { return _shape; }

	/// 
	/// Get display
	///
	/// @return the display attribute
	///
	Boolean  &display() { return _display; }

    ///
    /// Get position
    ///
    /// @return the position attribute
    ///
    String &position() { return _position; }

    ///
    /// Get orientation
    ///
    /// @return the orientation attribute
    ///
    String &orientation() { return _orientation; }

    ///
    /// Get zoom
    ///
    /// @return the zoom element
    ///
    String &zoom() { return _zoom; }

    ///
    /// Get source
    ///
    /// @return the source element
    ///
    String &source() { return _source; }

    ///
    /// Get at
    ///
    /// @return the at attribute
    ///
    Unsigned  &at() { return _at; }

    ///
    /// Get duration
    ///
    /// @return the duration attribute
    ///
    Unsigned  &duration() { return _duration; }

    ///
    /// Get x
    ///
    /// @return the x attribute
    ///
    Unsigned  &x() { return _x; }

    ///
    /// Get y
    ///
    /// @return the y attribute
    ///
    Unsigned  &y() { return _y; }

    ///
    /// Get width
    ///
    /// @return the width attribute
    ///
    Unsigned  &width() { return _width; }

    ///
    /// Get height
    ///
    /// @return the height attribute
    ///
    Unsigned  &height() { return _height; }

    ///
    /// Get margin
    ///
    /// @return the margin attribute
    ///
    Unsigned  &margin() { return _margin; }

    ///
    /// Get marginLeft
    ///
    /// @return the margin left attribute
    ///
    Decimal  &marginLeft() { return _margin_left; }

    ///
    /// Get marginRight
    ///
    /// @return the margin right attribute
    ///
    Decimal  &marginRight() { return _margin_right; }

    ///
    /// Get marginTop
    ///
    /// @return the margin top attribute
    ///
    Decimal  &marginTop() { return _margin_top; }

    ///
    /// Get marginBottom
    ///
    /// @return the margin bottom attribute
    ///
    Decimal  &marginBottom() { return _margin_bottom; }

    ///
    /// Get padding
    ///
    /// @return the padding attribute
    ///
    Unsigned  &padding() { return _padding; }

    ///
    /// Get paddingLeft
    ///
    /// @return the padding left attribute
    ///
    Decimal  &paddingLeft() { return _padding_left; }

    ///
    /// Get paddingRight
    ///
    /// @return the padding right attribute
    ///
    Decimal  &paddingRight() { return _padding_right; }

    ///
    /// Get paddingTop
    ///
    /// @return the padding top attribute
    ///
    Decimal  &paddingTop() { return _padding_top; }

    ///
    /// Get paddingBottom
    ///
    /// @return the padding bottom attribute
    ///
    Decimal  &paddingBottom() { return _padding_bottom; }

    ///
    /// Get border
    ///
    /// @return the border attribute
    ///
    Decimal  &border() { return _border; }

    ///
    /// Get border color
    ///
    /// @return the border color element
    ///
    String &borderColor() { return _bordercolor; }

	///
	/// Get round corner 
	/// 
	/// @return the round corner attribute
	///
	Decimal  &roundCorner() { return _round_corner; }

    ///
    /// Get background color
    ///
    /// @return the background color element
    ///
    String &backgroundColor() { return _bgcolor; }

    ///
    /// Get text 
    ///
    /// @return the text element
    ///
    String &text() { return _text; }

    ///
    /// Get line space
    ///
    /// @return the line space element
    ///
    Decimal &lineSpace() { return _line_space; }

	///
	/// Get text orientation
	/// 
	/// @return the text orientation element
	///
	String &textOrientation() { return _text_orientation; }

    ///
    /// Get icon name
    ///
    /// @return the icon name element
    ///
    String &icon() { return _icon_name; }

	///
	/// Get icon size
	/// 
	/// @return the icon size elememnt
	///
	Decimal &iconSize() { return _icon_size; }

    ///
    /// Get icon color
    ///
    /// @return the icon color element
    ///
    String &iconColor() { return _icon_color; }

    ///
    /// Get label font family
    ///
    /// @return the label font family element
    ///
    String &labelFontFamily() { return _label_fontfamily; }

    ///
    /// Get label font size
    ///
    /// @return the label font size element
    ///
    Decimal  &labelFontSize() { return _label_fontsize; }

    ///
    /// Get label font style
    ///
    /// @return the label font style element
    ///
    String &labelFontStyle() { return _label_fontstyle; }

    ///
    /// Get label font weight
    ///
    /// @return the label font weight element
    ///
    String &labelFontWeight() { return _label_fontweight; }

    ///
    /// Get label horizontal align
    ///
    /// @return the label horizontal align element
    ///
    String &labelHorizontalAlign() { return _label_horizontal_align; }

    ///
    /// Get label vertical align
    ///
    /// @return the label vertical align element
    ///
    String &labelVerticalAlign() { return _label_vertical_align; }

    ///
    /// Get label color
    ///
    /// @return the label color element
    ///
    String &labelColor() { return _label_color; }

    ///
    /// Get label shadow opacity
    ///
    /// @return the label shadow opacity element
    ///
    Decimal  &labelShadowOpacity() { return _label_shadowopacity; }

    ///
    /// Get label shadow distance
    ///
    /// @return the label shadow distance element
    ///
    Decimal  &labelShadowDistance() { return _label_shadowdistance; }

    ///
    /// Get label border width
    ///
    /// @return the label border width element
    ///
    Decimal  &labelBorderWidth() { return _label_borderwidth; }

    ///
    /// Get label border color
    ///
    /// @return the label border color element
    ///
    String &labelBorderColor() { return _label_bordercolor; }

    ///
    /// Get value font family
    ///
    /// @return the value font family element
    ///
    String &valueFontFamily() { return _value_fontfamily; }

    ///
    /// Get value font size
    ///
    /// @return the value font size element
    ///
    Decimal  &valueFontSize() { return _value_fontsize; }

    ///
    /// Get value font style
    ///
    /// @return the value font style element
    ///
    String &valueFontStyle() { return _value_fontstyle; }

    ///
    /// Get value font weight
    ///
    /// @return the value font weight element
    ///
    String &valueFontWeight() { return _value_fontweight; }

    ///
    /// Get value horizontal align
    ///
    /// @return the value horizontal align element
    ///
    String &valueHorizontalAlign() { return _value_horizontal_align; }

    ///
    /// Get value vertical align
    ///
    /// @return the value vertical align element
    ///
    String &valueVerticalAlign() { return _value_vertical_align; }

    ///
    /// Get value color
    ///
    /// @return the value color element
    ///
    String &valueColor() { return _value_color; }

    ///
    /// Get value shadow opacity
    ///
    /// @return the value shadow opacity element
    ///
    Decimal  &valueShadowOpacity() { return _value_shadowopacity; }

    ///
    /// Get value shadow distance
    ///
    /// @return the value shadow distance element
    ///
    Decimal  &valueShadowDistance() { return _value_shadowdistance; }

    ///
    /// Get value border width
    ///
    /// @return the value border width element
    ///
    Decimal  &valueBorderWidth() { return _value_borderwidth; }

    ///
    /// Get value border color
    ///
    /// @return the value border color element
    ///
    String &valueBorderColor() { return _value_bordercolor; }

    ///
    /// Get min
    ///
    /// @return the min element
    ///
    Decimal &valueMin() { return _value_min; }

    ///
    /// Get max
    ///
    /// @return the max element
    ///
    Decimal &valueMax() { return _value_max; }

    ///
    /// Get unit
    ///
    /// @return the unit element
    ///
    String &valueUnit() { return _value_unit; }

    ///
    /// Get format
    ///
    /// @return the format element
    ///
    String &valueFormat() { return _value_format; }

    ///
    /// Get unit font family
    ///
    /// @return the unit font family element
    ///
    String &unitFontFamily() { return _unit_fontfamily; }

    ///
    /// Get unit font size
    ///
    /// @return the unit font size element
    ///
    Decimal  &unitFontSize() { return _unit_fontsize; }

    ///
    /// Get unit font style
    ///
    /// @return the unit font style element
    ///
    String &unitFontStyle() { return _unit_fontstyle; }

    ///
    /// Get unit font weight
    ///
    /// @return the unit font weight element
    ///
    String &unitFontWeight() { return _unit_fontweight; }

    ///
    /// Get unit horizontal align
    ///
    /// @return the unit horizontal align element
    ///
    String &unitHorizontalAlign() { return _unit_horizontal_align; }

    ///
    /// Get unit vertical align
    ///
    /// @return the unit vertical align element
    ///
    String &unitVerticalAlign() { return _unit_vertical_align; }

    ///
    /// Get unit color
    ///
    /// @return the unit color element
    ///
    String &unitColor() { return _unit_color; }

    ///
    /// Get unit shadow opacity
    ///
    /// @return the unit shadow opacity element
    ///
    Decimal  &unitShadowOpacity() { return _unit_shadowopacity; }

    ///
    /// Get unit shadow distance
    ///
    /// @return the unit shadow distance element
    ///
    Decimal  &unitShadowDistance() { return _unit_shadowdistance; }

    ///
    /// Get unit border width
    ///
    /// @return the unit border width element
    ///
    Decimal  &unitBorderWidth() { return _unit_borderwidth; }

    ///
    /// Get unit border color
    ///
    /// @return the unit border color element
    ///
    String &unitBorderColor() { return _unit_bordercolor; }

    ///
    /// Get unit distance
    ///
    /// @return the unit distance element
    ///
    Decimal  &unitDistance() { return _unit_distance; }

    ///
    /// Get gauge angle
    ///
    /// @return the gauge angle attribute
    ///
    Decimal  &gaugeAngle() { return _gauge_angle; }

    ///
    /// Get gauge rotation
    ///
    /// @return the gauge rotation attribute
    ///
    Decimal  &gaugeRotation() { return _gauge_rotation; }

    ///
    /// Get gauge orientation
    ///
    /// @return the gauge orientation attribute
    ///
    String  &gaugeOrientation() { return _gauge_orientation; }

    ///
    /// Get gauge flip
    ///
    /// @return the gauge flip attribute
    ///
    Boolean  &gaugeFlip() { return _gauge_flip; }

    ///
    /// Get gauge width
    ///
    /// @return the gauge width attribute
    ///
    Decimal  &gaugeWidth() { return _gauge_width; }

    ///
    /// Get gauge offset
    ///
    /// @return the gauge offset attribute
    ///
    Decimal  &gaugeOffset() { return _gauge_offset; }

	///
	/// Get gauge cap
	/// @return the gauge cap element
	///
    String &gaugeCap() { return _gauge_cap; }

    ///
    /// Get gauge border
    ///
    /// @return the gauge border attribute
    ///
    Decimal  &gaugeBorder() { return _gauge_border; }

	///
	/// Get gauge border color
	/// @return the gauge border color element
	///
    String &gaugeBorderColor() { return _gauge_border_color; }

	///
	/// Get gauge background color
	/// @return the gauge background color element
	///
    String &gaugeBackgroundColor() { return _gauge_background_color; }

	///
	/// Get gauge primary color
	/// @return the gauge primary color element
	///
    String &gaugePrimaryColor() { return _gauge_primary_color; }

	///
	/// Get gauge secondary color
	/// @return the gauge secondary color element
	///
    String &gaugeSecondaryColor() { return _gauge_secondary_color; }

	///
	/// Get needle type
	/// @return the needle type element
	///
    String &needleType() { return _needle_type; }

    ///
    /// Get needle distance
    ///
    /// @return the needle distance attribute
    ///
    Decimal &needleDistance() { return _needle_distance; }

    ///
    /// Get needle width
    ///
    /// @return the needle width attribute
    ///
    Decimal  &needleWidth() { return _needle_width; }

    ///
    /// Get needle border
    ///
    /// @return the needle border attribute
    ///
    Decimal  &needleBorder() { return _needle_border; }

	///
	/// Get needle border color
	/// @return the needle border color element
	///
    String &needleBorderColor() { return _needle_border_color; }

	///
	/// Get needle background color
	/// @return the needle background color element
	///
    String &needleBackgroundColor() { return _needle_background_color; }

	///
	/// Get needle primary color
	/// @return the needle primary color element
	///
    String &needlePrimaryColor() { return _needle_primary_color; }

	///
	/// Get needle secondary color
	/// @return the needle secondary color element
	///
    String &needleSecondaryColor() { return _needle_secondary_color; }

    ///
    /// Get tick size
    ///
    /// @return the tick size attribute
    ///
    Decimal  &tickSize() { return _tick_size; }

    ///
    /// Get tick color
    ///
    /// @return the tick color element
    ///
    String &tickColor() { return _tick_color; }

    ///
    /// Get tick label distance
    ///
    /// @return the tick label distance attribute
    ///
    Decimal  &tickLabelDistance() { return _tick_label_distance; }

    ///
    /// Get tick label font size
    ///
    /// @return the tick label font size element
    ///
    Decimal  &tickLabelFontSize() { return _tick_label_fontsize; }

    ///
    /// Get tick label color
    ///
    /// @return the tick label color element
    ///
    String &tickLabelColor() { return _tick_label_color; }

    ///
    /// Get tick label border color
    ///
    /// @return the tick label border color element
    ///
    String &tickLabelBorderColor() { return _tick_label_bordercolor; }

    ///
    /// Get nbr lap
    ///
    /// @return the nbr lap element
    ///
    Unsigned  &nbrLap() { return _nbr_lap; }

	/// 
	/// Get label flag
	///
	/// @return the label flag element
	///
	Boolean  &withLabel() { return _with_label; }

	/// 
	/// Get value flag
	///
	/// @return the value flag element
	///
	Boolean  &withValue() { return _with_value; }

	/// 
	/// Get icon flag
	///
	/// @return the icon flag element
	///
	Boolean  &withIcon() { return _with_icon; }

	/// 
	/// Get unit flag
	///
	/// @return the unit flag element
	///
	Boolean  &withUnit() { return _with_unit; }

	/// 
	/// Get tick flag
	///
	/// @return the tick flag element
	///
	Boolean  &withTick() { return _with_tick; }

	/// 
	/// Get tick label flag
	///
	/// @return the tick label flag element
	///
	Boolean  &withTickLabel() { return _with_tick_label; }

	/// 
	/// Get gauge flag
	///
	/// @return the gauge flag element
	///
	Boolean  &withGauge() { return _with_gauge; }

	/// 
	/// Get needle flag
	///
	/// @return the needle flag element
	///
	Boolean  &withNeedle() { return _with_needle; }

    // Methods

    private:
    
    // Members
    String       _name;
    String       _type;
    String       _shape;
	Boolean      _display;
    String       _position;
    String       _orientation;
    Unsigned     _at, _duration;
    Unsigned     _x, _y;
	Unsigned     _width, _height;
	Unsigned     _margin;
	Decimal      _margin_left;
	Decimal      _margin_right;
	Decimal      _margin_top;
	Decimal      _margin_bottom;
	Unsigned     _padding;
	Decimal      _padding_left;
	Decimal      _padding_right;
	Decimal      _padding_top;
	Decimal      _padding_bottom;

	Decimal      _border;
	String       _bordercolor;
	Decimal      _round_corner;
	String       _bgcolor;

	String       _text;

	Decimal      _line_space;
	String       _text_orientation;

    String       _icon_name;
	Decimal      _icon_size;
	String       _icon_color;

	String       _label_fontfamily;
	Decimal      _label_fontsize;
	String       _label_fontstyle;
	String       _label_fontweight;
	String       _label_horizontal_align;
	String       _label_vertical_align;
	String       _label_color;
	Decimal      _label_shadowopacity;
	Decimal      _label_shadowdistance;
	Decimal      _label_borderwidth;
	String       _label_bordercolor;

	String       _value_fontfamily;
	Decimal      _value_fontsize;
	String       _value_fontstyle;
	String       _value_fontweight;
	String       _value_horizontal_align;
	String       _value_vertical_align;
	String       _value_color;
	Decimal      _value_shadowopacity;
	Decimal      _value_shadowdistance;
	Decimal      _value_borderwidth;
	String       _value_bordercolor;
    Decimal      _value_min;
    Decimal      _value_max;
    String       _value_unit;
    String       _value_format;

	String       _unit_fontfamily;
	Decimal      _unit_fontsize;
	String       _unit_fontstyle;
	String       _unit_fontweight;
	String       _unit_horizontal_align;
	String       _unit_vertical_align;
	String       _unit_color;
	Decimal      _unit_shadowopacity;
	Decimal      _unit_shadowdistance;
	Decimal      _unit_borderwidth;
	String       _unit_bordercolor;
	Decimal      _unit_distance;

	Decimal      _gauge_angle;
	Decimal      _gauge_rotation;
	String       _gauge_orientation;
	Boolean      _gauge_flip;
	Decimal      _gauge_width;
	Decimal      _gauge_offset;
	String       _gauge_cap;
	Decimal      _gauge_border;
	String       _gauge_border_color;
	String       _gauge_background_color;
	String       _gauge_primary_color;
	String       _gauge_secondary_color;

	String       _needle_type;
	Decimal      _needle_distance;
	Decimal      _needle_width;
	Decimal      _needle_border;
	String       _needle_border_color;
	String       _needle_background_color;
	String       _needle_primary_color;
	String       _needle_secondary_color;

	Decimal      _tick_size;
	String       _tick_color;
	Decimal      _tick_label_distance;
	Decimal      _tick_label_fontsize;
	String       _tick_label_color;
	String       _tick_label_bordercolor;

    String       _zoom;
	String       _source;
	Unsigned     _nbr_lap;

	Boolean      _with_label;
	Boolean      _with_value;
	Boolean      _with_icon;
	Boolean      _with_unit;
	Boolean      _with_tick;
	Boolean      _with_tick_label;
	Boolean      _with_gauge;
	Boolean      _with_needle;
    
    // Disable copy constructors
    Widget(const Widget &);
    Widget& operator=(const Widget &);  
  };
}

#endif

