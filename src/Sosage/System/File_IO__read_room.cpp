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
#include <Sosage/Component/Dialog.h>
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

namespace C = Component;

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
  set<C::String> (id + ":name", name);

  std::string color = input["color"].string();
  set<C::String> (id + ":color", color);

  auto visible = request<C::Boolean>(id + ":visible");
  if (!visible)
    visible = set<C::Boolean>(id + ":visible", true);

  std::string mouth = input["mouth"]["skin"].string("images", "characters", "png");
  auto amouth
    = C::make_handle<C::Animation>(id + "_mouth:image", local_file_name(mouth),
                                          0, 11, 2, true);
  set<C::Conditional>(amouth->id(), visible, amouth);
  amouth->set_relative_origin(0.5, 1.0);

  std::string head = input["head"]["skin"].string("images", "characters", "png");
  int head_size = input["head"]["size"].integer();
  auto ahead
    = C::make_handle<C::Animation>(id + "_head:image", local_file_name(head),
                                   0, head_size, 2, true);
  set<C::Conditional>(ahead->id(), visible, ahead);
  ahead->set_relative_origin(0.5, 1.0);

  std::string walk = input["walk"]["skin"].string("images", "characters", "png");
  auto awalk = C::make_handle<C::Animation>(id + "_walking:image", local_file_name(walk),
                                            0, 8, 4, true);
  set<C::Conditional>(awalk->id(), visible, awalk);
  awalk->set_relative_origin(0.5, 0.95);
  awalk->on() = false;

  std::string idle = input["idle"]["skin"].string("images", "characters", "png");
  std::vector<std::string> positions;
  for (std::size_t i = 0; i < input["idle"]["positions"].size(); ++ i)
    positions.push_back (input["idle"]["positions"][i].string());

  set<C::Vector<std::string> >(id + "_idle:values", positions);

  auto aidle = C::make_handle<C::Animation>(id + "_idle:image", local_file_name(idle),
                                            0, positions.size(), 2, true);
  set<C::Conditional>(aidle->id(), visible, aidle);
  aidle->set_relative_origin(0.5, 0.95);


  int hdx_right = input["head"]["dx_right"].integer();
  int hdx_left = input["head"]["dx_left"].integer();
  int hdy = input["head"]["dy"].integer();
  set<C::Position>(id + "_head:gap_right", Point(hdx_right,hdy), false);
  set<C::Position>(id + "_head:gap_left", Point(hdx_left,hdy), false);

  int mdx_right = input["mouth"]["dx_right"].integer();
  int mdx_left = input["mouth"]["dx_left"].integer();
  int mdy = input["mouth"]["dy"].integer();
  set<C::Position>(id + "_mouth:gap_right", Point(mdx_right,mdy), false);
  set<C::Position>(id + "_mouth:gap_left", Point(mdx_left,mdy), false);

  // Init position objects if they don't already exist
  auto pbody = request<C::Position>(id + "_body:position");
  if (!pbody)
  {
    pbody = set<C::Position>(id + "_body:position", Point(x, y), false);
    set<C::Position>(id + "_head:position", Point(x - hdx_right, y - hdy), false);
    set<C::Position>(id + "_mouth:position", Point(x - hdx_right - mdx_right,
                                                   y - hdy - mdy), false);
  }
  else
  {
    pbody->absolute() = false;
    get<C::Position>(id + "_head:position")->absolute() = false;
    get<C::Position>(id + "_mouth:position")->absolute() = false;
  }

  set<C::Variable>(id + "_walking:position", pbody);
  set<C::Variable>(id + "_idle:position", pbody);

  auto new_char = request<C::Vector<std::pair<std::string, bool> > >("game:new_characters");
  if (!new_char)
    new_char = set<C::Vector<std::pair<std::string, bool> > >("game:new_characters");

  new_char->push_back (std::make_pair (id, looking_right));
}

