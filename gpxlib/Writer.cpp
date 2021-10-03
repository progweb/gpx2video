//==============================================================================
//
//             Writer - the writer class in the gpx library
//
//               Copyright (C) 2016  Dick van Oudheusden
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

#include "Writer.h"


using namespace std;

namespace gpx
{
  
  Writer::Writer()
  {
  }

  Writer::~Writer()
  {
  }

  // Writing

  bool Writer::write(std::ostream &stream, Node *node, bool prettyPrinted)
  {
    bool ok = stream.good();

    if (ok)
    {
      if (node != nullptr)
      {
        GPX *root = dynamic_cast<GPX*>(node);

        if (root != nullptr)
        {
          // Root node
          stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
        }

        doWrite(stream, node, (prettyPrinted ? 0 : -1));
      }
    }

    return stream.good();
  }


  // Privates

  void Writer::indent(std::ostream &stream, int level)
  {
    while (level > 0)
    {
      stream << ' ';

      level--;
    }
  }

  std::string Writer::translate(const std::string &value)
  {
    string output;

    for (string::const_iterator ch = value.begin(); ch != value.end(); ++ch)
    {
      switch(*ch)
      {
        case '<' : output += "&lt;";   break;
        case '>' : output += "&gt;";   break;
        case '&' : output += "&amp;";  break;
        case '"' : output += "&quot;"; break;
        case '\'': output += "&apos;"; break;
        default  : output += *ch;      break;
      }
    }

    return output;
  }

  bool Writer::doWrite(std::ostream &stream, Node *node, int level)
  {
    indent(stream, level);

    // Start of tag
    stream << '<' << node->getName();

    // attributes
    for (list<Node*>::const_iterator iter = node->getAttributes().begin(); iter != node->getAttributes().end(); ++iter)
    {
      stream << ' ' << (*iter)->getName() << "=\"" << translate((*iter)->getValue()) << '"';
    }

    // End of tag
    stream << '>';

    if ((node->hasElements()) && (level >= 0))
    {
      stream << endl;
    }

    // child tags
    int next = (level >= 0 ? level+1 : level);

    for (list<Node*>::const_iterator iter = node->getElements().begin(); iter != node->getElements().end(); ++iter)
    {
      doWrite(stream, (*iter), next);
    }

    // Value
    stream << translate(node->getTrimmedValue());

    // Close tag
    if ((node->hasElements()) && (level >= 0))
    {
      indent(stream, level);
    }

    stream << "</" << node->getName() << '>';

    if (level >= 0)
    {
      stream << endl;
    }

    return stream.good();
  }
}
