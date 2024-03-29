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
#include <Sosage/Component/Font.h>
#include <Sosage/Component/Ground_map.h>
#include <Sosage/Component/Image.h>
#include <Sosage/Component/Inventory.h>
#include <Sosage/Component/Locale.h>
#include <Sosage/Component/Music.h>
#include <Sosage/Component/Position.h>
#include <Sosage/Component/Simple.h>
#include <Sosage/Component/Sound.h>
#include <Sosage/Component/Status.h>
#include <Sosage/Component/Variable.h>
#include <Sosage/Config/platform.h>
#include <Sosage/Config/version.h>
#include <Sosage/System/File_IO.h>
#include <Sosage/Utils/Asset_manager.h>
#include <Sosage/Utils/color.h>
#include <Sosage/Utils/conversions.h>
#include <Sosage/Utils/Gamepad_info.h>
#include <Sosage/Utils/helpers.h>
#include <Sosage/Utils/locale.h>
#include <Sosage/Utils/profiling.h>

#include <locale>

#define INIT_DISPATCHER(id, func) \
  m_dispatcher.insert (std::make_pair(id, std::bind(&File_IO::func, this, std::placeholders::_1, std::placeholders::_2)))


namespace Sosage::System
{

namespace C = Component;

File_IO::File_IO (Content& content)
  : Base (content)
{
  INIT_DISPATCHER("actions", read_action);
  INIT_DISPATCHER("animations", read_animation);
  INIT_DISPATCHER("characters", read_character);
  INIT_DISPATCHER("codes", read_code);
  INIT_DISPATCHER("dialogs", read_dialog);
  INIT_DISPATCHER("integers", read_integer);
  INIT_DISPATCHER("objects", read_object);
  INIT_DISPATCHER("musics", read_music);
  INIT_DISPATCHER("scenery", read_scenery);
  INIT_DISPATCHER("sounds", read_sound);
  INIT_DISPATCHER("texts", read_text);
  INIT_DISPATCHER("windows", read_window);
}

void File_IO::run()
{
  SOSAGE_TIMER_START(System_File_IO__run);
  SOSAGE_UPDATE_DBG_LOCATION("File_IO::run()");

  if (receive("Game", "load"))
  {
    clean_content();
    auto save_id = get<C::String>("Savegame", "id");
    read_savefile (save_id->value());
    remove (save_id);
  }

  if (auto new_room = request<C::String>("Game", "new_room"))
  {
    receive ("Time", "speedup");
    read_room (new_room->value());
    remove ("Game", "new_room");
    emit ("Game", "clear_notifications");
  }

  if (receive("Game", "save"))
    write_savefile();

  SOSAGE_TIMER_STOP(System_File_IO__run);
}

void File_IO::clean_content()
{
  bool full_reset = receive ("Game", "reset");

  std::unordered_set<std::string> force_keep;
  auto inventory = get<C::Inventory>("Game", "inventory");

  if (full_reset)
  {
    // Reset camera, zoom, etc.
    auto blackscreen = get<C::Image>("Blackscreen", "image");
    blackscreen->on() = true;
    blackscreen->set_alpha(255);

    get<C::Absolute_position>(CAMERA__POSITION)->set (Point(0,0));
    get<C::Double>(CAMERA__ZOOM)->set(1.);
    get<C::Status>(GAME__STATUS)->reset();
    get<C::Action>("Logic", "action")->clear();
    remove ("Character", "action", true);
    remove ("Character", "triggered_action", true);
    remove ("Game", "current_dialog", true);

    for (const std::string& entity : *inventory)
      get<C::Image>(entity , "image")->on() = false;
    inventory->clear();
  }
  else
  {
    if (auto phone_numbers = request<C::Vector<std::string>>("phone_numbers", "list"))
    {
      force_keep.insert("phone_numbers");
      for (const std::string& entity : phone_numbers->value())
        force_keep.insert (entity);
    }
  }

  m_content.clear
    ([&](C::Handle c) -> bool
     {
       if (!full_reset)
       {
         // keep inventory + other forced kept
         if (contains(force_keep, c->entity()))
           return false;

         // keep global objects
         if (signal(c->entity(), "is_global"))
           return false;

         // keep states, positions, signals and music info
         if (c->component() == "state" || c->component() == "position"
             || c->component() == "signal" || c->component() == "resume_at")
           return false;

         // keep integers
         if (C::cast<C::Int>(c))
           return false;
       }

       // Remove finishing actions
       if (startswith(c->entity(), "Finishing_"))
         return true;

       // Force remove comments
       if (startswith(c->entity(), "Comment"))
         return true;

       // Keep system components
       return !c->is_system();
     });

  if (full_reset)
  {
    // Remove player/follower
    remove("Player", "name", true);
    remove("Follower", "name", true);

    // Reload global objects when restarting the game
    Core::File_IO input ("data/init.yaml");
    input.parse();
    read_init_global_items (input);
  }

  emit ("Game", "clear_managers");
  emit ("Dialog", "clean");
}

void File_IO::read_config()
{
  // Default config values
  std::string locale = "";
  bool fullscreen = !Config::emscripten;
  int input_mode = (Config::android ? TOUCHSCREEN : MOUSE);

  int dialog_speed = Config::MEDIUM_SPEED;
  int interface_scale = Config::SMALL;

  int music_volume = 7;
  int sounds_volume = 9;

  bool autosave = true;

#ifdef SOSAGE_EMSCRIPTEN
  int window_width = 640;
  int window_height = 400;
#else
  int window_width = -1;
  int window_height = -1;
#endif

  Core::File_IO input ("config" + value<C::String>("Save", "suffix", "") +  ".yaml", true);
  if (input.parse())
  {
    if (input.has("locale")) locale = input["locale"].string();
    if (input.has("fullscreen")) fullscreen = input["fullscreen"].boolean();
    if (input.has("input_mode")) input_mode = input["input_mode"].integer();

    if (input.has("gamepads"))
      for (std::size_t i = 0; i < input["gamepads"].size(); ++ i)
      {
        const auto& node = input["gamepads"][i];
        set<C::Simple<Gamepad_info>>("Gamepad(" + node["id"].string() +")",
                                     "gamepad",
                                     Gamepad_info(node["id"].string(),
                                                  Gamepad_labels(node["labels"].integer()),
                                                  node["ok_down"].boolean()));
      }


    if (input.has("dialog_speed")) dialog_speed = input["dialog_speed"].floating();
    if (input.has("interface_scale"))
      interface_scale = input["interface_scale"].integer();
    if (input.has("music_volume")) music_volume = input["music_volume"].integer();
    if (input.has("sounds_volume")) sounds_volume = input["sounds_volume"].integer();
    if (input.has("autosave")) autosave = input["autosave"].boolean();
    if (input.has("window"))
    {
      window_width = input["window"][0].integer();
      window_height = input["window"][1].integer();
    }
  }

  // No sound for automatic tests
  if (signal("Game", "test_input_mode"))
  {
    music_volume = 0;
    sounds_volume = 0;
  }

  set_fac<C::String>(GAME__CURRENT_LOCAL, "Game", "current_locale", locale);
  set<C::Boolean>("Window", "fullscreen", fullscreen);
  debug << "INPUT MODE = " << input_mode << std::endl;
  set_fac<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE, "Interface", "input_mode", Input_mode(input_mode));

