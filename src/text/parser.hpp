//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef __TEXT_PARSER_HPP
#define __TEXT_PARSER_HPP

#include <string>
#include <sstream>
#include "text/template.hpp"

namespace text
{

enum class TemplateBlock
{
  Head,
  Body,
  Foot
};

enum class TemplateState
{
  None,
  Open,
  Close,
  ReadFilter,
  ReadLogic,
  Escape,
  Skip
};

class TemplateBuffer
{
  std::ostringstream buffer;
  std::ostringstream var;
  std::stringstream ss;

  TemplateState state;
  TemplateBlock block;
  Template templ;

  int charPos;
  int linePos;
  
public:
  TemplateBuffer() : 
    ss(),
    state(TemplateState::None), 
    block(TemplateBlock::Head),
    charPos(1),
    linePos(1)
  {}

  void Parse();
  void ParseState(char c); 
  void ParseChar(char c); 
  void ParseFilter();
  void ParseLogic();
  void ParseBlock();
  void ParseInclude(const std::string& file);

  std::stringstream& Stream() { return ss; }

  void Append(char c) { buffer << c; }

  int LinePos() const { return linePos; }
  int CharPos() const { return charPos; }

  const TemplateState& State() const { return state; }
  void State(const TemplateState& state) { this->state = state; }

  Template GetTemplate() { return templ; }
};

class TemplateParser
{
  std::string file;
  TemplateBuffer buf;
public:
  TemplateParser(const std::string& file) :
    file(file)
  {}

  Template Create();

};

/* end */
}
#endif

