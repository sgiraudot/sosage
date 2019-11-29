#include <Sosage/Component/Action.h>
#include <Sosage/Component/Animation.h>
#include <Sosage/Component/Font.h>
#include <Sosage/Component/Ground_map.h>
#include <Sosage/Component/Image.h>
#include <Sosage/Component/Position.h>
#include <Sosage/Component/Text.h>
#include <Sosage/System/IO.h>
#include <Sosage/version.h>
#include <Sosage/Utils/profiling.h>

namespace Sosage::System
{

IO::IO (Content& content)
  : m_content (content)
{
}

std::string IO::read_init (const std::string& folder_name)
{
  m_folder_name = folder_name;
  std::string file_name = folder_name + "resources/init.xml";
  
  Core::IO::Element input (file_name);
  check (input.name() == "sosage_init", file_name + " is not a Sosage init file.");

  std::string v = input.property("version");
  check (version(v) == SOSAGE_VERSION,
         "Error: room version " + v + " incompatible with Sosage " + Sosage::version());

  std::string cursor = input.property("sprites/", "cursor", ".png");
//  m_graphic.set_cursor (local_file_name(cursor));
  
  std::string debug_font = input.property("fonts/", "debug_font", ".ttf");
  m_content.set<Component::Font> ("debug:font", local_file_name(debug_font), 15);

  std::string interface_font = input.property("fonts/", "interface_font", ".ttf");
  m_content.set<Component::Font> ("interface:font", local_file_name(interface_font), 80);
  
  std::string interface_color = input.property("interface_color");
  m_content.set<Component::Text> ("interface:color", interface_color);

  input = input.next_child();
  check (input.name() == "load", "Init file should load room");
  return input.property("resources/", "room", ".xml");
}

void IO::read_character (const std::string& file_name, int x, int y)
{
  Core::IO::Element input (local_file_name(file_name));
  check (input.name() == "sosage_character", file_name + " is not a Sosage character.");

  std::string name = input.property("name");
  input = input.next_child();
  check (input.name() == "head", "Expected head, got " + input.name());
  
  std::string head = input.property("sprites/", "skin", ".png");
  int dx_right = input.int_property("dx_right");
  int dx_left = input.int_property("dx_left");
  int dy = input.int_property("dy");
  
  input = input.next().next();
  check (input.name() == "body", "Expected body, got " + input.name());
  
  std::string body = input.property("sprites/", "skin", ".png");
  
  Component::Animation_handle abody
    = m_content.set<Component::Animation>("character_body:image", local_file_name(body),
                                          0, 8, 6);
  abody->set_relative_origin(0.5, 0.95);
  
  Component::Animation_handle ahead
    = m_content.set<Component::Animation>("character_head:image", local_file_name(head),
                                          0, 7, 2);
  ahead->set_relative_origin(0.5, 1.0);
  
  Component::Position_handle pbody
    = m_content.set<Component::Position>("character_body:position", Point(x, y));

  m_content.set<Component::Position>("character_head:gap_right", Point(dx_right,dy));
  m_content.set<Component::Position>("character_head:gap_left", Point(dx_left,dy));
  
  Component::Position_handle phead
    = m_content.set<Component::Position>("character_head:position", Point(x - dx_right, y - dy));
  
  Component::Ground_map_handle ground_map
    = m_content.get<Component::Ground_map>("background:ground_map");
  
  Point pos_body = pbody->value();

  double z_at_point = ground_map->z_at_point (pos_body);
  abody->rescale (z_at_point);
  ahead->rescale (z_at_point);
  ahead->z() += 1;
  phead->set (pbody->value() - abody->core().scaling * Vector(dx_right, dy));

}

std::string IO::read_room (const std::string& file_name)
{
  Timer t ("Room reading");
  t.start();
  Core::IO::Element input (local_file_name(file_name));
  check (input.name() == "sosage_room", file_name + " is not a Sosage room.");

  std::string name = input.property("name");
  std::string background = input.property("backgrounds/", "background", ".png");
  std::string ground_map = input.property("backgrounds/", "ground_map", ".png");
  int front_z = input.int_property("front_z");
  int back_z = input.int_property("back_z");
  
  m_content.set<Component::Image>("background:image", local_file_name(background), 0);
  m_content.set<Component::Position>("background:position", Point(0, 0));
  m_content.set<Component::Ground_map>("background:ground_map", local_file_name(ground_map),
                                       front_z, back_z);

  std::string character = input.property("resources/", "character", ".xml");
  int x = input.int_property("x");
  int y = input.int_property("y");
  read_character (character, x, y);
  
  input = input.next_child();

  do
  {
    if (input.name() == "boolean")
    {
      std::string id = input.property("id");
      bool value = input.bool_property("value");
      m_content.set<Component::Boolean>(id + ":boolean", value);
    }
    if (input.name() == "object")
    {
      std::string id = input.property("id");
      std::string name = input.property("name");
      int x = input.int_property("x");
      int y = input.int_property("y");
      int z = input.int_property("z");
      
      m_content.set<Component::Text>(id + ":name", name);
      Component::State_handle state_handle
        = m_content.set<Component::State>(id + ":state");
      Component::Position_handle pos
        = m_content.set<Component::Position>(id + ":position", Point(x,y));

      Component::State_conditional_handle conditional_handle;

      Core::IO::Element child = input.next_child();
      do
      {
        if (child.name() == "text")
          continue;
        
        check (child.name() == "state", "Expected state for  " + id + ", got " + child.name());
        std::string state = child.property("id");
        std::string skin = child.property("sprites/", "skin", ".png");

        // init with first state found
        bool init = false;
        if (state_handle->value() == "")
        {
          state_handle->set(state);
          conditional_handle = m_content.set<Component::State_conditional>(id + ":conditional_image", state_handle);
          init = true;
        }
        else
          conditional_handle = m_content.get<Component::State_conditional>(id + ":conditional_image");
    
        Component::Image_handle img
          = Component::make_handle<Component::Image>(id + ":image", local_file_name(skin), 0);
        img->set_relative_origin(0.5, 1.0);

        img->z() = z;
        debug("Object " + id + ":" + state + " at position " + std::to_string(img->z()));

        conditional_handle->add(state, img);
      }
      while ((child = child.next()));

    }
    else if (input.name() == "action")
    {
      std::string id = input.property("id");
      std::string target = input.property("target");

      Component::Action_handle action
        = m_content.set<Component::Action>(target + ":" + id);
      
      Core::IO::Element child = input.next_child();
      do
      {
        if (child.name() == "text")
          continue;

        if (child.name() == "comment")
          action->add ({ child.name(),
                child.property("text") });
        else if (child.name() == "move")
          action->add ({ child.name(),
                child.property("target"),
                child.property("x"),
                child.property("y"),
                child.property("z") });
        else if (child.name() == "pick_animation")
          action->add ({ child.name(),
                child.property("duration") });
        else if (child.name() == "set_state")
          action->add ({ child.name(),
                child.property("target"),
                child.property("state") });
        else if (child.name() == "sync")
          action->add ({ child.name() });
      }
      while ((child = child.next()));
    }
    else if (input.name() == "npc")
    {

    }
    else if (input.name() == "scenery")
    {
      std::string id = input.property("id");
      int x = input.int_property("x");
      int y = input.int_property("y");
      int z = input.int_property("z");
      std::string skin = input.property("sprites/", "skin", ".png");
      
      Component::Position_handle pos
        = m_content.set<Component::Position>(id + ":position", Point(x,y));
      Component::Image_handle img
        = m_content.set<Component::Image>(id + ":image", local_file_name(skin), 0);
      img->set_relative_origin(0.5, 1.0);
      img->z() = z;
      debug("Scenery " + id + " at position " + std::to_string(img->z()));
    }
  }
  while ((input = input.next()));

  t.stop();
  return std::string();
}

std::string IO::local_file_name (const std::string& file_name) const
{
  return m_folder_name + file_name;
}

} // namespace Sosage::System
