/*
  [src/Sosage/Component/Console.cpp]
  Virtual GUI console to display debug info ingame.

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

#include <Sosage/Component/Console.h>

#include <algorithm>

namespace Sosage::Component
{

Console::Console (const std::string& id)
  : Boolean(id, false)
{
  m_content << "SOSAGE CONSOLE" << std::endl;
}

std::string Console::console_str()
{
  std::string content = m_content.str();

  std::size_t nb_lines = std::count (content.begin(), content.end(), '\n');

  std::size_t start = 0;
  while (nb_lines > 20)
  {
    start = content.find ('\n', start + 1);
    -- nb_lines;
  }

  std::string out = "";

  std::size_t begin = start;
  while (begin != content.size())
  {
    std::size_t end = content.find ('\n', begin + 1);
    if (end == std::string::npos)
      end = content.size();

    out += "> " + std::string (content.begin() + begin + 1, content.begin() + end) + "\n";

    begin = end;
  }
  
  m_content = std::stringstream();
  m_content << std::string (content.begin() + start, content.end());

  return out;
}

} // namespace Sosage::Component
