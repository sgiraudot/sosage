/*
  [src/Sosage/System/File_IO__read_room.cpp]
  Reads levels.

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
#include <Sosage/Component/Event.h>
#include <Sosage/Component/Font.h>
#include <Sosage/Component/Ground_map.h>
#include <Sosage/Component/Hints.h>
#include <Sosage/Component/Image.h>
#include <Sosage/Component/Inventory.h>
#include <Sosage/Component/Music.h>
#include <Sosage/Component/Position.h>
#include <Sosage/Component/Simple.h>
#include <Sosage/Component/Sound.h>
#include <Sosage/Component/Status.h>
#include <Sosage/Component/Variable.h>
#include <Sosage/Config/platform.h>
#include <Sosage/Config/version.h>
#include <Sosage/System/File_IO.h>
#include <Sosage/Utils/color.h>
#include <Sosage/Utils/profiling.h>
#include <Sosage/Utils/file.h>

namespace Sosage::System
{

void File_IO::read_character (const Core::File_IO::Node& node, const std::string& id)
{
  std::string file_name = node["id"].string("data", "characters", "yaml");
  int x = node["coordinates"][0].integer();
  int y = node["coordinates"][1].integer();
  bool looking_right = node["looking_right"].boolean();

  if (node.has("actions"))
    read_actions(node, id);

  Core::File_IO input (local_file_name(file_name));
  input.parse();

  std::string name = input["name"].string();
  m_content.set<Component::String> (id + ":name", name);

  std::string color = input["color"].string();
  m_content.set<Component::String> (id + ":color", color);

  std::string mouth = input["mouth"]["skin"].string("images", "characters", "png");
  auto amouth
    = m_content.set<Component::Animation>(id + "_mouth:image", local_file_name(mouth),
                                          0, 11, 2, true);
  amouth->set_relative_origin(0.5, 1.0);

  std::string head = input["head"]["skin"].string("images", "characters", "png");
  auto ahead
    = m_content.set<Component::Animation>(id + "_head:image", local_file_name(head),
                                          0, 7, 2, true);
  ahead->set_relative_origin(0.5, 1.0);

  std::string walk = input["walk"]["skin"].string("images", "characters", "png");
  auto awalk = m_content.set<Component::Animation>(id + "_walking:image", local_file_name(walk),
                                                   0, 8, 4, true);
  awalk->set_relative_origin(0.5, 0.95);
  awalk->on() = false;

  std::string idle = input["idle"]["skin"].string("images", "characters", "png");
  std::vector<std::string> positions;
  for (std::size_t i = 0; i < input["idle"]["positions"].size(); ++ i)
    positions.push_back (input["idle"]["positions"][i].string());

  m_content.set<Component::Vector<std::string> >(id + "_idle:values", positions);

  auto aidle = m_content.set<Component::Animation>(id + "_idle:image", local_file_name(idle),
                                                   0, positions.size(), 2, true);
  aidle->set_relative_origin(0.5, 0.95);

  auto pbody = m_content.set<Component::Position>(id + "_body:position", Point(x, y), false);
  m_content.set<Component::Variable>(id + "_walking:position", pbody);
  m_content.set<Component::Variable>(id + "_idle:position", pbody);

  int hdx_right = input["head"]["dx_right"].integer();
  int hdx_left = input["head"]["dx_left"].integer();
  int hdy = input["head"]["dy"].integer();
  m_content.set<Component::Position>(id + "_head:gap_right", Point(hdx_right,hdy), false);
  m_content.set<Component::Position>(id + "_head:gap_left", Point(hdx_left,hdy), false);

  int mdx_right = input["mouth"]["dx_right"].integer();
  int mdx_left = input["mouth"]["dx_left"].integer();
  int mdy = input["mouth"]["dy"].integer();
  m_content.set<Component::Position>(id + "_mouth:gap_right", Point(mdx_right,mdy), false);
  m_content.set<Component::Position>(id + "_mouth:gap_left", Point(mdx_left,mdy), false);
  m_content.set<Component::Position>(id + "_head:position", Point(x - hdx_right, y - hdy), false);
  m_content.set<Component::Position>(id + "_mouth:position", Point(x - hdx_right - mdx_right,
                                                                       y - hdy - mdy), false);

  auto new_char = m_content.request<Component::Vector<std::pair<std::string, bool> > >("game:new_characters");
  if (!new_char)
    new_char = m_content.set<Component::Vector<std::pair<std::string, bool> > >("game:new_characters");

  new_char->push_back (std::make_pair (id, looking_right));
}

void File_IO::read_room (const std::string& file_name)
{
  std::unordered_set<std::string> force_keep;
  auto inventory = m_content.get<Component::Inventory>("game:inventory");
  for (const std::string& entity : *inventory)
    force_keep.insert (entity);

  const std::string& player = m_content.get<Component::String>("player:name")->value();
  force_keep.insert (player + "_body");
  force_keep.insert (player + "_head");
  force_keep.insert (player + "_mouth");
  force_keep.insert (player + "_walking");
  force_keep.insert (player + "_idle");

  m_content.clear
    ([&](Component::Handle c) -> bool
     {
       // keep inventory + other forced kept
       if (force_keep.find(c->entity()) != force_keep.end())
         return false;

       // keep states and positions
       if (c->component() == "state" || c->component() == "position")
         return false;

       // else, remove component if belonged to the latest room
       return (m_latest_room_entities.find(c->entity()) != m_latest_room_entities.end());
     });

  SOSAGE_TIMER_START(File_IO__read_room);

  Core::File_IO input (local_file_name("data", "rooms", file_name, "yaml"));
  input.parse();

  std::string name = input["name"].string();
  std::string music = input["music"].string("sounds", "musics", "ogg");
  m_content.set<Component::Music>("game:music", local_file_name(music));

  std::string background = input["background"].string("images", "backgrounds", "png");
  std::string ground_map = input["ground_map"].string("images", "backgrounds", "png");
  int front_z = input["front_z"].integer();
  int back_z = input["back_z"].integer();

  auto background_img
    = m_content.set<Component::Image>("background:image", local_file_name(background), 0);
  background_img->collision() = BOX;

  m_content.set<Component::Position>("background:position", Point(0, 0), false);
  m_content.set<Component::Ground_map>("background:ground_map", local_file_name(ground_map),
                                       front_z, back_z);

  // First instantiate all states
  for (std::size_t i = 0; i < input["content"].size(); ++ i)
  {
    const Core::File_IO::Node& node = input["content"][i];
    std::string id = node["id"].string();
    m_latest_room_entities.insert (id);
    if(node["type"].string() == "character")
    {
      m_latest_room_entities.insert (id + "_body");
      m_latest_room_entities.insert (id + "_head");
      m_latest_room_entities.insert (id + "_mouth");
      m_latest_room_entities.insert (id + "_idle");
      m_latest_room_entities.insert (id + "_walking");
    }

    if (node.has("states"))
      // Add state if does not exist (it might for inventory objects for example)
      if (!m_content.request<Component::String>(id + ":state"))
        m_content.set<Component::String>(id + ":state");
  }

  for (std::size_t i = 0; i < input["content"].size(); ++ i)
  {
    const Core::File_IO::Node& node = input["content"][i];
    std::string id = node["id"].string();
    std::string type = node["type"].string();

    if (type == "animation")
      read_animation (node, id);
    else if (type == "character")
      read_character (node, id);
    else if (type == "code")
      read_code (node, id);
    else if (type == "object")
      read_object (node, id);
    else if (type == "origin")
      read_origin (node, id);
    else if (type == "scenery")
      read_scenery (node, id);
    else if (type == "window")
      read_window (node, id);
    else
      debug ("Unknown content type " + type);
  }

  auto hints = m_content.set<Component::Hints>("game:hints");

  if (input.has("hints"))
    for (std::size_t i = 0; i < input["hints"].size(); ++ i)
    {
      const Core::File_IO::Node& node = input["hints"][i];
      std::string id = node["id"].string();
      std::string state = node["state"].string();
      std::string text = node["text"].string();

      auto condition = Component::make_handle<Component::String_conditional>
        ("hint:condition", m_content.get<Component::String>(id + ":state"));
      condition->add(state, Component::make_handle<Component::String>("hint:text", text));
      hints->add (condition);
    }


  const std::string& origin = m_content.get<Component::String>("game:new_room_origin")->value();
  auto origin_coord = m_content.get<Component::Position>(origin + ":position");
  auto origin_looking = m_content.get<Component::Boolean>(origin + ":looking_right");

  m_content.get<Component::Position>(player + "_body:position")->set(origin_coord->value());
  m_content.set<Component::Boolean>("game:in_new_room", origin_looking->value());

  m_content.remove ("game:new_room");
  m_content.remove ("game:new_room_origin");

  m_content.set<Component::Event>("music:start");
  m_content.get<Component::Status>(GAME__STATUS)->pop();

  m_thread.notify();

  SOSAGE_TIMER_STOP(File_IO__read_room);
}

void File_IO::read_animation (const Core::File_IO::Node& node, const std::string& id)
{
  int x = node["coordinates"][0].integer();
  int y = node["coordinates"][1].integer();
  int z = node["coordinates"][2].integer();
  std::string skin = node["skin"].string("images", "animations", "png");
  int length = node["length"].integer();

  auto pos = m_content.set<Component::Position>(id + ":position", Point(x,y), false);
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

  auto state_handle = m_content.get<Component::String>(id + ":state");
  Component::String_conditional_handle conditional_handle_off;
  Component::String_conditional_handle conditional_handle_on;

  for (std::size_t j = 0; j < node["states"].size(); ++ j)
  {
    const Core::File_IO::Node& istate = node["states"][j];

    std::string state = istate["id"].string();

    // init with first state found
    if (j == 0)
    {
      if (state_handle->value() == "")
        state_handle->set(state);
      conditional_handle_off
        = m_content.set<Component::String_conditional>(id + ":image", state_handle);
      conditional_handle_on
        = m_content.set<Component::String_conditional>(id + "_button:image", state_handle);
    }
    else
    {
      conditional_handle_off = m_content.get<Component::String_conditional>(id + ":image");
      conditional_handle_on = m_content.get<Component::String_conditional>(id + "_button:image");
    }

    std::string skin_off = istate["skin"][0].string("images", "windows", "png");
    auto img_off
      = Component::make_handle<Component::Image>(id + ":conditional_image", local_file_name(skin_off),
                                                 Config::inventory_back_depth);
    img_off->set_relative_origin(0.5, 0.5);
    img_off->on() = false;

    m_content.set<Component::Position>(id + ":position",
                                       Point(Config::world_width / 2,
                                             Config::world_height / 2));

    std::string skin_on = istate["skin"][1].string("images", "windows", "png");
    auto img_on
      = Component::make_handle<Component::Cropped>(id + "_button:conditional_image",
                                                   local_file_name(skin_on),
                                                   Config::inventory_front_depth);
    img_on->set_relative_origin(0.5, 0.5);
    img_on->on() = false;

    m_content.set<Component::Position>(id + "_button:position",
                                       Point(Config::world_width / 2,
                                             Config::world_height / 2));

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
  // First, check if object already exists in inventory (if so, skip)
  auto state_handle = m_content.get<Component::String>(id + ":state");
  if (state_handle->value() == "inventory")
  {
    std::cerr << "Skipping " << id << std::endl;
    return;
  }

  std::string name = node["name"].string();
  int x = node["coordinates"][0].integer();
  int y = node["coordinates"][1].integer();
  int z = node["coordinates"][2].integer();
  int vx = node["view"][0].integer();
  int vy = node["view"][1].integer();
  bool box_collision = node["box_collision"].boolean();

  m_content.set<Component::String>(id + ":name", name);

  // Position might already exists if room was already loaded
  auto pos = m_content.request<Component::Position>(id + ":position");
  if (!pos)
    pos = m_content.set<Component::Position>(id + ":position", Point(x,y), false);

  m_content.set<Component::Position>(id + ":view", Point(vx,vy), false);

  Component::String_conditional_handle conditional_handle;
  for (std::size_t j = 0; j < node["states"].size(); ++ j)
  {
    const Core::File_IO::Node& istate = node["states"][j];

    std::string state = istate["id"].string();

    // init with first state found
    if (j == 0)
    {
      if (state_handle->value() == "")
        state_handle->set(state);
      conditional_handle = m_content.set<Component::String_conditional>(id + ":image", state_handle);
    }
    else
      conditional_handle = m_content.get<Component::String_conditional>(id + ":image");

    if (state == "none")
      conditional_handle->add(state, nullptr);
    else
    {
      std::string skin;
      if (state == "inventory")
        skin = istate["skin"].string("images", "inventory", "png");
      else
        skin = istate["skin"].string("images", "objects", "png");

      Component::Image_handle img;
      if (istate.has("frames")) // Animation
      {
        int nb_frames = istate["frames"].integer();
        int duration = istate["duration"].integer();
        auto anim = Component::make_handle<Component::Animation>(id + ":conditional_image",
                                                                 local_file_name(skin), z,
                                                                 nb_frames, 1, true);
        anim->reset(true, duration);
        img = anim;
      }
      else
        img = Component::make_handle<Component::Image>(id + ":conditional_image", local_file_name(skin), z);

      img->set_relative_origin(0.5, 1.0);
      img->collision() = (box_collision ? BOX : PIXEL_PERFECT);

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
    img->z() = Config::inventory_back_depth;
  }

  read_actions(node, id);

}


void File_IO::read_actions (const Core::File_IO::Node& node, const std::string& id)
{
  bool look_found = false;
  bool has_default = false;
  for (std::size_t i = 0; i < node["actions"].size(); ++ i)
  {
    const Core::File_IO::Node& iaction = node["actions"][i];

    std::size_t nb_actions = iaction["id"].size();
    std::size_t j = 0;
    do
    {
      std::string a_id = (nb_actions == 0 ? iaction["id"].string() : iaction["id"][j].string());
      if (a_id == "default")
      {
        has_default = true;
        break;
      }

      std::string corrected_id = id;

      if (iaction.has("target"))
      {
        a_id = a_id + "_" + id;
        corrected_id = iaction["target"].string();
      }

      if (a_id == "look")
        look_found = true;

      Component::Action_handle action;

      if (iaction.has("state"))
      {
        std::string state = iaction["state"].string();
        auto conditional_handle
          = m_content.request<Component::String_conditional>(corrected_id + ":" + a_id);

        if (!conditional_handle)
        {
          auto state_handle
            = m_content.get<Component::String>(corrected_id + ":state");
          conditional_handle
            = m_content.set<Component::String_conditional>(corrected_id + ":" + a_id, state_handle);
        }

        action = Component::make_handle<Component::Action>(corrected_id + ":" + a_id + ":" + state);
        conditional_handle->add (state, action);
      }
      else
        action = m_content.set<Component::Action>(corrected_id + ":" + a_id);


      for (std::size_t k = 0; k < iaction["effect"].size(); ++ k)
        action->add (iaction["effect"][k].string_array());

      ++ j;
    }
    while (j < nb_actions);
  }

  check (look_found, "Object " + id + " has no \"look\" action");
}

void File_IO::read_origin(const Core::File_IO::Node& node, const std::string& id)
{
  int x = node["coordinates"][0].integer();
  int y = node["coordinates"][1].integer();
  bool looking_right = node["looking_right"].boolean();

  m_content.set<Component::Position>(id + ":position", Point(x, y));
  m_content.set<Component::Boolean>(id + ":looking_right", looking_right);
}

void File_IO::read_scenery (const Core::File_IO::Node& node, const std::string& id)
{
  int x = node["coordinates"][0].integer();
  int y = node["coordinates"][1].integer();
  int z = node["coordinates"][2].integer();

  auto pos = m_content.set<Component::Position>(id + ":position", Point(x,y), false);

  if (node.has("states"))
  {
    Component::String_conditional_handle conditional_handle;

    auto state_handle = m_content.get<Component::String>(id + ":state");
    for (std::size_t i = 0; i < node["states"].size(); ++ i)
    {
      const Core::File_IO::Node& istate = node["states"][i];
      std::string state = istate["id"].string();

      if (i == 0)
      {
        if (state_handle->value() == "")
          state_handle->set(state);
        conditional_handle = m_content.set<Component::String_conditional>(id + ":image", state_handle);
      }
      else
        conditional_handle = m_content.get<Component::String_conditional>(id + ":image");

      std::string skin = istate["skin"].string("images", "scenery", "png");
      auto img = Component::make_handle<Component::Image>(id + ":conditional_image",
                                                          local_file_name(skin), z);
      conditional_handle->add (state, img);
    }
  }
  else
  {
    std::string skin = node["skin"].string("images", "scenery", "png");

    auto img = m_content.set<Component::Image>(id + ":image", local_file_name(skin), z);
    img->collision() = UNCLICKABLE;
    img->set_relative_origin(0.5, 1.0);
    debug("Scenery " + id + " at position " + std::to_string(img->z()));
  }
}

void File_IO::read_window (const Core::File_IO::Node& node, const std::string& id)
{
  std::string skin = node["skin"].string("images", "windows", "png");

  auto img = m_content.set<Component::Image>(id + ":image", local_file_name(skin),
                                             Config::inventory_front_depth);
  img->set_relative_origin(0.5, 0.5);
  img->on() = false;

  m_content.set<Component::Position>(id + ":position",
                                     Point(Config::world_width / 2,
                                           Config::world_height / 2));
}

} // namespace Sosage::System
