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
#include <Sosage/Component/Group.h>
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
#include <Sosage/Utils/conversions.h>
#include <Sosage/Utils/helpers.h>
#include <Sosage/Utils/profiling.h>

namespace Sosage::System
{

namespace C = Component;

void File_IO::read_character (const std::string& id, const Core::File_IO::Node& input)
{
  int x = input["coordinates"][0].integer();
  int y = input["coordinates"][1].integer();

  // Init position objects if they don't already exist
  auto position = request<C::Absolute_position>(id, "position");
  if (!position)
    position = set<C::Absolute_position>(id, "position", Point(x, y), false);

  if (input.has("label"))
  {
    int lx = input["label"][0].integer();
    int ly = input["label"][1].integer();
    if (input["label"][0].is_relative())
    {
      set<C::Absolute_position>(id, "relative_label", Point(lx,ly), false);
      set<C::Functional_position>
          (id, "label",
           [&](const std::string& id) -> Point
      {
        const Point& position = value<C::Position>(id, "position");
        const Point& relative = value<C::Position>(id, "relative_label");
        auto img = request<C::Image>(id + "_body", "image");
        return position + (img ? img->scale() : 1.) * relative;
      }, id);
    }
    else
      set<C::Absolute_position>(id , "label", Point(lx,ly), false);
  }

  if (input.has("view"))
  {
    int view_x = input["view"][0].integer();
    int view_y = input["view"][1].integer();
    if (input["view"][0].is_relative())
    {
      set<C::Absolute_position>(id, "relative_view", Point(view_x, view_y), false);
      set<C::Functional_position>
          (id, "view",
           [&](const std::string& id) -> Point
      {
        const Point& position = value<C::Position>(id, "position");
        Vector relative = value<C::Position>(id, "relative_view");
        if (!is_looking_right(id))
          relative = Vector(-relative.x(), relative.y());
        return position + relative;
      }, id);

    }
    else
      set<C::Absolute_position>(id, "view", Point (view_x, view_y), false);
  }
  else
    set<C::Variable>(id, "view", position);

  bool looking_right = input["looking_right"].boolean();

  std::string name = input["name"].string();
  set<C::String> (id , "name", name);

  std::string color = input["color"].string();
  set<C::String> (id , "color", color);

  set<C::Boolean>(id, "uses_2nd_map", (input.has("uses_secondary_ground_map")
                                       ? input["uses_secondary_ground_map"].boolean() : false));


  std::string default_state = "default";
  if (input.has("states"))
    default_state = input["states"][0].string();

  auto state_handle = get_or_set<C::String>(id , "state", default_state);

  for (std::string action : Config::possible_actions)
      if (input.has(action))
      {
        if (action == "inventory")
        {
          for (std::size_t i = 0; i < input["inventory"].size(); ++ i)
          {
            auto act = read_object_action(id, action, input["inventory"][i]);
            if (act.first) set(act.first);
            if (act.second) set(act.second);
          }
        }
        else
        {
          auto act = read_object_action(id, action, input[action]);
          if (act.first) set(act.first);
          if (act.second) set(act.second);
        }
      }

  if (input.has("skin"))
  {
    const Core::File_IO::Node& skin = input["skin"];

    if (skin.has("head_move_radius"))
      set<C::Int> (id, "head_move_radius", skin["head_move_radius"].integer());

    bool visible = value<C::Boolean>(id , "visible", true);
    remove(id , "visible", true);

    auto walking = set<C::Boolean>(id , "walking", false);

    auto group = set<C::Group>(id , "group");

    int size = 11;
    if (skin["mouth"].has("size"))
      size = skin["mouth"]["size"].integer();

    std::string mouth = skin["mouth"]["skin"].string("images", "characters", "png");
    auto amouth
        = set<C::Animation>(id + "_mouth", "image", mouth,
                            0, size, 2, true);
    amouth->set_relative_origin(0.5, 1.0);
    amouth->on() = visible;
    group->add(amouth);

    std::vector<std::string> hpositions;
    for (std::size_t i = 0; i < skin["head"]["positions"].size(); ++ i)
      hpositions.push_back (skin["head"]["positions"][i].string());

    set<C::Vector<std::string> >(id + "_head", "values", hpositions);

    std::string head = skin["head"]["skin"].string("images", "characters", "png");
    int head_size = int(hpositions.size());
    auto ahead
        = set<C::Animation>(id + "_head", "image", head,
                            0, head_size, 2, true);
    ahead->set_relative_origin(0.5, 1.0);
    ahead->on() = visible;
    group->add(ahead);

    C::Animation_handle awalk;

    if (skin.has("walk"))
    {
      int steps = 8;
      if (skin["walk"].has("steps"))
        steps = skin["walk"]["steps"].integer();
      std::string walk = skin["walk"]["skin"].string("images", "characters", "png");
      awalk = C::make_handle<C::Animation>(id + "_body", "image", walk,
                                           0, steps, 4, true);
      awalk->set_relative_origin(0.5, 0.95);
      awalk->on() = visible;
    }

    std::string idle = skin["idle"]["skin"].string("images", "characters", "png");
    std::vector<std::string> positions;
    for (std::size_t i = 0; i < skin["idle"]["positions"].size(); ++ i)
      positions.push_back (skin["idle"]["positions"][i].string());

    set<C::Vector<std::string> >(id + "_idle", "values", positions);

    auto aidle = C::make_handle<C::Animation>(id + "_body", "image", idle,
                                              0, positions.size(), 2, true);
    aidle->set_relative_origin(0.5, 0.95);
    aidle->on() = visible;

    auto abody = set<C::Conditional>(id + "_body", "image", walking, awalk, aidle);
    group->add(abody);

    int width = aidle->width() * 0.5;
    int height = aidle->height() * 1.05;

    auto amask = C::make_handle<C::Image>(id, "conditional_image",
                                          width, height, 0, 0, 0, 0);
    amask->z() = 0;
    amask->set_relative_origin(0.5, 0.95);

    auto camask = set<C::String_conditional>(id , "image", state_handle);

    if (input.has("states"))
      for (std::size_t j = 0; j < input["states"].size(); ++ j)
      {
        const std::string& istate = input["states"][j].string();
        camask->add(istate, amask);
      }
    else
      camask->add(default_state, amask);

    camask->add("player", nullptr); // No mask/interaction if character is player
    group->add(camask);

    int hdx_right = skin["head"]["dx_right"].integer();
    int hdx_left = skin["head"]["dx_left"].integer();
    int hdy = skin["head"]["dy"].integer();
    set<C::Simple<Vector>>(id + "_head", "gap_right", Vector(hdx_right, hdy));
    set<C::Simple<Vector>>(id + "_head", "gap_left", Vector(hdx_left, hdy));

    int mdx_right = skin["mouth"]["dx_right"].integer();
    int mdx_left = skin["mouth"]["dx_left"].integer();
    int mdy = skin["mouth"]["dy"].integer();
    set<C::Simple<Vector>>(id + "_mouth", "gap_right", Vector(mdx_right, mdy));
    set<C::Simple<Vector>>(id + "_mouth", "gap_left", Vector(mdx_left, mdy));

    auto pbody = set<C::Variable>(id + "_body", "position", position);

    auto move_head = set<C::Absolute_position>(id + "_head_move", "position", Point(0,0), false);

    auto phead = set<C::Functional_position>
                 (id + "_head", "position",
                  [&](const std::string& id) -> Point
    {
      auto abody = get<C::Image>(id + "_body", "image");
      auto pbody = get<C::Position>(id + "_body", "position");
      auto mhead = get<C::Position>(id + "_head_move", "position");
      return (pbody->value() - abody->scale()
              * (value<C::Simple<Vector>>(id + "_head",
                                          (is_looking_right(id) ? "gap_right" : "gap_left"))
                 + Vector(mhead->value())));
    }, id);

    set<C::Functional_position>
        (id + "_mouth", "position",
         [&](const std::string& id) -> Point
    {
      auto ahead = get<C::Animation>(id + "_head", "image");
      auto phead = get<C::Position>(id + "_head", "position");
      return (phead->value() - ahead->scale()
              * value<C::Simple<Vector>>(id + "_mouth",
                                         (is_looking_right(id) ? "gap_right" : "gap_left")));
    }, id);

    set<C::Absolute_position>(id, "lookat",
                              looking_right ? Point::right() : Point::left());
  }

}

void File_IO::read_room (const std::string& file_name)
{
  auto callback = get<C::Simple<std::function<void()> > >("Game", "loading_callback");
  callback->value()();

  clean_content();

  SOSAGE_TIMER_START(File_IO__read_room);

  callback->value()();

  Core::File_IO input ("data/rooms/" + file_name + ".yaml");
  input.parse();

  callback->value()();

  std::string name = input["name"].string(); // unused so far

  set<C::String>("Game", "current_room", file_name);

  if (input.has("background"))
  {
    std::string background = input["background"].string("images", "backgrounds", "png");
    auto background_img
        = set<C::Image>("background", "image", background, 0, BOX);
  }
  if (input.has("ground_map"))
  {
    emit ("Player", "not_moved_yet");
    int front_z = input["front_z"].integer();
    int back_z = input["back_z"].integer();
    if (input["ground_map"].size() == 0)
    {
      std::string ground_map = input["ground_map"].string("images", "backgrounds", "png");
      set<C::Ground_map>("background", "ground_map", ground_map,
                         front_z, back_z, callback->value());
    }
    else
    {
      if (input["ground_map"][0].string() != "null")
      {
        std::string ground_map = input["ground_map"][0].string("images", "backgrounds", "png");
        set<C::Ground_map>("background", "ground_map", ground_map,
                           front_z, back_z, callback->value());
      }
      std::string sec_ground_map = input["ground_map"][1].string("images", "backgrounds", "png");
      set<C::Ground_map>("background", "2nd_ground_map", sec_ground_map,
                         front_z, back_z, callback->value());
    }
  }
  else
    // Just in case a garbage signal remains...
    receive("Player", "not_moved_yet");

  callback->value()();

  set<C::Absolute_position>("background", "position", Point(0, 0), false);

  callback->value()();

  for (const auto& d : m_dispatcher)
  {
    const std::string& section = d.first;
    const Function& func = d.second;
    if (input.has(section))
      for (std::size_t i = 0; i < input[section].size(); ++ i)
      {
        const Core::File_IO::Node& s = input[section][i];
        if (s.string() == "")
          func (s["id"].string(), s);
        else
        {
          Core::File_IO subfile ("data/" + section + "/" + s.string() + ".yaml");
          bool okay = subfile.parse();
          check(okay, "Can't open data/" + section + "/" + s.string() + ".yaml");
          func (s.string(), subfile.root());
        }
        callback->value()();
      }
  }

  // Special handling for inventory/numbers after reloading save (may need to
  // search in other rooms for the object)
  auto inventory = get<C::Inventory>("Game", "inventory");
  for (std::size_t i = 0; i < inventory->size(); ++ i)
    if (!request<C::String>(inventory->get(i) , "name"))
    {
      Core::File_IO subfile ("data/objects/" + inventory->get(i) + ".yaml");
      subfile.parse();
      read_object (inventory->get(i), subfile.root());
      callback->value();
    }
  if (auto numbers = request<C::Vector<std::string>>("phone_numbers", "list"))
    for (std::size_t i = 0; i < numbers->value().size(); ++ i)
      if (!request<C::Action>(numbers->value()[i], "action"))
      {
        Core::File_IO subfile ("data/actions/" + numbers->value()[i] + ".yaml");
        subfile.parse();
        read_action (numbers->value()[i], subfile.root());
        callback->value();
      }

  emit ("Game", "in_new_room");
  emit ("Game", "loading_done");
  emit ("Window", "rescaled");

#ifdef SOSAGE_DEBUG
  // Display layers of images for easy room creation
  std::vector<C::Image_handle> images;
  for (C::Handle h : components("image"))
    if (auto img = C::cast<C::Image>(h))
      images.push_back(img);

  std::sort (images.begin(), images.end(),
             [](const C::Image_handle& a, const C::Image_handle& b) -> bool
             { return a->z() < b->z(); });

  int current_depth = -Config::cursor_depth;

  debug << "[LAYERS BEGIN]" << std::endl;
  for (C::Image_handle img : images)
  {
    if (img->z() != current_depth)
    {
      current_depth = img->z();
      debug << "# Depth " << current_depth << ":" << std::endl;
    }
    debug << "  * " << img->str() << std::endl;
  }
  debug << "[LAYERS END]" << std::endl;
#endif

  SOSAGE_TIMER_STOP(File_IO__read_room);
}

void File_IO::read_animation (const std::string& id, const Core::File_IO::Node& node)
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

