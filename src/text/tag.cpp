#include <cctype>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
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
  name(boost::to_lower_copy(name)),
  value(""),
  alignment(Alignment::Right), 
  unitConv(UnitConversion::Kbyte), 
  caseConv(CaseConversion::None),
  pretty(false)
{ }

void Tag::SetFilter(std::string filter)
{
  boost::to_lower(filter);

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
      boost::to_upper(s);
      break;
    case CaseConversion::Lower  :
      boost::to_lower(s);
      break;
    case CaseConversion::Title  :
      s = util::string::TitleCase(s);
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
