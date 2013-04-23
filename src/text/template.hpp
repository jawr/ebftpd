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
  Template() :
    head(false),
    body(false),
    foot(true)
  { }

  TemplateSection& Head() { return head; }
  TemplateSection& Body() { return body; }
  TemplateSection& Foot() { return foot; }

  friend class TemplateParser;
};

// end
}
#endif

