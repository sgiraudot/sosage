#ifndef SOSAGE_VERSION_H
#define SOSAGE_VERSION_H

#define SOSAGE_VERSION 000001

#define SOSAGE_VERSION_MAJOR (SOSAGE_VERSION / 10000)
#define SOSAGE_VERSION_MINOR (SOSAGE_VERSION / 100 % 100)
#define SOSAGE_VERSION_PATCH (SOSAGE_VERSION % 100)

namespace Sosage
{

inline std::string version()
{
  return std::to_string(SOSAGE_VERSION_MAJOR) + "."
    + std::to_string(SOSAGE_VERSION_MINOR) + "."
    + std::to_string(SOSAGE_VERSION_PATCH);
}

inline int version (const std::string& vstring)
{
  std::size_t pos = vstring.find('.');
  std::string vmajor (vstring.begin(), vstring.begin() + pos);
  std::size_t pos2 = vstring.find('.', pos+1);
  std::string vminor (vstring.begin() + pos+1, vstring.begin() + pos2);
  std::string vpatch (vstring.begin() + pos2+1, vstring.end());
  return std::atoi(vmajor.c_str()) * 10000
    + std::atoi(vminor.c_str()) * 100
    + std::atoi(vpatch.c_str());
}


}

#endif // SOSAGE_VERSION_H