  auto pos = set<C::Absolute_position>(id , "position", Point(x,y), false);
  auto img = set<C::Animation>(id , "image", skin, z,
                               width, height, node["loop"].boolean());

  int duration = (node.has("duration") ? node["duration"].integer() : 1);

  if (node.has("frames"))
  {
    img->frames().clear();
    for (std::size_t i = 0; i < node["frames"].size(); ++ i)
    {
      int idx = 0;
      int nb = 1;
      const std::string& str = node["frames"][i].string();
      std::size_t pos = str.find("x");
      if (pos != std::string::npos)
      {
        nb = to_int (std::string(str.begin(), str.begin() + pos));
        idx = to_int (std::string(str.begin() + pos + 1, str.end()));
      }
      else
        idx = to_int(str);

      int x = idx % width;
      int y = idx / width;
      for (int j = 0; j < nb; ++ j)
        img->frames().push_back ({x, y, duration});
    }
  }
  else
    img->reset(true, duration);

  img->on() = false;
  img->set_relative_origin(0.5, 1.0);
  img->set_collision(UNCLICKABLE);
  debug << "Animation " << id << " at position " << img->z() << std::endl;
}

void File_IO::read_code (const std::string& id, const Core::File_IO::Node& input)
{
  std::string button_sound = input["button_sound"].string("sounds", "effects", "ogg");
  set<C::Sound>(id + "_button", "sound", button_sound);
  std::string success_sound = input["success_sound"].string("sounds", "effects", "ogg");
  set<C::Sound>(id + "_success", "sound", success_sound);
  std::string failure_sound = input["failure_sound"].string("sounds", "effects", "ogg");
  set<C::Sound>(id + "_failure", "sound", failure_sound);

  auto code = set<C::Code>(id , "code");

  auto state_handle = get_or_set<C::String>(id , "state");
  C::String_conditional_handle conditional_handle_off;
  C::String_conditional_handle conditional_handle_on;

  for (std::size_t j = 0; j < input["states"].size(); ++ j)
  {
    const Core::File_IO::Node& istate = input["states"][j];

    std::string state = istate["id"].string();

    // init with first state found
    if (j == 0)
    {
      if (state_handle->value() == "")
        state_handle->set(state);
      conditional_handle_off
        = set<C::String_conditional>(id , "image", state_handle);
      conditional_handle_on
        = set<C::String_conditional>(id + "_button", "image", state_handle);
    }
    else
    {
      conditional_handle_off = get<C::String_conditional>(id , "image");
      conditional_handle_on = get<C::String_conditional>(id + "_button", "image");
    }

    std::string skin_off = istate["skin"][0].string("images", "windows", "png");
    auto img_off
      = C::make_handle<C::Image>(id , "conditional_image", skin_off,
                                 Config::interface_depth, BOX);
    img_off->set_relative_origin(0.5, 0.5);
    img_off->on() = false;

    set<C::Absolute_position>(id , "position",
                                       Point(Config::world_width / 2,
                                             Config::world_height / 2));

    std::string skin_on = istate["skin"][1].string("images", "windows", "png");
    auto img_on
      = C::make_handle<C::Cropped>(id + "_button", "conditional_image",
                                   skin_on,
                                   Config::inventory_depth);
    img_on->set_relative_origin(0.5, 0.5);
    img_on->on() = false;

    set<C::Absolute_position>(id + "_button", "position",
                                       Point(Config::world_width / 2,
                                             Config::world_height / 2));

    conditional_handle_off->add(state, img_off);
    conditional_handle_on->add(state, img_on);
  }

  for (std::size_t j = 0; j < input["buttons"].size(); ++ j)
  {
    const Core::File_IO::Node& ibutton = input["buttons"][j];

    std::string value = ibutton["value"].string();

    const Core::File_IO::Node& coordinates = ibutton["coordinates"];
    code->add_button (value,
                      coordinates[0].integer(),
                      coordinates[1].integer(),
                      coordinates[2].integer(),
                      coordinates[3].integer());
  }

  for (std::size_t j = 0; j < input["answer"].size(); ++ j)
    code->add_answer_item (input["answer"][j].string());

  auto action = set<C::Action> (id , "action");
  for (std::size_t j = 0; j < input["on_success"].size(); ++ j)
  {
    std::string function = input["on_success"][j].nstring();
    action->add (function, input["on_success"][j][function].string_array());
  }

  // If code is global, so are the sounds/buttons
  if (signal (id, "is_global"))
  {
    emit(id + "_failure", "is_global");
    emit(id + "_success", "is_global");
    emit(id + "_button", "is_global");
  }
}

