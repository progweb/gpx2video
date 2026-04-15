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
    /// Get unit
    ///
    /// @return the unit element
    ///
    String &unit() { return _unit; }

    ///
    /// Get zoom
    ///
    /// @return the zoom element
    ///
    String &zoom() { return _zoom; }

    ///
    /// Get format
    ///
    /// @return the format element
    ///
    String &format() { return _format; }

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
    /// Get font
    ///
    /// @return the font element
    ///
    String &font() { return _font; }

    ///
    /// Get text color
    ///
    /// @return the text color element
    ///
    String &text() { return _text; }

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
    /// Get label align
    ///
    /// @return the label align element
    ///
    String &labelAlign() { return _label_align; }

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
    /// Get value align
    ///
    /// @return the value align element
    ///
    String &valueAlign() { return _value_align; }

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
	/// Get needle type
	/// @return the needle type element
	///
    String &needleType() { return _needle_type; }

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
    /// Get border
    ///
    /// @return the border attribute
    ///
    Unsigned  &border() { return _border; }

    ///
    /// Get border color
    ///
    /// @return the border color element
    ///
    String &borderColor() { return _bordercolor; }

    ///
    /// Get background color
    ///
    /// @return the background color element
    ///
    String &backgroundColor() { return _bgcolor; }

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

	Unsigned     _border;
	String       _bordercolor;
	String       _bgcolor;

	String       _font;
	String       _text;

	String       _label_fontfamily;
	Decimal      _label_fontsize;
	String       _label_fontstyle;
	String       _label_fontweight;
	String       _label_align;
	String       _label_color;
	Decimal      _label_shadowopacity;
	Decimal      _label_shadowdistance;
	Decimal      _label_borderwidth;
	String       _label_bordercolor;

	String       _value_fontfamily;
	Decimal      _value_fontsize;
	String       _value_fontstyle;
	String       _value_fontweight;
	String       _value_align;
	String       _value_color;
	Decimal      _value_shadowopacity;
	Decimal      _value_shadowdistance;
	Decimal      _value_borderwidth;
	String       _value_bordercolor;

	String       _needle_type;
	String       _needle_primary_color;
	String       _needle_secondary_color;

    String       _unit;
    String       _zoom;
    String       _format;
	String       _source;
	Unsigned     _nbr_lap;

	Boolean      _with_label;
	Boolean      _with_value;
	Boolean      _with_icon;
	Boolean      _with_unit;
	Boolean      _with_tick;
	Boolean      _with_tick_label;
	Boolean      _with_needle;
    
    // Disable copy constructors
    Widget(const Widget &);
    Widget& operator=(const Widget &);  
  };
}

#endif

