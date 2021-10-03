#ifndef PT_SEG_H
#define PT_SEG_H

//==============================================================================
//
//              PTSeg - the Point Segment class in the GPX library
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

#include <list>

#include "Node.h"
#include "PT.h"
#include "List.h"


namespace gpx
{
  ///
  /// @class PTSeg
  ///
  /// @brief The ordered sequence of points class.
  ///
  
  class PTSeg : public Node
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

    PTSeg(Node *parent, const char *name, Type type, bool mandatory = false);

    ///
    /// Deconstructor
    ///
    virtual ~PTSeg();
    
    // Properties
    
    ///
    /// Get PTs
    ///
    /// @return the PT elements
    ///
    List<PT> &pts() {return _pts;}

    // Methods
    
    private:
    
    // Members
    List<PT>  _pts;
        
    // Disable copy constructors
    PTSeg(const PTSeg &);
    PTSeg& operator=(const PTSeg &);  
  };
}

#endif

