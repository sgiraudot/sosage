#include <Sosage/Component/Music.h>
#include <Sosage/Config.h>

namespace Sosage::Component
{

Music::Music (const std::string& id, const std::string& file_name)
  : Base(id), m_on(false)
{
  m_core = Core::Sound::load_music (file_name);
}

Music::~Music()
{
  Core::Sound::delete_music(m_core);
}

std::string Music::str() const
{
  return this->id();
}



} // namespace Sosage::Component
