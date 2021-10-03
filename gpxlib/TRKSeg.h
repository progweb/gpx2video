#ifndef TRK_SEG_H
#define TRK_SEG_H

//==============================================================================
//
//             TRKSeg - the track segment class in the GPX library
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

#include "Node.h"

#include "Extensions.h"
#include "List.h"
#include "WPT.h"


namespace gpx
{
  ///
  /// @class TRKSeg
  ///
  /// @brief The track segment class.
  ///
  
  class TRKSeg : public Node
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
    TRKSeg(Node *parent, const char *name, Node::Type type, bool mandatory = false);

    ///
    /// Deconstructor
    ///
    virtual ~TRKSeg();

    ///
    /// Get trkpt
    ///
    /// @return the trkpt elements
    ///
    List<WPT>  &trkpts() {return _trkpts;}

    ///
    /// Get extensions
    ///
    /// @return the extensions element
    ///
    Extensions  &extensions() { return _extensions; }

    // Methods
    
    private:
    
    // Members
    Extensions  _extensions;
    List<WPT>   _trkpts;

    // Disable copy constructors
    TRKSeg(const TRKSeg &);
    TRKSeg& operator=(const TRKSeg &);  
  };
}

#endif

