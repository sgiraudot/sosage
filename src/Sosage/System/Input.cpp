#include <Sosage/Component/Path.h>
#include <Sosage/System/Input.h>

namespace Sosage::System
{

Input::Input (Content& content)
  : m_content (content)
  , m_core()
{
}

void Input::main()
{
  while (this->running())
  {
    Input_core::Event ev = m_core.next_event();
    if (m_core.is_exit(ev))
      this->stop();
    
    if (m_core.is_left_click(ev))
      m_content.set<Component::Path>("character", "target_query",
                                     Point(m_core.click_target(ev),
                                           CAMERA));
  }
}

} // namespace Sosage::System