void File_IO::read_room (const std::string& file_name)
{
  auto callback = get<C::Simple<std::function<void()> > >("game:loading_callback");
  callback->value()();

  clean_content();

  SOSAGE_TIMER_START(File_IO__read_room);

  callback->value()();

  Core::File_IO input (local_file_name("data", "rooms", file_name, "yaml"));
  input.parse();

  callback->value()();

  std::string name = input["name"].string(); // unused so far

  set<C::String>("game:current_room", file_name);
  get<C::Set<std::string> >("game:visited_rooms")->insert (file_name);

  std::string background = input["background"].string("images", "backgrounds", "png");
  std::string ground_map = input["ground_map"].string("images", "backgrounds", "png");
  int front_z = input["front_z"].integer();
  int back_z = input["back_z"].integer();

  auto background_img
    = set<C::Image>("background:image", local_file_name(background), 0);
  background_img->collision() = BOX;
  m_latest_room_entities.insert ("background");

  callback->value()();

  set<C::Position>("background:position", Point(0, 0), false);
  set<C::Ground_map>("background:ground_map", local_file_name(ground_map),
                                       front_z, back_z, callback->value());

  callback->value()();

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
      if (!request<C::String>(id + ":state"))
        set<C::String>(id + ":state");
  }

  callback->value()();

  for (std::size_t i = 0; i < input["content"].size(); ++ i)
  {
    const Core::File_IO::Node& node = input["content"][i];
    std::string id = node["id"].string();
    std::string type = node["type"].string();

    if (type == "action")
      read_action (node, id);
    else if (type == "animation")
      read_animation (node, id);
    else if (type == "character")
      read_character (node, id);
    else if (type == "code")
      read_code (node, id);
    else if (type == "dialog")
      read_dialog (node, id);
    else if (type == "integer")
      read_integer (node, id);
    else if (type == "music")
      read_music (node, id);
    else if (type == "object")
      read_object (node, id);
    else if (type == "origin")
      read_origin (node, id);
    else if (type == "scenery")
      read_scenery (node, id);
    else if (type == "sound")
      read_sound (node, id);
    else if (type == "window")
      read_window (node, id);
    else
      debug ("Unknown content type " + type);

    callback->value()();
  }

  // Special handling for inventory after reloading save (may need to
  // search in other rooms for the object)
  auto inventory = get<C::Inventory>("game:inventory");
  std::unordered_set<std::string> missing_objects;
  for (std::size_t i = 0; i < inventory->size(); ++ i)
    if (!request<C::String>(inventory->get(i) + ":name"))
    {
      std::cerr << inventory->get(i) << " is missing" << std::endl;
      missing_objects.insert (inventory->get(i));
    }

  if (!missing_objects.empty())
  {
    bool all_found = false;
    for (const std::string& room_name : *get<C::Set<std::string> >("game:visited_rooms"))
      if (room_name != file_name)
      {
        Core::File_IO room (local_file_name("data", "rooms", room_name, "yaml"));
        room.parse();
        for (std::size_t i = 0; i < room["content"].size(); ++ i)
        {
          const Core::File_IO::Node& node = room["content"][i];
          std::string id = node["id"].string();
          std::string type = node["type"].string();
          if (missing_objects.find (id) != missing_objects.end())
          {
            read_object (node, id);
            missing_objects.erase (id);
            if (missing_objects.empty())
            {
              all_found = true;
              break;
            }
          }
          callback->value();
        }
        if (all_found)
          break;
      }
  }

  auto hints = set<C::Hints>("game:hints");

  if (input.has("hints"))
    for (std::size_t i = 0; i < input["hints"].size(); ++ i)
    {
      const Core::File_IO::Node& node = input["hints"][i];
      std::string id = node["id"].string();
      std::string state = node["state"].string();
      std::string text = node["text"].string();

      auto condition = C::make_handle<C::String_conditional>
        ("hint:condition", get<C::String>(id + ":state"));
      condition->add(state, C::make_handle<C::String>("hint:text", text));
      hints->add (condition);
    }

  callback->value()();

  const std::string& origin = get<C::String>("game:new_room_origin")->value();
  if (origin == "saved_game")
    set<C::Boolean>("game:in_new_room", true);
  else
  {
    auto origin_coord = get<C::Position>(origin + ":position");
    auto origin_looking = get<C::Boolean>(origin + ":looking_right");
    const std::string& player = get<C::String>("player:name")->value();
    get<C::Position>(player + "_body:position")->set(origin_coord->value());
    set<C::Boolean>("game:in_new_room", origin_looking->value());
  }
  emit ("game:loading_done");

