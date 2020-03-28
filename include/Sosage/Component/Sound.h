#ifndef SOSAGE_COMPONENT_SOUND_H
#define SOSAGE_COMPONENT_SOUND_H

#include <Sosage/Component/Handle.h>
#include <Sosage/Core/Sound.h>

namespace Sosage::Component
{

class Sound : public Base
{
private:
  Core::Sound::Sound m_core;
  
public:

  Sound (const std::string& id, const std::string& file_name);
  virtual ~Sound();
  virtual std::string str() const;
  const Core::Sound::Sound& core() const { return m_core; }
};

typedef std::shared_ptr<Sound> Sound_handle;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_SOUND_H
