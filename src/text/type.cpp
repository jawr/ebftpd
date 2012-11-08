#include <cctype>
#include <boost/algorithm/string.hpp>
#include "text/type.hpp"
#include "text/error.hpp"
#include "logs/logs.hpp"

namespace text
{

void Type::Register(const std::string& filter)
{
  logs::debug << "Type::Register: " << filter << logs::endl;
  if (filter == "left")
    alignment = Alignment::Left;

  else if (filter == "right")
    alignment = Alignment::Right;

  else if (filter == "center")
    alignment == Alignment::Center;

  else if (filter == "kb")
    measurement = Measurement::Kbyte;

  else if (filter == "mb")
    measurement = Measurement::Mbyte;

  else if (filter == "gb")
    measurement = Measurement::Gbyte;

  else if (filter == "auto")
    measurement = Measurement::Auto;

  else
  {
    std::vector<std::string> args;
    boost::split(args, filter, boost::is_any_of(".")); 
    if (args.size() > 2) throw TemplateFilterMalform("Error parsing filter, should be '^\\d+(\\.\\d+)?$': " + filter);
    for (auto& c: args.front())
      if (!std::isdigit(c)) throw TemplateFilterMalform("Error parsing filter, should be an integer: " + args.front());
    width = args.front();
  
    if (args.size() == 2)
    {
      for (auto& c: args.back())
        if (!std::isdigit(c)) 
          throw TemplateFilterMalform("Error parsing filter, should be an integer: " + args.back());
      precision = args.back();
    }
  }
}

void Type::Compile()
{ 
  std::ostringstream os;
  os << "%";

  if (alignment == Alignment::Left)
    os << "-";
  else if (alignment == Alignment::Center)
    os << "=";

  if (!width.empty())
  {
    os << width;  
    if (!precision.empty()) os << "." << precision << "f";
  }
  format = os.str();
}

// end
}
