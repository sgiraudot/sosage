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

  if (auto new_room = request<C::String>("Game", "new_room"))
  {
    read_room (new_room->value());
    remove ("Game", "new_room");
  }
  SOSAGE_TIMER_STOP(System_File_IO__run);
}

void File_IO::clean_content()
{
  bool full_reset = receive ("Game", "reset");

  std::unordered_set<std::string> force_keep;
  auto inventory = get<C::Inventory>("Game", "inventory");

  if (!full_reset)
  {
    if (auto phone_numbers = request<C::Vector<std::string>>("phone_numbers", "list"))
    {
      force_keep.insert("phone_numbers");
      for (const std::string& entity : phone_numbers->value())
        force_keep.insert (entity);
    }
  }
  else
  {
    for (const std::string& entity : *inventory)
      get<C::Image>(entity , "image")->on() = false;
    inventory->clear();
  }

  m_content.clear
    ([&](C::Handle c) -> bool
     {
       if (!full_reset)
       {
         // keep inventory + other forced kept
         if (contains(force_keep, c->entity()))
           return false;

         // keep states and positions
         if (c->component() == "state" || c->component() == "position")
           return false;

         // keep integers
         if (C::cast<C::Int>(c))
           return false;
       }

       // Keep system components
       return !c->is_system();
     });

  emit("Game", "clear_managers");
}

void File_IO::read_config()
{
  // Default config values
  std::string locale = "";
  bool fullscreen = !Config::emscripten;
  int input_mode = (Config::android ? TOUCHSCREEN : MOUSE);
  int gamepad_type = NO_LABEL;

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

  Core::File_IO input ("config.yaml", true);
  if (input.parse())
  {
    if (input.has("locale")) locale = input["locale"].string();
    if (input.has("fullscreen")) fullscreen = input["fullscreen"].boolean();
    if (input.has("input_mode")) input_mode = input["input_mode"].integer();
    if (input.has("gamepad_type")) gamepad_type = input["gamepad_type"].integer();
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

  set_fac<C::String>(GAME__CURRENT_LOCAL, "Game", "current_locale", locale);
  set<C::Boolean>("Window", "fullscreen", fullscreen);
  debug << "INPUT MODE = " << input_mode << std::endl;
  set_fac<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE, "Interface", "input_mode", Input_mode(input_mode));
  set_fac<C::Simple<Gamepad_type>>(GAMEPAD__TYPE, "Gamepad", "type", Gamepad_type(gamepad_type));

  set<C::Int>("Dialog", "speed", dialog_speed);
  set<C::Int>("Dialog", "size", dialog_size);

  set<C::Int>("Music", "volume", music_volume);
  set<C::Int>("Sounds", "volume", sounds_volume);

  set<C::Boolean>("Game", "autosave", autosave);
  set<C::Boolean>("Game", "hints_on", hints);

  set<C::Int>("Window", "width", window_width);
  set<C::Int>("Window", "height", window_height);
}

void File_IO::write_config()
{
  Core::File_IO output ("config.yaml", true, true);

  output.write ("locale", value<C::String>(GAME__CURRENT_LOCAL));
  output.write ("fullscreen", value<C::Boolean>("Window", "fullscreen"));
  output.write ("input_mode", value<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE));
  output.write ("gamepad_type", value<C::Simple<Gamepad_type>>(GAMEPAD__TYPE));

  output.write ("dialog_speed", value<C::Int>("Dialog", "speed"));
  output.write ("dialog_size", value<C::Int>("Dialog", "size"));

  output.write ("music_volume", value<C::Int>("Music", "volume"));
  output.write ("sounds_volume", value<C::Int>("Sounds", "volume"));

  output.write ("autosave", value<C::Boolean>("Game", "autosave"));
  output.write ("hints", value<C::Boolean>("Game", "hints_on"));

  output.write ("window", value<C::Int>("Window", "width"), value<C::Int>("Window", "height"));
}

