/*
  [include/Sosage/System/Graphic.h]
  Renders the content of the game at each frame.

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

#ifndef SOSAGE_SYSTEM_GRAPHIC_H
#define SOSAGE_SYSTEM_GRAPHIC_H

#include <Sosage/Component/Image.h>
#include <Sosage/Core/Graphic.h>
#include <Sosage/Content.h>

#include <vector>

namespace Sosage::System
{

class Graphic
{
private:

  Content& m_content;
  Core::Graphic m_core;

public:

  Graphic (Content& content);

  void init();

  void run();

private:

  void get_images (std::vector<Component::Image_handle>& images);

  void display_images (std::vector<Component::Image_handle>& images);
};

} // namespace Sosage::System

#endif // SOSAGE_SYSTEM_GRAPHIC_H