#ifdef SOSAGE_DEBUG
  // Display layers of images for easy room creation
  std::vector<C::Image_handle> images;
  for (C::Handle h : m_content)
    if (auto img = C::cast<C::Image>(h))
      images.push_back(img);

  std::sort (images.begin(), images.end(),
             [](const C::Image_handle& a, const C::Image_handle& b) -> bool
             { return a->z() < b->z(); });

  int current_depth = -Config::cursor_depth;

  std::cerr << "[LAYERS BEGIN]" << std::endl;
  for (C::Image_handle img : images)
  {
    if (img->z() != current_depth)
    {
      current_depth = img->z();
      std::cerr << "# Depth " << current_depth << ":" << std::endl;
    }
    std::cerr << "  * " << img->id() << std::endl;
  }
  std::cerr << "[LAYERS END]" << std::endl;
#endif

  SOSAGE_TIMER_STOP(File_IO__read_room);
}

void File_IO::read_animation (const Core::File_IO::Node& node, const std::string& id)
{
  int x = node["coordinates"][0].integer();
  int y = node["coordinates"][1].integer();
  int z = node["coordinates"][2].integer();
  std::string skin = node["skin"].string("images", "animations", "png");

  int width, height;
  if (node["length"].size() == 0)
  {
    width = node["length"].integer();
    height = 1;
  }
  else
  {
    width = node["length"][0].integer();
    height = node["length"][1].integer();
  }

  auto pos = set<C::Position>(id + ":position", Point(x,y), false);
  auto img = set<C::Animation>(id + ":image", local_file_name(skin), z,
                               width, height, node["loop"].boolean());

  int duration = (node.has("duration") ? node["duration"].integer() : 1);

  if (node.has("frames"))
  {
    img->frames().clear();
    for (std::size_t i = 0; i < node["frames"].size(); ++ i)
    {
      int idx = node["frames"][i].integer();
      int x = idx % width;
      int y = idx / width;
      img->frames().emplace_back (x, y, duration);
    }
  }
  else
    img->reset(true, duration);

  img->on() = false;
  img->set_relative_origin(0.5, 1.0);
  debug("Animation " + id + " at position " + std::to_string(img->z()));
}

void File_IO::read_code (const Core::File_IO::Node& node, const std::string& id)
{
  std::string button_sound = node["button_sound"].string("sounds", "effects", "ogg");
  set<C::Sound>(id + "_button:sound", local_file_name(button_sound));
  std::string success_sound = node["success_sound"].string("sounds", "effects", "ogg");
  set<C::Sound>(id + "_success:sound", local_file_name(success_sound));
  std::string failure_sound = node["failure_sound"].string("sounds", "effects", "ogg");
  set<C::Sound>(id + "_failure:sound", local_file_name(failure_sound));

  auto code = set<C::Code>(id + ":code");

  auto state_handle = get<C::String>(id + ":state");
  C::String_conditional_handle conditional_handle_off;
  C::String_conditional_handle conditional_handle_on;

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
        = set<C::String_conditional>(id + ":image", state_handle);
      conditional_handle_on
        = set<C::String_conditional>(id + "_button:image", state_handle);
    }
    else
    {
      conditional_handle_off = get<C::String_conditional>(id + ":image");
      conditional_handle_on = get<C::String_conditional>(id + "_button:image");
    }

    std::string skin_off = istate["skin"][0].string("images", "windows", "png");
    auto img_off
      = C::make_handle<C::Image>(id + ":conditional_image", local_file_name(skin_off),
                                                 Config::inventory_back_depth);
    img_off->set_relative_origin(0.5, 0.5);
    img_off->on() = false;

    set<C::Position>(id + ":position",
                                       Point(Config::world_width / 2,
                                             Config::world_height / 2));

    std::string skin_on = istate["skin"][1].string("images", "windows", "png");
    auto img_on
      = C::make_handle<C::Cropped>(id + "_button:conditional_image",
                                                   local_file_name(skin_on),
                                                   Config::inventory_front_depth);
    img_on->set_relative_origin(0.5, 0.5);
    img_on->on() = false;

    set<C::Position>(id + "_button:position",
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

  auto action = set<C::Action> (id + ":action");
  for (std::size_t j = 0; j < node["on_success"].size(); ++ j)
  {
    std::string function = node["on_success"][j].nstring();
    action->add (function, node["on_success"][j][function].string_array());
  }
}

