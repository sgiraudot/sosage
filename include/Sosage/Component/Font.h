#ifndef SOSAGE_COMPONENT_FONT_H
#define SOSAGE_COMPONENT_FONT_H

#include <Sosage/Component/Handle.h>
#include <Sosage/Core/Graphic.h>
#include <Sosage/Utils/geometry.h>

namespace Sosage::Component
{

class Font : public Base
{
private:
  Core::Graphic::Font m_core;
  
public:

  Font (const std::string& id, const std::string& file_name, int size);
  virtual ~Font();
  const Core::Graphic::Font& core() const { return m_core; }
};

typedef std::shared_ptr<Font> Font_handle;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_FONT_H