  set<C::Int>("Dialog", "speed", dialog_speed);
  set<C::Int>("Interface", "scale", interface_scale);
  Config::interface_scale = interface_scale / double(6.);

  set<C::Int>("Music", "volume", music_volume);
  set<C::Int>("Sounds", "volume", sounds_volume);

  set<C::Boolean>("Game", "autosave", autosave);

  set<C::Int>("Window", "width", window_width);
  set<C::Int>("Window", "height", window_height);
}

void File_IO::write_config()
{
  Core::File_IO output ("config" + value<C::String>("Save", "suffix", "") +  ".yaml", true, true);

  output.write ("locale", value<C::String>(GAME__CURRENT_LOCAL));
  output.write ("fullscreen", value<C::Boolean>("Window", "fullscreen"));
  output.write ("input_mode", value<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE));

  output.start_section ("gamepads");
  for (C::Handle c : components("gamepad"))
      if (auto info = C::cast<C::Simple<Gamepad_info>>(c))
      output.write_list_item("id", info->value().id,
                             "labels", info->value().labels,
                             "ok_down", info->value().ok_down);
  output.end_section();


  output.write ("dialog_speed", value<C::Int>("Dialog", "speed"));
  output.write ("interface_scale", value<C::Int>("Interface", "scale"));

  output.write ("music_volume", value<C::Int>("Music", "volume"));
  output.write ("sounds_volume", value<C::Int>("Sounds", "volume"));

  output.write ("autosave", value<C::Boolean>("Game", "autosave"));

  output.write ("window", value<C::Int>("Window", "width"), value<C::Int>("Window", "height"));
}

