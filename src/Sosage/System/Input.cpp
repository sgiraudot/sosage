#include <Sosage/Component/Condition.h>
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
  Input_core::Event ev;

  while (m_core.next_event(ev))
  {
    if (m_core.is_exit(ev))
      m_content.set<Component::Boolean>("game:exit", true);
    
    if (m_core.is_left_click(ev))
      m_content.set<Component::Path>("character:target_query",
                                     Point(m_core.click_target(ev),
                                           CAMERA));
  }
}

} // namespace Sosage::System
