#ifndef __LAYOUT__MAP_H__
#define __LAYOUT__MAP_H__

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
  /// @class Map
  ///
  /// @brief The track class.
  ///
  
  class DLL_API Map : public Node
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
    Map(Node *parent, const char *name, Type type, bool mandatory = false);

    ///
    /// Deconstructor
    ///
    virtual ~Map();
    
    ///
    /// Get source
    ///
    /// @return the source element
    ///
    Unsigned  &source() { return _source; }

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
    /// Get orientation
    ///
    /// @return the orientation attribute
    ///
    String  &orientation() { return _orientation; }

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
    /// Get view
    ///
    /// @return the view element
    ///
    String  &view() { return _view; }

    ///
    /// Get zoom
    ///
    /// @return the zoom element
    ///
    Unsigned  &zoom() { return _zoom; }

	///
	/// Get factor
	/// 
	/// @return the factor elememnt
	///
	Decimal &factor() { return _factor; }

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
    /// Get path smooth
    ///
    /// @return the path smooth element
    ///
    Unsigned  &pathSmooth() { return _path_smooth; }

    ///
    /// Get path thick
    ///
    /// @return the path thick element
    ///
    Decimal  &pathThick() { return _path_thick; }

    ///
    /// Get path border
    ///
    /// @return the path border element
    ///
    Decimal  &pathBorder() { return _path_border; }

    ///
    /// Get path border color
    ///
    /// @return the path border color element
    ///
    String  &pathBorderColor() { return _path_border_color; }

    ///
    /// Get path primary color
    ///
    /// @return the path primary color element
    ///
    String  &pathPrimaryColor() { return _path_primary_color; }

    ///
    /// Get path secondary color
    ///
    /// @return the path secondary color element
    ///
    String  &pathSecondaryColor() { return _path_secondary_color; }

    ///
    /// Get icon name
    ///
    /// @return the icon name element
    ///
    String &iconEnd() { return _icon_end_name; }
    String &iconStart() { return _icon_start_name; }
    String &iconPosition() { return _icon_position_name; }

    ///
    /// Get icon color
    ///
    /// @return the icon color element
    ///
    String &iconEndColor() { return _icon_end_color; }
    String &iconStartColor() { return _icon_start_color; }
    String &iconPositionColor() { return _icon_position_color; }

    ///
    /// Get icon size
    ///
    /// @return the icon size element
    ///
    Decimal  &iconEndSize() { return _icon_end_size; }
    Decimal  &iconStartSize() { return _icon_start_size; }
    Decimal  &iconPositionSize() { return _icon_position_size; }

	/// 
	/// Get icon flag
	///
	/// @return the icon flag element
	///
	Boolean  &withIconEnd() { return _with_icon_end; }
	Boolean  &withIconStart() { return _with_icon_start; }
	Boolean  &withIconPosition() { return _with_icon_position; }

    // Methods

    private:
    
    // Members
    Unsigned     _source;
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

	String       _view;
	Unsigned     _zoom;
	Decimal      _factor;
	Unsigned     _border;
	String       _bordercolor;
	String       _bgcolor;
	Unsigned     _path_smooth;
	Decimal      _path_thick;
	Decimal      _path_border;
	String       _path_border_color;
	String       _path_primary_color;
	String       _path_secondary_color;
    
    String       _icon_end_name;
    String       _icon_start_name;
    String       _icon_position_name;
	String       _icon_end_color;
	String       _icon_start_color;
	String       _icon_position_color;
	Decimal      _icon_end_size;
	Decimal      _icon_start_size;
	Decimal      _icon_position_size;

	Boolean      _with_icon_end;
	Boolean      _with_icon_start;
	Boolean      _with_icon_position;

    // Disable copy constructors
    Map(const Map &);
    Map& operator=(const Map &);  
  };
}

#endif