bool File_IO::read_savefile (const std::string& save_id)
{
  Core::File_IO input ("save" + value<C::String>("Save", "suffix", "") +
                       + "_" + save_id + ".yaml", true);
  if (!input.parse())
  {
    return false;
  }

  get<C::Double>(CLOCK__SAVED_TIME)->set(input["time"].floating());

  set<C::String>("Game", "new_room", input["room"].string());
  set<C::String>("Game", "new_room_origin", "Saved_game");

  std::string player = input["player"].string();
  set<C::String>("Player", "name", player);
  if (input.has("follower"))
  {
    std::string follower = input["follower"].string();
    set<C::String>("Follower", "name", follower);
  }

  auto inventory = get<C::Inventory>("Game", "inventory");
  for (std::size_t i = 0; i < input["inventory"].size(); ++ i)
    inventory->add(input["inventory"][i].string());

  if (input.has("phone_numbers"))
  {
    auto numbers = set<C::Vector<std::string>>("phone_numbers", "list");
    for (std::size_t i = 0; i < input["phone_numbers"].size(); ++ i)
      numbers->push_back(input["phone_numbers"][i].string());
  }

  double camera_target = input["camera"].floating();
  get<C::Absolute_position>(CAMERA__POSITION)->set (Point(camera_target, 0));
  auto action = set<C::Action>("Saved_game", "action");

  if (input.has("music"))
  {
    for (std::size_t i = 0; i < input["music_disabled_sources"].size(); ++ i)
      action->add ("fadeout", { input["music"].string(), input["music_disabled_sources"][i].string(), "0" });
    action->add ("play", { input["music"].string(), "0.5" });
  }
  action->add ("fadein", { "0.5" });

  if (input.has("music_positions"))
    for (std::size_t i = 0; i < input["music_positions"].size(); ++ i)
    {
      const Core::File_IO::Node& iresume = input["music_positions"][i];
      auto ra = set<C::Double>(iresume["id"].string(), "resume_at",
          iresume["value"].floating());
    }

  std::unordered_map<std::string, std::string> looking_right;
  std::unordered_map<std::string, std::string> char_anims;
  for (std::size_t i = 0; i < input["characters"].size(); ++ i)
  {
    const Core::File_IO::Node& ichar = input["characters"][i];
    looking_right.insert (std::make_pair(ichar["id"].string(), ichar["value"].string()));
    if (ichar.has("animation"))
      char_anims.insert (std::make_pair (ichar["id"].string(), ichar["animation"].string()));
  }

  for (std::size_t i = 0; i < input["states"].size(); ++ i)
  {
    const Core::File_IO::Node& istate = input["states"][i];
    auto state = get_or_set<C::String>(istate["id"].string() , "state");
    state->set (istate["value"].string());
    state->mark_as_altered();
  }

  for (std::size_t i = 0; i < input["signals"].size(); ++ i)
  {
    const Core::File_IO::Node& isignal = input["signals"][i];
    emit (isignal.string(), "signal");
  }

  if (input.has("achievements"))
    for (std::size_t i = 0; i < input["achievements"].size(); ++ i)
    {
      const Core::File_IO::Node& iach = input["achievements"][i];
      emit (iach["id"].string(), "done");
      if (iach["stored"].boolean())
        emit (iach["id"].string(), "stored");
    }

  for (std::size_t i = 0; i < input["positions"].size(); ++ i)
  {
    const Core::File_IO::Node& iposition = input["positions"][i];
    std::string id = iposition["id"].string();
    const auto& values = iposition["value"];
    Point point (values[0].floating(), values[1].floating());
    auto pos = set<C::Absolute_position>(id, "position", point, false);
    pos->mark_as_altered();

    auto iter = looking_right.find (id);
    if (iter != looking_right.end())
    {
      if (values.size() > 2)
        action->add ("move", { id, values[0].string(),
                               values[1].string(), values[2].string(),
                               iter->second });
      else
        action->add ("move", { id, values[0].string(),
                               values[1].string(), iter->second });
    }
    auto anim = char_anims.find (id);
    if (anim != char_anims.end())
      action->add ("play", { id, anim->second, "-1" });
  }

  for (std::size_t i = 0; i < input["integers"].size(); ++ i)
  {
    const Core::File_IO::Node& iint = input["integers"][i];
    auto integer = set<C::Int>(iint["id"].string() , "value", iint["value"].integer());
    integer->mark_as_altered();
  }

  for (std::size_t i = 0; i < input["hidden"].size(); ++ i)
    action->add ("hide", { input["hidden"][i].string() });

  for (std::size_t i = 0; i < input["active_animations"].size(); ++ i)
  {
    const Core::File_IO::Node& ianimation = input["active_animations"][i];
    action->add ("play", { ianimation.string() });
  }

  if (input.has("dialog"))
  {
    action->add ("trigger", { input["dialog"].string() });
    set<C::Int>("Saved_game", "dialog_position", input["dialog_position"].integer());
  }

  return true;
}

