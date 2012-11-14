#ifndef __TEXT_TEMPLATE_HPP
#define __TEXT_TEMPLATE_HPP

#include <string>
#include "text/templatesection.hpp"

namespace text
{

class Template
{
  std::string file;
  TemplateSection head;
  TemplateSection body;
  TemplateSection foot;
public:
  Template(const std::string& file) :
    file(file),
    head(SectionType::Head),
    body(SectionType::Body),
    foot(SectionType::Foot)
  {}

  void Initalize();

  TemplateSection& Head() { return head; }
  TemplateSection& Body() { return body; }
  TemplateSection& Foot() { return foot; }
};

// end
}
#endif

