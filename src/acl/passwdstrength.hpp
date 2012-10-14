#ifndef __ACL_PASSWDSTRENGTH_HPP
#define __ACL_PASSWDSTRENGTH_HPP

#include <string>
#include <cctype>

namespace acl
{

class PasswdStrength
{
  unsigned upperCase;
  unsigned lowerCase;
  unsigned length;
  unsigned digits;
  unsigned others;
  
public:
  PasswdStrength() :
    upperCase(0), lowerCase(0), length(0), 
    digits(0), others(0) { }
    
  PasswdStrength(const std::string& password) :
    upperCase(0), lowerCase(0), length(password.length()), 
    digits(0), others(0)
  {
    for (auto ch : password)
    {
      if (std::isdigit(ch)) ++digits;
      else if (std::isalpha(ch))
      {
        if (std::isupper(ch)) ++upperCase;
        else ++lowerCase;
      }
      else if (ch != '.') ++others;
    }
  }
  
  
  bool Allowed(const PasswdStrength& userStrength) const
  {
    return userStrength.length >= length &&
           userStrength.upperCase >= upperCase &&
           userStrength.lowerCase >= lowerCase &&
           userStrength.digits >= digits &&
           userStrength.others >= others;
  }

  unsigned Length() const { return length; }
  unsigned UpperCase() const { return upperCase; }
  unsigned LowerCase() const { return lowerCase; }
  unsigned Digits() const { return digits; }
  unsigned Others() const { return others; }
};

} /* acl namespace */

#endif