void File_IO::read_dialog (const Core::File_IO::Node& node, const std::string& id)
{
  auto dialog = set<C::Dialog>(id + ":dialog",
                               node.has("end") ? node["end"].string() : "");

  // First, instantiate all vertices
  std::vector<C::Dialog::GVertex> vec_vertices;
  std::unordered_map<std::string, C::Dialog::GVertex> map_vertices;
  for (std::size_t i = 0; i < node["lines"].size(); ++ i)
  {
    const Core::File_IO::Node& l = node["lines"][i];

    C::Dialog::GVertex vertex;
    if (l.has("choices"))
      vertex = dialog->add_vertex ();
    else
    {
      std::string character = l["character"].string();
      std::string line = l["line"].string();
      vertex = dialog->add_vertex (character, line);
    }    

    if (l.has("id"))
      map_vertices.insert (std::make_pair (l["id"].string(), vertex));
    vec_vertices.push_back (vertex);
  }

  // Then, edges
  dialog->add_edge (dialog->vertex_in(), vec_vertices.front());
  for (std::size_t i = 0; i < node["lines"].size(); ++ i)
  {
    const Core::File_IO::Node& l = node["lines"][i];

    if (l.has("choices"))
      for (std::size_t j = 0; j < l["choices"].size(); ++ j)
      {
        const Core::File_IO::Node& c = l["choices"][j];
        bool once = c["once"].boolean();
        std::string line = c["line"].string();
        const std::string& target = c["goto"].string();
        if (target == "end")
          dialog->add_edge (vec_vertices[i], dialog->vertex_out(), once, line);
        else
          dialog->add_edge (vec_vertices[i], map_vertices[target], once, line);
      }
    else
    {
      if (l.has("goto"))
      {
        const std::string& target = l["goto"].string();
        if (target == "end")
          dialog->add_edge (vec_vertices[i], dialog->vertex_out());
        else
          dialog->add_edge (vec_vertices[i], map_vertices[target]);
      }
      else if (i != node["lines"].size() - 1)
        dialog->add_edge (vec_vertices[i], vec_vertices[i+1]);
      else
        dialog->add_edge (vec_vertices[i], dialog->vertex_out());
    }
  }
}

void File_IO::read_integer (const Core::File_IO::Node& node, const std::string& id)
{
  int value = node["value"].integer();
  auto integer = request<C::Int>(id + ":value");
  if (!integer)
    integer = set<C::Int>(id + ":value", value);

  for (std::size_t i = 0; i < node["triggers"].size(); ++ i)
  {
    const Core::File_IO::Node& itrigger = node["triggers"][i];

    std::string value = itrigger["value"].string();

    auto action = set<C::Action>(id + ":" + value);

    for (std::size_t k = 0; k < itrigger["effect"].size(); ++ k)
    {
      std::string function = itrigger["effect"][k].nstring();
      action->add (function, itrigger["effect"][k][function].string_array());
    }
  }
}

