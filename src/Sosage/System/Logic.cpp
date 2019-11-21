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
  if (!m_chosen_verb)
    m_chosen_verb = m_content.get<Component::Text> ("verb_goto:text");
  
  Component::Position_handle mouse
    = m_content.request<Component::Position>("mouse:position");
  if (mouse)
    detect_collision (mouse);

  Component::Position_handle clicked
    = m_content.request<Component::Position>("mouse:clicked");
  if (clicked && m_collision)
  {
    std::cerr << m_collision->id() << std::endl;
    if (m_collision->id() == "background:image")
    {
      compute_path_from_target(clicked);
      m_chosen_verb = m_content.get<Component::Text> ("verb_goto:text");
    }
    else if (m_collision->entity().find("verb_") == 0)
      m_chosen_verb = m_content.get<Component::Text> (m_collision->entity() + ":text");
    else if (Component::Text_handle name = m_content.request<Component::Text>(m_collision->entity() + ":name"))
    {
      if (m_chosen_verb->entity() == "verb_goto")
        compute_path_from_target(m_content.get<Component::Position>(m_collision->entity() + ":position"));
      else
      {
        // Action !
#if 0
        if (Component::Action_handle action = m_content.request<Component::Action>
            (m_collision->entity() + ":" + m_chosen_verb->entity()))
        {

        }
#endif
      }
    }

    m_content.remove("mouse:clicked");
  }

  update_interface();
  
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

void Logic::detect_collision (Component::Position_handle mouse)
{
  // Deactive previous collisions
  if (m_collision)
  {
    if (m_collision->entity().find("verb_") == 0)
      m_collision->set_scale(0.75);
  }

  m_collision = Component::Image_handle();
  
  for (const auto& e : m_content)
    if (Component::Image_handle img
        = Component::cast<Component::Image>(e))
    {
      if (!img->on() ||
          img->id().find("character") == 0 ||
          img->id().find("debug") == 0)
        continue;
      
      Component::Position_handle p = m_content.get<Component::Position>(img->entity() + ":position");

      Point screen_position = p->value() - img->core().scaling * Vector(img->origin());
      int xmin = screen_position.x();
      int ymin = screen_position.y();
      int xmax = xmin + (img->core().scaling * (img->xmax() - img->xmin()));
      int ymax = ymin + (img->core().scaling * (img->ymax() - img->ymin()));

      if (mouse->value().x() < xmin ||
          mouse->value().x() > xmax ||
          mouse->value().y() < ymin ||
          mouse->value().y() > ymax)
        continue;

      if (img->id() != "background:image" && img->id().find("verb_") != 0)
      {
        int x_in_image = mouse->value().x() - xmin;
        int y_in_image = mouse->value().y() - ymin;
        if (!img->is_target_inside (x_in_image, y_in_image))
          continue;
      }
      
      // Now, collision happened
      
      if (m_collision)
      {
        // Keep image closest to screen
        if (img->z() > m_collision->z())
          m_collision = img;
      }
      else
        m_collision = img;

    }

}

void Logic::update_interface ()
{
  std::string target_object = "";
  
  if (m_collision)
  {
    if (m_collision->entity().find("verb_") == 0)
      m_collision->set_scale(1.0);
    if (Component::Text_handle name = m_content.request<Component::Text>(m_collision->entity() + ":name"))
      target_object = name->value();
  }

  Component::Image_handle text_img
    = m_content.set<Component::Image>("chosen_verb:image",
                                      m_content.get<Component::Font>("interface:font"), "FFFFFF",
                                      m_chosen_verb->value() + " " + target_object);
  text_img->set_relative_origin(0.5, 0.5);
  text_img->set_scale(0.5);
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
