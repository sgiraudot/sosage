/*
  [include/Sosage/Config/version.h]
  Version of SOSAGE.

  =====================================================================

  This file is part of SOSAGE.

  SOSAGE is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  SOSAGE is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with SOSAGE.  If not, see <https://www.gnu.org/licenses/>.

  =====================================================================

  Author(s): Simon Giraudot <sosage@ptilouk.net>
*/

#ifndef SOSAGE_CONFIG_VERSION_H
#define SOSAGE_CONFIG_VERSION_H

#include <string>

namespace Sosage
{

namespace Version
{

constexpr unsigned int major = SOSAGE_VERSION_MAJOR;
constexpr unsigned int minor = SOSAGE_VERSION_MINOR;
constexpr unsigned int patch = SOSAGE_VERSION_PATCH;
constexpr const char* name = SOSAGE_VERSION_NAME;

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

#endif // SOSAGE_CONFIG_VERSION_H