void File_IO::read_dialog (const std::string& id, const Core::File_IO::Node& input)
{
  auto dialog = set<C::Dialog>(id , "dialog",
                               input.has("end") ? input["end"].string() : "");

  // First, instantiate all vertices and regular edges
  std::unordered_map<std::string, C::Dialog::GVertex> targets;
  targets.insert (std::make_pair ("end", dialog->vertex_out()));

  std::vector<std::tuple<int, std::string, std::string, std::string, bool, bool>> go_to;
  std::string target = "";

  C::Dialog::GVertex latest_vertex = dialog->vertex_in();
  for (std::size_t i = 0; i < input["lines"].size(); ++ i)
  {
    const Core::File_IO::Node& l = input["lines"][i];

    if (l.has("choices"))
    {
      C::Dialog::GVertex vertex = dialog->add_vertex ();
      for (std::size_t j = 0; j < l["choices"].size(); ++ j)
      {
        const Core::File_IO::Node& c = l["choices"][j];
        std::string line = c["line"].string();
        std::string condition = "";
        bool unless = false;
        if (c.has("if"))
          condition = c["if"].string();
        else if (c.has("unless"))
        {
          condition = c["unless"].string();
          unless = true;
        }

        const std::string& target = c["goto"].string();
        bool displayed = true;
        if (c.has("displayed") && !c["displayed"].boolean())
          displayed = false;
        go_to.emplace_back (vertex, target, line, condition, unless, displayed);
      }

      if (latest_vertex != C::Dialog::GVertex())
        dialog->add_edge (latest_vertex, vertex);
      if (target != "")
      {
        targets.insert (std::make_pair (target, vertex));
        target = "";
      }
      latest_vertex = C::Dialog::GVertex();
    }
    else if (l.has("line"))
    {
      std::string character = l["line"][0].string();
      std::string line = l["line"][1].string();
      std::string signal = "";
      if (l["line"].size() == 3)
        signal = l["line"][2].string();
      C::Dialog::GVertex vertex = dialog->add_vertex (character, line, signal);
      if (latest_vertex != C::Dialog::GVertex())
        dialog->add_edge (latest_vertex, vertex);
      if (target != "")
      {
        targets.insert (std::make_pair (target, vertex));
        target = "";
      }
      latest_vertex = vertex;
    }

    if (l.has("target"))
      target = l["target"].string();

    if (l.has("goto"))
    {
      go_to.emplace_back (latest_vertex, l["goto"].string(), "", "", false, true);
      latest_vertex = C::Dialog::GVertex();
    }
  }

  // Then, add jump edges
  for (const auto& g : go_to)
    dialog->add_edge (std::get<0>(g), targets[std::get<1>(g)],
        std::get<2>(g), std::get<3>(g), std::get<4>(g), std::get<5>(g));
}

