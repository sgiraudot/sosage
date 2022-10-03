/*
  [src/Sosage/Component/Music.cpp]
  Background music played in loop.

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

#include <Sosage/Component/Music.h>

namespace Sosage::Component
{

Music::Music (const std::string& entity, const std::string& component)
  : Base(entity, component), m_on(false)
{ }

Music::~Music()
{
  for (auto c : m_core)
    Core::Sound::delete_music(c);
}

void Music::init()
{
  // Init with point at infinity, to just trigger general sources
  adjust_mix (Point (std::numeric_limits<double>::infinity(),
                     std::numeric_limits<double>::infinity()));
}

void Music::add_track (const std::string& file_name)
{
  m_core.push_back (Core::Sound::load_music (file_name));
  m_mix.push_back(0);
}

void Music::add_source (const std::string& id, const std::vector<double>& mix,
                        double x, double y, double radius)
{
  Source& source =  m_sources.insert (std::make_pair (id, Source())).first->second;
  if (radius != 0)
  {
    source.position = Point(x,y);
    source.radius = radius;
  }
  source.mix = mix;
}

void Music::adjust_mix (const Point& position)
{
  for (double& m : m_mix)
    m = 0.;

  std::size_t gain = 0.;
  for (const auto& s : m_sources)
  {
    const Source& source = s.second;
    if (!source.on)
      continue;

    // General source, sound is 100% everywhere
    if (source.radius == 0)
    {
      for (std::size_t i = 0; i < m_mix.size(); ++ i)
        m_mix[i] += source.mix[i];
      gain += 1.;
    }
    else
    {
      // Source becomes louder as we get close to the center
      double dist = distance (source.position, position);
      debug << position << " VS " << source.position << std::endl,
      debug << dist << " / " << source.radius << std::endl;
      if (dist < source.radius)
      {
        double g = (source.radius - dist) / source.radius;
        for (std::size_t i = 0; i < m_mix.size(); ++ i)
          m_mix[i] += g * source.mix[i];
        gain += g;
      }
    }
  }

  // Normalize
  for (double& m : m_mix)
    m /= gain;
}

std::size_t Music::tracks() const
{
  return m_core.size();
}

const Core::Sound::Music& Music::core (std::size_t i) const
{
  return m_core[i];
}

double Music::mix (std::size_t i) const
{
  return m_mix[i];
}

const bool& Music::on() const
{
  return m_on;
}

bool& Music::on()
{
  return m_on;
}

} // namespace Sosage::Component
