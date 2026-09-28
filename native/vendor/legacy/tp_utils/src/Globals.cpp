#include "tp_utils/Globals.h"

#ifdef TP_ENABLE_PRINT_FUNCTION_NAME
#include "tp_utils/DebugUtils.h"
#endif

#include <codecvt>
#include <locale>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>

#if defined(TP_WIN32) && (defined(_MSC_VER) && (_MSC_VER >= 1900))
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

//##################################################################################################
std::string tpToHex(const std::string& input)
{
  static const char* const lut = "0123456789ABCDEF";

  size_t len = input.length();
  const auto* s = reinterpret_cast<const uint8_t*>(input.data());
  const uint8_t* sMax = s + len;

  std::string output;
  output.reserve(2 * len);

  for(; s<sMax; s++)
  {
    const unsigned char c = *s;
    output.push_back(lut[c >> 4]);
    output.push_back(lut[c & 15]);
  }

  return output;
}

//##################################################################################################
std::string tpFromHEX(const std::string& input)
{
  static const char* const lut = "0123456789ABCDEF";
  size_t len = input.size();
  if(len & 1)
    return std::string();

  std::string output;
  output.resize(len / 2);

  auto* o = reinterpret_cast<uint8_t*>(&output[0]);

  for(size_t i = 0; i < len; i+=2, o++)
  {
    char a = input[i];
    const char* p = std::lower_bound(lut, lut + 16, a);
    if(*p != a)
      return std::string();

    char b = input[i + 1];
    const char* q = std::lower_bound(lut, lut + 16, b);
    if(*q != b)
      return std::string();

    (*o) = uint8_t(((p - lut) << 4) | (q - lut));
  }

  return output;
}

//##################################################################################################
bool tpStartsWith(const std::string& input, const std::string& s)
{
  return input.size() >= s.size() && std::equal(s.begin(), s.end(), input.begin());
}

//##################################################################################################
bool tpEndsWith(const std::string& input, const std::string& s)
{
  return input.size() >= s.size() && std::equal(s.begin(), s.end(), input.end()-int(s.size()));
}
//##################################################################################################
std::string tpToLower(const std::string& str)
{
  std::string out;
  out.resize(str.size());
  std::transform(str.begin(), str.end(), out.begin(), ::tolower);
  return out;
}

//##################################################################################################
std::string tpToUpper(const std::string& str)
{
  std::string out;
  out.resize(str.size());
  std::transform(str.begin(), str.end(), out.begin(), ::toupper);
  return out;
}

//##################################################################################################
std::string tpTrim(const std::string& str, const std::string& whitespace)
{
  const auto strBegin = str.find_first_not_of(whitespace);
  if(strBegin == std::string::npos)
    return "";

  const auto strEnd = str.find_last_not_of(whitespace);
  const auto strRange = strEnd - strBegin + 1;

  return str.substr(strBegin, strRange);
}

//##################################################################################################
bool tpContains(const std::string& input, const std::string& s)
{
  return(input.find(s) != std::string::npos);
}

//##################################################################################################
bool tpContains(const std::string& input, const char* s)
{
  return(input.find(s) != std::string::npos);
}

//##################################################################################################
void* tpVoidLiteral(size_t value)
{
  return reinterpret_cast<void*>(value);
}

//##################################################################################################
namespace
{
void addPart(std::vector<std::string>& result, const std::string& input, size_t pos, size_t n, TPSplitBehavior behavior)
{
  if(behavior==TPSplitBehavior::SkipEmptyParts && n==0)
    return;

  result.push_back(input.substr(pos, n));
}
}

//##################################################################################################
void tpSplit(std::vector<std::string>& result,
             const std::string& input,
             const std::string& del,
             TPSplitBehavior behavior)
{
  std::string::size_type start = 0;
  auto end = input.find(del);
  while (end != std::string::npos)
  {
    addPart(result, input, start, end - start, behavior);
    start = end + del.length();
    end = input.find(del, start);
  }

  addPart(result, input, start, input.size()-start, behavior);
}