void File_IO::read_integer (const std::string& id, const Core::File_IO::Node& node)
{
  int value = node["value"].integer();
  auto integer = request<C::Int>(id , "value");
  if (!integer)
    integer = set<C::Int>(id , "value", value);

  if (node.has("triggers"))
    for (std::size_t i = 0; i < node["triggers"].size(); ++ i)
    {
      const Core::File_IO::Node& itrigger = node["triggers"][i];

      std::string value = itrigger["value"].string();

      auto action = set<C::Action>(id, value);

      for (std::size_t k = 0; k < itrigger["effect"].size(); ++ k)
      {
        std::string function = itrigger["effect"][k].nstring();
        action->add (function, itrigger["effect"][k][function].string_array());
      }
    }
}

void File_IO::read_object (const std::string& id, const Core::File_IO::Node& input)
{
  auto state_handle = get_or_set<C::String>(id , "state");

  std::string name = input["name"].string();
  int x = input["coordinates"][0].integer();
  int y = input["coordinates"][1].integer();
  int z = input["coordinates"][2].integer();
  bool box_collision = input["box_collision"].boolean();

  set<C::String>(id , "name", name);

  // Position might already exists if room was already loaded
  auto pos = request<C::Absolute_position>(id , "position");
  if (!pos)
    pos = set<C::Absolute_position>(id , "position", Point(x,y), false);
  else
    pos->is_interface() = false;

  if (input.has("label"))
  {
    int lx = input["label"][0].integer();
    int ly = input["label"][1].integer();
    if (input["label"][0].is_relative())
      set<C::Relative_position>(id, "label", pos, Vector (lx,ly));
    else
      set<C::Absolute_position>(id , "label", Point(lx,ly), false);
  }

  int vx = input["view"][0].integer();
  int vy = input["view"][1].integer();
  if (input["view"][0].is_relative())
    set<C::Relative_position>(id , "view", pos, Vector(vx,vy));
  else
    set<C::Absolute_position>(id , "view", Point(vx,vy), false);

  if (input.has("reach_factor"))
    set<C::Double>(id, "reach_factor", input["reach_factor"].floating());

  C::String_conditional_handle conditional_handle;
  for (std::size_t j = 0; j < input["states"].size(); ++ j)
  {
    const Core::File_IO::Node& istate = input["states"][j];

    std::string state = istate["id"].string();

    // init with first state found
    if (j == 0)
    {
      if (state_handle->value() == "")
      {
        state_handle->set(state);
        state_handle->mark_as_unaltered();
        debug << "Set state " << state << " to " << id << std::endl;
      }
      conditional_handle = set<C::String_conditional>(id , "image", state_handle);
    }
    else
      conditional_handle = get<C::String_conditional>(id , "image");

    if (!istate.has("skin") && !istate.has("size") && !istate.has("mask"))
      conditional_handle->add(state, nullptr);
    else
    {
      std::string skin = "";
      if (istate.has("skin"))
      {
        if (startswith(state, "inventory"))
          skin = istate["skin"].string("images", "inventory", "png");
        else
          skin = istate["skin"].string("images", "objects", "png");
      }
      else if (istate.has("mask"))
        skin = istate["mask"].string("images", "masks", "png");

      C::Image_handle img;
      if (istate.has("frames")) // Animation
      {
        int nb_frames = istate["frames"].integer();
        int duration = istate["duration"].integer();
        auto anim = C::make_handle<C::Animation>(id , "conditional_image",
                                                 skin, z,
                                                 nb_frames, 1, true,
                                                 (box_collision ? BOX : PIXEL_PERFECT),
                                                 true);
        anim->reset(true, duration);
        img = anim;
      }
      else
      {
        if (skin == "") // No skin, just an empty rectangle
        {
          int width = istate["size"][0].integer();
          int height = istate["size"][1].integer();
          img = C::make_handle<C::Image>(id , "conditional_image",
                                         width, height, 0, 0, 0, 0);
          img->z() = z;
        }
        else
        {
          img = C::make_handle<C::Image>(id , "conditional_image", skin, z,
                                         (box_collision ? BOX : PIXEL_PERFECT),
                                         true);
          if (istate.has("mask"))
            img->set_alpha(0);
        }
      }

      if (startswith (state, "inventory"))
      {
        img->set_relative_origin(0.5, 0.5);
        img->z() = Config::inventory_depth;
      }
      else
        img->set_relative_origin(0.5, 1.0);
      img->set_collision(box_collision ? BOX : PIXEL_PERFECT);

      debug << "Object " << id << ":" << state << " at position " << img->z() << std::endl;

      conditional_handle->add(state, img);
    }
  }

  for (std::string action : Config::possible_actions)
    if (input.has(action))
    {
      if (action == "inventory")
      {
        for (std::size_t i = 0; i < input["inventory"].size(); ++ i)
        {
          auto act = read_object_action(id, action, input["inventory"][i]);
          if (act.first) set(act.first);
          if (act.second) set(act.second);
        }
      }
      else
      {
        auto act = read_object_action(id, action, input[action]);
        if (act.first) set(act.first);
        if (act.second) set(act.second);
      }
    }
}

