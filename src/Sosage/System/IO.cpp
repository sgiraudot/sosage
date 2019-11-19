#include <Sosage/Component/Animation.h>
#include <Sosage/Component/Font.h>
#include <Sosage/Component/Ground_map.h>
#include <Sosage/Component/Image.h>
#include <Sosage/Component/Position.h>
#include <Sosage/Component/Text.h>
#include <Sosage/System/IO.h>
#include <Sosage/version.h>

namespace Sosage::System
{

IO::IO (Content& content)
  : m_content (content)
{
}

std::string IO::read_init (const std::string& folder_name)
{
  m_folder_name = folder_name;
  std::string file_name = folder_name + "init.xml";
  
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
  return input.property("rooms/", "room", ".xml");
}

std::string IO::read_room (const std::string& file_name)
{
  Core::IO::Element input (local_file_name(file_name));
  check (input.name() == "sosage_room", file_name + " is not a Sosage room.");

  std::string name = input.property("name");
  std::string background = input.property("backgrounds/", "background", ".png");
  std::string ground_map = input.property("backgrounds/", "ground_map", ".png");
  
  m_content.set<Component::Image>("background:image", local_file_name(background), 0);
  m_content.set<Component::Position>("background:position", Point(0, 0));
  m_content.set<Component::Ground_map>("background:ground_map", local_file_name(ground_map));

  std::string body = input.property("sprites/", "body", ".png");
  std::string head = input.property("sprites/", "head", ".png");
  int x = input.int_property("x");
  int y = input.int_property("y");

  {
    Component::Animation_handle abody
      = m_content.set<Component::Animation>("character_body:image", local_file_name(body),
                                            0, 9, 5);
    abody->origin() = Point (abody->width() / 2, 0.9 * abody->height());
  
    Component::Animation_handle ahead
      = m_content.set<Component::Animation>("character_head:image", local_file_name(head),
                                            0, 7, 2);
    ahead->origin() = Point (ahead->width() / 2, ahead->height());
  
    Component::Position_handle pbody
      = m_content.set<Component::Position>("character_body:position", Point(x, y));

  
    Component::Position_handle phead
      = m_content.set<Component::Position>("character_head:position", Point(x, y - 290));
  
    Component::Ground_map_handle ground_map
      = m_content.get<Component::Ground_map>("background:ground_map");
  
    Point pos_body = pbody->value();

    // abody->z() = 1;
    // ahead->z() = 2;
    double z_at_point = ground_map->z_at_point (pos_body);
    abody->rescale (z_at_point);
    ahead->rescale (z_at_point);
    ahead->z() += 1;
    phead->set (pbody->value() - abody->core().second * Vector(0, 290));

  }
  
  input = input.next_child();

  do
  {
    if (input.name() == "object")
    {
      std::string id = input.property("id");
      std::string name = input.property("name");
      int x = input.int_property("x");
      int y = input.int_property("y");
      
      m_content.set<Component::Text>(id + ":name", name);
      Component::State_handle state_handle
        = m_content.set<Component::State>(id + ":state");
      Component::Position_handle pos
        = m_content.set<Component::Position>(id + ":position", Point(x,y));

      Component::State_conditional_handle conditional_handle;

      input = input.next_child();
//      do
      {
        check (input.name() == "state", "Expected state for  " + id + ", got " + input.name());
        std::string state = input.property("id");
        std::string skin = input.property("sprites/", "skin", ".png");

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
        img->origin() = Point (img->width() / 2, img->height());

        if (init)
          pos->set (Point(pos->value().x() + img->width() / 2, pos->value().y() + img->height()));

        Component::Ground_map_handle ground_map = m_content.get<Component::Ground_map>("background:ground_map");
  
        img->z() = ground_map->z_at_point (pos->value()) - 1;
        debug("Object " + id + ":" + state + " at position " + std::to_string(img->z()));

        conditional_handle->add(state, img);
      }
//      while ((input = input.next()));

    }
    else if (input.name() == "npc")
    {

    }
    else if (input.name() == "scenery")
    {

    }
  }
  while ((input = input.next()));
  
}

std::string IO::read_character (const std::string& file_name)
{

}


} // namespace Sosage::System
