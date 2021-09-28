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
#include <Sosage/Utils/locale.h>
#include <Sosage/Utils/profiling.h>

#include <locale>

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
      status()->push (CUTSCENE);
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
         if (contains(force_keep, c->entity()))
           return false;

         // keep states and positions
         if (c->component() == "state" || c->component() == "position")
           return false;

         // keep integers
         if (C::cast<C::Int>(c))
           return false;
       }

       // else, remove component if belonged to the latest room
       return contains (m_latest_room_entities, c->entity());
     });
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

  try
  {
    Core::File_IO input ("config.yaml", true);
    input.parse();
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
  catch (Sosage::No_such_file&)
  {
  }

  set_fac<C::String>(GAME__CURRENT_LOCAL, "Game:current_locale", locale);
  set<C::Boolean>("Window:fullscreen", fullscreen);
  set_fac<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE, "Interface:input_mode", Input_mode(input_mode));
  set_fac<C::Simple<Gamepad_type>>(GAMEPAD__TYPE, "Gamepad:type", Gamepad_type(gamepad_type));

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
  Core::File_IO output ("config.yaml", true, true);

  output.write ("locale", get<C::String>(GAME__CURRENT_LOCAL)->value());
  output.write ("fullscreen", get<C::Boolean>("Window:fullscreen")->value());
  output.write ("input_mode", get<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE)->value());
  output.write ("gamepad_type", get<C::Simple<Gamepad_type>>(GAMEPAD__TYPE)->value());

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
  Core::File_IO input ("save.yaml", true);
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
  get<C::Absolute_position>(CAMERA__POSITION)->set (Point(camera_target, 0));
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
  Core::File_IO output ("save.yaml", true, true);

  output.write("room", get<C::String>("Game:current_room")->value());
  output.write("camera", get<C::Absolute_position>(CAMERA__POSITION)->value().x());
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


