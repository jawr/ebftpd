#ifndef __TEXT_TAG_HPP
#define __TEXT_TAG_HPP

#include <string>
#include <vector>

namespace text
{

enum class TagType
{
  String,
  Float,
  Decimal
};

enum class Alignment
{
  Left,
  Right,
  Center
};

enum class Measurement
{
  Kbyte,
  Mbyte,
  Gbyte,
  Auto,
  None
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
  std::string name;
  std::string value;
  std::string width;
  std::string precision;
  Alignment alignment;
  Measurement measurement;
  CaseConversion caseConv;
  std::string format;
  TagType type;
  bool pretty;
public:
  // oportuinity to set defaults from cofig here
  Tag(const std::string& name);

  template <typename T> std::string Format(T value);
  void Register(const std::string& filter);
  void Compile();
  
  void Parse(std::string value);
  void ParseSize(long long kBytes);
  void ParseSpeed(double speed);

  const std::string& Name() const { return name; }
  const std::string& Format() const { return format; }
  const std::string& Value() const { return value; }
  const Measurement& Unit() const { return measurement; } // rename all to unit?
 
  void SetType(TagType type) { this->type = type; }
};

// end
}
#endif
