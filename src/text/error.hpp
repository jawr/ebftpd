#ifndef __TEXT_ERROR_HPP
#define __TEXT_ERROR_HPP

#include <sstream>
#include "util/error.hpp"

namespace text
{

struct TemplateError : public util::RuntimeError
{
  TemplateError() : std::runtime_error("Template Error.") {}
  TemplateError(const std::string& message) : std::runtime_error(message) {}
};

struct TemplateFilterMalform : public TemplateError
{
  TemplateFilterMalform(const std::string& message) : std::runtime_error(message) {}
};

struct TemplateNoTag : public TemplateError
{
  TemplateNoTag(const std::string& message) : std::runtime_error(message) {}
};

class TemplateMalform : public TemplateError
{
  static std::string Format(int line, int i)
  {
    std::ostringstream os;
    os << "Error parsing template (Line " << line << ": Char: " << i << ")";
    return os.str();
  }
public:
  TemplateMalform(int line, int i) :
    std::runtime_error(Format(line, i)) { }
};

// end
}
#endif
