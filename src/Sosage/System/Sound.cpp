#include <Sosage/Component/Event.h>
#include <Sosage/Component/Music.h>
#include <Sosage/Component/Sound.h>
#include <Sosage/System/Sound.h>

namespace Sosage::System
{

Sound::Sound (Content& content)
  : m_content (content), m_core()
{

}

void Sound::main()
{
  Component::Music_handle music = m_content.request<Component::Music>("game:music");

  Component::Event_handle start = m_content.request<Component::Event>("music:start");
  if (start)
  {
    m_core.start_music (music->core());
    music->on() = true;
    m_content.remove ("music:start");
  }
  
  bool paused = m_content.get<Component::Boolean>("game:paused")->value();
  if (paused && music->on())
  {
    m_core.pause_music (music->core());
    music->on() = false;
  }
  else if (!paused && !music->on())
  {
    m_core.resume_music (music->core());
    music->on() = true;
  }

  Component::Event_handle clicked = m_content.request<Component::Event>("game:verb_clicked");
  if (clicked)
  {
    m_core.play_sound (m_content.get<Component::Sound>("click:sound")->core());
    m_content.remove ("game:verb_clicked");
  }
}

} // namespace Sosage::System
