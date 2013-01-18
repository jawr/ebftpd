#ifndef __TEXT_TEMPLATE_HPP
#define __TEXT_TEMPLATE_HPP

#include "text/templatesection.hpp"

namespace text
{

class Template
{
  TemplateSection head;
  TemplateSection body;
  TemplateSection foot;
public:
  Template() {}

  TemplateSection& Head() { return head; }
  TemplateSection& Body() { return body; }
  TemplateSection& Foot() { return foot; }

  friend class TemplateParser;
};

// end
}
#endif