bool File_IO::read_savefile()
{
  Core::File_IO input ("save" + value<C::String>("Save", "suffix", "") +  ".yaml", true);
  if (!input.parse())
  {
    debug << "Can't parse save file" << std::endl;
    return false;
  }

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
  action->add ("play", { input["music"].string() });
  action->add ("fadein", { "0.5" });

  std::unordered_map<std::string, std::string> looking_right;
  for (std::size_t i = 0; i < input["characters"].size(); ++ i)
  {
    const Core::File_IO::Node& ichar = input["characters"][i];
    looking_right.insert (std::make_pair(ichar["id"].string(), ichar["value"].string()));
  }

  for (std::size_t i = 0; i < input["states"].size(); ++ i)
  {
    const Core::File_IO::Node& istate = input["states"][i];
    auto state = set<C::String>(istate["id"].string() , "state", istate["value"].string());
    state->mark_as_altered();
  }

  for (std::size_t i = 0; i < input["positions"].size(); ++ i)
  {
    const Core::File_IO::Node& iposition = input["positions"][i];
    std::string id = iposition["id"].string();
    Point point (iposition["value"][0].floating(), iposition["value"][1].floating());
    auto pos = set<C::Absolute_position>(id, "position", point, false);
    pos->mark_as_altered();

    if (endswith(id, "_body"))
    {
      id.resize (id.size() - 5);
      auto iter = looking_right.find (id);
      if (iter != looking_right.end())
      {
        debug << "FOUND " << id << std::endl;
        action->add ("move", { id, iposition["value"][0].string(),
                               iposition["value"][1].string(), iter->second });
      }
    }
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
  Core::File_IO output ("save" + value<C::String>("Save", "suffix", "") +  ".yaml", true, true);

  output.write("room", value<C::String>("Game", "current_room"));
  output.write("player", value<C::String>("Player", "name"));
  if (auto follower = request<C::String>("Follower", "name"))
    output.write("follower", value<C::String>("Follower", "name"));
  output.write("camera", value<C::Absolute_position>(CAMERA__POSITION).x());
  output.write("inventory", get<C::Inventory>("Game", "inventory")->data());
  if (auto numbers = request<C::Vector<std::string>>("phone_numbers", "list"))
    output.write("phone_numbers", numbers->value());
  output.write("music", get<C::Music>("Game", "music")->entity());

  if (auto dialog = request<C::String>("Game", "current_dialog"))
  {
    // Do not save hint dialog, as it's generated on the fly
    if (dialog->value() != "Hints")
    {
      output.write("dialog", dialog->value());
      output.write("dialog_position", value<C::Int>("Game", "dialog_position"));
    }
  }

  output.start_section("characters");
  for (C::Handle c : components("group"))
    if (!c->is_system())
      if (auto lr = request<C::Animation>(c->entity() + "_head", "image"))
        output.write_list_item ("id", c->entity(), "value", is_looking_right(c->entity()));
  output.end_section();

  output.start_section("states");
  for (C::Handle c : components("state"))
    if (!c->is_system() && c->was_altered())
      if (auto s = C::cast<C::String>(c))
        output.write_list_item ("id", c->entity(), "value", s->value());
  output.end_section();

  output.start_section("positions");
  for (C::Handle c : components("position"))
    if (!c->is_system() && c->was_altered())
      if (auto pos = C::cast<C::Position>(c))
        output.write_list_item ("id", c->entity(), "value",
                                { pos->value().X(), pos->value().Y() });

  output.end_section();

  output.start_section("integers");
  for (C::Handle c : components("value"))
    if (!c->is_system() && c->was_altered())
      if (auto i = C::cast<C::Int>(c))
        output.write_list_item ("id", c->entity(), "value", i->value());
  output.end_section();

  output.start_section("hidden");
  for (C::Handle c : components("group"))
    if (!c->is_system())
      if (auto lr = request<C::Animation>(c->entity() + "_body", "image"))
        if (!lr->on())
          output.write_list_item (c->entity());
  output.end_section();

  output.start_section("active_animations");
  for (C::Handle c : components("image"))
    if (!c->is_system())
      if (auto a = C::cast<C::Animation>(c))
        if (a->on() && a->loop())
          if (auto s = request<C::String>(a->entity() , "state"))
            if (s->value() == "Dummy")
              output.write_list_item (a->entity());
  output.end_section();
}


void File_IO::read_init ()
{
  Core::File_IO input ("data/init.yaml");
  input.parse();

  std::string v = input["version"].string();
  check (Version::parse(v) <= Version::get(),
         "Error: room version " + v + " incompatible with Sosage " + Version::str());

  std::string game_name = input["name"].string();
  set<C::String>("Game", "name", game_name);
  emit("Game", "name_changed");

  std::string icon = input["icon"].string("images", "interface", "png");
  set<C::String>("Icon", "filename", icon);

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

  auto cursor_state = set<C::String>("Cursor", "state", "default");
  auto cursor_img = C::make_handle<C::String_conditional>("Cursor", "image", cursor_state);
  cursor_img->add("default", cursor_default);
  cursor_img->add("object", cursor_object);
  cursor_img->add("goto_left", goto_left_img);
  cursor_img->add("goto_right", goto_right_img);

  // Cursor displayed = mouse mode AND NOT paused
  set<C::Conditional>
      ("Cursor", "image"
       "",
       C::make_and
       (C::make_simple_condition
        (get<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE), MOUSE),
        get<C::Condition>("Unlocked", "condition")),
       cursor_img);

  set_fac<C::Absolute_position> (CURSOR__POSITION, "Cursor", "position", Point(0,0));

  std::string loading_spin = input["loading_spin"][0].string("images", "interface", "png");
  int nb_img = input["loading_spin"][1].integer();
  auto loading_spin_img = set_fac<C::Animation> (LOADING_SPIN__IMAGE, "Loading_spin", "image", loading_spin,
                                                                   Config::loading_depth, nb_img, 1, true);
  loading_spin_img->on() = false;
  loading_spin_img->set_relative_origin(0.5, 0.5);
  set_fac<C::Absolute_position> (LOADING_SPIN__POSITION, "Loading_spin", "position",
                                 Point(Config::world_width - loading_spin_img->width() * 1.5,
                                       Config::world_height - loading_spin_img->height() * 1.5));

  std::string left_arrow = input["inventory_arrows"][0].string("images", "interface", "png");
  auto left_arrow_img = set<C::Image>("Left_arrow", "image", left_arrow);
  left_arrow_img->z() = Config::inventory_depth;
  left_arrow_img->set_relative_origin (0, 0.5);
  std::string right_arrow = input["inventory_arrows"][1].string("images", "interface", "png");
  auto right_arrow_img = set<C::Image>("Right_arrow", "image", right_arrow);
  right_arrow_img->z() = Config::inventory_depth;
  right_arrow_img->set_relative_origin (1, 0.5);

  std::string chamfer = input["inventory_chamfer"].string("images", "interface", "png");
  auto chamfer_img = set<C::Image>("Chamfer", "image", chamfer);
  chamfer_img->z() = Config::interface_depth;

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

  std::string big_circle = input["circle"][2].string("images", "interface", "png");
  auto big_circle_img = C::make_handle<C::Image>("Cursor", "image", big_circle,
                                                 Config::cursor_depth);
  big_circle_img->set_relative_origin(0.5, 0.5);

  cursor_img->add("selected", big_circle_img);


  std::string debug_font = input["debug_font"].string("fonts", "ttf");
  set<C::Font> ("Debug", "font", debug_font, 40);

  std::string interface_font = input["interface_font"].string("fonts", "ttf");
  set<C::Font> ("Interface", "font", interface_font, 80);

  std::string interface_light_font = input["interface_light_font"].string("fonts", "ttf");
  set<C::Font> ("Interface", "light_font", interface_light_font, 80);

  std::string dialog_font = input["dialog_font"].string("fonts", "ttf");
  set<C::Font> ("Dialog", "font", dialog_font, 80);

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

  for (std::size_t i = 0; i < input["text"].size(); ++ i)
  {
    const Core::File_IO::Node& itext = input["text"][i];
    std::string id = itext["id"].string();
    id[0] = toupper(id[0]); // system id start with uppercase
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

  set<C::String>("Inventory", "label", input["default"]["inventory_button"]["label"].string());

  set<C::String>("Game", "init_new_room", input["load"][0].string());
  set<C::String>("Game", "init_new_room_origin", input["load"][1].string());

  if (!read_savefile())
  {
    set<C::Variable>("Game", "new_room", get<C::String>("Game", "init_new_room"));
    if (auto orig = request<C::String>("Game", "init_new_room_origin"))
      set<C::Variable>("Game", "new_room_origin", orig);
  }

  read_locale();
  read_hints();
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
    std::string prefered = "";
    std::string user_locale = get_locale();
    if (user_locale.size() > 5)
      user_locale.resize(5);

    user_locale = value<C::String>("Cmdline", "locale", user_locale);

    for (const std::string& l : available->value())
      if (user_locale == l)
      {
        debug << "Locale exactly detected as " << l << std::endl;
        prefered = l;
        break;
      }
    if (prefered == "")
    {
      std::string reduced_locale = user_locale;
      if (reduced_locale.size() > 2)
        reduced_locale.resize(2);
      for (const std::string& l : available->value())
      {
        std::string reduced = l;
        reduced.resize(2);
        if (reduced_locale == reduced)
        {
          debug << "Locale partly detected as " << l << " (instead of " << user_locale << ")" << std::endl;
          prefered = l;
          break;
        }
      }
    }
    if (prefered == "")
    {
      debug << "Locale " << user_locale << " not available, fallback to en_US" << std::endl;
      prefered = "en_US";
    }
    get<C::String>(GAME__CURRENT_LOCAL)->set(prefered);
  }
}

