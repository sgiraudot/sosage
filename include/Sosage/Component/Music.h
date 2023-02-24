/*
  [include/Sosage/Component/Music.h]
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

#ifndef SOSAGE_COMPONENT_MUSIC_H
#define SOSAGE_COMPONENT_MUSIC_H

#include <Sosage/Component/Base.h>
#include <Sosage/Core/Sound.h>
#include <Sosage/Utils/geometry.h>

#include <unordered_map>
#include <vector>

namespace Sosage::Component
{

class Music : public Base
{
public:
  enum Source_status { FADING_IN, ON, FADING_OUT, OFF };

private:

  struct Source
  {
    Point position = Point::invalid();
    double small_radius = 0;
    double big_radius = 0;
    std::vector<double> mix;
    double fade_origin = 0.;
    Source_status status = ON;
  };

  std::vector<Core::Sound::Music> m_core;
  std::vector<double> m_mix;
  std::unordered_map<std::string, Source> m_sources;
  bool m_on;
  
public:

  Music (const std::string& entity, const std::string& component);
  virtual ~Music();
  void init();
  void add_track (const std::string& file_name);
  void add_source (const std::string& id, const std::vector<double>& mix,
                   double x = 0, double y = 0,
                   double small_radius = 0, double big_radius = 0);
  bool adjust_mix (const Point& position, const double& time);
  std::size_t tracks() const;
  const Core::Sound::Music& core (std::size_t i) const;
  double mix (std::size_t i) const;
  const bool& on() const;
  bool& on();
  void disable_source (const std::string& id, double time);
  void enable_source (const std::string& id, double time);
  const std::unordered_map<std::string, Source>& sources() const;

  STR_NAME("Music");
};

using Music_handle = std::shared_ptr<Music>;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_MUSIC_H
