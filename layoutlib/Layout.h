#ifndef __LAYOUT__LAYOUT_H__
#define __LAYOUT__LAYOUT_H__

//==============================================================================
//
//                   LAYOUT - the LAYOUT class
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

#include "String_.h"
//#include "Metadata.h" 
//#include "Extensions.h"
#include "List.h"
//#include "RTE.h"
//#include "TRK.h"
//#include "WPT.h"
#include "Widget.h"

///
/// @mainpage
///
/// This is the documentation of the layoutlib library.
///
/// See Related Pages for the ToDo list.
///
/// See Namespaces for the layout namespace documentation.
///
/// See Classes for the Class list, Class Index, Class Hierarchy and Class Members documentation.
///
/// Enjoy.
///

///
/// @namespace layout
///
/// @brief The namespace of the layout library
///

namespace layout
{
  ///
  /// @class Layout
  ///
  /// @brief The root node of a Layout document
  ///

  class DLL_API Layout : public Node
  {
  public:

    ///
    /// Constructor
    ///
    Layout();

    ///
    /// Deconstructor
    ///
    virtual ~Layout();
    
    ///
    /// Get version
    ///
    /// @return the version attribute
    ///
    String  &version() { return _version; }

    ///
    /// Get creator
    ///
    /// @return the creator attribute
    ///
    String  &creator() { return _creator; }

//    ///
//    /// Get metadata
//    ///
//    /// @return the metadata element
//    ///
//    Metadata  &metadata() { return _metadata; }
//
//    ///
//    /// Get wpt
//    ///
//    /// @return the list of wpt elements
//    ///
//    List<WPT> &wpts() { return _wpts;}
//
//    ///
//    /// Get rte
//    ///
//    /// @return the list of rte elements
//    ///
//    List<RTE> &rtes() {return _rtes;}
//
//    ///
//    /// Get trk
//    ///
//    /// @return the list of trk elements
//    ///
//    List<TRK> &trks() {return _trks;}
//
//    ///
//    /// Get extensions
//    ///
//    /// @return the extensions element
//    ///
//    Extensions  &extensions() { return _extensions; }

    ///
    /// Get widget
    ///
    /// @return the list of widget elements
    ///
    List<Widget> &widgets() {return _widgets;}

  private:

    // Members

    String      _version;
    String      _creator;
//    Metadata    _metadata;
//    Extensions  _extensions;
//    List<WPT>   _wpts;
//    List<RTE>   _rtes;
//    List<TRK>   _trks;

    List<Widget>   _widgets;

    // Disable copy constructors
    Layout(const Layout &);
    Layout& operator=(const Layout &);  
  };
}


#endif