void File_IO::read_init ()
{
  Core::File_IO input ("data/init.yaml");
  input.parse();

  std::string v = input["version"].string();
  check (Version::parse(v) <= Version::get(),
         "Error: room version " + v + " incompatible with Sosage " + Version::str());

  std::string game_name = input["name"].string();
  set<C::String>("Game:name", game_name);

  std::string icon = input["icon"].string("images", "interface", "png");
  set<C::String>("Icon:filename", icon);

  std::string cursor = input["cursor"][0].string("images", "interface", "png");
  auto cursor_default = C::make_handle<C::Image> ("Cursor:image", cursor, Config::cursor_depth);
  cursor_default->set_relative_origin(0.1, 0.1);

  std::string cursor_o = input["cursor"][1].string("images", "interface", "png");
  auto cursor_object = C::make_handle<C::Image> ("Cursor:image", cursor_o,
                                                              Config::cursor_depth);
  cursor_object->set_relative_origin(0.5, 0.5);

  std::string goto_left = input["cursor"][2].string("images", "interface", "png");
  auto goto_left_img = C::make_handle<C::Image>("Cursor:image", goto_left, Config::cursor_depth);
  goto_left_img->set_relative_origin(1., 0.5);
  auto goto_left_copy = set<C::Image>("Goto_left:image", goto_left_img);
  goto_left_copy->on() = false;

  std::string goto_right = input["cursor"][3].string("images", "interface", "png");
  auto goto_right_img = C::make_handle<C::Image>("Cursor:image", goto_right, Config::cursor_depth);
  goto_right_img->set_relative_origin(0., 0.5);
  auto goto_right_copy = set<C::Image>("Goto_right:image", goto_right_img);
  goto_right_copy->on() = false;

  auto cursor_state = set<C::String>("Cursor:state", "default");
  auto cursor_img = C::make_handle<C::String_conditional>("Cursor:image", cursor_state);
  cursor_img->add("default", cursor_default);
  cursor_img->add("object", cursor_object);
  cursor_img->add("goto_left", goto_left_img);
  cursor_img->add("goto_right", goto_right_img);

  // Cursor displayed = mouse mode AND NOT paused
  set<C::Conditional>
      ("Cursor:image"
       "",
       C::make_and
       (C::make_simple_condition
        (get<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE), MOUSE),
        get<C::Condition>("Unlocked:condition")),
       cursor_img);

  set_fac<C::Absolute_position> (CURSOR__POSITION, "Cursor:position", Point(0,0));

  std::string loading_spin = input["loading_spin"][0].string("images", "interface", "png");
  int nb_img = input["loading_spin"][1].integer();
  auto loading_spin_img = set_fac<C::Animation> (LOADING_SPIN__IMAGE, "Loading_spin:image", loading_spin,
                                                                   Config::loading_depth, nb_img, 1, true);
  loading_spin_img->on() = false;
  loading_spin_img->set_relative_origin(0.5, 0.5);
  set_fac<C::Absolute_position> (LOADING_SPIN__POSITION, "Loading_spin:position",
                                 Point(Config::world_width - loading_spin_img->width() * 1.5,
                                       Config::world_height - loading_spin_img->height() * 1.5));

  std::string left_arrow = input["inventory_arrows"][0].string("images", "interface", "png");
  auto left_arrow_img = set<C::Image>("Left_arrow:image", left_arrow);
  left_arrow_img->z() = Config::inventory_depth;
  left_arrow_img->set_relative_origin (0, 0.5);
  std::string right_arrow = input["inventory_arrows"][1].string("images", "interface", "png");
  auto right_arrow_img = set<C::Image>("Right_arrow:image", right_arrow);
  right_arrow_img->z() = Config::inventory_depth;
  right_arrow_img->set_relative_origin (1, 0.5);

  std::string chamfer = input["inventory_chamfer"].string("images", "interface", "png");
  auto chamfer_img = set<C::Image>("Chamfer:image", chamfer);
  chamfer_img->z() = Config::interface_depth;

  std::string click_sound = input["click_sound"].string("sounds", "effects", "ogg");
  set<C::Sound>("Click:sound", click_sound);

  std::string left_circle = input["circle"][0].string("images", "interface", "png");
  auto left_circle_img = set<C::Image>("Left_circle:image", left_circle, 1, BOX, true);
  left_circle_img->on() = false;
  std::string right_circle = input["circle"][1].string("images", "interface", "png");
  auto right_circle_img = set<C::Image>("Right_circle:image", right_circle, 1, BOX, true);
  right_circle_img->on() = false;

  std::string big_circle = input["circle"][2].string("images", "interface", "png");
  auto big_circle_img = C::make_handle<C::Image>("Cursor:image", big_circle,
                                                 Config::cursor_depth);
  big_circle_img->set_relative_origin(0.5, 0.5);

  cursor_img->add("selected", big_circle_img);


  std::string debug_font = input["debug_font"].string("fonts", "ttf");
  set<C::Font> ("Debug:font", debug_font, 40);

  std::string interface_font = input["interface_font"].string("fonts", "ttf");
  set<C::Font> ("Interface:font", interface_font, 80);

  std::string dialog_font = input["dialog_font"].string("fonts", "ttf");
  set<C::Font> ("Dialog:font", dialog_font, 80);

  std::string menu_color = input["menu_color"].string();
  set<C::String> ("Menu:color", menu_color);

  std::string logo_id = input["menu_logo"].string("images", "interface", "png");
  auto logo
    = set<C::Image> ("Menu_logo:image", logo_id);
  logo->on() = false;
  auto credits_logo
    = set<C::Image> ("Credits_logo:image", logo_id);
  credits_logo->on() = false;


  std::string credits_id = input["credits_image"].string("images", "interface", "png");
  auto credits
    = set<C::Image> ("Credits_text:image", credits_id);
  credits->on() = false;

  std::string left_arrow_id = input["menu_arrows"][0].string("images", "interface", "png");
  auto arrow_left = set<C::Image>("Menu_left_arrow:image", left_arrow_id);
  arrow_left->set_relative_origin(0, 0.5);
  arrow_left->on() = false;

  std::string right_arrow_id = input["menu_arrows"][1].string("images", "interface", "png");
  auto arrow_right = set<C::Image>("Menu_right_arrow:image", right_arrow_id);
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

  set<C::String>("Inventory:label", input["default"]["inventory_button"]["label"].string());

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

  read_locale();
}