void File_IO::write_savefile()
{
  auto save_id = get<C::String>("Savegame", "id");
  Core::File_IO output ("save" + value<C::String>("Save", "suffix", "") + "_" + save_id->value() + ".yaml", true, true);
  remove (save_id);

  int date = std::time(nullptr);
  output.write("date", date);
  double time = value<C::Double>(CLOCK__SAVED_TIME) + value<C::Double>(CLOCK__TIME) - value<C::Double>(CLOCK__DISCOUNTED_TIME);
  output.write("time", time);
  output.write("room", value<C::String>("Game", "current_room"));
  output.write("player", value<C::String>("Player", "name"));
  if (auto follower = request<C::String>("Follower", "name"))
    output.write("follower", value<C::String>("Follower", "name"));
  output.write("camera", value<C::Absolute_position>(CAMERA__POSITION).x());
  output.write("inventory", get<C::Inventory>("Game", "inventory")->data());
  if (auto numbers = request<C::Vector<std::string>>("phone_numbers", "list"))
    output.write("phone_numbers", numbers->value());

  if (auto music = request<C::Music>("Game", "music"))
  {
    std::string music_id = music->entity();
    if (auto music = request<C::Music>(music_id, "music"))
    {
      output.write("music", music_id);
      output.start_section("music_disabled_sources");
      {
        for (const auto& s : music->sources())
          if (s.second.status == C::Music::OFF)
            output.write_list_item(s.first);
      }
      output.end_section();
    }
  }

  output.start_section("music_positions");
  for (C::Handle c : components("resume_at"))
    if (auto r= C::cast<C::Double>(c))
      output.write_list_item ("id", c->entity(), "value", r->value());
  output.end_section();

  if (auto dialog = request<C::String>("Game", "current_dialog"))
  {
    output.write("dialog", dialog->value());
    output.write("dialog_position", value<C::Int>("Game", "dialog_position"));
  }

  output.start_section("characters");
  for (C::Handle c : components("group"))
    if (!c->is_system())
      if (auto lr = request<C::Animation>(c->entity() + "_head", "image"))
      {
        auto anim = request<C::String>(c->entity(), "animation");
        if (anim && anim->value() != "action")
          output.write_list_item ("id", c->entity(), "value", is_looking_right(c->entity()), "animation", anim->value());
        else
          output.write_list_item ("id", c->entity(), "value", is_looking_right(c->entity()));
      }
  output.end_section();

  output.start_section("states");
  for (C::Handle c : components("state"))
  {
    C::String_handle set_state = request<C::String>(c->entity(), "set_state");
    if (!c->is_system() &&
        (c->was_altered() || set_state))
      if (auto s = C::cast<C::String>(c))
        output.write_list_item ("id", c->entity(), "value",
                                (set_state ? set_state->value() : s->value()));
  }
  output.end_section();

  output.start_section("signals");
  for (C::Handle c : components("signal"))
    if (!c->is_system())
      if (auto s = C::cast<C::Signal>(c))
        output.write_list_item (c->entity());
  output.end_section();

  if (auto achievements = request<C::Vector<std::string>>("Achievements", "list"))
  {
    output.start_section("achievements");
    for (const std::string& ach : achievements->value())
      if (signal (ach, "done"))
        output.write_list_item ("id", ach, "stored", signal (ach, "stored"));
    output.end_section();
  }

  output.start_section("positions");
  for (C::Handle c : components("position"))
    if (!c->is_system() && c->was_altered())
      if (auto pos = C::cast<C::Position>(c))
      {
        if (auto z = request<C::Int>(pos->entity(), "z"))
        {
          std::string zstr = std::to_string(z->value());
          if (request<C::Base>(pos->entity(), "z_rescaled"))
            zstr = "+" + zstr;
          output.write_list_item ("id", c->entity(), "value",
                                  { std::to_string(pos->value().X()),
                                    std::to_string(pos->value().Y()),
                                    zstr });
        }
        else
          output.write_list_item ("id", c->entity(), "value",
                                  { pos->value().X(), pos->value().Y() });
      }

  output.end_section();

  output.start_section("integers");
  for (C::Handle c : components("value"))
    if (!c->is_system() && c->was_altered())
      if (auto i = C::cast<C::Int>(c))
        output.write_list_item ("id", c->entity(), "value", i->value());
  output.end_section();

  std::unordered_set<std::string> hidden;
  for (C::Handle c : components("group"))
    if (!c->is_system())
      if (auto lr = request<C::Animation>(c->entity() + "_body", "image"))
        if (!lr->on())
          hidden.insert (c->entity());

  // Some elements might have not been process by System::Animation yet
  for (auto c : components("set_hidden"))
    hidden.insert (c->entity());

  output.start_section("hidden");
  for (const std::string& id : hidden)
    if (!signal(id, "set_visible"))
      output.write_list_item (id);
  output.end_section();

  output.start_section("active_animations");
  for (C::Handle c : components("image"))
    if (!c->is_system())
      if (auto a = C::cast<C::Animation>(c))
        if (a->loop() && (a->on() || signal(a->entity(), "start_animation"))
            && !signal(a->entity(), "stop_animation"))
          if (auto s = request<C::String>(a->entity() , "state"))
            if (s->value() == "Dummy")
              output.write_list_item (a->entity());
  output.end_section();

  set<C::Tuple<std::string, double, int>>
      ("Save_" + save_id->value(),
       "info", value<C::String>("Game", "current_room_name"),
       time, date);
  emit("Saves", "have_changed");
}


void File_IO::read_init ()
{
  Core::File_IO input ("data/init.yaml");
  input.parse();

  read_init_general (input);
  read_init_achievement (input);
  read_init_cursor (input);
  read_init_inventory (input);
  read_init_interface (input);
  read_init_functions (input);
  read_init_global_items (input);
  read_init_text_defaults (input);

  set<C::String>("Game", "init_new_room", input["load"][0].string());
  set<C::String>("Game", "init_new_room_origin", input["load"][1].string());

  read_savefiles (input);

  read_locale();
}

void File_IO::read_init_general (const Core::File_IO& input)
{
  std::string data_version = input["data_version"].string();
  std::string data_variant = input["data_variant"].string();
  std::string min_sosage_version = input["min_sosage_version"].string();
  check (Version::parse(min_sosage_version) <= Version::get(),
         "Error: min version " + min_sosage_version + " incompatible with Sosage " + Version::str());

  set<C::String>("Version", "string", "v" + Version::str(false) + "-d" + data_version + '-' + data_variant);

  std::string game_name = input["name"].string();
  set<C::String>("Game", "name", game_name);
  emit("Game", "name_changed");

  std::string icon = input["icon"].string("images", "interface", "png");
  set<C::String>("Icon", "filename", icon);

  std::string loading_spin = input["loading_spin"][0].string("images", "interface", "png");
  int nb_img = input["loading_spin"][1].integer();
  auto loading_spin_img = set_fac<C::Animation> (LOADING_SPIN__IMAGE, "Loading_spin", "image", loading_spin,
                                                                   Config::loading_depth, nb_img, 1, true);
  loading_spin_img->on() = false;
  loading_spin_img->set_relative_origin(0.5, 0.5);
  set_fac<C::Absolute_position> (LOADING_SPIN__POSITION, "Loading_spin", "position",
                                 Point(Config::world_width - loading_spin_img->width() * 1.5,
                                       Config::world_height - loading_spin_img->height() * 1.5));

  std::string code_success = input["code_results"][0].string
                             ("images", "interface", "png");
  auto code_success_img = set<C::Image>("Code_success", "image", code_success,
                                        Config::label_depth, UNCLICKABLE);
  code_success_img->set_relative_origin(0.5, 0.5);
  code_success_img->on() = false;

  std::string code_failure = input["code_results"][1].string
                             ("images", "interface", "png");
  auto code_failure_img = set<C::Image>("Code_failure", "image", code_failure,
                                        Config::label_depth, UNCLICKABLE);
  code_failure_img->set_relative_origin(0.5, 0.5);
  code_failure_img->on() = false;

  set<C::Absolute_position>("Code_result", "position", Point(Config::world_width / 2,
                                                              Config::world_height / 2));

  std::string debug_font = input["debug_font"].string("fonts", "ttf");
  set<C::Font> ("Debug", "font", debug_font, 40);

  std::string interface_font = input["interface_font"].string("fonts", "ttf");
  set<C::Font> ("Interface", "font", interface_font, 80);

  std::string interface_light_font = input["interface_light_font"].string("fonts", "ttf");
  set<C::Font> ("Interface", "light_font", interface_light_font, 80);

  std::string dialog_font = input["dialog_font"].string("fonts", "ttf");
  set<C::Font> ("Dialog", "font", dialog_font, 80);
}

