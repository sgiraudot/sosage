#include <Sosage/Component/Image.h>
#include <Sosage/Component/Path.h>
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

  display_path();

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
      Component::Path_handle p = m_content.get<Component::Path>(img->entity() + ":position");
      m_core.draw (img->core(),
                   (*p)[0].x(CAMERA),
                   (*p)[0].y(CAMERA),
                   img->xmin(), img->xmax(),
                   img->ymin(), img->ymax());
    }
  }
}

void Graphic::display_path()
{
  Component::Path_handle path = m_content.request<Component::Path>("character:path");
  if (path)
  {
    for (std::size_t i = 0; i < path->size() - 1; ++ i)
      m_core.draw_line ((*path)[i].x(CAMERA),
                        (*path)[i].y(CAMERA),
                        (*path)[i+1].x(CAMERA),
                        (*path)[i+1].y(CAMERA));

  }
}

} // namespace Sosage::System
