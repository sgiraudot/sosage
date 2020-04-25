#ifndef SOSAGE_VERSION_H
#define SOSAGE_VERSION_H

namespace Sosage
{

namespace version
{

constexpr unsigned int major = 0;
constexpr unsigned int minor = 1;
constexpr unsigned int patch = 0;
constexpr const char* name = "alpha1";

inline std::string str()
{
  return std::to_string(major) + "."
    + std::to_string(minor) + "."
    + std::to_string(patch) + "-"
    + name;
}

inline unsigned int get()
{
  return major * 10000 + minor * 100 + patch;
}

inline unsigned int parse (const std::string& vstring)
{
  std::size_t pos = vstring.find('.');
  std::string vmajor (vstring.begin(), vstring.begin() + pos);
  std::size_t pos2 = vstring.find('.', pos+1);
  std::string vminor (vstring.begin() + pos+1, vstring.begin() + pos2);
  std::string vpatch (vstring.begin() + pos2+1, vstring.end());
  return (unsigned int)(std::atoi(vmajor.c_str()) * 10000
                        + std::atoi(vminor.c_str()) * 100
                        + std::atoi(vpatch.c_str()));
}

}

}

#endif // SOSAGE_VERSION_H
