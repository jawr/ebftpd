#include <fstream>
#include <memory>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include "text/template.hpp"
#include "text/parser.hpp"
#include "text/templatesection.hpp"
#include "text/error.hpp"
#include "logs/logs.hpp"

namespace text
{

Template TemplateParser::Create()
{
  std::ifstream io(file.c_str()); 
  if (!io) throw TemplateError("Unable to open template file: " + file);

  std::stringstream ss;

  while (io.good())
  {
    char c;
    io >> std::noskipws >> c;
    ss << c;
  }

  Parse(ss);

  return buf.GetTemplate();
}

void TemplateParser::Parse(std::stringstream& ss)
{ 
  while (ss.good())
  {
    char c = ss.get();

    switch (buf.State())
    {
      case TemplateState::Escape:
        buf.Append(c);
        continue;

      case TemplateState::Skip:
        if (c == '\n' || c == '\r' || c == '}' || c == ' ')
          continue;
        buf.State(TemplateState::None);
        break;

      case TemplateState::Close:
        if (c != '}')
          throw TemplateMalform(buf.LinePos(), buf.CharPos());
        buf.State(TemplateState::None);
        continue; /* don't need this in the buffer */

      case TemplateState::Open:
      case TemplateState::None:
      case TemplateState::ReadLogic:
      case TemplateState::ReadFilter:
      default:
        break;
    }

    buf.ParseChar(c);
  }
}
        
void TemplateBuffer::ParseChar(char& c)
{
  ++charPos;
  if (c == '\n' || c == '\r')
  {
    charPos = 0;
    ++linePos;
  }

  switch (c)
  {
    case '\\':
      state = TemplateState::Escape;
      return;

    case '{':
      if (state == TemplateState::Open)
      {
        state = TemplateState::ReadFilter;
        return; // we don't need to capture the {
      }
      else
      {
        state = TemplateState::Open;
        return;
      }
      break;

    case '%':
      if (state == TemplateState::ReadLogic)
      {
        ParseLogic();
        state = TemplateState::Skip;
        return; // we don't need to capture this %
      }
      else if (state == TemplateState::Open)
      {
        state = TemplateState::ReadLogic;
        return; // we don't need to capture this %
      }
      break;

    case '}':
      if (state == TemplateState::ReadFilter)
      {
        ParseFilter();
        state = TemplateState::Close;
        return; // we don't need to capture this }
      }
      break;

    default:
      break;
  }

  if (state == TemplateState::ReadLogic ||
      state == TemplateState::ReadFilter)
  {
    var << c;
  }
  else
  {
    buffer << c;
  }
}

void TemplateBuffer::ParseBlock()
{
  if (block == TemplateBlock::Head)
    templ.Head().RegisterBuffer(buffer.str());
  
  else if (block == TemplateBlock::Body)
    templ.Body().RegisterBuffer(buffer.str());

  else if (block == TemplateBlock::Foot)
    templ.Foot().RegisterBuffer(buffer.str());
  

  /* reset */
  state = TemplateState::None;
  block = TemplateBlock::Head;
  linePos = 1;
  charPos = 1;
  var.str(std::string());
  buffer.str(std::string());
}

void TemplateBuffer::ParseLogic()
{
  std::string logic = var.str();
  boost::trim(logic);
  boost::to_lower(logic);

  if (logic == "endblock")
    ParseBlock();

  else if (logic == "head")
  {
    buffer.str(std::string()); /* clear buffer because the start of a new block */
    block = TemplateBlock::Head; 
  }
  else if (logic == "body")
  {
    buffer.str(std::string());
    block = TemplateBlock::Body; 
  }
  else if (logic == "foot")
  {
    buffer.str(std::string());
    block = TemplateBlock::Foot; 
  }
  else
    throw TemplateMalform(linePos, charPos, "(" + logic + ")"
      + " incorrect syntax. Must be {% endblock|head|body|foot %}");

  /* cleanup */
  var.str(std::string());
}

void TemplateBuffer::ParseFilter()
{
  std::string filter = var.str();
  boost::trim(filter);
  boost::to_lower(filter);

  buffer << "{{";

  if (block == TemplateBlock::Head)
    buffer << templ.Head().RegisterTag(filter);
  
  else if (block == TemplateBlock::Body)
    buffer << templ.Body().RegisterTag(filter);

  else if (block == TemplateBlock::Foot)
    buffer << templ.Foot().RegisterTag(filter);

   buffer << "}}";

  /* cleanup */
  var.str(std::string());
}

}
#ifdef TEXT_TEMPLATE_TEST
#include "logs/logs.hpp"
#include "text/factory.hpp"
#include "cfg/get.hpp"
#include "cfg/config.hpp"
int main()
{
  cfg::UpdateShared(std::shared_ptr<cfg::Config>(new cfg::Config("ftpd.conf"))); 
  text::Factory::Initalize();

  try
  {
    text::Template groups = text::Factory::GetTemplate("groups");
    std::ostringstream os;
    text::TemplateSection& head = groups.Head();
    os << head.Compile();
    text::TemplateSection& body = groups.Body();
    for (int i = 0; i < 5; ++i)
    {
      body.Reset();
      body.RegisterValue("users", i);
      body.RegisterValue("group", i);
      body.RegisterValue("description", "HELLO THIS IS A DESCR");
      os << body.Compile();
    }
    text::TemplateSection& foot = groups.Foot();
    foot.RegisterValue("total_groups", 5);
    os << foot.Compile();
    logs::debug << "COMPILED:" << logs::endl;
    logs::debug << os.str() << logs::endl;
  }
  catch (const text::TemplateError& e)
  {
    logs::error << e.Message() << logs::endl;
    return 1;
  }
  
  return 0;
}
#endif
  