void File_IO::read_hints()
{
  Core::File_IO input ("data/hints.yaml");
  input.parse();

  for (std::size_t i = 0; i < input["hints"].size(); ++ i)
  {
    const Core::File_IO::Node& hint = input["hints"][i];
    std::string id = hint["id"].string();
    set<C::String>("Hint_" + id , "question", hint["question"].string());
    set<C::String>("Hint_" + id , "answer", hint["answer"].string());
  }

  set<C::Set<std::string>>("Hints", "list");
}

void File_IO::create_locale_dependent_text (const std::string& id, Component::Font_handle font,
                                            const std::string& color, const std::string& text)
{
  auto available = value<C::Vector<std::string>>("Game", "available_locales");
  if (available.size() == 1)
  {
    auto img = set<C::Image>(id , "image", font, color, text);
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
    img->set_scale(0.75);
    img->set_collision(UNCLICKABLE);
    img->on() = false;
    cond_img->add(l, img);
  }
  get<C::String>(GAME__CURRENT_LOCAL)->set(current);
}

void File_IO::load_locale_dependent_image (const std::string& entity, const std::string& component,
                                           const std::string& filename,
                                           const std::function<C::Image_handle(std::string)>& func)
{
  auto img = func (filename);

  auto available = value<C::Vector<std::string>>("Game", "available_locales");
  if (available.size() == 1)
  {
    set<C::Image>(entity, component, img);
    return;
  }

  auto cond_img = set<C::String_conditional>(entity, component, get<C::String>(GAME__CURRENT_LOCAL));
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
    remove (cond_img);
    set<C::Image> (entity, component, img);
  }
}



} // namespace Sosage::System
