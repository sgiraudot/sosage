#include <Sosage/Component/Condition.h>
#include <Sosage/Component/Position.h>
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
  Core::Input::Event ev;

  while (m_core.next_event(ev))
  {
    if (m_core.is_exit(ev))
      m_content.set<Component::Boolean>("game:exit", true);

    if (m_core.is_pause(ev))
      m_content.get<Component::Boolean>("game:paused")->toggle();

    Component::Boolean_handle paused
      = m_content.request<Component::Boolean>("game:paused");
    if (paused && paused->value())
      continue;

    if (m_core.is_debug(ev))
      m_content.get<Component::Boolean>("game:debug")->toggle();
    
    if (m_core.is_mouse_motion(ev))
      m_content.set<Component::Position>("mouse:position", Point(m_core.mouse_position(ev)));
    
    if (m_core.is_left_click(ev))
      m_content.set<Component::Position>("mouse:clicked", Point(m_core.mouse_position(ev)));

    if (m_core.is_window_resized(ev))
    {
      std::tie (config().window_width, config().window_height)
        = m_core.window_size(ev);
      m_content.set<Component::Boolean>("window:rescaled", true);
    }
  }
}

} // namespace Sosage::System
