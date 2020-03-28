#include <Sosage/Component/Sound.h>
#include <Sosage/Config.h>

namespace Sosage::Component
{

Sound::Sound (const std::string& id, const std::string& file_name)
  : Base(id)
{
  m_core = Core::Sound::load_sound (file_name);
}

Sound::~Sound()
{
  Core::Sound::delete_sound(m_core);
}

std::string Sound::str() const
{
  return this->id();
}



} // namespace Sosage::Component
