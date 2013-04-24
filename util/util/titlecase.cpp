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

#include <algorithm>
#include <vector>
#include <boost/regex.hpp>
#include "util/titlecase.hpp"
#include "util/string.hpp"

/*
 
 This TitleCase is a port of the below Perl and Python scripts.
 
 Original Perl version by: John Gruber http://daringfireball.net/ 10 May 2008
 Python version by Stuart Colville http://muffinresearch.co.uk
 License: http://www.opensource.org/licenses/mit-license.php
 
 */

namespace util
{

namespace
{
const std::string small("a|an|and|as|at|but|by|en|for|if|in|of|on|or|the|to|v\\.?|via|vs\\.?");
const std::string punct("!\"#$%&'\x91()*+,\\-./:;?@[\\\\\\]_`{|}~");

boost::regex smallWords("^" + small + "$", boost::regex::icase);
boost::regex inlinePeriod("[a-z][.][a-z]", boost::regex::icase);
boost::regex ucElsewhere("[" + punct + "]*?[a-zA-Z][A-Z][a-zA-Z]*");
boost::regex capFirst("^[" + punct + "]*?([A-Za-z])");
boost::regex smallFirst("^([" + punct + "]*)(" + small +")\\b", boost::regex::icase);
boost::regex smallLast("\\b(" + small + ")[" + punct + "]?$", boost::regex::icase);
boost::regex subPhrase("([:.;?!][ ]['\"]?)(" + small + ")");
boost::regex aposSecond("^[dol]{1}['\x91]{1}[a-z]+$", boost::regex::icase);
boost::regex allCaps("^[A-Z\\s" + punct + "]+$");
boost::regex ucInitials("^(?:[A-Z]{1}\\.{1}|[A-Z]{1}\\.{1}[A-Z]{1})+$");
boost::regex macName("^([Mm]a?c)(\\w+)");
boost::regex wordSep("[ \t]");

}

std::string TitleCase(const std::string& s)
{
  std::vector<std::string> words;
  
  {
    boost::sregex_token_iterator first(s.begin(), s.end(), wordSep, -1);
    boost::sregex_token_iterator last;  
    std::copy(first, last, std::back_inserter(words));
  }
  
  bool isAllCaps = boost::regex_match(s, allCaps);
  
  for (std::string& word : words)
  {
    if (isAllCaps)
    {
      if (boost::regex_match(word, ucInitials)) continue;
      util::ToLower(word);
    }
    
    if (boost::regex_match(word, aposSecond))
    {
      word[0] = std::toupper(word[0]);
      word[2] = std::toupper(word[2]);
      continue;
    }
    
    if (boost::regex_search(word, inlinePeriod) ||
        boost::regex_match(word, ucElsewhere))
      continue;
      
    if (boost::regex_match(word, smallWords)) continue;
    
    boost::smatch match;
    if (boost::regex_match(word, match, macName))
    {
      word[0] = std::toupper(word[0]);
      char& ch = word[match[1].length()];
      ch = std::toupper(ch);
      continue;
    }
    
    if (word.find("/") != std::string::npos &&
        word.find("//") == std::string::npos)
    {
      std::vector<std::string> slashed;
      util::Split(slashed, word, "/");
      for (std::string& subWord : slashed)
        subWord[0] = std::toupper(subWord[0]);
      word = util::Join(slashed, "/");
      continue;
    }
    
    std::vector<std::string> hyphenated;
    util::Split(hyphenated, word, "-");
    for (std::string& subWord : hyphenated)
      for (auto it = subWord.begin(); it != subWord.end(); ++it)
        if (std::isalnum(*it))
        {
          *it = std::toupper(*it);
          break;
        }
        
    word = util::Join(hyphenated, "-");
  }
  
  std::string result = util::Join(words, " ");
  
  boost::smatch match;
  if (boost::regex_search(result, match, smallFirst))
  {
    std::string small = match[2];
    small[0] = std::toupper(small[0]);
    result = regex_replace(result, smallFirst, match[1] + small);
  }
  
  if (boost::regex_search(result, match, smallLast))
  {
    std::string small = match[1];
    small[0] = std::toupper(small[0]);
    result = regex_replace(result, smallLast, small + match[2]);
  }
  
  while (boost::regex_search(result, match, subPhrase))
  {
    std::string small = match[2];
    small[0] = std::toupper(small[0]);
    result = regex_replace(result, subPhrase, match[1] + small, 
                           boost::format_first_only);
  }
  
  return result;
}

} /* util namespace */
