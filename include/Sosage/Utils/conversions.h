/*
  [include/Sosage/Utils/conversions.h]
  Convert strings to int, double, bool.

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

#ifndef SOSAGE_UTILS_CONVERSIONS_H
#define SOSAGE_UTILS_CONVERSIONS_H

#include <array>
#include <sstream>
#include <string>
#include <utility>

namespace Sosage
{

inline int to_int (const std::string& str)
{
  return std::atoi (str.c_str());
}

inline double to_double (const std::string& str)
{
  return std::atof (str.c_str());
}

inline bool  to_bool (const std::string& str)
{
  return (str == "true");
}

template <typename T>
inline std::string to_string (const T& t)
{
  return std::to_string(t);
}

inline const std::string& to_string (const std::string& str)
{
  return str;
}

inline std::string to_string (const char* str)
{
  return std::string(str);
}

inline std::string to_string (const bool& t)
{
  return (t ? "true" : "false");
}

template <typename T, typename ... Ts>
inline std::string to_string (const T& t, const Ts& ... ts)
{
  return to_string(t) + to_string(std::forward<const Ts>(ts)...);
}

} // namespace Sosage

#endif // SOSAGE_UTILS_CONVERSIONS_H
