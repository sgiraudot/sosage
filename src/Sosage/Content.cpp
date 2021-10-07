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
#ifdef SOSAGE_LOG_CONTENT
  , m_log ("content.log")
#endif
{
  // For some components, we want to quickly access to the whole
  // list, so we separate them in specific Handle sets
  auto reserved_components
      = { "animation",
          "image",
          "label",
          "lookat",
          "path",
          "play_sound",
          "position",
          "set_visible",
          "set_hidden",
          "start_animation",
          "start_talking",
          "state",
          "stop_animation",
          "stop_talking",
          "stop_walking",
          "value",
          "visible" };

  m_data.resize(reserved_components.size() + 1);
  std::size_t idx = 1;
  for (const auto& c : reserved_components)
    m_map_component.insert (std::make_pair(c, idx ++));
}

void Content::remove (const std::string& key, bool optional)
{
  auto t = std::make_shared<Component::Base>(key);
  Component::Handle_set& hset = components(t->component());
  Component::Handle_set::iterator iter = hset.find(t);
  if (optional && iter == hset.end())
    return;
  
  check (iter != hset.end(), "Entity " + key + " doesn't exist");
  hset.erase(iter);
}


} // namespace Sosage