//##################################################################################################
void tpSplit(std::vector<std::string>& result,
             const std::string& input,
             char del,
             TPSplitBehavior behavior)
{
  std::string::size_type start = 0;
  auto end = input.find(del);
  while (end != std::string::npos)
  {
    addPart(result, input, start, end - start, behavior);
    start = end+1;
    end = input.find(del, start);
  }

  addPart(result, input, start, input.size()-start, behavior);
}

//##################################################################################################
void tpRemoveChar(std::string& s, char c)
{
  s.erase(std::remove_if(s.begin(), s.end(), [c](int a){return a==c;}), s.end());
}

//##################################################################################################
std::string tpToUTF8(const std::u16string& source)
{
#if _MSC_VER >= 1900
  auto p = reinterpret_cast<const int16_t *>(source.data());
  return std::wstring_convert<std::codecvt_utf8_utf16<int16_t>, int16_t>().to_bytes(p, p + source.size());
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>().to_bytes(source);
#pragma GCC diagnostic pop
#endif
}

//##################################################################################################
std::string tpToUTF8(const std::wstring& source)
{
#if _MSC_VER >= 1900
  auto p = reinterpret_cast<const wchar_t *>(source.data());
  return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>().to_bytes(p, p + source.size());
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>().to_bytes(source);
#pragma GCC diagnostic pop
#endif
}

//##################################################################################################
std::u16string tpFromUTF8(const std::string& source)
{
#if defined(TP_WIN32) && (defined(_MSC_VER) && (_MSC_VER >= 1900))
  int len = MultiByteToWideChar(CP_UTF8, 0, source.c_str(), -1, nullptr, 0);
  if(len == 0)
    return {};

  std::wstring wstr(size_t(len), 0);
  int result = MultiByteToWideChar(CP_UTF8, 0, source.c_str(), -1, &wstr[0], len);
  if(result == 0)
    return {};

  wstr.resize(source.size());
  return std::u16string(reinterpret_cast<const char16_t*>(wstr.c_str()));
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  std::wstring_convert<std::codecvt_utf8_utf16<char16_t>,char16_t> cvt;
  std::u16string result = cvt.from_bytes(source);
  return result;
#pragma GCC diagnostic pop
#endif
}

//##################################################################################################
std::wstring tpWStringFromUTF8(const std::string& source)
{
#if _MSC_VER >= 1900
  TP_UNUSED(source);
  return std::wstring();
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>().from_bytes(source);
#pragma GCC diagnostic pop
#endif
}

