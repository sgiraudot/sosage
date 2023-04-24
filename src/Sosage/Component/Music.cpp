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

  static std::vector<Source*> general_sources;
  Source* circle_source = nullptr;

  for (auto& s : m_sources)
  {
    Source& source = s.second;
    if (source.status == OFF)
      continue;

    if (source.big_radius != 0)
    {
      double dist = distance (source.position, position);
      if (source.status == FADING_OUT || dist < source.big_radius)
        circle_source = &source;
      continue;
    }

    if (source.status == FADING_OUT)
    {
      if (time - source.fade_origin > source.fade_duration)
      {
        source.status = OFF;
        continue;
      }
    }

    general_sources.push_back (&source);
  }

  double circle_strength = 0;
  if (circle_source)
  {
    circle_strength = 1.;

    if (circle_source->status == FADING_OUT)
    {
      if (time - circle_source->fade_origin > circle_source->fade_duration)
        circle_source->status = OFF;
      else
      {
        still_fading = true;
        circle_strength = (time - circle_source->fade_origin) / circle_source->fade_duration;
      }
      still_fading = true;
      circle_strength = 1. - ((time - circle_source->fade_origin) / circle_source->fade_duration);
    }
    else if (circle_source->status == FADING_IN)
    {
      if (time - circle_source->fade_origin > circle_source->fade_duration)
        circle_source->status = ON;
      else
      {
        still_fading = true;
        circle_strength = (time - circle_source->fade_origin) / circle_source->fade_duration;
      }
    }

    // Always remember latest strength to avoid jumps in sound
    // when changing room and coordinates may change abruplty
    if (circle_source->status == ON)
    {
      // Source becomes louder as we get close to the center
      double dist = distance (circle_source->position, position);
      if (dist > circle_source->small_radius)
        circle_strength *= (dist - circle_source->big_radius) /
                           (circle_source->small_radius - circle_source->big_radius);
      circle_source->fade_strength = circle_strength;
    }
    else
      circle_strength *= circle_source->fade_strength;

    for (std::size_t i = 0; i < m_mix.size(); ++ i)
      m_mix[i] += circle_strength * circle_source->mix[i];
  }

  if (circle_strength != 1.)
    for (auto s : general_sources)
    {
      Source& source = *s;
      double fade_factor = 1.;
      if (source.status == FADING_OUT)
      {
        still_fading = true;
        fade_factor = 1. - ((time - source.fade_origin) / source.fade_duration);
      }
      else if (source.status == FADING_IN)
      {
        if (time - source.fade_origin > source.fade_duration)
          source.status = ON;
        else
        {
          still_fading = true;
          fade_factor = (time - source.fade_origin) / source.fade_duration;
        }
      }

      for (std::size_t i = 0; i < m_mix.size(); ++ i)
        m_mix[i] += source.mix[i] * fade_factor * (1. - circle_strength);
    }

  general_sources.clear();

#if 0
  debug << "Mix = ";
  for (double& m : m_mix)
  {
    debug << m << " ";
  }
  debug << std::endl;
#endif

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

void Music::disable_source (const std::string& id, double time, double duration)
{
  auto& source = m_sources[id];
  if (duration != 0 && m_on && source.status == ON)
  {
    source.status = FADING_OUT;
    source.fade_origin = time;
    source.fade_duration = duration;
  }
  else
    source.status = OFF;
}

void Music::enable_source (const std::string& id, double time, double duration)
{
  auto& source = m_sources[id];
  if (duration != 0 && m_on && source.status == OFF)
  {
    source.status = FADING_IN;
    source.fade_origin = time;
    source.fade_duration = duration;
  }
  else
    source.status = ON;
}

const std::unordered_map<std::string, Music::Source>& Music::sources() const
{
  return m_sources;
}

} // namespace Sosage::Component
