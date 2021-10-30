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

Content::~Content()
{
  display_access();
}

void Content::clear()
{
  for (std::size_t i = 0; i < NUMBER_OF_KEYS; ++ i)
    m_fast_access_components[i] = nullptr;
  m_data.clear();
}

void Content::clear (const std::function<bool(Component::Handle)>& filter)
{
  for (auto& hset : m_data)
  {
    Component::Handle_set& old_set = hset;
    Component::Handle_set new_set;
    for (Component::Handle c : old_set)
      if (!filter(c))
        new_set.insert(c);
    old_set.swap (new_set);
  }
}

std::size_t Content::size() const
{
  return m_data.size();
}

Component::Component_map::const_iterator Content::begin() const
{
  return m_data.begin();
}

Component::Component_map::const_iterator Content::end() const
{
  return m_data.end();
}

Component::Handle_set& Content::components (const std::string& s)
{
  auto iter = m_map_component.find(s);
  if (iter == m_map_component.end())
      return m_data[0];
  return m_data[iter->second];
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

void Content::emit (const std::string& signal)
{
  set<Component::Signal>(signal);
}

bool Content::receive (const std::string& signal)
{
  count_access(signal);
  count_request();

  auto cmp = std::make_shared<Component::Base>(signal);
  Component::Handle_set& hset = components(cmp->component());

  Component::Handle_set::iterator iter
      = hset.find(std::make_shared<Component::Base>(signal));
  if (iter == hset.end())
    return false;
  if (!Component::cast<Component::Signal>(*iter))
    return false;
  hset.erase (iter);
  return true;
}

void Content::count_set_ptr()
{
  SOSAGE_COUNT (Content__set_ptr);
}

void Content::count_set_args()
{
  SOSAGE_COUNT (Content__set_args);
}

void Content::count_request()
{
  SOSAGE_COUNT (Content__request);
}

void Content::count_get()
{
  SOSAGE_COUNT (Content__get);
}

#ifdef SOSAGE_PROFILE

void Content::count_access (const std::string& k)
{
  auto inserted = m_access_count.insert (std::make_pair (k, 1));
  if (!inserted.second)
    inserted.first->second ++;
}

void Content::display_access()
{
  std::vector<std::pair<std::string, std::size_t> > sorted;
  sorted.reserve (m_access_count.size());
  std::copy (m_access_count.begin(), m_access_count.end(),
             std::back_inserter (sorted));
  auto end = std::partition
             (sorted.begin(), sorted.end(),
              [](const auto& p) -> bool { return isupper(p.first[0]); });
  std::sort (sorted.begin(), end,
             [](const auto& a, const auto& b) -> bool
             {
               return a.second > b.second;
             });
  debug << "[Profiling system component access count (25 first)]" << std::endl;
  for (std::size_t i = 0; i < 25; ++ i)
    debug << " * " << sorted[i].first << " (accessed "
          << sorted[i].second << " times)" << std::endl;
}

#else

void Content::count_access (const std::string&) { }
void Content::display_access () { }

#endif

} // namespace Sosage
