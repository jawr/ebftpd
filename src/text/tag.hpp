#ifndef __TEXT_TAG_HPP
#define __TEXT_TAG_HPP

#include <string>
#include <vector>
#include <boost/variant.hpp>

namespace text
{

enum class Alignment
{
  Left,
  Right,
  Center
};

enum class UnitConversion
{
  Kbyte,
  Mbyte,
  Gbyte,
  Auto
};

enum class CaseConversion
{
  None,
  Upper,
  Lower,
  Title
};

class Tag
{
private:
  std::string name;
  boost::variant<std::string, long long, double> value;
  Alignment alignment;
  UnitConversion unitConv;
  CaseConversion caseConv;
  std::string format;
  bool pretty;
  
  std::string AlignmentStr() const
  {
    switch (alignment)
    {
      case Alignment::Left    : return std::string("-");
      case Alignment::Center  : return std::string("=");
      case Alignment::Right   : break;
    }
    
    return std::string("");
 }
  
  void CaseConvert(std::string& s) const;
  template <typename T> std::string Format(const std::string& format, const T& value) const;
  
  std::string CompileString() const;
  std::string CompileInteger() const;
  std::string CompileDouble() const;
  
public:
  explicit Tag(const std::string& name);

  void SetFilter(std::string filter);
  std::string Compile() const;
  
  void RegisterValue(std::string value) { this->value = value; }
  void RegisterValue(long long value) { this->value = value; }
  void RegisterSize(long long kBytes) { this->value = static_cast<double>(kBytes); }
  void RegisterSpeed(double speed) { this->value = speed; }

  const std::string& Name() const { return name; }
};

// end
}
#endif
