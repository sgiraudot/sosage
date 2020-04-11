#include <Sosage/Component/Event.h>
#include <Sosage/Component/Image.h>
#include <Sosage/Component/Position.h>
#include <Sosage/System/Graphic.h>

#include <algorithm>
#include <vector>

namespace Sosage::System
{

Graphic::Graphic (Content& content, const std::string& game_name)
  : m_content (content)
  , m_core (game_name)
{
}

void Graphic::run()
{
  if (m_content.request<Component::Event>("window:rescaled"))
  {
    m_core.update_view();
    m_content.remove ("window:rescaled");
  }
  
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

  bool locked = (m_content.get<Component::State>("game:status")->value() == "locked");
  
  for (const auto& img : images)
  {
    if (img->on())
    {
      if (locked &&
           img->entity() == "cursor")
        continue;
      
      Component::Position_handle p = m_content.get<Component::Position>(img->entity() + ":position");

      int xmin = img->xmin();
      int ymin = img->ymin();
      int xmax = img->xmax();
      int ymax = img->ymax();
      
      Point screen_position = p->value() - img->core().scaling * Vector(img->origin());

      m_core.draw (img->core(), xmin, ymin, (xmax - xmin), (ymax - ymin),
                   screen_position.x(), screen_position.y(),
                   img->core().scaling * (xmax - xmin),
                   img->core().scaling * (ymax - ymin));
    }
  }

  // Component::Position_handle cursor
  //   = m_content.request<Component::Position>("cursor:position");
  // m_core.draw_square (cursor->value().x(), cursor->value().y(), 50);
}

} // namespace Sosage::System
