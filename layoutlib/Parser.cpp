//==============================================================================
//
//             Parser - the parser class in the layout library
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
//  $Date: 2013-03-10 12:02:27 +0100 (Sun, 10 Mar 2013) $ $Revision: 5 $
//
//==============================================================================

#include <string>
#include <cstring>

#include "Parser.h"


using namespace std;

namespace layout
{
  
  Parser::Parser(Report *report) :
    _parser(nullptr),
    _errorText(),
    _errorLineNumber(0),
    _errorColumnNumber(0),
    _current(nullptr),
    _root(nullptr),
    _report(report)
  {
  }

  Parser::~Parser()
  {
    if (_parser != nullptr)
    {
      stopExpat();
    }
  }

  // Building

  void Parser::makeElement(const char *name)
  {
    // cout << "BuildElement:" << name << " for " << (_current != 0 ? _current->name().c_str() : "null") << endl;

    if (strcasecmp(name, "layout") == 0)
    {
      if (_root == nullptr)
      {
        _current = _root = new Layout();
      }
      else if (_report != nullptr)
      {
        _report->report(_root, Report::DOUBLE_LAYOUT, "");
      }
    }
    else if (_current != nullptr)
    {
      _current = _current->add(name, Node::ELEMENT, _report);
      _current->setValue("");
    }
    else if (_report != nullptr)
    {
      _report->report(nullptr, Report::MISSING_LAYOUT, "");
    }
  }

  void Parser::makeAttribute(const char *name)
  {
    // cout << "BuildAttribute:" << name << " for " << (_current != 0 ? _current->name().c_str() : "null") << endl;

    if (_current != nullptr)
    {
      _current = _current->add(name, Node::ATTRIBUTE, _report);
      _current->setValue("");
    }
    else if (_report != nullptr)
    {
      _report->report(nullptr, Report::MISSING_LAYOUT, "");
    }
  }

  void Parser::made()
  {
    // cout << "Built for " << (_current != 0 ? _current->name().c_str() : "null") << endl;

    if (_current != nullptr)
    {
      _current = _current->added();
    }
    else if (_report != nullptr)
    {
      _report->report(nullptr, Report::MISFORMED_LAYOUT, "");
    }
  }

  void Parser::value(std::string value)
  {
    if (_current != nullptr)
    {
      _current->appendValue(value);
    }
    else if (_report != nullptr)
    {
      _report->report(nullptr, Report::MISFORMED_LAYOUT, "");
    }
  }


  // Parsing

  bool Parser::parse(const char *data, size_t length, bool isFinal)
  {
    bool ok = false;

    if (_parser == nullptr)
    {
      startExpat();

      if (_parser == nullptr)
      {
        return false;
      }
    }

    ok = (XML_Parse(_parser, data, int(length), isFinal) != XML_STATUS_ERROR);

    if (!ok)
    {
      _errorText         = XML_ErrorString(XML_GetErrorCode(_parser));
      _errorLineNumber   = XML_GetCurrentLineNumber(_parser);
      _errorColumnNumber = XML_GetCurrentColumnNumber(_parser);
    }

    if (isFinal)
    {
      stopExpat();
    }

    return ok;
  }

  bool Parser::parse(const char *text, bool isFinal)
  {
    return parse(text, strlen(text), isFinal);
  }

  bool Parser::parse(const std::string &data, bool isFinal)
  {
    return parse(data.c_str(), data.length(), isFinal);
  }

  // Parse a stream

  Layout *Parser::parse(std::istream &stream)
  {
    bool ok = stream.good();

    if (ok)
    {
      char buffer[4096];

      while ((ok) && (stream.good()))
      {
        stream.read(buffer, sizeof(buffer));

        ok = parse(buffer, size_t(stream.gcount()), (size_t(stream.gcount()) < sizeof(buffer)));
      }
    }

    return _root;
  }


  // Privates

  void Parser::startExpat()
  {
    _parser = XML_ParserCreate(nullptr);
    
    XML_SetUserData(_parser, this);
  
    XML_SetXmlDeclHandler(_parser, xmlDeclHandler);
    XML_SetElementHandler(_parser, startElementHandler, endElementHandler);
    XML_SetCharacterDataHandler(_parser, characterDataHandler);
    XML_SetCommentHandler(_parser, commentHandler);
    XML_SetProcessingInstructionHandler(_parser, processingInstructionHandler);
    XML_SetCdataSectionHandler(_parser, startCdataSectionHandler, endCdataSectionHandler);
    XML_SetDefaultHandlerExpand(_parser, defaultHandler);
    XML_SetNamespaceDeclHandler(_parser, startNamespaceDeclHandler, endNamespaceDeclHandler);
  }
  
  void Parser::stopExpat()
  {
    XML_ParserFree(_parser);
    
    _parser = nullptr;
  }

  void Parser::xmlDeclHandler(void *userData, const XML_Char *version, const XML_Char *encoding, int standalone)
  {
//    Parser *self = static_cast<Parser*>(userData);
  }

  void Parser::startElementHandler(void *userData, const XML_Char *name, const XML_Char **atts)
  {
    Parser *self = static_cast<Parser*>(userData);

    self->makeElement(name);
    
    for (int i = 0; atts[i] != nullptr; i+=2)
    {
      self->makeAttribute(atts[i]);
      
      self->value(atts[i+1]);
      
      self->made();
    }
  }

  void Parser::endElementHandler(void *userData, const XML_Char *name)
  {
    Parser *self = static_cast<Parser*>(userData);

    self->made();
  }

  void Parser::characterDataHandler(void *userData, const XML_Char *s, int len)
  {
    Parser *self = static_cast<Parser*>(userData);
    
    string data(s, size_t(len));
    
    self->value(data);
  }

  void Parser::commentHandler(void *userData, const XML_Char *data)
  {
//    Parser *self = static_cast<Parser*>(userData);
  }

  void Parser::processingInstructionHandler(void *userData, const XML_Char *target, const XML_Char *data)
  {
//    Parser *self = static_cast<Parser*>(userData);
  }

  void Parser::startCdataSectionHandler(void *userData)
  {
//    Parser *self = static_cast<Parser*>(userData);
  }

  void Parser::endCdataSectionHandler(void *userData)
  {
//    Parser *self = static_cast<Parser*>(userData);
  }

  void Parser::defaultHandler(void *userData, const XML_Char *s, int len)
  {
//    Parser *self = static_cast<Parser*>(userData);
  }

  void Parser::startNamespaceDeclHandler(void *userData, const XML_Char *prefix, const XML_Char *uri)
  {
//    Parser *self = static_cast<Parser*>(userData);
  }

  void Parser::endNamespaceDeclHandler(void *userData, const XML_Char *prefix)
  {
//    Parser *self = static_cast<Parser*>(userData);
  }
}
