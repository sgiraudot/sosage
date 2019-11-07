#include <Sosage/Component/Image.h>
#include <Sosage/Component/Position.h>
#include <Sosage/System/Graphic.h>

#include <algorithm>
#include <vector>

namespace Sosage::System
{

Graphic::Graphic (Content& content, const std::string& game_name)
  : m_content (content)
  , m_core (game_name, config().camera_width, config().camera_height, config().fullscreen)
{


}

void Graphic::main()
{
  std::vector<Component::Image_handle> images;

  m_core.begin();

  get_images (images);
  display_images (images);
  images.clear();

  m_core.end();
}

void Graphic::get_images (std::vector<Component::Image_handle>& images)
{
  for (const auto& e : m_content)
    if (Component::Image_handle img
        = Component::cast<Component::Image>(e))
      images.push_back(img);
}

void Graphic::display_images (std::vector<Component::Image_handle>& images)
{
  std::sort (images.begin(), images.end(),
             [](Component::Image_handle a, Component::Image_handle b) -> bool
             {
               return (a->z() < b->z());
             });

  for (const auto& img : images)
  {
    if (img->on())
    {
      Component::Position_handle p = m_content.get<Component::Position>(img->entity() + ":position");
      m_core.draw (img->core(),
                   p->value().x(),
                   p->value().y(),
                   img->xmin(), img->xmax(),
                   img->ymin(), img->ymax());
    }
  }
}

} // namespace Sosage::System