void File_IO::read_action (const std::string& id, const Core::File_IO::Node& node)
{
  if (node.has("label"))
    set<C::String>(id , "label", node["label"].string());

  if (node.has("states"))
  {
    auto state_handle = get_or_set<C::String>(id , "state");
    auto conditional_handle = set<C::String_conditional>(id , "action", state_handle);

    for (std::size_t i = 0; i < node["states"].size(); ++ i)
    {
      const Core::File_IO::Node& istate = node["states"][i];

      std::string state = istate["id"].string();
      if (i == 0 && state_handle->value() == "")
        state_handle->set(state);

      auto action = C::make_handle<C::Action>(id , "action");

      for (std::size_t k = 0; k < istate["effect"].size(); ++ k)
      {
        std::string function = istate["effect"][k].nstring();
        if (function == "func")
          parse_function (istate["effect"][k][function].string_array(), action);
        else
          action->add (function, istate["effect"][k][function].string_array());
      }
      conditional_handle->add(state, action);
    }
  }
  else
  {
    auto action = set<C::Action>(id , "action");

    for (std::size_t k = 0; k < node["effect"].size(); ++ k)
    {
      std::string function = node["effect"][k].nstring();
      if (function == "func")
        parse_function (node["effect"][k][function].string_array(), action);
      else
        action->add (function, node["effect"][k][function].string_array());
    }
  }
}

