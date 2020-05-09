/*
  [src/Sosage/System/File_IO.cpp]
  Reads levels/savegames/configs, writes savegames/config.

  =====================================================================

  This file is part of SOSAGE.

  SOSAGE is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  SOSAGE is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with SOSAGE.  If not, see <https://www.gnu.org/licenses/>.

  =====================================================================

  Author(s): Simon Giraudot <sosage@ptilouk.net>
*/

#include <Sosage/Component/Action.h>
#include <Sosage/Component/Animation.h>
#include <Sosage/Component/Code.h>
#include <Sosage/Component/Cropped.h>
#include <Sosage/Component/Font.h>
#include <Sosage/Component/Ground_map.h>
#include <Sosage/Component/Image.h>
#include <Sosage/Component/Inventory.h>
#include <Sosage/Component/Music.h>
#include <Sosage/Component/Position.h>
#include <Sosage/Component/Sound.h>
#include <Sosage/Component/Text.h>
#include <Sosage/System/File_IO.h>
#include <Sosage/platform.h>
#include <Sosage/version.h>
#include <Sosage/Utils/color.h>
#include <Sosage/Utils/profiling.h>

namespace Sosage::System
{

File_IO::File_IO (Content& content)
  : m_content (content)
{
}

std::string File_IO::read_init (const std::string& folder_name)
{
  m_folder_name = folder_name;
  std::string file_name = folder_name + "data" + Sosage::folder_separator + "init.yaml";

  Core::File_IO input (file_name);

  std::string v = input["version"].string();
  check (version::parse(v) <= version::get(),
         "Error: room version " + v + " incompatible with Sosage " + version::str());

#ifndef SOSAGE_ANDROID
  std::string cursor = input["cursor"].string("images", "interface", "png");
  auto cursor_img = m_content.set<Component::Image> ("cursor:image", local_file_name(cursor),
                                                     Sosage::cursor_depth);
  cursor_img->set_relative_origin(0.5, 0.5);
#endif
  
  m_content.set<Component::Position> ("cursor:position", Point(0,0));
  
  std::string turnicon = input["turnicon"].string("images", "interface", "png");
  auto turnicon_img
    = m_content.set<Component::Image>("turnicon:image", local_file_name(turnicon), 0);
  turnicon_img->on() = false;

  std::string click_sound = input["click_sound"].string("sounds", "effects", "wav");
  m_content.set<Component::Sound>("click:sound", local_file_name(click_sound));
  
  std::string debug_font = input["debug_font"].string("fonts", "ttf");
  m_content.set<Component::Font> ("debug:font", local_file_name(debug_font), 40);

  std::string interface_font = input["interface_font"].string("fonts", "ttf");
  m_content.set<Component::Font> ("interface:font", local_file_name(interface_font), 80);
  
  std::string interface_color = input["interface_color"].string();
  m_content.set<Component::Text> ("interface:color", interface_color);

  std::array<unsigned char, 3> color = color_from_string (interface_color);

  for (std::size_t i = 0; i < input["inventory_arrows"].size(); ++ i)
  {
    std::string id = input["inventory_arrows" ][i].string("images", "interface", "png");
    auto arrow
      = m_content.set<Component::Image> ("inventory_arrow_" + std::to_string(i) + ":image",
                                         local_file_name(id),
                                         Sosage::inventory_front_depth);
    arrow->set_relative_origin(0.5, 0.5);
    auto arrow_background
      = m_content.set<Component::Image> ("inventory_arrow_background_" + std::to_string(i)
                                         + ":image",
                                         arrow->width(), arrow->height(),
                                         color[0], color[1], color[2]);
    arrow_background->set_relative_origin(0.5, 0.5);
    arrow_background->z() = Sosage::inventory_back_depth;
  }

  for (std::size_t i = 0; i < input["actions"].size(); ++ i)
  {
    const Core::File_IO::Node& idefault = input["actions"][i];
    std::string id = idefault["id"].string();

    auto action = m_content.set<Component::Random_conditional>("default:" + id);

    for (std::size_t j = 0; j < idefault["effect"].size(); ++ j)
    {
      const Core::File_IO::Node& iaction = idefault["effect"][j];
      auto rnd_action = Component::make_handle<Component::Action>
        ("default:" + id + "_" + std::to_string(j));
      rnd_action->add ({ "look" });
      rnd_action->add (iaction.string_array());

      action->add (1.0, rnd_action);
    }

  }
    
  return input["load_room"].string("data", "rooms", "yaml");
}

void File_IO::read_character (const std::string& file_name, int x, int y)
{
  Core::File_IO input (local_file_name(file_name));

  std::string name = input["name"].string();

  std::string mouth = input["mouth"]["skin"].string("images", "characters", "png");
  int mdx_right = input["mouth"]["dx_right"].integer();
  int mdx_left = input["mouth"]["dx_left"].integer();
  int mdy = input["mouth"]["dy"].integer();
  
  std::string head = input["head"]["skin"].string("images", "characters", "png");
  int hdx_right = input["head"]["dx_right"].integer();
  int hdx_left = input["head"]["dx_left"].integer();
  int hdy = input["head"]["dy"].integer();
  
  std::string body = input["body"]["skin"].string("images", "characters", "png");
  
  auto abody = m_content.set<Component::Animation>("character_body:image", local_file_name(body),
                                                   0, 8, 6, true);
  abody->set_relative_origin(0.5, 0.95);
  
  auto ahead
    = m_content.set<Component::Animation>("character_head:image", local_file_name(head),
                                          0, 7, 2, true);
  ahead->set_relative_origin(0.5, 1.0);
  
  auto amouth
    = m_content.set<Component::Animation>("character_mouth:image", local_file_name(mouth),
                                          0, 11, 2, true);
  amouth->set_relative_origin(0.5, 1.0);
  
  auto pbody = m_content.set<Component::Position>("character_body:position", Point(x, y));

  m_content.set<Component::Position>("character_head:gap_right", Point(hdx_right,hdy));
  m_content.set<Component::Position>("character_head:gap_left", Point(hdx_left,hdy));

  m_content.set<Component::Position>("character_mouth:gap_right", Point(mdx_right,mdy));
  m_content.set<Component::Position>("character_mouth:gap_left", Point(mdx_left,mdy));
  
  auto phead
    = m_content.set<Component::Position>("character_head:position", Point(x - hdx_right, y - hdy));

  auto pmouth
    = m_content.set<Component::Position>("character_mouth:position", Point(x - hdx_right - mdx_right,
                                                                           y - hdy - mdy));

  auto ground_map = m_content.get<Component::Ground_map>("background:ground_map");
  
  Point pos_body = pbody->value();

  double z_at_point = ground_map->z_at_point (pos_body);
  abody->rescale (z_at_point);
  
  ahead->rescale (z_at_point);
  ahead->z() += 1;
  phead->set (pbody->value() - abody->core().scaling * Vector(hdx_right, hdy));
  
  amouth->rescale (z_at_point);
  amouth->z() += 2;
  pmouth->set (phead->value() - ahead->core().scaling * Vector(mdx_right, mdy));

}

std::string File_IO::read_room (const std::string& file_name)
{
  Timer t ("Room reading");
  t.start();
  Core::File_IO input (local_file_name(file_name));

  std::string name = input["name"].string();
  std::string music = input["music"].string("sounds", "musics", "ogg");
  m_content.set<Component::Music>("game:music", local_file_name(music));
  
  std::string background = input["background"].string("images", "backgrounds", "png");
  std::string ground_map = input["ground_map"].string("images", "backgrounds", "png");
  int front_z = input["front_z"].integer();
  int back_z = input["back_z"].integer();

  auto background_img
    = m_content.set<Component::Image>("background:image", local_file_name(background), 0);
  background_img->box_collision() = true;
  
  m_content.set<Component::Position>("background:position", Point(0, 0));
  m_content.set<Component::Ground_map>("background:ground_map", local_file_name(ground_map),
                                       front_z, back_z);

  std::string character = input["character"].string("data", "characters", "yaml");
  int x = input["coordinates"][0].integer();
  int y = input["coordinates"][1].integer();
  read_character (character, x, y);

  for (std::size_t i = 0; i < input["content"].size(); ++ i)
  {
    const Core::File_IO::Node& node = input["content"][i];
    std::string id = node["id"].string();
    std::string type = node["type"].string();

    if (type == "animation")
      read_animation (node, id);
    else if (type == "code")
      read_code (node, id);
    else if (type == "object")
      read_object (node, id);
    else if (type == "scenery")
      read_scenery (node, id);
    else if (type == "window")
      read_window (node, id);
    else
      check (false, "Unknown content type " + type);
  }
  
  t.stop();
  return std::string();
}

std::string File_IO::local_file_name (const std::string& file_name) const
{
  return m_folder_name + file_name;
}


void File_IO::read_animation (const Core::File_IO::Node& node, const std::string& id)
{
  int x = node["coordinates"][0].integer();
  int y = node["coordinates"][1].integer();
  int z = node["coordinates"][2].integer();
  std::string skin = node["skin"].string("images", "animations", "png");
  int length = node["length"].integer();
      
  auto pos = m_content.set<Component::Position>(id + ":position", Point(x,y));
  auto img = m_content.set<Component::Animation>(id + ":image", local_file_name(skin), z,
                                                 length, 1, false);
  img->on() = false;
  img->set_relative_origin(0.5, 1.0);
  debug("Animation " + id + " at position " + std::to_string(img->z()));
}

void File_IO::read_code (const Core::File_IO::Node& node, const std::string& id)
{
  std::string button_sound = node["button_sound"].string("sounds", "effects", "wav");
  m_content.set<Component::Sound>(id + "_button:sound", local_file_name(button_sound));
  std::string success_sound = node["success_sound"].string("sounds", "effects", "wav");
  m_content.set<Component::Sound>(id + "_success:sound", local_file_name(success_sound));
  std::string failure_sound = node["failure_sound"].string("sounds", "effects", "wav");
  m_content.set<Component::Sound>(id + "_failure:sound", local_file_name(failure_sound));

  auto code = m_content.set<Component::Code>(id + ":code");
      
  auto state_handle = m_content.set<Component::State>(id + ":state");
  Component::State_conditional_handle conditional_handle_off;
  Component::State_conditional_handle conditional_handle_on;

  for (std::size_t j = 0; j < node["states"].size(); ++ j)
  {
    const Core::File_IO::Node& istate = node["states"][j];

    std::string state = istate["id"].string();

    // init with first state found
    bool init = false;
    if (state_handle->value() == "")
    {
      state_handle->set(state);
      conditional_handle_off
        = m_content.set<Component::State_conditional>(id + ":image", state_handle);
      conditional_handle_on
        = m_content.set<Component::State_conditional>(id + "_button:image", state_handle);
      init = true;
    }
    else
    {
      conditional_handle_off = m_content.get<Component::State_conditional>(id + ":image");
      conditional_handle_on = m_content.get<Component::State_conditional>(id + "_button:image");
    }

    std::string skin_off = istate["skin"][0].string("images", "windows", "png");
    auto img_off
      = Component::make_handle<Component::Image>(id + ":conditional_image", local_file_name(skin_off),
                                                 Sosage::inventory_back_depth);
    img_off->set_relative_origin(0.5, 0.5);
    img_off->on() = false;
        
    m_content.set<Component::Position>(id + ":position",
                                       Point(Sosage::world_width / 2,
                                             Sosage::world_height / 2));

    std::string skin_on = istate["skin"][1].string("images", "windows", "png");
    auto img_on
      = Component::make_handle<Component::Cropped>(id + "_button:conditional_image",
                                                   local_file_name(skin_on),
                                                   Sosage::inventory_front_depth);
    img_on->set_relative_origin(0.5, 0.5);
    img_on->on() = false;

    m_content.set<Component::Position>(id + "_button:position",
                                       Point(Sosage::world_width / 2,
                                             Sosage::world_height / 2));

    conditional_handle_off->add(state, img_off);
    conditional_handle_on->add(state, img_on);
  }
      
  for (std::size_t j = 0; j < node["buttons"].size(); ++ j)
  {
    const Core::File_IO::Node& ibutton = node["buttons"][j];

    std::string value = ibutton["value"].string();

    const Core::File_IO::Node& coordinates = ibutton["coordinates"];
    code->add_button (value,
                      coordinates[0].integer(),
                      coordinates[1].integer(),
                      coordinates[2].integer(),
                      coordinates[3].integer());
  }
      
  for (std::size_t j = 0; j < node["answer"].size(); ++ j)
    code->add_answer_item (node["answer"][j].string());
      
  auto action = m_content.set<Component::Action> (id + ":action");
  for (std::size_t j = 0; j < node["on_success"].size(); ++ j)
    action->add (node["on_success"][j].string_array());
}

void File_IO::read_object (const Core::File_IO::Node& node, const std::string& id)
{
  std::string name = node["name"].string();
  int x = node["coordinates"][0].integer();
  int y = node["coordinates"][1].integer();
  int z = node["coordinates"][2].integer();
  int vx = node["view"][0].integer();
  int vy = node["view"][1].integer();
  bool box_collision = node["box_collision"].boolean();
      
  m_content.set<Component::Text>(id + ":name", name);
  auto state_handle = m_content.set<Component::State>(id + ":state");
  auto pos = m_content.set<Component::Position>(id + ":position", Point(x,y));
  m_content.set<Component::Position>(id + ":view", Point(vx,vy));

  Component::State_conditional_handle conditional_handle;

  for (std::size_t j = 0; j < node["states"].size(); ++ j)
  {
    const Core::File_IO::Node& istate = node["states"][j];

    std::string state = istate["id"].string();

    // init with first state found
    bool init = false;
    if (state_handle->value() == "")
    {
      state_handle->set(state);
      conditional_handle = m_content.set<Component::State_conditional>(id + ":image", state_handle);
      init = true;
    }
    else
      conditional_handle = m_content.get<Component::State_conditional>(id + ":image");

    if (state == "none")
      conditional_handle->add(state, nullptr);
    else
    {
      std::string skin;
      if (state == "inventory")
        skin = istate["skin"].string("images", "inventory", "png");
      else
        skin = istate["skin"].string("images", "objects", "png");
          
      auto img
        = Component::make_handle<Component::Image>(id + ":conditional_image", local_file_name(skin), z);
      img->set_relative_origin(0.5, 1.0);
      img->box_collision() = box_collision;

      debug("Object " + id + ":" + state + " at position " + std::to_string(img->z()));

      conditional_handle->add(state, img);
    }
  }
      
  if (state_handle->value() == "inventory")
  {
    m_content.get<Component::Inventory>("game:inventory")->add(id);
    auto img
      = m_content.get<Component::Image>(id + ":image");
    img->set_relative_origin(0.5, 0.5);
    img->z() = Sosage::inventory_back_depth;
  }

  for (std::size_t i = 0; i < node["actions"].size(); ++ i)
  {
    const Core::File_IO::Node& iaction = node["actions"][i];

    std::size_t nb_actions = iaction["id"].size();
    std::size_t j = 0;
    do
    {
      std::string a_id = (nb_actions == 0 ? iaction["id"].string() : iaction["id"][j].string());
      
      if (iaction.has("target"))
      {
        a_id = a_id + "_" + iaction["target"].string();
        std::cerr << id << ":" << a_id << std::endl;
      }

      Component::Action_handle action;
              
      if (iaction.has("state"))
      {
        std::string state = iaction["state"].string();
        auto conditional_handle
          = m_content.request<Component::State_conditional>(id + ":" + a_id);

        if (!conditional_handle)
        {
          auto state_handle
            = m_content.get<Component::State>(id + ":state");
          conditional_handle
            = m_content.set<Component::State_conditional>(id + ":" + a_id, state_handle);
        }

        action = Component::make_handle<Component::Action>(id + ":" + a_id + ":" + state);
        conditional_handle->add (state, action);
      }
      else
        action = m_content.set<Component::Action>(id + ":" + a_id);


      for (std::size_t k = 0; k < iaction["effect"].size(); ++ k)
        action->add (iaction["effect"][k].string_array());

      ++ j;
    }
    while (j < nb_actions);

  }
}

void File_IO::read_scenery (const Core::File_IO::Node& node, const std::string& id)
{
  int x = node["coordinates"][0].integer();
  int y = node["coordinates"][1].integer();
  int z = node["coordinates"][2].integer();
  std::string skin = node["skin"].string("images", "scenery", "png");
      
  auto pos = m_content.set<Component::Position>(id + ":position", Point(x,y));
  auto img = m_content.set<Component::Image>(id + ":image", local_file_name(skin), z);
  img->set_relative_origin(0.5, 1.0);
  debug("Scenery " + id + " at position " + std::to_string(img->z()));
}

void File_IO::read_window (const Core::File_IO::Node& node, const std::string& id)
{
  std::string skin = node["skin"].string("images", "windows", "png");
      
  auto img = m_content.set<Component::Image>(id + ":image", local_file_name(skin),
                                             Sosage::inventory_front_depth);
  img->set_relative_origin(0.5, 0.5);
  img->on() = false;
      
  m_content.set<Component::Position>(id + ":position",
                                     Point(Sosage::world_width / 2,
                                           Sosage::world_height / 2));
}

} // namespace Sosage::System
