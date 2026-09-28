#include "tp_utils/BinaryUtils.h"

namespace tp_utils
{

//##################################################################################################
std::string binaryDebug(const std::string& data, size_t maxLen, size_t maxLenBinary)
{
  const char* c = data.c_str();

  bool truncated = false;
  bool isBinary = false;

  if(data.size()>=maxLen)
    truncated = true;

  size_t iMax = tpMin(data.size(), maxLen);

  std::string result;
  result.reserve(iMax+32);

  for(size_t i=0; i<iMax; i++)
  {
    if(*c>=32 && *c<=126)
      result += (*c);
    else
    {
      result += ' ';
      isBinary = true;

      if(maxLenBinary>0)
        iMax = tpMin(iMax, maxLenBinary);
    }

    c++;
  }

  if(!truncated && !isBinary)
    return result;

  if(truncated)
    result += "...";

  return "(" + std::to_string(data.size()) + ", \"" + result + "\")";
}

//##################################################################################################
std::string binaryNumericDebug(const std::string& data, size_t maxLen)
{
  const auto* c = reinterpret_cast<const uint8_t*>(data.c_str());

  int iMax = int(tpMin(size_t(data.size()), maxLen));

  std::string result;
  result.reserve(iMax+32);

  for(int i=0; i<iMax; i++)
  {
    result.append(std::to_string(int(*c)));
    result += ' ';
    c++;
  }

  return result;
}

namespace
{
char cleanChar(char ch)
{
  return (ch>=' ' && ch<='~')?ch:'_';
}
}

//##################################################################################################
std::string summaryBinaryDebug(const std::string& data)
{
  std::string result = std::to_string(data.size());

  result += " (";

  size_t iMax = data.size();
  if(iMax<30)
    for(size_t i=0; i<iMax; i++)
      result += cleanChar(data.at(i));
  else
  {
    for(size_t i=0; i<10; i++)
      result += cleanChar(data.at(i));

    result += " ~~~ ";

    for(size_t i=iMax-11; i<iMax; i++)
      result += cleanChar(data.at(i));
  }

  result += ")";

  return result;
}

}
