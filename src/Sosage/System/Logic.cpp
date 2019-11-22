#include <Sosage/Component/Condition.h>
#include <Sosage/Component/Ground_map.h>
#include <Sosage/Component/Font.h>
#include <Sosage/Component/Image.h>
#include <Sosage/Component/Path.h>
#include <Sosage/Component/Position.h>
#include <Sosage/Component/Text.h>
#include <Sosage/System/Logic.h>
#include <Sosage/Utils/geometry.h>

#include <vector>

namespace Sosage::System
{

Logic::Logic (Content& content)
  : m_content (content)
{
}

void Logic::main()
{
  Component::Image_handle collision = m_content.request<Component::Image> ("mouse:target");
  Component::Position_handle clicked
    = m_content.request<Component::Position>("mouse:clicked");
  if (clicked && collision)
  {
    if (collision->id() == "background:image")
      compute_path_from_target(clicked);
    else if (Component::Text_handle name = m_content.request<Component::Text>(collision->entity() + ":name"))
    {
      if (m_content.get<Component::Text> ("chosen_verb:text")->entity() == "verb_goto")
        compute_path_from_target(m_content.get<Component::Position>(collision->entity() + ":position"));
      else
      {
        // Action !

      }
    }
    m_content.remove("mouse:clicked");
    m_content.remove("mouse:target");
  }

  update_debug_info (m_content.get<Component::Debug>("game:debug"));
}

bool Logic::exit()
{
  Component::Boolean_handle exit
    = m_content.request<Component::Boolean>("game:exit");
  if (exit && exit->value())
    return true;
  return false;
}

bool Logic::paused()
{
  return m_content.get<Component::Boolean>("game:paused")->value();
}

void Logic::compute_path_from_target (Component::Position_handle target)
{
  std::vector<Point> path;
  
  Component::Ground_map_handle ground_map
    = m_content.get<Component::Ground_map>("background:ground_map");

  Component::Position_handle position
    = m_content.get<Component::Position>("character_body:position");
      
  Point origin = position->value();

  ground_map->find_path (origin, target->value(), path);
      
  m_content.set<Component::Path>("character:path", path);
  path.clear();
      
}

void Logic::update_debug_info (Component::Debug_handle debug_info)
{
  if (debug_info->value())
  {
    Component::Font_handle debug_font
      = m_content.get<Component::Font> ("debug:font");
    Component::Image_handle dbg_img
      = m_content.set<Component::Image> ("debug:image",
                                         debug_font, "FF0000",
                                         debug_info->debug_str());
    Component::Position_handle dbg_pos
      = m_content.set<Component::Position>("debug:position", Point(0,0));
  }
  else
  {
    Component::Image_handle dbg_img = m_content.request<Component::Image> ("debug:image");
    if (dbg_img)
      dbg_img->on() = false;
    
  }

}

} // namespace Sosage::System
