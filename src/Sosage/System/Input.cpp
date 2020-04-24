#include <Sosage/Component/Condition.h>
#include <Sosage/Component/Console.h>
#include <Sosage/Component/Event.h>
#include <Sosage/Component/Position.h>
#include <Sosage/System/Input.h>

namespace Sosage::System
{

Input::Input (Content& content)
  : m_content (content)
  , m_core()
{
}

void Input::run()
{
  Core::Input::Event ev;

  while (m_core.next_event(ev))
  {
    if (m_core.is_exit(ev))
      m_content.set<Component::Event>("game:exit");

    if (m_core.is_pause(ev))
      m_content.get<Component::Boolean>("game:paused")->toggle();

    auto paused = m_content.request<Component::Boolean>("game:paused");

    if (paused && paused->value())
    {
#ifndef SOSAGE_ANDROID
      m_content.get<Component::Image>("cursor:image")->on() = false;
#endif
      continue;
    }
#ifndef SOSAGE_ANDROID
    m_content.get<Component::Image>("cursor:image")->on() = true;
#endif

    if (m_core.is_debug(ev))
      m_content.get<Component::Boolean>("game:debug")->toggle();
    
    if (m_core.is_console(ev))
      m_content.get<Component::Boolean>("game:console")->toggle();

    if (m_core.is_f1(ev))
      m_content.set<Component::Event>("window:set_auto");
    else if (m_core.is_f2(ev))
      m_content.set<Component::Event>("window:set_widescreen");
    else if (m_core.is_f3(ev))
      m_content.set<Component::Event>("window:set_standard");
    else if (m_core.is_f4(ev))
      m_content.set<Component::Event>("window:set_square");
    else if (m_core.is_f5(ev))
      m_content.set<Component::Event>("window:set_portrait");
    
    if (m_core.is_window_resized(ev))
    {
      std::tie (config().window_width, config().window_height)
        = m_core.window_size(ev);
      m_content.set<Component::Event>("window:rescaled");
    }

    // If paused, ignore mouse events
    if (m_content.get<Component::State>("game:status")->value() == "locked")
      continue;
  
    if (m_core.is_mouse_motion(ev))
      m_content.set<Component::Position>("cursor:position", Point(m_core.mouse_position(ev)));
    
    if (m_core.is_left_click(ev))
    {
      m_content.set<Component::Position>("cursor:position", Point(m_core.mouse_position(ev)));
      m_content.set<Component::Event>("cursor:clicked");
      DBG_CERR << "Mouse clicked = " << Point(m_core.mouse_position(ev)) << std::endl;
    }
  }
}

} // namespace Sosage::System
