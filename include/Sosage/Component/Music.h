#ifndef SOSAGE_COMPONENT_MUSIC_H
#define SOSAGE_COMPONENT_MUSIC_H

#include <Sosage/Component/Handle.h>
#include <Sosage/Core/Sound.h>

namespace Sosage::Component
{

class Music : public Base
{
private:
  Core::Sound::Music m_core;
  bool m_on;
  
public:

  Music (const std::string& id, const std::string& file_name);
  virtual ~Music();
  virtual std::string str() const;
  const Core::Sound::Music& core() const { return m_core; }
  const bool& on() const { return m_on; }
  bool& on() { return m_on; }

};

typedef std::shared_ptr<Music> Music_handle;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_MUSIC_H