void File_IO::read_object (const Core::File_IO::Node& node, const std::string& id)
{
  // First, check if object already exists in inventory (if so, skip)
  auto state_handle = get<C::String>(id + ":state");

  std::string name = node["name"].string();
  int x = node["coordinates"][0].integer();
  int y = node["coordinates"][1].integer();
  int z = node["coordinates"][2].integer();
  int vx = node["view"][0].integer();
  int vy = node["view"][1].integer();
  bool box_collision = node["box_collision"].boolean();

  set<C::String>(id + ":name", name);

  // Position might already exists if room was already loaded
  auto pos = request<C::Position>(id + ":position");
  if (!pos)
    pos = set<C::Position>(id + ":position", Point(x,y), false);
  else
    pos->absolute() = false;

  set<C::Position>(id + ":view", Point(vx,vy), false);

  C::String_conditional_handle conditional_handle;
  for (std::size_t j = 0; j < node["states"].size(); ++ j)
  {
    const Core::File_IO::Node& istate = node["states"][j];

    std::string state = istate["id"].string();

    // init with first state found
    if (j == 0)
    {
      if (state_handle->value() == "")
        state_handle->set(state);
      conditional_handle = set<C::String_conditional>(id + ":image", state_handle);
    }
    else
      conditional_handle = get<C::String_conditional>(id + ":image");

    if (state == "none")
      conditional_handle->add(state, nullptr);
    else
    {
      std::string skin = "";
      if (istate.has("skin"))
      {
        if (state == "inventory")
          skin = istate["skin"].string("images", "inventory", "png");
        else
          skin = istate["skin"].string("images", "objects", "png");
      }

      C::Image_handle img;
      if (istate.has("frames")) // Animation
      {
        int nb_frames = istate["frames"].integer();
        int duration = istate["duration"].integer();
        auto anim = C::make_handle<C::Animation>(id + ":conditional_image",
                                                                 local_file_name(skin), z,
                                                                 nb_frames, 1, true);
        anim->reset(true, duration);
        img = anim;
      }
      else
      {
        if (skin == "") // No skin, just an empty rectangle
        {
          int width = istate["size"][0].integer();
          int height = istate["size"][1].integer();
          img = C::make_handle<C::Image>(id + ":conditional_image",
                                         width, height, 0, 0, 0, 0);
        }
        else
          img = C::make_handle<C::Image>(id + ":conditional_image", local_file_name(skin), z);
      }

      if (state == "inventory")
        img->set_relative_origin(0.5, 0.5);
      else
        img->set_relative_origin(0.5, 1.0);
      img->collision() = (box_collision ? BOX : PIXEL_PERFECT);

      debug("Object " + id + ":" + state + " at position " + std::to_string(img->z()));

      conditional_handle->add(state, img);
    }
  }

  read_actions(node, id);

}

void File_IO::read_action (const Core::File_IO::Node& node, const std::string& id)
{
  auto state_handle = get<C::String>(id + ":state");
  auto conditional_handle = set<C::String_conditional>(id + ":action", state_handle);

  for (std::size_t i = 0; i < node["states"].size(); ++ i)
  {
    const Core::File_IO::Node& istate = node["states"][i];

    std::string state = istate["id"].string();
    if (i == 0 && state_handle->value() == "")
        state_handle->set(state);

    auto action = C::make_handle<C::Action>(id + ":action");

    for (std::size_t k = 0; k < istate["effect"].size(); ++ k)
    {
      std::string function = istate["effect"][k].nstring();
      action->add (function, istate["effect"][k][function].string_array());
    }
    conditional_handle->add(state, action);
  }
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

      std::string target_id = id;

      if (iaction.has("target"))
      {
        target_id = iaction["target"].string();
        a_id = a_id + "_" + target_id;
      }

      if (a_id == "look")
        look_found = true;

      C::Action_handle action;

      debug("Read action " + id + ":" + a_id);

      if (iaction.has("state"))
      {
        std::string state = iaction["state"].string();
        auto conditional_handle
          = request<C::String_conditional>(id + ":" + a_id);

        if (!conditional_handle)
        {
          auto state_handle
            = get<C::String>(target_id + ":state");
          conditional_handle
            = set<C::String_conditional>(id + ":" + a_id, state_handle);
        }

        action = C::make_handle<C::Action>(id + ":" + a_id);
        conditional_handle->add (state, action);
      }
      else
        action = set<C::Action>(id + ":" + a_id);


      for (std::size_t k = 0; k < iaction["effect"].size(); ++ k)
      {
        std::string function = iaction["effect"][k].nstring();
        action->add (function, iaction["effect"][k][function].string_array());
      }
      ++ j;
    }
    while (j < nb_actions);
  }

  check (look_found, "Object " + id + " has no \"look\" action");
}

