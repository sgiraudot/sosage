#include <Sosage/Component/Path.h>
#include <Sosage/Component/Status.h>
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
  Input_core::Event ev;

  while (m_core.next_event(ev))
  {
    if (m_core.is_exit(ev))
      m_content.set<Component::Status>("game:status", Component::EXIT);
    
    if (m_core.is_left_click(ev))
      m_content.set<Component::Path>("character:target_query",
                                     Point(m_core.click_target(ev),
                                           CAMERA));
  }
}

} // namespace Sosage::System
