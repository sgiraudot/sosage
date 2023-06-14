/*
  [src/Sosage/Utils/datetime.cpp]
  Useful date/time tools

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

#include <Sosage/Utils/datetime.h>

#include <ctime>

namespace Sosage
{

std::string time_to_string (double time)
{
  int seconds = int(time);

  auto tts = [](int val) -> std::string
  { return (val < 10 ? "0" : "") + std::to_string(val); };

  return tts(seconds / 3600) + ":"
      + tts((seconds / 60) % 60);
}

std::string date_to_string (int date)
{
  std::time_t time = date;
  std::tm* tm = std::localtime(&time);

  char timeString[std::size("yyyy-mm-dd, hh:mm")];
  std::strftime(std::data(timeString), std::size(timeString), "%F, %R", tm);
  return std::string(timeString);
}


} // namespace Sosage
