#ifndef __LAYOUT__TRACK_H__
#define __LAYOUT__TRACK_H__

//==============================================================================
//
//               Track - the track class in the LAYOUT library
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
  /// @class Track
  ///
  /// @brief The track class.
  ///
  
  class DLL_API Track : public Node
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
    Track(Node *parent, const char *name, Type type, bool mandatory = false);

    ///
    /// Deconstructor
    ///
    virtual ~Track();
    
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
    /// Get align
    ///
    /// @return the align attribute
    ///
    String  &align() { return _align; }

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

    // Methods

    private:
    
    // Members
    Unsigned     _source;
	Boolean      _display;	
    String       _position;
    String       _align;
    Unsigned     _x, _y;
	Unsigned     _width, _height;
	Unsigned     _margin;
	Decimal      _margin_left;
	Decimal      _margin_right;
	Decimal      _margin_top;
	Decimal      _margin_bottom;
	Unsigned     _border;
	String       _bordercolor;
	String       _bgcolor;
    
    // Disable copy constructors
    Track(const Track &);
    Track& operator=(const Track &);  
  };
}

#endif

