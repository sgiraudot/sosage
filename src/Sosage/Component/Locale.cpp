/*
  [src/Sosage/Component/Locale.cpp]
  Handle translations of text.

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

#include <Sosage/Component/Locale.h>
#include <Sosage/Utils/error.h>

namespace Sosage::Component
{

Locale::Locale (const std::string& entity, const std::string& component)
  : Base(entity, component)
{ }

void Locale::add (const std::string& line, const std::string& translation)
{
  m_dict.insert (std::make_pair(line, translation));
}

const std::string& Locale::get (const std::string& line)
{
  auto iter = m_dict.find (line);
  check (iter != m_dict.end(), "Line " + line + " not found in locale " + str());
  return iter->second;
}

} // namespace Sosage::Component
