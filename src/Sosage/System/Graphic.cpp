#include <Sosage/Component/Image.h>
#include <Sosage/Component/Path.h>
#include <Sosage/System/Graphic.h>

#include <algorithm>
#include <vector>

namespace Sosage::System
{

Graphic::Graphic (Content& content, const std::string& game_name)
  : m_content (content)
  , m_core (game_name, config().camera_width, config().camera_height, false)
{


}

void Graphic::main()
{
  std::vector<std::pair<Component::Image_handle,
                        Component::Path_handle> >
    components;

  while (this->running())
  {
    m_core.begin();

    bool image_found = false, coordinates_found = false;
    
    m_content.lock_components();
    for (const auto& e : m_content)
    {
      Component::Image_handle image;
      Component::Path_handle coordinates;
      
      for (const auto& c : e.second)
        if (c.first == "image")
          image = Component::component_cast<Component::Image>(c.second);
        else if (c.first == "position")
          coordinates = Component::component_cast<Component::Path>(c.second);
      
      if (image != Component::Image_handle() &&
          coordinates != Component::Path_handle())
        components.push_back (std::make_pair (image, coordinates));
    }
    m_content.unlock_components();

    for (const auto& c : components)
    {
      c.first->lock();
      c.second->lock();
    }
    
    std::sort (components.begin(), components.end(),
               [](const std::pair<Component::Image_handle, Component::Path_handle>& a,
                  const std::pair<Component::Image_handle, Component::Path_handle>& b)
               -> bool
               {
                 return (a.first->z() < b.first->z());
               });

    for (const auto& c : components)
    {
      m_core.draw (c.first->core(),
                   (*c.second)[0].x(CAMERA),
                   (*c.second)[0].y(CAMERA));
      c.first->unlock();
      c.second->unlock();
    }

    components.clear();

    Component::Path_handle path = m_content.get<Component::Path>("character", "path");
    if (path)
    {
      path->lock();

      for (std::size_t i = 0; i < path->size() - 1; ++ i)
        m_core.draw_line ((*path)[i].x(CAMERA),
                          (*path)[i].y(CAMERA),
                          (*path)[i+1].x(CAMERA),
                          (*path)[i+1].y(CAMERA));

      path->unlock();
    }

    m_core.end();
    
    this->wait();
  }

}

} // namespace Sosage::System
