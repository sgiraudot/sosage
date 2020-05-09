/*
  [src/Sosage/Content.cpp]
  Stores and handles access to all variable game content (components).

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

#include <Sosage/Content.h>

namespace Sosage
{

Content::Content()
  : m_data ()
{

}

void Content::remove (const std::string& key, bool optional)
{
  Component::Handle_set::iterator iter = m_data.find(std::make_shared<Component::Base>(key));
  if (optional && iter == m_data.end())
    return;
  
  check (iter != m_data.end(), "Entity " + key + " doesn't exist");
  m_data.erase(iter);
}


} // namespace Sosage