namespace tp_utils
{

namespace detail
{
//##################################################################################################
int& staticInit()
{
  static int staticInit=0;
  return staticInit;
}
}

//##################################################################################################
int staticInit()
{
  return detail::staticInit();
}

//##################################################################################################
void leftJustified(std::string& text, size_t maxLength, char padding)
{
  if(text.size() >= maxLength)
    return;

  text+=std::string(maxLength-text.size(), padding);
}

//##################################################################################################
void rightJustified(std::string& text, size_t maxLength, char padding)
{
  if(text.size() >= maxLength)
    return;

  text = std::string(maxLength-text.size(), padding)+text;
}

//##################################################################################################
std::string fixedWidthKeepRight(std::string data, size_t len, char pad)
{
  if(data.size()>=len)
    return data.substr(data.size()-len);

  size_t missing = size_t(len) - size_t(data.size());
  if(missing>0)
    data = std::string(missing, pad) + data;

  return data;
}

//##################################################################################################
std::string fixedWidthKeepLeft(std::string data, size_t len, char pad)
{
  if(data.size()>=len)
    return data.substr(0, len);

  size_t missing = size_t(len) - size_t(data.size());
  if(missing>0)
    data += std::string(missing, pad);


  return data;
}

//##################################################################################################
void replace(std::string& result, const std::string& key, const std::string& value)
{
  size_t pos = result.find(key);
  while(pos != std::string::npos)
  {
    result.replace(pos, key.size(), value);
    pos = result.find(key, pos + value.size());
  }
}

//##################################################################################################
std::string replaced(std::string input, const std::string& key, const std::string& value)
{
  replace(input, key, value);
  return input;
}

//##################################################################################################
std::string formatFloat(float value, int precision)
{
  std::stringstream stream;
  stream << std::fixed << std::setprecision(precision) << value;
  return stream.str();
}

//##################################################################################################
std::string formatDouble(double value, int precision)
{
  std::stringstream stream;
  stream << std::fixed << std::setprecision(precision) << value;
  return stream.str();
}

//##################################################################################################
std::string formatSizeBytes(int64_t sizeInBytes, bool alignRight)
{
  static const std::vector<const char*> suffixes{"  B", " kB", " MB", " GB", " TB", " PB", " EB", " ZB", " YB"};

  size_t i=0;
  double s=double(sizeInBytes);
  while(std::abs(s)>4000 && i<suffixes.size()-1)
  {
    i++;
    s = s / 1000.0;
  }

  std::string result;
  if(i == 0)
    result = std::to_string(sizeInBytes)+suffixes.at(i);

  else
  {
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2) << s;
    return stream.str() + suffixes.at(i);
  }

  if(alignRight)
    rightJustified(result, 8, ' ');

  return result;
}

//##################################################################################################
std::string formatSizeBinaryBytes(int64_t sizeInBytes, bool alignRight)
{
  static const std::vector<const char*> suffixes{"   B", " KiB", " MiB", " GiB", " TiB", " PiB", " EiB", " ZiB", " YiB"};

  size_t i=0;
  double s=double(sizeInBytes);
  while(std::fabs(s)>4096 && i<suffixes.size()-1)
  {
    i++;
    s = s / 1024.0;
  }

  std::string result;
  if(i == 0)
    result = std::to_string(sizeInBytes)+suffixes.at(i);

  else
  {
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2) << s;
    result = stream.str() + suffixes.at(i);
  }

  if(alignRight)
    rightJustified(result, 12, ' ');

  return result;
}

//##################################################################################################
bool parseColor(const std::string& color, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a)
{
  r =   0;
  g =   0;
  b =   0;
  a = 255;

  if(color.size() != 7)
    return false;

  if(color.at(0)!='#')
    return false;

  uint32_t acc = 0;
  for(size_t i=1; i<7; i++)
  {
    char c = color.at(i);

    uint32_t v=0;
    if(c>='0' && c<='9')
      v = uint32_t(c-'0');

    if(c>='A' && c<='F')
      v = uint32_t(10 + (c-'A'));

    if(c>='a' && c<='f')
      v = uint32_t(10 + (c-'a'));

    acc<<=4;
    acc|=v;
  }

  b = uint8_t(acc); acc>>=8;
  g = uint8_t(acc); acc>>=8;
  r = uint8_t(acc);
  return true;
}

//##################################################################################################
bool parseColorF(const std::string& color, float& r, float& g, float& b, float& a)
{
  uint8_t r_;
  uint8_t g_;
  uint8_t b_;
  uint8_t a_;
  bool ret = parseColor(color, r_, g_, b_, a_);
  r = float(r_) / 255.0f;
  g = float(g_) / 255.0f;
  b = float(b_) / 255.0f;
  a = float(a_) / 255.0f;
  return ret;
}

}

#ifdef TP_ENABLE_PRINT_FUNCTION_NAME
namespace detail
{
bool tpPrintFunctionNames{false};
}

//##################################################################################################
void PRINT_FUNCTION_NAME(const std::string& name)
{
  if(detail::tpPrintFunctionNames)
    tpWarning() << name;
}
#endif
