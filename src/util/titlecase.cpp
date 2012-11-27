#include <algorithm>
#include <string>
#include <vector>
#include <boost/regex.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/classification.hpp>

/*
 
 This TitleCase is a port of the below Perl and Python scripts.
 
 Original Perl version by: John Gruber http://daringfireball.net/ 10 May 2008
 Python version by Stuart Colville http://muffinresearch.co.uk
 License: http://www.opensource.org/licenses/mit-license.php
 
 */

namespace util { namespace string
{

namespace
{
const std::string small("a|an|and|as|at|but|by|en|for|if|in|of|on|or|the|to|v\\.?|via|vs\\.?");
const std::string punct("!\"#$%&'‘()*+,\\-./:;?@[\\\\\\]_`{|}~");

boost::regex smallWords("^" + small + "$", boost::regex::icase);
boost::regex inlinePeriod("[a-z][.][a-z]", boost::regex::icase);
boost::regex ucElsewhere("[" + punct + "]*?[a-zA-Z][A-Z][a-zA-Z]*");
boost::regex capFirst("^[" + punct + "]*?([A-Za-z])");
boost::regex smallFirst("^([" + punct + "]*)(" + small +")\\b", boost::regex::icase);
boost::regex smallLast("\\b(" + small + ")[" + punct + "]?$", boost::regex::icase);
boost::regex subPhrase("([:.;?!][ ]['\"]?)(" + small + ")");
boost::regex aposSecond("^[dol]{1}['‘]{1}[a-z]+$", boost::regex::icase);
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
      boost::to_lower(word);
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
      boost::split(slashed, word, boost::is_any_of("/"));
      for (std::string& subWord : slashed)
        subWord[0] = std::toupper(subWord[0]);
      word = boost::join(slashed, "/");
      continue;
    }
    
    std::vector<std::string> hyphenated;
    boost::split(hyphenated, word, boost::is_any_of("-"));
    for (std::string& subWord : hyphenated)
      for (auto it = subWord.begin(); it != subWord.end(); ++it)
        if (std::isalnum(*it))
        {
          *it = std::toupper(*it);
          break;
        }
        
    word = boost::join(hyphenated, "-");
  }
  
  std::string result = boost::join(words, " ");
  
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

} /* string namespace */
} /* util namespace */

#ifdef UTIL_STRING_TITLECASE_TEST

const std::vector<std::string> tests =
{
  "word/word",
  "Word/Word",
  "dance with me/let’s face the music and dance",
  "Dance With Me/Let’s Face the Music and Dance",
  "34th 3rd 2nd",
  "34th 3rd 2nd",
  "Q&A with steve jobs: 'that's what happens in technology'",
  "Q&A With Steve Jobs: 'That's What Happens in Technology'",
  "What is AT&T's problem?",
  "What Is AT&T's Problem?",
  "Apple deal with AT&T falls through",
  "Apple Deal With AT&T Falls Through",
  "this v that",
  "This v That",
  "this v. that",
  "This v. That",
  "this vs that",
  "This vs That",
  "this vs. that",
  "This vs. That",
  "The SEC's Apple probe: what you need to know",
  "The SEC's Apple Probe: What You Need to Know",
  "'by the Way, small word at the start but within quotes.'",
  "'By the Way, Small Word at the Start but Within Quotes.'",
  "Small word at end is nothing to be afraid of",
  "Small Word at End Is Nothing to Be Afraid Of",
  "Starting Sub-Phrase With a Small Word: a Trick, Perhaps?",
  "Starting Sub-Phrase With a Small Word: A Trick, Perhaps?",
  "Sub-Phrase With a Small Word in Quotes: 'a Trick, Perhaps?'",
  "Sub-Phrase With a Small Word in Quotes: 'A Trick, Perhaps?'",
  "sub-phrase with a small word in quotes: \"a trick, perhaps?\"",
  "Sub-Phrase With a Small Word in Quotes: \"A Trick, Perhaps?\"",
  "\"Nothing to Be Afraid of?\"",
  "\"Nothing to Be Afraid Of?\"",
  "\"Nothing to be Afraid Of?\"",
  "\"Nothing to Be Afraid Of?\"",
  "a thing",
  "A Thing",
  "2lmc Spool: 'gruber on OmniFocus and vapo(u)rware'",
  "2lmc Spool: 'Gruber on OmniFocus and Vapo(u)rware'",
  "this is just an example.com",
  "This Is Just an example.com",
  "this is something listed on del.icio.us",
  "This Is Something Listed on del.icio.us",
  "iTunes should be unmolested",
  "iTunes Should Be Unmolested",
  "reading between the lines of steve jobs’s ‘thoughts on music’",
  "Reading Between the Lines of Steve Jobs’s ‘Thoughts on Music’",
  "seriously, ‘repair permissions’ is voodoo",
  "Seriously, ‘Repair Permissions’ Is Voodoo",
  "generalissimo francisco franco: still dead; kieren McCarthy: still a jackass",
  "Generalissimo Francisco Franco: Still Dead; Kieren McCarthy: Still a Jackass",
  "O'Reilly should be untouched",
  "O'Reilly Should Be Untouched",
  "my name is o'reilly",
  "My Name Is O'Reilly",
  "WASHINGTON, D.C. SHOULD BE FIXED BUT MIGHT BE A PROBLEM",
  "Washington, D.C. Should Be Fixed but Might Be a Problem",
  "THIS IS ALL CAPS AND SHOULD BE ADDRESSED",
  "This Is All Caps and Should Be Addressed",
  "Mr McTavish went to MacDonalds",
  "Mr McTavish Went to MacDonalds",
  "this is http://foo.com",
  "This Is http://foo.com"
};

int main()
{
  unsigned failed = 0;

  for (auto it = tests.begin(); it != tests.end(); it += 2)
  {
    const std::string& original = *it;
    const std::string& expected = *(it + 1);
    std::string result = util::string::TitleCase(original);
    if (result != expected)
    {
      ++failed;
      std::cout << "FAILED - original: " << original << "\n"
                << "         expected: " << expected << "\n"
                << "         result  : " << result << std::endl;
    }
    else
      std::cout << "original: " << original << "\n"
                << "result  : " << result << std::endl;
  }
  
  if (!failed) std::cout << "All tests passed" << std::endl;
}

#endif