void File_IO::read_music(const std::string& id, const Core::File_IO::Node& node)
{
  // Do not reload if it's already playing
  if (auto current = request<C::Music>("Game", "music"))
    if (current->entity() == id)
    {
      debug << "DO NOT RELOAD MUSIC" << std::endl;
      set(current);
      return;
    }
  debug << "RELOAD MUSIC" << std::endl;
  SOSAGE_TIMER_START(File_IO__read_music);
  auto music = set<C::Music>(id, "music");

  for (std::size_t i = 0; i < node["tracks"].size(); ++ i)
  {
    std::string track = node["tracks"][i].string("sounds", "musics", "ogg");
    music->add_track (track);
  }

  for (std::size_t i = 0; i < node["sources"].size(); ++ i)
  {
    const Core::File_IO::Node& isource= node["sources"][i];
    std::string sid = isource["id"].string();
    std::vector<double> mix (music->tracks());
    for (std::size_t j = 0; j < music->tracks(); ++ j)
      mix[j] = isource["mix"][j].floating();
    if (isource.has("radius"))
      music->add_source (sid, mix,
                         isource["coordinates"][0].integer(),
                         isource["coordinates"][1].integer(),
                         isource["radius"][0].integer(),
                         isource["radius"][1].integer());
    else
      music->add_source (sid, mix);
  }
  music->init();
  SOSAGE_TIMER_STOP(File_IO__read_music);

}

