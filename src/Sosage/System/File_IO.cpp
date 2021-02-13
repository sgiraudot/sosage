/*
  [src/Sosage/System/File_IO.cpp]
  Reads/writes savegames/configs.

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
#include <Sosage/Component/Cutscene.h>
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

File_IO::File_IO (Content& content)
  : Base (content)
{
}

void File_IO::run()
{
  if (auto new_room = request<C::String>("Game:new_room"))
  {
    if (request<C::String>("Game:new_room_origin"))
      read_room (new_room->value());
    else
    {
      read_cutscene (new_room->value());
      get<C::Status>(GAME__STATUS)->push (CUTSCENE);
    }
    remove ("Game:new_room");
  }
}

void File_IO::clean_content()
{
  bool full_reset = receive ("Game:reset");

  std::unordered_set<std::string> force_keep;
  auto inventory = get<C::Inventory>("Game:inventory");
  if (!full_reset)
  {
    for (const std::string& entity : *inventory)
      force_keep.insert (entity);

    const std::string& player = get<C::String>("Player:name")->value();
    force_keep.insert (player);
    force_keep.insert (player + "_body");
    force_keep.insert (player + "_head");
    force_keep.insert (player + "_mouth");
    force_keep.insert (player + "_walking");
    force_keep.insert (player + "_idle");
  }
  else
  {
    for (const std::string& entity : *inventory)
      get<C::Image>(entity + ":image")->on() = false;
    inventory->clear();
  }

  m_content.clear
    ([&](C::Handle c) -> bool
     {
       if (!full_reset)
       {
         // keep inventory + other forced kept
         if (force_keep.find(c->entity()) != force_keep.end())
           return false;

         // keep states and positions
         if (c->component() == "state" || c->component() == "position")
           return false;

         // keep integers
         if (C::cast<C::Int>(c))
           return false;
       }

       // else, remove component if belonged to the latest room
       return (m_latest_room_entities.find(c->entity()) != m_latest_room_entities.end());
     });
}

void File_IO::read_config()
{
  std::string file_name = Sosage::pref_path() + "config.yaml";

  // Default config values
  bool fullscreen = !Config::emscripten;
  bool virtual_cursor = false;

  int dialog_speed = Config::MEDIUM_SPEED;
  int dialog_size = Config::MEDIUM;

  int music_volume = 7;
  int sounds_volume = 9;

  bool autosave = true;
  bool hints = true;

#ifdef SOSAGE_EMSCRIPTEN
  int window_width = 640;
  int window_height = 400;
#else
  int window_width = -1;
  int window_height = -1;
#endif

  try
  {
    Core::File_IO input (file_name);
    input.parse();
    if (input.has("fullscreen")) fullscreen = input["fullscreen"].boolean();
    if (input.has("virtual_cursor")) virtual_cursor = input["virtual_cursor"].boolean();
    if (input.has("dialog_speed")) dialog_speed = input["dialog_speed"].floating();
    if (input.has("dialog_size")) dialog_size = input["dialog_size"].floating();
    if (input.has("music_volume")) music_volume = input["music_volume"].integer();
    if (input.has("sounds_volume")) sounds_volume = input["sounds_volume"].integer();
    if (input.has("autosave")) autosave = input["autosave"].boolean();
    if (input.has("hints")) hints = input["hints"].boolean();
    if (input.has("window"))
    {
      window_width = input["window"][0].integer();
      window_height = input["window"][1].integer();
    }
  }
  catch (Sosage::No_such_file&)
  {
    if constexpr (Config::android)
    {
      emit("Show:menu");
      set<C::String>("Game:triggered_menu", "Cursor");
    }
  }

  set<C::Boolean>("Window:fullscreen", fullscreen);
  set<C::Boolean>("Interface:virtual_cursor", virtual_cursor);

  set<C::Int>("Dialog:speed", dialog_speed);
  set<C::Int>("Dialog:size", dialog_size);

  set<C::Int>("Music:volume", music_volume);
  set<C::Int>("Sounds:volume", sounds_volume);

  set<C::Boolean>("Game:autosave", autosave);
  set<C::Boolean>("Game:hints_on", hints);

  set<C::Int>("Window:width", window_width);
  set<C::Int>("Window:height", window_height);
}

void File_IO::write_config()
{
  Core::File_IO output (Sosage::pref_path() + "config.yaml", true);

  output.write ("fullscreen", get<C::Boolean>("Window:fullscreen")->value());
  output.write ("virtual_cursor", get<C::Boolean>("Interface:virtual_cursor")->value());

  output.write ("dialog_speed", get<C::Int>("Dialog:speed")->value());
  output.write ("dialog_size", get<C::Int>("Dialog:size")->value());

  output.write ("music_volume", get<C::Int>("Music:volume")->value());
  output.write ("sounds_volume", get<C::Int>("Sounds:volume")->value());

  output.write ("autosave", get<C::Boolean>("Game:autosave")->value());
  output.write ("hints", get<C::Boolean>("Game:hints_on")->value());

  output.write ("window",
                get<C::Int>("Window:width")->value(),
                get<C::Int>("Window:height")->value());
}

void File_IO::read_savefile()
{
  std::string file_name = Sosage::pref_path() + "save.yaml";
  Core::File_IO input (file_name);
  input.parse();
  set<C::String>("Game:new_room", input["room"].string());
  set<C::String>("Game:new_room_origin", "Saved_game");

  auto visited = get<C::Set<std::string> >("Game:visited_rooms");
  for (std::size_t i = 0; i < input["visited_rooms"].size(); ++ i)
    visited->insert(input["visited_rooms"][i].string());

  auto inventory = get<C::Inventory>("Game:inventory");
  for (std::size_t i = 0; i < input["inventory"].size(); ++ i)
    inventory->add(input["inventory"][i].string());

  double camera_target = input["camera"].floating();
  get<C::Double>(CAMERA__POSITION)->set (camera_target);
  set<C::Double>("Camera:target")->set (camera_target);
  auto action = set<C::Action>("Saved_game:action");
  action->add ("play", { "music", input["music"].string() });
  action->add ("camera", { "fadein", "0.5" });

  for (std::size_t i = 0; i < input["states"].size(); ++ i)
  {
    const Core::File_IO::Node& istate = input["states"][i];
    set<C::String>(istate["id"].string() + ":state", istate["value"].string());
  }

  for (std::size_t i = 0; i < input["positions"].size(); ++ i)
  {
    const Core::File_IO::Node& iposition = input["positions"][i];
    Point point (iposition["value"][0].floating(), iposition["value"][1].floating());
    set<C::Absolute_position>(iposition["id"].string() + ":position", point);
  }

  for (std::size_t i = 0; i < input["integers"].size(); ++ i)
  {
    const Core::File_IO::Node& iint = input["integers"][i];
    set<C::Int>(iint["id"].string() + ":value", iint["value"].integer());
  }

  for (std::size_t i = 0; i < input["visibility"].size(); ++ i)
  {
    const Core::File_IO::Node& ivisibility = input["visibility"][i];
    set<C::Boolean>(ivisibility["id"].string() + ":visible",
                    ivisibility["value"].boolean());
  }

  for (std::size_t i = 0; i < input["active_animations"].size(); ++ i)
  {
    const Core::File_IO::Node& ianimation = input["active_animations"][i];
    action->add ("play", { "animation", ianimation.string() });
  }

  if (input.has("dialog"))
  {
    action->add ("dialog", { input["dialog"].string() });
    set<C::Int>("Saved_game:dialog_position", input["dialog_position"].integer());
  }
}

void File_IO::write_savefile()
{
  Core::File_IO output (Sosage::pref_path() + "save.yaml", true);

  output.write("room", get<C::String>("Game:current_room")->value());
  output.write("camera", get<C::Double>(CAMERA__POSITION)->value());
  output.write("inventory", get<C::Inventory>("Game:inventory")->data());
  output.write("music", get<C::Music>("Game:music")->entity());

  if (auto dialog = request<C::String>("Game:current_dialog"))
  {
    output.write("dialog", dialog->value());
    output.write("dialog_position", get<C::Int>("Game:dialog_position")->value());
  }

  output.start_section("visited_rooms");
  for (const std::string& room_name : *get<C::Set<std::string> >("Game:visited_rooms"))
    output.write_list_item (room_name);
  output.end_section();

  output.start_section("states");
  for (C::Handle c : m_content)
    if (!c->is_system())
      if (auto s = C::cast<C::String>(c))
        if (c->component() == "state")
        output.write_list_item ("id", c->entity(), "value", s->value());
  output.end_section();

  output.start_section("positions");
  for (C::Handle c : m_content)
    if (c->component() == "position" && c->entity() != "Cursor" && c->entity() != "Loading_spin")
      if (auto pos = C::cast<C::Position>(c))
        output.write_list_item ("id", c->entity(), "value",
                                { pos->value().x(), pos->value().y() });

  output.end_section();

  output.start_section("integers");
  for (C::Handle c : m_content)
    if (!c->is_system())
      if (auto i = C::cast<C::Int>(c))
        output.write_list_item ("id", c->entity(), "value", i->value());
  output.end_section();

  output.start_section("visibility");
  for (C::Handle c : m_content)
    if (!c->is_system())
      if (auto b = C::cast<C::Boolean>(c))
        if (b->component() == "visible")
        output.write_list_item ("id", b->entity(), "value", b->value());
  output.end_section();

  output.start_section("active_animations");
  for (C::Handle c : m_content)
    if (!c->is_system())
      if (auto a = C::cast<C::Animation>(c))
        if (a->on() && a->loop())
          if (auto s = request<C::String>(a->entity() + ":state"))
            if (s->value() == "Dummy")
              output.write_list_item (a->entity());
  output.end_section();
}


void File_IO::read_init (const std::string& folder_name)
{
  m_folder_name = folder_name;
  std::string file_name = folder_name + "data" + Config::folder_separator + "init.yaml";

  Core::File_IO input (file_name);
  input.parse();

  std::string v = input["version"].string();
  check (Version::parse(v) <= Version::get(),
         "Error: room version " + v + " incompatible with Sosage " + Version::str());

  std::string game_name = input["name"].string();
  set<C::String>("Game:name", game_name);

  std::string icon = input["icon"].string("images", "interface", "png");
  set<C::String>("Icon:filename", local_file_name(icon));

  std::string cursor = input["cursor"][0].string("images", "interface", "png");
  auto cursor_default = C::make_handle<C::Image> ("Cursor:image", local_file_name(cursor),
                                                              Config::cursor_depth);
  cursor_default->set_relative_origin(0.1, 0.1);

  std::string cursor_o = input["cursor"][1].string("images", "interface", "png");
  auto cursor_object = C::make_handle<C::Image> ("Cursor:image", local_file_name(cursor_o),
                                                              Config::cursor_depth);
  cursor_object->set_relative_origin(0.5, 0.5);

  auto cursor_state = set<C::String>("Cursor:state", "default");
  auto cursor_img = C::make_handle<C::String_conditional>("Cursor:image", cursor_state);
  cursor_img->add("default", cursor_default);
  cursor_img->add("object", cursor_object);

  auto status = get<C::Status>(GAME__STATUS);

  if constexpr (Config::android)
  {
    // Cursor displayed = NOT paused AND virtual
    set<C::Conditional>
        ("Cursor:conditional",
         C::make_and
         (get<C::Condition>("Unlocked:condition"),
          get<C::Boolean>("Interface:virtual_cursor")),
         cursor_img);
  }
  else
  {
    // Cursor displayed = NOT paused
    set<C::Conditional>
        ("Cursor:conditional",
         get<C::Condition>("Unlocked:condition"),
         cursor_img);
  }

  set_fac<C::Absolute_position> (CURSOR__POSITION, "Cursor:position",
                                 (get<C::Boolean>("Interface:virtual_cursor")->value()
                                  ? Point(Config::world_width / 2, Config::world_height / 2) : Point(0,0)));

  std::string loading_spin = input["loading_spin"][0].string("images", "interface", "png");
  int nb_img = input["loading_spin"][1].integer();
  auto loading_spin_img = set_fac<C::Animation> (LOADING_SPIN__IMAGE, "Loading_spin:image", local_file_name(loading_spin),
                                                                   Config::loading_depth, nb_img, 1, true);
  loading_spin_img->on() = false;
  loading_spin_img->set_relative_origin(0.5, 0.5);
  set_fac<C::Absolute_position> (LOADING_SPIN__POSITION, "Loading_spin:position",
                                 Point(Config::world_width / 2,
                                       Config::world_height / 2));

  std::string left_arrow = input["inventory_arrows"][0].string("images", "interface", "png");
  auto left_arrow_img = set<C::Image>("Left_arrow:image", local_file_name(left_arrow));
  left_arrow_img->on() = false;
  std::string right_arrow = input["inventory_arrows"][1].string("images", "interface", "png");
  auto right_arrow_img = set<C::Image>("Left_arrow:image", local_file_name(right_arrow));
  right_arrow_img->on() = false;

  std::string chamfer = input["inventory_chamfer"].string("images", "interface", "png");
  auto chamfer_img = set<C::Image>("Chamfer:image", local_file_name(chamfer));
  chamfer_img->z() = Config::interface_depth;

  std::string click_sound = input["click_sound"].string("sounds", "effects", "ogg");
  set<C::Sound>("Click:sound", local_file_name(click_sound));

  std::string left_circle = input["circle"][0].string("images", "interface", "png");
  auto left_circle_img = set<C::Image>("Left_circle:image", local_file_name(left_circle), 1, BOX);
  left_circle_img->on() = false;
  std::string right_circle = input["circle"][1].string("images", "interface", "png");
  auto right_circle_img = set<C::Image>("Right_circle:image", local_file_name(right_circle), 1, BOX);
  right_circle_img->on() = false;

  std::string debug_font = input["debug_font"].string("fonts", "ttf");
  set<C::Font> ("Debug:font", local_file_name(debug_font), 40);

  std::string interface_font = input["interface_font"].string("fonts", "ttf");
  set<C::Font> ("Interface:font", local_file_name(interface_font), 80);

  std::string dialog_font = input["dialog_font"].string("fonts", "ttf");
  set<C::Font> ("Dialog:font", local_file_name(dialog_font), 80);

  std::string menu_color = input["menu_color"].string();
  set<C::String> ("Menu:color", menu_color);

  std::string logo_id = input["menu_logo"].string("images", "interface", "png");
  auto logo
    = set<C::Image> ("Menu_logo:image", local_file_name(logo_id));
  logo->on() = false;
  auto credits_logo
    = set<C::Image> ("Credits_logo:image", local_file_name(logo_id));
  credits_logo->on() = false;


  std::string credits_id = input["credits_image"].string("images", "interface", "png");
  auto credits
    = set<C::Image> ("Credits_text:image", local_file_name(credits_id));
  credits->on() = false;

  std::string left_arrow_id = input["menu_arrows"][0].string("images", "interface", "png");
  auto arrow_left = set<C::Image>("Menu_left_arrow:image", local_file_name(left_arrow_id));
  arrow_left->set_relative_origin(0, 0.5);
  arrow_left->on() = false;

  std::string right_arrow_id = input["menu_arrows"][1].string("images", "interface", "png");
  auto arrow_right = set<C::Image>("Menu_right_arrow:image", local_file_name(right_arrow_id));
  arrow_right->set_relative_origin(1, 0.5);
  arrow_right->on() = false;

  for (std::size_t i = 0; i < input["text"].size(); ++ i)
  {
    const Core::File_IO::Node& itext = input["text"][i];
    std::string id = itext["id"].string();
    id[0] = toupper(id[0]); // system id start with uppercase
    set<C::String>(id + ":text", itext["value"].string());
  }

  for (std::string id : Config::possible_actions)
  {
    const Core::File_IO::Node& idefault = input["default"][id];
    std::string label = idefault["label"].string();

    set<C::String>("Default_" + id + ":label", label);

    if (idefault.has("effect"))
    {
      auto action = set<C::Random_conditional>("Default_" + id + ":action");
      for (std::size_t j = 0; j < idefault["effect"].size(); ++ j)
      {
        const Core::File_IO::Node& iaction = idefault["effect"][j];
        std::string function = iaction.nstring();

        auto rnd_action = C::make_handle<C::Action>
                          ("Default_" + id + ":" + std::to_string(j));
        rnd_action->add ("look", {});
        rnd_action->add (function, iaction[function].string_array());
        action->add (1.0, rnd_action);
      }
    }
  }

  set<C::String>("Inventory:label", input["default"]["inventory_button"].string());

  std::string player = input["player"].string();
  set<C::String>("Player:name", player);

  if (input.has("load_room"))
  {
    set<C::String>("Game:init_new_room", input["load_room"][0].string());
    set<C::String>("Game:init_new_room_origin", input["load_room"][1].string());
  }
  else
  {
    check (input.has("load_cutscene"), "Init should either load a room or a cutscene");
    set<C::String>("Game:init_new_room", input["load_cutscene"].string());
  }

  try
  {
    read_savefile();
  }
  catch(Sosage::No_such_file&)
  {
    set<C::Variable>("Game:new_room", get<C::String>("Game:init_new_room"));
    if (auto orig = request<C::String>("Game:init_new_room_origin"))
      set<C::Variable>("Game:new_room_origin", orig);
  }
}

void File_IO::read_cutscene (const std::string& file_name)
{
  auto callback = get<C::Simple<std::function<void()> > >("Game:loading_callback");
  callback->value()();

  clean_content();

  SOSAGE_TIMER_START(File_IO__read_cutscene);

  Core::File_IO input (local_file_name("data", "cutscenes", file_name, "yaml"));
  input.parse();

  callback->value()();

  std::string name = input["name"].string();

  auto interface_font = get<C::Font> ("Interface:font");
  std::string color = "000000";

  std::unordered_map<std::string, const Core::File_IO::Node*> map_id2node;

  for (std::size_t i = 0; i < input["content"].size(); ++ i)
  {
    const Core::File_IO::Node& node = input["content"][i];
    std::string id = node["id"].string();
    map_id2node.insert (std::make_pair (id, &node));
    m_latest_room_entities.insert (id);

    if (node.has("music")) // Music
    {
      std::string music = node["music"].string("sounds", "musics", "ogg");
      set<C::Music>(id + ":music", local_file_name(music));
      callback->value()();
      continue;
    }

    C::Image_handle img;
    if (node.has("loop")) // Animation
    {
      std::string skin = node["skin"].string("images", "cutscenes", "png");
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
      auto anim = set<C::Animation>(id + ":image", local_file_name(skin), 1,
                                    width, height, node["loop"].boolean());
      int duration = (node.has("duration") ? node["duration"].integer() : 1);

      if (node.has("frames"))
      {
        anim->frames().clear();
        for (std::size_t i = 0; i < node["frames"].size(); ++ i)
        {
          int idx = node["frames"][i].integer();
          int x = idx % width;
          int y = idx / width;
          anim->frames().emplace_back (x, y, duration);
        }
      }
      else
        anim->reset(true, duration);
      img = anim;
    }
    else if (node.has("skin")) // Image
    {
      std::string skin = node["skin"].string("images", "cutscenes", "png");
      img = set<C::Image>(id + ":image", local_file_name(skin));
    }
    else // Text
    {
      check (node.has("text"), "Node should either have music, image or text");
      std::string text = node["text"].string();
      img = set<C::Image>(id + ":image", interface_font, color, text);
    }
    img->set_collision(UNCLICKABLE);
    img->on() = false;
    callback->value()();
  }

  std::unordered_map<std::string, double> map_id2begin;

  auto cutscene = set<C::Cutscene>("Game:cutscene");
  for (std::size_t i = 0; i < input["timeline"].size(); ++ i)
  {
    const Core::File_IO::Node& node = input["timeline"][i];
    double time = node["time"].time();

    // Special case if cutscene ends by loading room
    if (node.has("load"))
    {
      cutscene->add (node["load"].string(), time);
      continue;
    }

    std::vector<std::string> elements;
    std::string type = node.has("begin") ? "begin" : "end";

    if (node[type].size() == 0)
      elements.emplace_back (node[type].string());
    else
      elements = node[type].string_array();

    if (type == "begin")
      for (const std::string& el : elements)
        map_id2begin[el] = time;
    else
      for (const std::string& el : elements)
      {
        auto found = map_id2begin.find(el);
        check (found != map_id2begin.end(), "Ending cutscene element " + el + " with no beginning");
        double begin = found->second;

        auto found_node = map_id2node.find(el);
        if (found_node == map_id2node.end())
        {
          cutscene->add(el, begin, time);
          continue;
        }

        const Core::File_IO::Node& el_node = *found_node->second;

        if (el_node.has("coordinates"))
        {
          int x = el_node["coordinates"][0].integer();
          int y = el_node["coordinates"][1].integer();
          int z = el_node["coordinates"][2].integer();
          double zoom = 1.;
          if (el_node.has("text"))
            zoom = 0.75;
          cutscene->add (el + ":image", begin, time, x, y, z, zoom);
        }
        else if (el_node.has("keyframes"))
        {
          C::Cutscene::Element& element = cutscene->add(el + ":image");
          for (std::size_t j = 0; j < el_node["keyframes"].size(); ++ j)
          {
            const Core::File_IO::Node& keyframe = el_node["keyframes"][j];
            double relative_time = keyframe["time"].floating();
            int x = keyframe["coordinates"][0].integer();
            int y = keyframe["coordinates"][1].integer();
            int z = keyframe["coordinates"][2].integer();
            double zoom = keyframe.has("zoom") ? keyframe["zoom"].floating() : 1;

            double real_time = relative_time * time + (1. - relative_time) * begin;
            if (relative_time == 0)
              real_time = begin;
            else if (relative_time == 1)
              real_time = time;

            element.keyframes.emplace_back (real_time, x, y, z, zoom);
          }
        }
        else
          cutscene->add (el + ":music", begin, time);
      }
    callback->value()();
  }

  emit ("Window:rescaled");
  cutscene->finalize();

  SOSAGE_TIMER_STOP(File_IO__read_cutscene);
}

std::string File_IO::local_file_name (const std::string& file_name) const
{
  return m_folder_name + file_name;
}

std::string File_IO::local_file_name (const std::string& folder, const std::string& subfolder,
                                      const std::string& file_name, const std::string& extension) const
{
  return m_folder_name + folder + Config::folder_separator + subfolder + Config::folder_separator
    + file_name + '.' + extension;
}

} // namespace Sosage::System