void File_IO::read_init_achievement (const Core::File_IO& input)
{
  if (input.has("achievements"))
  {
    auto achievements = set<C::Vector<std::string>>("Achievements", "list");

    for (std::size_t i = 0; i < input["achievements"].size(); ++ i)
    {
      const Core::File_IO::Node& iach = input["achievements"][i];

      achievements->push_back (iach[0].string());
      set<C::String>(iach[0].string(), "text", iach[1].string());

      if (iach.size() == 3)
        set<C::Int>(iach[0].string(), "value", iach[2].integer());
    }

    std::string left_circle = input["achievement_notif"][0].string
                              ("images", "interface", "png");
    auto left_circle_img = set<C::Image>("Yellow_left_circle", "image",
                                         left_circle, 1, BOX, true);
    left_circle_img->on() = false;

    std::string right_circle = input["achievement_notif"][1].string
                               ("images", "interface", "png");
    auto right_circle_img = set<C::Image>("Yellow_right_circle", "image",
                                          right_circle, 1, BOX, true);
    right_circle_img->on() = false;

    std::string star = input["achievement_notif"][2].string
                       ("images", "interface", "png");
    auto star_img = set<C::Image>("Star", "image",
                                  star, 1, BOX, true);
    star_img->on() = false;

  }
}

void File_IO::read_init_cursor (const Core::File_IO& input)
{
  std::string cursor = input["cursor"][0].string("images", "interface", "png");
  auto cursor_default = C::make_handle<C::Image> ("Cursor", "image", cursor, Config::cursor_depth);
  cursor_default->set_relative_origin(0.1, 0.1);

  std::string cursor_o = input["cursor"][1].string("images", "interface", "png");
  auto cursor_object = C::make_handle<C::Image> ("Cursor", "image", cursor_o,
                                                              Config::cursor_depth);
  cursor_object->set_relative_origin(0.5, 0.5);

  std::string goto_left = input["cursor"][2].string("images", "interface", "png");
  auto goto_left_img = C::make_handle<C::Image>("Cursor", "image", goto_left, Config::cursor_depth);
  goto_left_img->set_relative_origin(1., 0.5);
  auto goto_left_copy = set<C::Image>("Goto_left", "image", goto_left_img);
  goto_left_copy->on() = false;

  std::string goto_right = input["cursor"][3].string("images", "interface", "png");
  auto goto_right_img = C::make_handle<C::Image>("Cursor", "image", goto_right, Config::cursor_depth);
  goto_right_img->set_relative_origin(0., 0.5);
  auto goto_right_copy = set<C::Image>("Goto_right", "image", goto_right_img);
  goto_right_copy->on() = false;

  // For fake touchscreen
  std::string hand = input["hand"].string("images", "interface", "png");
  auto hand_img = C::make_handle<C::Image>("Cursor", "image", hand, Config::cursor_depth);
  hand_img->set_relative_origin(0.28, 0.12);

  auto cursor_state = set<C::String>("Cursor", "state", "default");
  auto cursor_img = set<C::String_conditional>("Cursor", "image", cursor_state);
  cursor_img->add("default", cursor_default);
  cursor_img->add("object", cursor_object);
  cursor_img->add("goto_left", goto_left_img);
  cursor_img->add("goto_right", goto_right_img);
  cursor_img->add("fake_touchscreen", hand_img);

  set_fac<C::Absolute_position> (CURSOR__POSITION, "Cursor", "position", Point(0,0));

  std::string big_circle = input["circle"][2].string("images", "interface", "png");
  auto big_circle_img = C::make_handle<C::Image>("Cursor", "image", big_circle,
                                                 Config::cursor_depth);
  big_circle_img->set_relative_origin(0.5, 0.5);

  cursor_img->add("selected", big_circle_img);
}

void File_IO::read_init_inventory (const Core::File_IO& input)
{
  std::string left_arrow = input["inventory_arrows"][0].string("images", "interface", "png");
  auto left_arrow_img = set<C::Image>("Left_arrow", "image", left_arrow, Config::inventory_depth, PIXEL_PERFECT);
  left_arrow_img->set_relative_origin (0, 0.5);
  std::string right_arrow = input["inventory_arrows"][1].string("images", "interface", "png");
  auto right_arrow_img = set<C::Image>("Right_arrow", "image", right_arrow, Config::inventory_depth, PIXEL_PERFECT);
  right_arrow_img->set_relative_origin (1, 0.5);

  std::string chamfer = input["inventory_chamfer"].string("images", "interface", "png");
  auto chamfer_img = set<C::Image>("Chamfer", "image", chamfer);
  chamfer_img->z() = Config::interface_depth;
}

