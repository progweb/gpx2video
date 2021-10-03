#ifndef NODE_H
#define NODE_H

//==============================================================================
//
//                   Node - the gpx node class
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

#include <string>
#include <list>

#ifdef WIN32
#ifndef strcasecmp
#define strcasecmp _stricmp
#endif
#pragma warning(disable:4355)
#pragma warning(disable:4251)
#pragma warning(disable:4275)
#endif

#ifdef __linux__
#include <strings.h>
#endif

#include "Report.h"


namespace gpx
{
  ///
  /// @class Node
  ///
  /// @brief The base node class.
  ///
  
  class Node
  {
    public:

    ///
    /// Node type
    ///
    enum Type
    {
      ATTRIBUTE,
      ELEMENT
    };
    
    ///
    /// Constructor
    ///
    /// @param  parent     the parent node
    /// @param  name       the name of the attribute or element
    /// @param  type       the node type (ATTRIBUTE or ELEMENT)
    /// @param  mandatory  is the attribute or element mandatory ?
    ///
    Node(Node *parent, const char *name, Type type, bool mandatory);

    ///
    /// Deconstructor
    ///
    virtual ~Node();
    
    
    // Properties
    
    ///
    /// Return the name
    ///
    /// @return the name of the attribute or element
    ///
    const std::string &getName() const { return _name; }

    ///
    /// Return the type
    ///
    /// @return the name of the attribute or element
    ///
    Node::Type getType() const { return _type; }

    ///
    /// Return the value
    ///
    /// @return the value of the attribute or element
    ///
    virtual const std::string &getValue() const { return _value; }

    ///
    /// Return the trimmed value
    ///
    /// @return the trimmed value of the attribute or element
    ///
    std::string getTrimmedValue() const;

    ///
    /// Is mandatory
    ///
    /// @return is the attribute or element mandatory
    ///
    bool isMandatory() const { return _mandatory; }
    
    ///
    /// Set the value
    ///
    /// @param value   the value of the attribute or element
    ///
    virtual void setValue(std::string value) { _value = value; }

    ///
    /// Append to the value of the element
    ///
    /// @param value   the value to append to the attribute or element
    ///
    virtual void appendValue(const std::string& value) { _value.append(value); }

    ///
    /// Get the parent node of this node
    ///
    /// @return the parent node (or 0)
    ///
    Node *getParent() const { return _parent; }

    ///
    /// Get the interfaces list
    ///
    /// @return the interface list
    ///
    ///
    std::list<Node*> &getInterfaces() { return _interfaces; }

    ///
    /// Get the attributes list
    ///
    /// @return the attributes list
    ///
    std::list<Node*> &getAttributes() { return _attributes; }
    
    ///
    /// Get the elements list
    ///
    /// @return the elements list
    ///
    std::list<Node*> &getElements() { return _elements; }
        
    // Methods
    
    ///
    /// Add this child node
    ///
    /// @param  report  the optional report stream
    ///
    /// @return the node (or 0 if not found)
    ///
    virtual Node *add(Report *report = nullptr);

    ///
    /// Add a child node by name
    ///
    /// @param  name    the name of the node
    /// @param  type    the type of the node
    /// @param  report  the optional report stream
    ///
    /// @return the node (or 0 if not found)
    ///
    virtual Node *add(const char *name, Type type, Report *report = nullptr);

    ///
    /// Done building the node
    ///
    /// @return the parent node (or 0 if not present)
    ///
    virtual Node *added() { return _parent; }

    ///
    /// Insert this child node before a node
    ///
    /// @param  before  the node for which this node must be inserted (0 for append)
    /// @param  report  the optional report stream
    ///
    /// @return the node (or 0 if not found)
    ///

    virtual Node *insert(Node *before, Report *report = nullptr);

    ///
    /// Insert a child node by name before a node
    ///
    /// @param  before  the node for which this node must be inserted (0 for append)
    /// @param  name    the name of the node
    /// @param  type    the type of the node
    /// @param  report  the optional report stream
    ///
    /// @return the node (or 0 if not found)
    ///
    virtual Node *insert(Node *before, const char *name, Type type, Report *report = nullptr);

    ///
    /// Copy the contents of the source node in this node
    ///
    /// @param  source  the source node
    /// @param  report  the optional report stream
    ///
    /// @return is validation succesfull
    ///
    virtual void copy(Node *source, Report *report = nullptr);

    ///
    /// Validate the object
    ///
    /// @param  report  the optional report stream
    ///
    /// @return is validation succesfull
    ///
    virtual bool validate(Report *report = nullptr) const;
          
    ///
    /// Remove the child node from this node, if removed the
    /// node can be deleted. If child is 0, all childs are
    /// removed
    ///
    /// @param  child   the child node or 0 for all childs
    /// @param  report  the optional report stream
    ///
    /// @return is it removed ?
    ///
    virtual bool remove(Node *child, Report *report = nullptr);

    ///
    /// Count the number of occurences of this node in its parent
    ///
    /// @return the number of occurences
    ///
    int count() const;

    ///
    /// Check if this node is used by its parent
    ///
    /// @return the number of occurences
    ///
    bool used() const;

    ///
    /// Check if this node has at least one element
    ///
    /// @return has it ?
    ///

    bool hasElements() const;

    ///
    /// Check if this node or one of its parents is an extension node
    ///
    /// @return is it ?
    ///
    
    bool isExtension() const;

  public:

    ///
    /// Insert this node in the nodes list before a node (internal)
    ///
    /// @param  before    the node for which this node must be inserted (0 for append)
    /// @param  nodese    the list with nodes in which this node must be inserted
    /// @param  report    the optional report stream
    ///
    void insert(Node *before, std::list<Node*> &nodes, Report *report = nullptr);

  private:

    ///
    /// Remove the node as child
    ///
    /// @param  node    the child node to be removed
    ///
    /// @return is it removed ?
    ///
    virtual bool removeAsChild(Node *node);

    // Members
    Node              *_parent;
    std::string        _name;
    Type               _type;
    std::string        _value;
    std::list<Node*>   _interfaces;
    std::list<Node*>   _attributes;
    std::list<Node*>   _elements;
    std::list<Node*>   _unknownElements;
    bool               _mandatory;
    
    // Do not implement
    Node(const Node &);
    Node& operator=(const Node &);
  };
}


#endif

