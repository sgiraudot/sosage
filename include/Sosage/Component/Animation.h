/*
  [include/Sosage/Component/Animation.h]
  Specialization of Image for animated objects.

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

#ifndef SOSAGE_COMPONENT_ANIMATION_H
#define SOSAGE_COMPONENT_ANIMATION_H

#include <Sosage/Component/Image.h>

#include <vector>

namespace Sosage::Component
{

class Animation : public Image
{
public:

  struct Frame
  {
    int x;
    int y;
    int duration;
    int ellapsed = 0;
  };

private:
  
  const int m_width_subdiv;
  const int m_height_subdiv;
  std::vector<Frame> m_frames;
  std::size_t m_current;
  bool m_loop;
  bool m_playing;

public:

  Animation (const std::string& entity, const std::string& component,
             const std::string& file_name, int z,
             int width_subdiv, int height_subdiv, bool loop,
             const Collision_type& collision = BOX, bool with_highlight = false);
  int width_subdiv() const;
  int height_subdiv() const;
  const std::vector<Frame>& frames() const;
  std::vector<Frame>& frames();
  bool loop() const;
  bool animated() const;
  const bool& playing() const;
  bool& playing();
  int reset(bool all_frames = true, int duration = 1);
  virtual int xmin() const;
  virtual int xmax() const;
  virtual int ymin() const;
  virtual int ymax() const;
  std::size_t current() const;
  const Frame& current_frame() const;
  bool next_frame();

  STR_NAME("Animation");
};

using Animation_handle = std::shared_ptr<Animation>;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_ANIMATION_H