void File_IO::read_init_interface (const Core::File_IO& input)
{
  std::string click_sound = input["click_sound"].string("sounds", "effects", "ogg");
  set<C::Sound>("Click", "sound", click_sound);
  std::string step_sound = input["step_sound"].string("sounds", "effects", "ogg");
  set<C::Sound>("Step", "sound", step_sound);

  std::string left_circle = input["circle"][0].string("images", "interface", "png");
  auto left_circle_img = set<C::Image>("Left_circle", "image", left_circle, 1, BOX, true);
  left_circle_img->on() = false;
  std::string right_circle = input["circle"][1].string("images", "interface", "png");
  auto right_circle_img = set<C::Image>("Right_circle", "image", right_circle, 1, BOX, true);
  right_circle_img->on() = false;

  std::string white_left_circle = input["circle"][3].string("images", "interface", "png");
  auto white_left_circle_img = set<C::Image>("White_left_circle", "image", white_left_circle, 1, BOX, true);
  white_left_circle_img->on() = false;
  std::string white_right_circle = input["circle"][4].string("images", "interface", "png");
  auto white_right_circle_img = set<C::Image>("White_right_circle", "image", white_right_circle, 1, BOX, true);
  white_right_circle_img->on() = false;

  for (const auto button : { std::make_pair("triangle", 0), std::make_pair("square", 1),
                             std::make_pair("cross", 2), std::make_pair("circle", 3) })
  {
    std::string ps_button = input["playstation"][button.second].string("images", "interface", "png");
    auto ps_img = set<C::Image>("Playstation_" + std::string(button.first),
                                "image", ps_button, 1, UNCLICKABLE, true);
    ps_img->on() = false;
  }

  std::string fast_forward = input["fast_forward"][0].string("images", "interface", "png");
  auto fast_forward_img = C::make_handle<C::Image>("Fast_forward", "image", fast_forward,
                                                   Config::notification_depth, BOX);
  fast_forward_img->set_relative_origin(1.0, 0.0);
  fast_forward_img->set_alpha(64);

  std::string forbidden = input["fast_forward"][1].string("images", "interface", "png");
  auto forbidden_img = C::make_handle<C::Image>("Fast_forward", "image", forbidden,
                                                Config::notification_depth, BOX);
  forbidden_img->set_relative_origin(1.0, 0.0);
  forbidden_img->set_alpha(64);

  set<C::Absolute_position>("Fast_forward", "position", Point(Config::world_width - 20, 20));

  auto func = set<C::Functional_conditional>("Fast_forward", "image",
                                             [&](const std::string&) -> std::size_t
  {
    if (status()->is(CUTSCENE))
    {
      // If CUTSCENE and speedup is wanted, display forbidden
      if (signal("Time", "speedup_wanted"))
        return 1;
      // else display nothing
      return 2;
    }
    // else, if TOUCHSCREEN, always display fast forward
    if (value<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE) == TOUCHSCREEN)
      return 0;

    // else, display if wanted
    if (signal("Time", "speedup_wanted"))
      return 0;

    // else, display nothing
    return 2;
  }, "");
  func->add(fast_forward_img);
  func->add(forbidden_img);
  func->add(C::Handle());

  std::string menu_background = input["menu_background"].string("images", "interface", "png");
  auto menu_background_img = set<C::Image> ("Menu_background", "image", menu_background);
  menu_background_img->z() = Config::menu_front_depth;
  menu_background_img->set_relative_origin(0.5, 0.5);
  menu_background_img->on() = false;
  set<C::Absolute_position>("Menu_background", "position", Point (Config::world_width / 2, Config::world_height / 2));

  std::string logo_id = input["menu_logo"].string("images", "interface", "png");
  auto logo = set<C::Image> ("Menu_logo", "image", logo_id);
  logo->on() = false;

  std::string left_arrow_id = input["menu_arrows"][0].string("images", "interface", "png");
  auto arrow_left = set<C::Image>("Menu_left_arrow", "image", left_arrow_id);
  arrow_left->set_relative_origin(0, 0.5);
  arrow_left->on() = false;

  std::string right_arrow_id = input["menu_arrows"][1].string("images", "interface", "png");
  auto arrow_right = set<C::Image>("Menu_right_arrow", "image", right_arrow_id);
  arrow_right->set_relative_origin(1, 0.5);
  arrow_right->on() = false;

  std::string menu_ok_id = input["menu_oknotok"][0].string("images", "interface", "png");
  auto menu_ok = set<C::Image>("Menu_ok", "image", menu_ok_id);
  menu_ok->set_relative_origin(0.5, 0.5);
  menu_ok->on() = false;

  std::string menu_cancel_id = input["menu_oknotok"][1].string("images", "interface", "png");
  auto menu_cancel = set<C::Image>("Menu_canel", "image", menu_cancel_id);
  menu_cancel->set_relative_origin(0.5, 0.5);
  menu_cancel->on() = false;

  std::string menu_button_id = input["menu_buttons"][0].string("images", "interface", "png");
  auto menu_button = set<C::Image>("Menu_main_button", "image", menu_button_id, 1, BOX);
  menu_button->set_relative_origin(0.5, 0.5);
  menu_button->on() = false;

  std::string menu_settings_button_id = input["menu_buttons"][1].string("images", "interface", "png");
  auto menu_settings_button = set<C::Image>("Menu_settings_button", "image", menu_settings_button_id, 1, BOX);
  menu_settings_button->set_relative_origin(0.5, 0.5);
  menu_settings_button->on() = false;
}

