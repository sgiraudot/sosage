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
#include <Sosage/Config/config.h>

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
                     std::numeric_limits<double>::infinity()), 0.);
}

void Music::add_track (const std::string& file_name)
{
  m_core.push_back (Core::Sound::load_music (file_name));
  m_mix.push_back(0);
}

void Music::add_source (const std::string& id, const std::vector<double>& mix,
                        double x, double y, double small_radius, double big_radius)
{
  Source& source =  m_sources.insert (std::make_pair (id, Source())).first->second;
  if (big_radius != 0)
  {
    source.position = Point(x,y);
    source.small_radius = small_radius;
    source.big_radius = big_radius;
  }
  source.mix = mix;
}

bool Music::adjust_mix (const Point& position, const double& time)
{
  for (double& m : m_mix)
    m = 0.;

  bool still_fading = false;

  double gain = 0.;
  for (auto& s : m_sources)
  {
    Source& source = s.second;
    double fade_factor = 1.;
    if (source.status == OFF)
      continue;
    else if (source.status == FADING_OUT)
    {
      if (time - source.fade_origin > Config::default_sound_fade_time)
      {
        source.status = OFF;
        continue;
      }
      still_fading = true;
      fade_factor = 1. - ((time - source.fade_origin) / Config::default_sound_fade_time);
    }
    else if (source.status == FADING_IN)
    {
      if (time - source.fade_origin > Config::default_sound_fade_time)
        source.status = ON;
      else
      {
        still_fading = true;
        fade_factor = (time - source.fade_origin) / Config::default_sound_fade_time;
      }
    }

    // General source, sound is 100% everywhere
    if (source.big_radius == 0)
    {
      for (std::size_t i = 0; i < m_mix.size(); ++ i)
        m_mix[i] += source.mix[i] * fade_factor;
      gain += fade_factor;
    }
    else
    {
      // Source becomes louder as we get close to the center
      double dist = distance (source.position, position);
      if (dist < source.big_radius)
      {
        double g = 1.;
        if (dist > source.small_radius)
          g = (dist - source.big_radius) / (source.small_radius - source.big_radius);
        debug << "GAIN = " << g << std::endl;
        for (std::size_t i = 0; i < m_mix.size(); ++ i)
          m_mix[i] += g * source.mix[i] * fade_factor;
        gain += g * fade_factor;
      }
    }
  }

  gain = std::sqrt(gain);
  // Normalize
  debug << "Mix = ";
  for (double& m : m_mix)
  {
    m /= gain;
    debug << m << " ";
  }
  debug << std::endl;

  return still_fading;
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

void Music::disable_source (const std::string& id, double time)
{
  auto& source = m_sources[id];
  source.status = FADING_OUT;
  source.fade_origin = time;
}

void Music::enable_source (const std::string& id, double time)
{
  auto& source = m_sources[id];
  source.status = FADING_IN;
  source.fade_origin = time;
}

const std::unordered_map<std::string, Music::Source>& Music::sources() const
{
  return m_sources;
}

} // namespace Sosage::Component
