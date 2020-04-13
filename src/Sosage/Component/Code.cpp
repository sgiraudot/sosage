#include <Sosage/Component/Code.h>

#include <iostream>

namespace Sosage::Component
{

Code::Code (const std::string& id)
  : Base (id)
{ }

void Code::add_button (const std::string& value, int x, int y, int w, int l)
{
  m_buttons.push_back (Button (value, x, y, w, l));
}

void Code::add_answer_item (const std::string& value)
{
  m_answer.push_back (value);
}

bool Code::click (int x, int y)
{
  for (const Button& button : m_buttons)
    if (button.x <= x && x <= button.x + button.w &&
        button.y <= y && y <= button.y + button.l)
    {
      m_proposal.push_back (button.value);
      return true;
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
  return (m_answer != m_proposal);
}

bool Code::success()
{
  return (m_answer == m_proposal);
}


} // namespace Sosage::Component