void File_IO::read_init_functions (const Core::File_IO& input)
{
  for (std::size_t i = 0; i < input["functions"].size(); ++ i)
  {
    const Core::File_IO::Node& imeta = input["functions"][i];
    std::string id = imeta["id"].string();
    capitalize(id);
    auto action = set<C::Action>(id, "function");
    for (std::size_t k = 0; k < imeta["effect"].size(); ++ k)
    {
      std::string function = imeta["effect"][k].nstring();
      action->add (function, imeta["effect"][k][function].string_array());
    }
  }
}

void File_IO::read_init_global_items (const Core::File_IO& input)
{
  for (const auto& d : m_dispatcher)
  {
    const std::string& section = d.first;
    const Function& func = d.second;
    if (input.has(section))
      for (std::size_t i = 0; i < input[section].size(); ++ i)
      {
        const Core::File_IO::Node& s = input[section][i];
        Core::File_IO subfile ("data/" + section + "/" + s.string() + ".yaml");
        bool okay = subfile.parse();
        check(okay, "Can't open data/" + section + "/" + s.string() + ".yaml");
        emit (s.string(), "is_global");
        func (s.string(), subfile.root());
      }
  }
}

void File_IO::read_init_text_defaults (const Core::File_IO& input)
{
  for (std::size_t i = 0; i < input["text"].size(); ++ i)
  {
    const Core::File_IO::Node& itext = input["text"][i];
    std::string id = itext["id"].string();
    capitalize(id); // system id start with uppercase
    set<C::String>(id , "text", itext["value"].string());

    if (itext.has("icon"))
    {
      std::string icon_id = itext["icon"].string("images", "interface", "png");
      auto icon = set<C::Image>(id + "_icon", "image", icon_id);
      icon->z() = Config::menu_text_depth;
      icon->set_relative_origin(0.5, 0.5);
      icon->on() = false;
    }
  }

  for (std::string id : Config::possible_actions)
  {
    const Core::File_IO::Node& idefault = input["default"][id];
    std::string label = idefault["label"].string();

    set<C::String>("Default_" + id , "label", label);

    if (idefault.has("effect"))
    {
      auto action = set<C::Random_conditional>("Default_" + id , "action");
      for (std::size_t j = 0; j < idefault["effect"].size(); ++ j)
      {
        const Core::File_IO::Node& iaction = idefault["effect"][j];
        std::string function = iaction.nstring();

        auto rnd_action = C::make_handle<C::Action>
                          ("Default_" + id, std::to_string(j));
        rnd_action->add ("look", {});
        rnd_action->add (function, iaction[function].string_array());
        action->add (rnd_action);
      }
    }
  }
  set<C::String>("Inventory", "label", input["default"]["inventory"]["label"].string());
}

void File_IO::read_savefiles (const Core::File_IO& input)
{
  int most_recent = -1;
  std::string most_recent_save_id = "";
  for (std::string save_id : Config::save_ids)
  {
    std::string save_path = "save" + value<C::String>("Save", "suffix", "") +
                            "_" + save_id + ".yaml";
    Core::File_IO input (save_path, true);
    if (!input.parse())
      continue;

    std::string room_id = input["room"].string();
    // Get room name (if file not found, save is outdated, just ignore all
    Core::File_IO room_content ("data/rooms/" + room_id + ".yaml");
    if (!room_content.parse())
      break;

    int date = 0;
    if (input.has("date"))
      date = input["date"].integer();
    if (date > most_recent)
    {
      most_recent = date;
      most_recent_save_id = save_id;
    }
    double time = input["time"].floating();

    std::string room_name = room_content["name"].string();

    set<C::Tuple<std::string, double, int>>("Save_" + save_id,
                                            "info", room_name, time, date);
  }
  if (auto force = request<C::String>("Force_load", "room"))
  {
    debug << "Force load " << force->value() << std::endl;
    set<C::Variable>("Game", "new_room", force);
    if (auto orig = request<C::String>("Force_load", "origin"))
      set<C::Variable>("Game", "new_room_origin", orig);
    else
      set<C::String>("Game", "new_room_origin", force->value() + "_test");
  }
  else if (most_recent_save_id != "")
    read_savefile (most_recent_save_id);
  else
  {
    set<C::Variable>("Game", "new_room", get<C::String>("Game", "init_new_room"));
    if (auto orig = request<C::String>("Game", "init_new_room_origin"))
      set<C::Variable>("Game", "new_room_origin", orig);
  }
}

