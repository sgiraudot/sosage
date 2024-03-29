/*
  [src/Sosage/Utils/conversions.cpp]
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

#include <Sosage/Utils/conversions.h>

namespace Sosage
{

bool is_int (const std::string& str)
{
  for (const char& c : str)
    if (!std::isdigit(c))
      return false;
  return true;
}

bool is_relative (const std::string& str)
{
  return str[0] == '+' || str[0] == '-';
}

int to_int (const std::string& str)
{
  return std::atoi (str.c_str());
}

double to_double (const std::string& str)
{
  return std::atof (str.c_str());
}

bool  to_bool (const std::string& str)
{
  return (str == "true");
}

const std::string& to_string (const std::string& str)
{
  return str;
}

std::string to_string (const char* str)
{
  return std::string(str);
}

std::string to_string (const bool& t)
{
  return (t ? "true" : "false");
}

void capitalize (std::string& str)
{
  if (startswith(str, "é"))
  {
    str = "É" + std::string(str.begin() + std::string("é").size(), str.end());
  }
  else
    str[0] = toupper(str[0]);
}

bool contains (const std::initializer_list<const char*>& list, const std::string& str)
{
  for (const char* s : list)
    if (std::string(s) == str)
      return true;
  return false;
}

bool contains (const std::string& str, const char* sub)
{
  return str.find(sub) != std::string::npos;
}

bool contains (const std::string& str, const std::string& sub)
{
  return str.find(sub) != std::string::npos;
}

bool startswith (const std::string& str, const std::string& sub)
{
  if (str.size() < sub.size())
    return false;
  return (str.compare (0, sub.size(), sub) == 0);
}

bool endswith (const std::string& str, const std::string& sub)
{
  if (str.size() < sub.size())
    return false;
  return (str.compare (str.size() - sub.size(), sub.size(), sub) == 0);
}

int random_int (int min, int max)
{
  return min + (rand() % (max - min));
}

double random_double (double min, double max)
{
  return min + (max - min) * (rand() / double(RAND_MAX));
}

bool random_chance (double chance)
{
  return (rand() / double(RAND_MAX)) < chance;
}

void random_do (const std::initializer_list<std::function<void(void)>>& list)
{
  random_choice(list)();
}

} // namespace Sosage