void File_IO::read_locale()
{
  Core::File_IO input ("data/locale.yaml");
  input.parse();

  auto available = set<C::Vector<std::string>>("Game:available_locales");
  std::vector<std::pair<std::string, C::Locale_handle>> map;

  auto locales = set_fac<C::String_conditional>(GAME__LOCALE, "Game:locale",
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
      auto locale = C::make_handle<C::Locale>("Game:locale");
      map.emplace_back (id, locale);
      locales->add (id, locale);
    }
    set<C::String>(id + ":description", description);
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

  if (get<C::String>(GAME__CURRENT_LOCAL)->value() == "")
  {
    std::string prefered = "";
    std::string user_locale = get_locale();
    if (user_locale.size() > 5)
      user_locale.resize(5);

    if (auto l = request<C::String>("Cmdline:locale"))
      user_locale = l->value();

    for (const std::string& l : available->value())
      if (user_locale == l)
      {
        debug("Locale exactly detected as " + l);
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
          debug("Locale partly detected as " + l + " (instead of " + user_locale + ")");
          prefered = l;
          break;
        }
      }
    }
    if (prefered == "")
    {
      debug("Locale " + user_locale + " not available, fallback to en_US");
      prefered = "en_US";
    }
    get<C::String>(GAME__CURRENT_LOCAL)->set(prefered);
  }
}

void File_IO::read_cutscene (const std::string& file_name)
{
  auto callback = get<C::Simple<std::function<void()> > >("Game:loading_callback");
  callback->value()();

  clean_content();

  SOSAGE_TIMER_START(File_IO__read_cutscene);

  Core::File_IO input ("data/cutscenes/" + file_name + ".yaml");
  input.parse();

  callback->value()();

  std::string name = input["name"].string();

  auto dialog_font = get<C::Font> ("Dialog:font");
  std::string default_color = "000000";

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
      set<C::Music>(id + ":music", music);
      callback->value()();
      continue;
    }

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
      auto anim = set<C::Animation>(id + ":image", skin, 1,
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
      anim->set_collision(UNCLICKABLE);
      anim->on() = false;
    }
    else if (node.has("skin")) // Image
    {
      std::string skin = node["skin"].string("images", "cutscenes", "png");
      load_locale_dependent_image
          (id + ":image", skin,
           [&](const std::string& skin) -> C::Image_handle
      {
        auto img = C::make_handle<C::Image>(id + ":image", skin);
        img->set_collision(UNCLICKABLE);
        img->on() = false;
        return img;
      });
    }
    else // Text
    {
      check (node.has("text"), "Node should either have music, image or text");
      std::string text = node["text"].string();
      std::string color = (node.has("color") ? node["color"].string() : default_color);
      create_locale_dependent_text (id, dialog_font, color, text);
    }
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

void File_IO::create_locale_dependent_text (const std::string& id, Component::Font_handle font,
                                            const std::string& color, const std::string& text)
{
  auto available = get<C::Vector<std::string>>("Game:available_locales")->value();
  if (available.size() == 1)
  {
    auto img = set<C::Image>(id + ":image", font, color, text);
    img->set_scale(0.75);
    img->set_collision(UNCLICKABLE);
    img->on() = false;
    return;
  }

  auto cond_img = set<C::String_conditional>(id + ":image", get<C::String>(GAME__CURRENT_LOCAL));

  // Save current locale to put it back after
  std::string current = get<C::String>(GAME__CURRENT_LOCAL)->value();

  for (const std::string& l : available)
  {
    get<C::String>(GAME__CURRENT_LOCAL)->set(l);
    auto img = C::make_handle<C::Image>(id + ":image", font, color, locale(text));
    img->set_scale(0.75);
    img->set_collision(UNCLICKABLE);
    img->on() = false;
    cond_img->add(l, img);
  }
  get<C::String>(GAME__CURRENT_LOCAL)->set(current);
}

void File_IO::load_locale_dependent_image (const std::string& id, const std::string& filename,
                                           const std::function<C::Image_handle(std::string)>& func)
{
  auto img = func (filename);

  auto available = get<C::Vector<std::string>>("Game:available_locales")->value();
  if (available.size() == 1)
  {
    set<C::Image>(id, img);
    return;
  }

  auto cond_img = set<C::String_conditional>(id, get<C::String>(GAME__CURRENT_LOCAL));
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
    remove (id);
    set<C::Image> (id, img);
  }
}



} // namespace Sosage::System
