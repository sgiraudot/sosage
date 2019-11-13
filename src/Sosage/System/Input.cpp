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
    {
      Component::Boolean_handle paused
        = m_content.request<Component::Boolean>("game:paused");
      if (paused == Component::Boolean_handle())
        m_content.set<Component::Boolean>("game:paused", true);
      else
        m_content.remove("game:paused");
    }
    
    if (m_core.is_mouse_motion(ev))
      m_content.set<Component::Position>("mouse:position", Point(m_core.mouse_position(ev)));
    
    if (m_core.is_left_click(ev))
    {
      if (m_core.mouse_position(ev).second < 880) // Out of interface
        m_content.set<Component::Position>("character:target_query",
                                           Point(m_core.mouse_position(ev)));
      else
        m_content.set<Component::Position>("mouse:clicked", Point(m_core.mouse_position(ev)));
    }

  }

}

} // namespace Sosage::System
