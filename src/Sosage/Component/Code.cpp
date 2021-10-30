/*
  [src/Sosage/Component/Code.cpp]
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

#include <Sosage/Component/Code.h>

namespace Sosage::Component
{

Code::Code (const std::string& id)
  : Base (id), m_hover(std::size_t(-1))
{ }

void Code::add_button (const std::string& value, int x, int y, int w, int h)
{
  m_buttons.push_back ({value, x, y, w, h});
}

void Code::add_answer_item (const std::string& value)
{
  m_answer.push_back (value);
}

bool Code::hover (int x, int y)
{
  if (x == -1 && y == -1)
  {
    m_hover = 0;
    return true;
  }
  m_hover = std::size_t(-1);
  for (std::size_t i = 0; i < m_buttons.size(); ++ i)
  {
    const Button& button = m_buttons[i];
    if (button.x <= x && x <= button.x + button.w &&
        button.y <= y && y <= button.y + button.h)
    {
      m_hover = i;
      return true;
    }
  }
  return false;
}

void Code::move (double x, double y)
{
  // Organize buttons
  if (m_indices.empty())
  {
    std::vector<int> x_coords, y_coords;

    for (std::size_t i = 0; i < m_buttons.size(); ++ i)
    {
      int x = m_buttons[i].x;
      if (x >= int(x_coords.size())) x_coords.resize (x+1, -1);
      if (x_coords[x] == -1) x_coords[x] = 0;

      int y = m_buttons[i].y;
      if (y >= int(y_coords.size())) y_coords.resize (y+1, -1);
      if (y_coords[y] == -1) y_coords[y] = 0;
    }

    int nb_columns = 0, nb_lines = 0;
    for (std::size_t i = 0; i < x_coords.size(); ++ i)
      if (x_coords[i] != -1)
        x_coords[i] = nb_columns ++;
    for (std::size_t i = 0; i < y_coords.size(); ++ i)
      if (y_coords[i] != -1)
        y_coords[i] = nb_lines ++;

    m_indices.resize (nb_columns, std::vector<std::size_t>(nb_lines, std::size_t(-1)));
    for (std::size_t i = 0; i < m_buttons.size(); ++ i)
    {
      m_indices[x_coords[m_buttons[i].x]][y_coords[m_buttons[i].y]] = i;
      m_buttons[i].x_index = x_coords[m_buttons[i].x];
      m_buttons[i].y_index = y_coords[m_buttons[i].y];
    }
  }

  std::size_t x_index = m_buttons[m_hover].x_index;
  std::size_t y_index = m_buttons[m_hover].y_index;

  int diff_x = 0;
  int diff_y = 0;

  if (x > 0.9)
    diff_x = 1;
  else if (x < -0.9)
    diff_x = -1;
  else if (y > 0.9)
    diff_y = 1;
  else if (y < -0.9)
    diff_y = -1;
  else if (x > 0.4)
  {
    diff_x = 1;
    if (y > 0)
      diff_y = 1;
    else
      diff_y = -1;
  }
  else if (x < -0.4)
  {
    diff_x = -1;
    if (y > 0)
      diff_y = 1;
    else
      diff_y = -1;
  }

  if (diff_x == -1 && x_index > 0)
    -- x_index;
  if (diff_x == 1 && x_index < m_indices.size() - 1)
    ++ x_index;
  if (diff_y == -1 && y_index > 0)
    -- y_index;
  if (diff_y == 1 && y_index < m_indices.front().size() - 1)
    ++ y_index;

  if (m_indices[x_index][y_index] != std::size_t(-1))
    m_hover = m_indices[x_index][y_index];
}

bool Code::click (int x, int y)
{
  if (x != -1 && y != -1)
  {
    if (!hover(x, y))
      return false;
  }
  m_proposal.push_back (m_hover);
  return true;
}

void Code::reset()
{
  m_proposal.clear();
}

bool Code::failure()
{
  // If proposal is not yet the size of answer, neither failure nor
  // success has happened yet
  if (m_answer.size() != m_proposal.size())
    return false;

  return !success();
}

bool Code::success()
{
  if (m_answer.size() != m_proposal.size())
    return false;
  
  for (std::size_t i = 0; i < m_answer.size(); ++ i)
    if (m_buttons[m_proposal[i]].value != m_answer[i])
      return false;

  return true;
}

int Code::xmin()
{
  return m_buttons[m_hover].x;
}

int Code::xmax()
{
  return m_buttons[m_hover].x + m_buttons[m_hover].w;
}

int Code::ymin()
{
  return m_buttons[m_hover].y;
}

int Code::ymax()
{
  return m_buttons[m_hover].y + m_buttons[m_hover].h;
}

} // namespace Sosage::Component