void File_IO::read_music(const Core::File_IO::Node& node, const std::string& id)
{
  if (auto state_handle = request<C::String>(id + ":state"))
  {
    C::String_conditional_handle conditional_handle;
    for (std::size_t j = 0; j < node["states"].size(); ++ j)
    {
      const Core::File_IO::Node& istate = node["states"][j];

      std::string state = istate["id"].string();

      // init with first state found
      if (j == 0)
      {
        if (state_handle->value() == "")
          state_handle->set(state);
        conditional_handle = set<C::String_conditional>(id + ":music", state_handle);
      }
      else
        conditional_handle = get<C::String_conditional>(id + ":music");

      std::string music = istate["sound"].string("sounds", "musics", "ogg");
      conditional_handle->add(state, C::make_handle<C::Music>(id + ":music", local_file_name(music)));
    }
  }
  else
  {
    std::string music = node["sound"].string("sounds", "musics", "ogg");
    set<C::Music>(id + ":music", local_file_name(music));
  }
}


void File_IO::read_origin(const Core::File_IO::Node& node, const std::string& id)
{
  int x = node["coordinates"][0].integer();
  int y = node["coordinates"][1].integer();
  bool looking_right = node["looking_right"].boolean();

  set<C::Position>(id + ":position", Point(x, y));
  set<C::Boolean>(id + ":looking_right", looking_right);

  auto action = set<C::Action>(id + ":action");
  for (std::size_t k = 0; k < node["action"].size(); ++ k)
  {
    std::string function = node["action"][k].nstring();
    action->add (function, node["action"][k][function].string_array());
  }
}

void File_IO::read_scenery (const Core::File_IO::Node& node, const std::string& id)
{
  int x = node["coordinates"][0].integer();
  int y = node["coordinates"][1].integer();
  int z = node["coordinates"][2].integer();

  auto pos = set<C::Position>(id + ":position", Point(x,y), false);

  if (node.has("states"))
  {
    C::String_conditional_handle conditional_handle;

    auto state_handle = get<C::String>(id + ":state");
    for (std::size_t i = 0; i < node["states"].size(); ++ i)
    {
      const Core::File_IO::Node& istate = node["states"][i];
      std::string state = istate["id"].string();

      if (i == 0)
      {
        if (state_handle->value() == "")
          state_handle->set(state);
        conditional_handle = set<C::String_conditional>(id + ":image", state_handle);
      }
      else
        conditional_handle = get<C::String_conditional>(id + ":image");

      if (state == "none")
        conditional_handle->add(state, nullptr);
      else
      {
        std::string skin = istate["skin"].string("images", "scenery", "png");
        auto img = C::make_handle<C::Image>(id + ":conditional_image",
                                            local_file_name(skin), z);
        img->collision() = UNCLICKABLE;
        img->set_relative_origin(0.5, 1.0);
        conditional_handle->add (state, img);
      }
    }
  }
  else
  {
    std::string skin = node["skin"].string("images", "scenery", "png");

    auto img = set<C::Image>(id + ":image", local_file_name(skin), z);
    img->collision() = UNCLICKABLE;
    img->set_relative_origin(0.5, 1.0);
    debug("Scenery " + id + " at position " + std::to_string(img->z()));
  }
}

void File_IO::read_sound (const Core::File_IO::Node& node, const std::string& id)
{
  std::string sound = node["sound"].string("sounds", "effects", "ogg");
  set<C::Sound>(id + ":sound", local_file_name(sound));
}

void File_IO::read_window (const Core::File_IO::Node& node, const std::string& id)
{
  std::string skin = node["skin"].string("images", "windows", "png");

  auto img = set<C::Image>(id + ":image", local_file_name(skin),
                                             Config::inventory_front_depth);
  img->set_relative_origin(0.5, 0.5);
  img->on() = false;

  set<C::Position>(id + ":position",
                                     Point(Config::world_width / 2,
                                           Config::world_height / 2));
}

} // namespace Sosage::System
