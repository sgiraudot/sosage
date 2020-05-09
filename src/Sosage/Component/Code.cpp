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

#include <iostream>

namespace Sosage::Component
{

Code::Code (const std::string& id)
  : Base (id)
{ }

void Code::add_button (const std::string& value, int x, int y, int w, int h)
{
  m_buttons.push_back (Button (value, x, y, w, h));
}

void Code::add_answer_item (const std::string& value)
{
  m_answer.push_back (value);
}

bool Code::click (int x, int y)
{
  for (std::size_t i = 0; i < m_buttons.size(); ++ i)
  {
    const Button& button = m_buttons[i];
    if (button.x <= x && x <= button.x + button.w &&
        button.y <= y && y <= button.y + button.h)
    {
      m_proposal.push_back (i);
      return true;
    }
  }
  return false;
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


} // namespace Sosage::Component
