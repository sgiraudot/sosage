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

#include <algorithm>
#include <string>
#include <vector>

namespace Sosage
{

bool is_int (const std::string& str);
int to_int (const std::string& str);
double to_double (const std::string& str);
bool to_bool (const std::string& str);

template <typename T>
inline std::string to_string (const T& t)
{
  return std::to_string(t);
}

const std::string& to_string (const std::string& str);
std::string to_string (const char* str);
std::string to_string (const bool& t);

template <typename T, typename ... Ts>
inline std::string to_string (const T& t, const Ts& ... ts)
{
  return to_string(t) + to_string(std::forward<const Ts>(ts)...);
}

template <typename Set, typename T>
bool contains (const Set& set, const T& t)
{
  return (set.find(t) != set.end());
}

template <typename T>
bool contains (const std::vector<T>& vec, const T& t)
{
  return std::find (vec.begin(), vec.end(), t) != vec.end();
}

bool contains (const std::string& str, const char* sub);
bool contains (const std::string& str, const std::string& sub);
bool startswith (const std::string& str, const std::string& sub);
bool endswith (const std::string& str, const std::string& sub);

int random_int (int min, int max);
double random_double (double min = 0., double max = 1.);
bool random_chance(double chance);

template <typename T>
typename T::value_type random_choice (const T& list)
{
  double accu = 0.;
  double rnd = rand() / double(RAND_MAX);
  for (const auto& p : list)
  {
    accu += 1. / double(list.size());
    if (accu > rnd)
      return p;
  }
  return *(list.end() - 1);
}

template <typename T>
T random_choice (const std::initializer_list<T>& list)
{
  return random_choice<std::initializer_list<T>> (list);
}

void random_do (const std::initializer_list<std::function<void(void)>>& list);

} // namespace Sosage

#endif // SOSAGE_UTILS_CONVERSIONS_H
