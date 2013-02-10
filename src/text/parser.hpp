#ifndef __TEXT_PARSER_HPP
#define __TEXT_PARSER_HPP

#include <string>
#include <sstream>
#include "text/template.hpp"
#include "text/templatesection.hpp"

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