std::pair<C::Handle, C::Handle>
File_IO::read_object_action (const std::string& id, const std::string& action,
                             const Core::File_IO::Node& node)
{
  if (node.size() != 0)
  {
    auto state_handle = get<C::String>(id , "state");
    auto label = set<C::String_conditional>(id + "_" + action , "label", state_handle);
    auto act = set<C::String_conditional>(id + "_" + action , "action", state_handle);

    for (std::size_t i = 0; i < node.size(); ++ i)
    {
      std::string state = node[i]["state"].string();
      auto labelact = read_object_action(id, action, node[i]);
      label->add (state, labelact.first);
      act->add (state, labelact.second);
    }

    return std::make_pair(label, act);
  }

  std::pair<C::Handle, C::Handle> out;

  std::string full_action = action;
  if (node.has("object"))
    full_action = action + "_" + node["object"].string();

  if (node.has("label"))
    out.first = C::make_handle<C::String>(id + "_" + action , "label", node["label"].string());

  if (node.has("right"))
    set<C::Boolean>(id + "_" + action , "right", node["right"].boolean());

  if (node.has("effect"))
  {
    auto act = C::make_handle<C::Action>(id + "_" + full_action , "action");
    for (std::size_t i = 0; i < node["effect"].size(); ++ i)
    {
      std::string function = node["effect"][i].nstring();
      if (function == "func")
        parse_function (node["effect"][i][function].string_array(), act);
      else
        act->add (function, node["effect"][i][function].string_array());
    }
    out.second = act;
  }

  if (node.has("state"))
  {
    auto state_handle = get<C::String>(id , "state");
    auto label = get_or_set<C::String_conditional>(id + "_" + action , "label", state_handle);
    auto act = get_or_set<C::String_conditional>(id + "_" + full_action , "action", state_handle);
    std::string state = node["state"].string();

    label->add (state, out.first);
    act->add (state, out.second);
    out.first = label;
    out.second = act;
  }

  // If object is global, actions are too
  if (signal(id, "is_global"))
  {
    emit (id + "_" + full_action, "is_global");
    emit (id + "_" + action, "is_global");
  }

  return out;
}

