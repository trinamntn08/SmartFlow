#ifndef tp_utils_TPPixel_h
#define tp_utils_TPPixel_h

#include "tp_utils/Globals.h" // IWYU pragma: keep

#include "tp_utils/JSONUtils.h"

union TPPixel
{
  //avoid initialization by default (will speed up to create pixel arrays for output)
  TPPixel() = default;

  TPPixel(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_=255):
    r(r_),
    g(g_),
    b(b_),
    a(a_)
  {

  }

  explicit TPPixel(uint32_t value):
    i(value)
  {

  }

  explicit TPPixel(const std::string& color)
  {
    r =   0;
    g =   0;
    b =   0;
    a = 255;

    if(color.size() != 7)
      return;

    if(color.at(0)!='#')
      return;

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
  }

  std::string toString() const
  {
    uint32_t acc = 0;
    acc|=r; acc<<=8;
    acc|=g; acc<<=8;
    acc|=b;

    std::string result = "#000000";

    for(size_t i=6; i>0; i--)
    {
      int v = acc&0xF; acc>>=4;
      result[i] = char(v+(v>9?('A'-10):'0'));
    }

    return result;
  }

  bool bestContrastIsBlack() const
  {
    auto l=[](uint8_t cc)
    {
      auto c = float(cc)/255.0f;
      if(c <= 0.03928f)
        return c/12.92f;
      else
        return std::pow(((c+0.055f)/1.055f), 2.4f);
    };

    auto r = l(this->r);
    auto g = l(this->g);
    auto b = l(this->b);

    return ((0.2126f*r) + (0.7152f*g) + (0.0722f*b)) > 0.179f;
  }

  template<typename T>
  T toFloat3() const
  {
    return {float(r)/255.0f, float(g)/255.0f, float(b)/255.0f};
  }

  template<typename T>
  T toFloat4() const
  {
    return {float(r)/255.0f, float(g)/255.0f, float(b)/255.0f, float(a)/255.0f};
  }

  template<typename T>
  static TPPixel fromFloat3(const T& v)
  {
    auto conv = [](float f){return uint8_t(std::clamp(int((f*255.0f)+0.5f), 0, 255));};
    return {conv(v[0]), conv(v[1]), conv(v[2])};
  }

  template<typename T>
  static TPPixel fromFloat4(const T& v)
  {
    auto conv = [](float f){return uint8_t(std::clamp(int((f*255.0f)+0.5f), 0, 255));};
    return {conv(v[0]), conv(v[1]), conv(v[2]), conv(v[3])};
  }

  template<typename T>
  T to() const
  {
    return {r, g, b, a};
  }

  void saveState(tp_utils::JSON& j) const
  {
    j = tp_utils::JSON::array({r, g, b, a});
  }

  void saveStateRGB(tp_utils::JSON& j) const
  {
    j = tp_utils::JSON::array({r, g, b});
  }

  void loadState(const tp_utils::JSON& j, const std::string& key)
  {
    auto itr=j.find(key);
    if(itr!=j.end())
      loadState(*itr);
    else
      i = 0;
  }

  void loadState(const tp_utils::JSON& j)
  {
    if(j.is_string())
      *this = TPPixel(j.get<std::string>());

    else if(j.is_array() && (j.size()==3 || j.size()==4))
    {
      for(size_t i=0; i<3; i++)
      {
        if(auto& ii = j[i]; ii.is_number_integer())
          v[i] = uint8_t(ii);
        else
          v[i] = 0;
      }

      if(j.size()==4)
      {
        if(auto& ii = j[3]; ii.is_number_integer())
          v[3] = uint8_t(ii);
        else
          v[3] = 255;
      }
    }
    else
      *this = TPPixel();
  }

  uint32_t i;
  uint8_t v[4];
#ifndef TP_WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
  struct
  {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
  };
#ifndef TP_WIN32
#pragma GCC diagnostic pop
#endif
};

//##################################################################################################
inline bool operator== (const TPPixel& a, const TPPixel& b)
{
  return a.i == b.i;
}

//##################################################################################################
inline bool operator!= (const TPPixel& a, const TPPixel& b)
{
  return a.i != b.i;
}

#endif
