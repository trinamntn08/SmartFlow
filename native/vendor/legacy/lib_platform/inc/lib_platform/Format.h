#pragma once

#include <sstream>
#include <string>
#include <locale>

namespace lib_platform
{

//##################################################################################################
// std::format C++20 short implementation
template<typename T>
void format_helper(std::ostringstream& oss, std::string_view& str, const T& value)
{
  size_t openBracket = str.find('{');
  if (openBracket == std::string::npos) { return; }
  size_t closeBracket = str.find('}', openBracket + 1);
  if (closeBracket == std::string::npos) { return; }
  oss << str.substr(0, openBracket) << value;
  str = str.substr(closeBracket + 1);
}

//##################################################################################################
template<typename... Targs>
std::string format(std::string_view str, Targs...args)
{
  std::ostringstream oss;
  (format_helper(oss, str, args),...);
  oss << str;
  return oss.str();
}

//##################################################################################################
// Fixed precision, locale independent alternative to std::to_string, mainly for floating point formatting
//! TODO: in c++20 we can use std::format instead
template<typename T>
inline std::string toString(const T value)
{
  std::ostringstream oss;
  oss.imbue(std::locale::classic());
  oss << std::fixed << value;
  return oss.str();
}

//##################################################################################################
// Same as the function above but with a reusable stringstream to reduce internal allocations
//! TODO: in c++20 we can use std::format instead
struct LocaleIndependentFormatter
{
  LocaleIndependentFormatter()
  {
    oss.imbue(std::locale::classic());
  }

  template<typename T>
  std::string toString(const T value)
  {
    oss.str("");
    oss << std::fixed << value;
    return oss.str();
  }

private:
  std::ostringstream oss;
};

}
