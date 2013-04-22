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

#ifndef __ACL_PASSWDSTRENGTH_HPP
#define __ACL_PASSWDSTRENGTH_HPP

#include <string>
#include <cctype>
#include <sstream>

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
  
  std::string String() const
  {
    std::ostringstream os;
    os << UpperCase() << " uppercase, "
       << LowerCase() << " lowercase, "
       << Digits() << " digits, "
       << Others() << " others, "
       << Length() << " length";
    return os.str();
  }
};

} /* acl namespace */

#endif
