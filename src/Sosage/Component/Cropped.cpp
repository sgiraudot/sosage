#include <Sosage/Component/Cropped.h>
#include <Sosage/Core/Graphic.h>
#include <Sosage/Config.h>

namespace Sosage::Component
{

Cropped::Cropped (const std::string& id, const std::string& file_name, int z)
  : Image(id, file_name, z)
  , m_xmin (0)
  , m_xmax (Core::Graphic::width(core()))
  , m_ymin (0)
  , m_ymax (Core::Graphic::height(core()))
{
}

void Cropped::crop (int xmin, int xmax, int ymin, int ymax)
{
  m_xmin = xmin;
  m_xmax = xmax;
  m_ymin = ymin;
  m_ymax = ymax;
}

} // namespace Sosage::Component
