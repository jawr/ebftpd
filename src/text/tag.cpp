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

#include <cctype>
#include <boost/format.hpp>
#include <boost/regex.hpp>
#include "text/tag.hpp"
#include "text/error.hpp"
#include "logs/logs.hpp"
#include "util/titlecase.hpp"

namespace text
{

namespace 
{

boost::regex formatPattern("\\d*(?:\\.\\d+)?");

class comma_numpunct : public std::numpunct<char>
{
  protected:
    virtual char do_thousands_sep() const { return ','; }
    virtual std::string do_grouping() const { return "\03"; }
};

std::locale prettyLocale(std::locale(), new comma_numpunct());

}

Tag::Tag(const std::string& name) : 
  name(util::ToLowerCopy(name)),
  value(""),
  alignment(Alignment::Right), 
  unitConv(UnitConversion::Kbyte), 
  caseConv(CaseConversion::None),
  pretty(false)
{ }

void Tag::SetFilter(std::string filter)
{
  util::ToLower(filter);

  if (filter == "left")
    alignment = Alignment::Left;

  else if (filter == "right")
    alignment = Alignment::Right;

  else if (filter == "center")
    alignment = Alignment::Center;

  else if (filter == "pretty")
    pretty = true;

  else if (filter == "kb")
    unitConv = UnitConversion::Kbyte;
    
  else if (filter == "mb")
    unitConv = UnitConversion::Mbyte;
    
  else if (filter == "gb")
    unitConv = UnitConversion::Gbyte;
    
  else if (filter == "auto")
    unitConv = UnitConversion::Auto;

  else if (filter == "upper")
    caseConv = CaseConversion::Upper;
    
  else if (filter == "lower")
    caseConv = CaseConversion::Lower;
    
  else if (filter == "title")
    caseConv = CaseConversion::Title;
    
  else if (boost::regex_match(filter, formatPattern))
    format = filter;

  else
  {
    throw TemplateFilterMalform("Invalid filter: " + filter);
  }
}

void Tag::CaseConvert(std::string& s) const
{
  switch (caseConv)
  {
    case CaseConversion::None   :
      break;
    case CaseConversion::Upper  :
      util::ToUpper(s);
      break;
    case CaseConversion::Lower  :
      util::ToLower(s);
      break;
    case CaseConversion::Title  :
      s = util::TitleCase(s);
      break;
  }
}

template <typename T>
std::string Tag::Format(const std::string& format, const T& value) const
{
  if (pretty)
    return (boost::format("%" + AlignmentStr() + format, prettyLocale) % value).str();
  else
    return (boost::format("%" + AlignmentStr() + format) % value).str();
}

std::string Tag::CompileString() const
{
  std::string value;
  try
  {
    value = boost::get<std::string>(this->value);
  }
  catch (const boost::bad_get&)
  {
    // no value registered for tag, continue with empty string
  }
  return Format(format + "s", value);
}

std::string Tag::CompileInteger() const
{
  return Format(format + "li", boost::get<long long>(this->value));
}

std::string Tag::CompileDouble() const
{
  double value = boost::get<double>(this->value);
  std::string unitStr;
  switch (unitConv)
  {
    case UnitConversion::Auto   :
    {
      bool negative = false;
      if (value < 0)
      {
        negative = true;
        value = -value;
      }
      
      if (value < 1024.0)
      {
        unitStr = "KB";
      }
      else if (value < 1024.0 * 1024.0)
      {
        value /= 1024;
        unitStr = "MB";
      }
      else
      {
        value /= 1024 * 1024;
        unitStr = "GB";
      }
      
      if (negative)
      {
        value = -value;
      }
      break;
    }
    case UnitConversion::Gbyte  :
      value /= 1024;
    case UnitConversion::Mbyte  :
      value /= 1024;
    case UnitConversion::Kbyte  :
      break;
  }

  std::string format(this->format.empty() ? ".2" : this->format);
  return Format(format + "f", value) + unitStr;
}

std::string Tag::Compile() const
{ 
  std::string compiled;
  switch (value.which())
  {
    case 0  :
    {
      compiled = CompileString();
      break;
    }
    case 1  :
    {
      compiled = CompileInteger();
      break;
    }
    case 2  :
    {
      compiled = CompileDouble();
      break;
    }
  }
  
  CaseConvert(compiled);
  return compiled;
}

}
