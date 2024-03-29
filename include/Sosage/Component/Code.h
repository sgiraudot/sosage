/*
  [include/Sosage/Component/Code.h]
  Code-based enigmas.

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

#ifndef SOSAGE_COMPONENT_CODE_H
#define SOSAGE_COMPONENT_CODE_H

#include <Sosage/Component/Base.h>

#include <vector>

namespace Sosage::Component
{

class Code : public Base
{

  struct Button
  {
    std::string value;
    int x;
    int y;
    int w;
    int h;
    std::size_t x_index = 0;
    std::size_t y_index = 0;
  };

  std::vector<Button> m_buttons;
  std::vector<std::string> m_answer;
  std::vector<std::size_t> m_proposal;
  std::size_t m_hover;
  std::vector<std::vector<std::size_t>> m_indices;
  
public:

  Code (const std::string& entity, const std::string& component);
  void add_button (const std::string& value, int x, int y, int w, int h);
  void add_answer_item (const std::string& value);
  bool hover (int x = -1, int y = -1);
  void move (double x, double y);
  bool click (int x = -1, int y = -1);
  void reset();
  bool failure();
  bool success();
  int xmin();
  int xmax();
  int ymin();
  int ymax();

  STR_NAME("Code");
};

using Code_handle = std::shared_ptr<Code>;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_CODE_H