void File_IO::read_scenery (const std::string& id, const Core::File_IO::Node& node)
{
  int x = node["coordinates"][0].integer();
  int y = node["coordinates"][1].integer();
  int z = node["coordinates"][2].integer();

  auto pos = set<C::Absolute_position>(id , "position", Point(x,y), false);

  if (node.has("states"))
  {
    C::String_conditional_handle conditional_handle;

    auto state_handle = get_or_set<C::String>(id , "state");
    for (std::size_t i = 0; i < node["states"].size(); ++ i)
    {
      const Core::File_IO::Node& istate = node["states"][i];
      std::string state = istate["id"].string();

      if (i == 0)
      {
        if (state_handle->value() == "")
          state_handle->set(state);
        conditional_handle = set<C::String_conditional>(id , "image", state_handle);
      }
      else
        conditional_handle = get<C::String_conditional>(id , "image");

      if (!istate.has("skin"))
        conditional_handle->add(state, nullptr);
      else
      {
        std::string skin = istate["skin"].string("images", "scenery", "png");
        auto img = C::make_handle<C::Image>(id , "conditional_image",
                                            skin, z);
        img->set_collision(UNCLICKABLE);
        img->set_relative_origin(0.5, 1.0);
        conditional_handle->add (state, img);
      }
    }
  }
  else
  {
    std::string skin = node["skin"].string("images", "scenery", "png");

    load_locale_dependent_image
        (id, "image", skin,
         [&](const std::string& skin) -> C::Image_handle
    {
      auto img = C::make_handle<C::Image>(id, "image", skin, z);
      img->set_collision(UNCLICKABLE);
      img->set_relative_origin(0.5, 1.0);
      debug << "Scenery " << id << " at position " << img->z() << std::endl;
      return img;
    });
  }
}

void File_IO::read_sound (const std::string& id, const Core::File_IO::Node& node)
{
  std::string sound = node["sound"].string("sounds", "effects", "ogg");
  set<C::Sound>(id , "sound", sound);
  debug << "SOUND = " << id  << ":sound" << std::endl;
}

void File_IO::read_text (const std::string& id, const Core::File_IO::Node& node)
{
 std::string text = node["text"].string();
 if (node.has("coordinates"))
 {
   auto dialog_font = get<C::Font> ("Dialog", "font");
   std::string default_color = "000000";
   std::string color = (node.has("color") ? node["color"].string() : default_color);
   int x = node["coordinates"][0].integer();
   int y = node["coordinates"][1].integer();
   set<C::Absolute_position>(id , "position", Point(x,y));
   create_locale_dependent_text (id, dialog_font, color, text);
 }
 else
   set<C::String>(id, "text", text);
}

void File_IO::read_window (const std::string& id, const Core::File_IO::Node& node)
{
  std::string skin = node["skin"].string("images", "windows", "png");

  load_locale_dependent_image
      (id , "image", skin,
       [&](const std::string& skin) -> C::Image_handle
  {
    auto img = C::make_handle<C::Image>(id , "image", skin,
                                        Config::interface_depth);
    img->set_relative_origin(0.5, 0.5);
    img->on() = false;
    return img;
  });
  set<C::Absolute_position>(id , "position",
                            Point(Config::world_width / 2,
                                  Config::world_height / 2));
}

} // namespace Sosage::System
