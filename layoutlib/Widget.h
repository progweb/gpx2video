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
    String  &name() { return _name; }
    
    ///
    /// Get type
    ///
    /// @return the type element
    ///
    String  &type() { return _type; }

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
    String  &position() { return _position; }

    ///
    /// Get align
    ///
    /// @return the align attribute
    ///
    String  &align() { return _align; }

    ///
    /// Get unit
    ///
    /// @return the unit element
    ///
    String  &unit() { return _unit; }

    ///
    /// Get zoom
    ///
    /// @return the zoom element
    ///
    String  &zoom() { return _zoom; }

    ///
    /// Get format
    ///
    /// @return the format element
    ///
    String  &format() { return _format; }

    ///
    /// Get source
    ///
    /// @return the source element
    ///
    String  &source() { return _source; }

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
    String  &font() { return _font; }

    ///
    /// Get text color
    ///
    /// @return the text color element
    ///
    String  &text() { return _text; }

    ///
    /// Get text color
    ///
    /// @return the text color element
    ///
    String  &textColor() { return _txtcolor; }

    ///
    /// Get text ratio
    ///
    /// @return the text ratio attribute
    ///
    Decimal  &textRatio() { return _txtratio; }

    ///
    /// Get text shadow
    ///
    /// @return the text shadow attribute
    ///
    Unsigned  &textShadow() { return _txtshadow; }

    ///
    /// Get text line space
    ///
    /// @return the text linespace attribute
    ///
    Unsigned  &textLineSpace() { return _txtlinespace; }

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
    String  &borderColor() { return _bordercolor; }

    ///
    /// Get background color
    ///
    /// @return the background color element
    ///
    String  &backgroundColor() { return _bgcolor; }

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
	/// Get picto flag
	///
	/// @return the picto flag element
	///
	Boolean  &withPicto() { return _with_picto; }

	/// 
	/// Get unit flag
	///
	/// @return the unit flag element
	///
	Boolean  &withUnit() { return _with_unit; }

    // Methods

    private:
    
    // Members
    String       _name;
    String       _type;
	Boolean      _display;
    String       _position;
    String       _align;
    String       _unit;
    String       _zoom;
    String       _format;
	String       _source;
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
	String       _font;
	String       _text;
	String       _txtcolor;
	Decimal      _txtratio;
	Unsigned     _txtshadow;
	Unsigned     _txtlinespace;
	Unsigned     _border;
	String       _bordercolor;
	String       _bgcolor;
	Unsigned     _nbr_lap;

	Boolean      _with_label;
	Boolean      _with_value;
	Boolean      _with_picto;
	Boolean      _with_unit;
    
    // Disable copy constructors
    Widget(const Widget &);
    Widget& operator=(const Widget &);  
  };
}

#endif

