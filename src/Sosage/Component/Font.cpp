#include <Sosage/Component/Font.h>
#include <Sosage/Config.h>

namespace Sosage::Component
{

Font::Font (const std::string& id, const std::string& file_name, int size)
  : Base(id)
{
  std::cerr << "Creating font " << file_name << std::endl;
  m_core = Core::Graphic::load_font (file_name, size);
}

Font::~Font()
{
  Core::Graphic::delete_font(m_core);
}

} // namespace Sosage::Component