void File_IO::read_locale()
{
  Core::File_IO input ("data/locale.yaml");
  input.parse();

  auto available = set<C::Vector<std::string>>("Game", "available_locales");
  std::vector<std::pair<std::string, C::Locale_handle>> map;

  auto locales = set_fac<C::String_conditional>(GAME__LOCALE, "Game", "locale",
                                                get<C::String>(GAME__CURRENT_LOCAL));
  std::string base;
  for (std::size_t i = 0; i < input["locales"].size(); ++ i)
  {
    std::string id = input["locales"][i]["id"].string();
    std::string description = input["locales"][i]["description"].string();
    available->push_back (id);
    if (i == 0)
    {
      base = id;
      locales->add (id, C::Handle());
    }
    else
    {
      auto locale = C::make_handle<C::Locale>("Game", "locale");
      map.emplace_back (id, locale);
      locales->add (id, locale);
    }
    set<C::String>("Locale_" + id , "description", description);
  }

  Core::File_IO::Node lines = input["lines"];
  for (std::size_t i = 0; i < lines.size(); ++ i)
  {
    Core::File_IO::Node l = lines[i];
    std::string line = l[base].string();
    for (const auto& m : map)
    {
      std::string translation = l[m.first].string();
      m.second->add(line, translation);
    }
  }

  if (value<C::String>(GAME__CURRENT_LOCAL) == "")
  {
    std::vector<std::string> user_locales = get_locales();

#ifdef SOSAGE_DEBUG
    debug << "User locales = [";
    for (const std::string& locale : user_locales)
      debug << " " << locale << " ";
    debug << "]" << std::endl;
#endif

    std::string prefered = "";

    for (const std::string& user : user_locales)
    {
      for (const std::string& available : available->value())
      {
        if (user == available)
        {
          debug << "Found locale " << user << ", using it" << std::endl;
          prefered = available;
          break;
        }
      }
      if (prefered != "")
        break;
    }

    if (prefered == "")
    {
      for (const std::string& user : user_locales)
      {
        for (const std::string& available : available->value())
        {
          std::string shortened = available;
          shortened.resize(2);
          if (user == shortened)
          {
            debug << "Found locale " << user << ", using " << available << std::endl;
            prefered = available;
            break;
          }
        }
        if (prefered != "")
          break;
      }
    }

    if (prefered == "")
    {
      for (const std::string& user : user_locales)
      {
        for (const std::string& available : available->value())
        {
          std::string shortened = available;
          shortened.resize(2);
          std::string ushortened = user;
          ushortened.resize(2);
          if (ushortened == shortened)
          {
            debug << "Found locale " << user << ", using " << available << std::endl;
            prefered = available;
            break;
          }
        }
        if (prefered != "")
          break;
      }
    }

    if (prefered == "")
    {
      debug << "No prefered locale available, fallback to en_US" << std::endl;
      prefered = "en_US";
    }

    if (auto cmd = request<C::String>("Cmdline", "locale"))
    {
      debug << "User forces " << cmd->value() << " locale" << std::endl;
      prefered = cmd->value();
    }

    get<C::String>(GAME__CURRENT_LOCAL)->set(prefered);
  }
}

void File_IO::parse_function (const std::vector<std::string>& args,
                              Component::Action_handle action)
{
  std::string id = args[0];
  capitalize(id);
  auto meta = get<C::Action>(id, "function");
  for (const auto& step : *meta)
  {
    std::vector<std::string> fargs = step.args();
    for (std::string& a : fargs)
    {
      if (startswith(a, "ARG"))
      {
        std::size_t idx = to_int(std::string(a.begin() + 3, a.end()));
        a = args[idx + 1];
      }
    }
    action->add (step.function(), fargs);
  }
}

void File_IO::create_locale_dependent_text (const std::string& id, Component::Font_handle font,
                                            const std::string& color, const std::string& text, int z)
{
  auto available = value<C::Vector<std::string>>("Game", "available_locales");
  if (available.size() == 1)
  {
    auto img = set<C::Image>(id , "image", font, color, text);
    img->z() = z;
    img->set_scale(0.75);
    img->set_collision(UNCLICKABLE);
    img->on() = false;
    return;
  }

  auto cond_img = set<C::String_conditional>(id , "image", get<C::String>(GAME__CURRENT_LOCAL));

  // Save current locale to put it back after
  std::string current = value<C::String>(GAME__CURRENT_LOCAL);

  for (const std::string& l : available)
  {
    get<C::String>(GAME__CURRENT_LOCAL)->set(l);
    auto img = C::make_handle<C::Image>(id , "image", font, color, locale(text));
    img->z() = z;
    img->set_scale(0.75);
    img->set_collision(UNCLICKABLE);
    img->on() = false;
    cond_img->add(l, img);
  }
  get<C::String>(GAME__CURRENT_LOCAL)->set(current);
}

C::Handle File_IO::load_locale_dependent_image (const std::string& entity,
                                                const std::string& component,
                                                const std::string& filename,
                                                const std::function<C::Image_handle(std::string)>& func,
                                                bool insert_to_data)
{
  auto img = func (filename);

  auto available = value<C::Vector<std::string>>("Game", "available_locales");
  if (available.size() == 1)
  {
    auto out = C::make_handle<C::Image>(entity, component, img);
    if (insert_to_data)
      set(out);
    return out;
  }

  auto cond_img = C::make_handle<C::String_conditional>(entity, component, get<C::String>(GAME__CURRENT_LOCAL));
  cond_img->add(available[0], img);

  bool has_locale = true;
  for (std::size_t i = 1; i < available.size(); ++ i)
  {
    std::string locale_filename
        = std::string (filename.begin(), filename.begin() + filename.size() - 3)
          + available[i] + ".png";
    if (!Asset_manager::exists (locale_filename))
    {
      has_locale = false;
      break;
    }

    auto img = func (locale_filename);
    cond_img->add(available[i], img);
  }

  if (!has_locale)
  {
    auto out = C::make_handle<C::Image> (entity, component, img);
    if (insert_to_data)
      set(out);
    return out;
  }

  if (insert_to_data)
    set(cond_img);

  return cond_img;
}



} // namespace Sosage::System
