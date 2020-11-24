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
  if (auto new_room = request<C::String>("game:new_room"))
  {
    if (request<C::String>("game:new_room_origin"))
      read_room (new_room->value());
    else
    {
      read_cutscene (new_room->value());
      get<C::Status>(GAME__STATUS)->push (CUTSCENE);
    }
    remove ("game:new_room");
  }
}

void File_IO::clean_content()
{
  std::unordered_set<std::string> force_keep;
  auto inventory = get<C::Inventory>("game:inventory");
  for (const std::string& entity : *inventory)
    force_keep.insert (entity);

  const std::string& player = get<C::String>("player:name")->value();
  force_keep.insert (player);
  force_keep.insert (player + "_body");
  force_keep.insert (player + "_head");
  force_keep.insert (player + "_mouth");
  force_keep.insert (player + "_walking");
  force_keep.insert (player + "_idle");

  m_content.clear
    ([&](C::Handle c) -> bool
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

       // else, remove component if belonged to the latest room
       return (m_latest_room_entities.find(c->entity()) != m_latest_room_entities.end());
     });
}

void File_IO::read_config()
{
  std::string file_name = Sosage::pref_path() + "config.yaml";

  // Default config values
  bool fullscreen = Config::android;
  int layout = int(Config::AUTO);
  bool virtual_cursor = Config::android;

  double dialog_speed = 1.;
  double dialog_size = 0.75;

  int music_volume = 64;
  int sounds_volume = 128;

  bool autosave = true;
  bool hints = true;

  int interface_width = 0;
  int interface_height = 200;

#ifdef SOSAGE_EMSCRIPTEN
  int window_width = 960;
  int window_height = 600;
#else
  int window_width = -1;
  int window_height = -1;
#endif

  try
  {
    Core::File_IO input (file_name);
    input.parse();
    fullscreen = input["fullscreen"].boolean();
    layout = input["layout"].integer();
    virtual_cursor = input["virtual_cursor"].boolean();
    dialog_speed = input["dialog_speed"].floating();
    dialog_size = input["dialog_size"].floating();
    music_volume = input["music_volume"].integer();
    sounds_volume = input["sounds_volume"].integer();
    autosave = input["autosave"].boolean();
    hints = input["hints"].boolean();
    window_width = input["window"][0].integer();
    window_height = input["window"][1].integer();
    interface_width = input["interface"][0].integer();
    interface_height = input["interface"][1].integer();
  }
  catch (Sosage::No_such_file&)
  {

  }

  set<C::Boolean>("window:fullscreen", fullscreen);
  set<C::Int>("interface:layout", layout);
  set<C::Boolean>("interface:virtual_cursor", virtual_cursor);

  set<C::Double>("text:dialog_speed", dialog_speed);
  set<C::Double>("text:dialog_size", dialog_size);

  set<C::Int>("music:volume", music_volume);
  set<C::Int>("sounds:volume", sounds_volume);

  set<C::Boolean>("game:autosave", autosave);
  set<C::Boolean>("game:hints_on", hints);

  set<C::Int>("interface:width", interface_width);
  set<C::Int>("interface:height", interface_height);
  set<C::Int>("window:width", window_width);
  set<C::Int>("window:height", window_height);
}

void File_IO::write_config()
{
  Core::File_IO output (Sosage::pref_path() + "config.yaml", true);

  output.write ("fullscreen", get<C::Boolean>("window:fullscreen")->value());
  output.write ("layout", get<C::Int>("interface:layout")->value());
  output.write ("virtual_cursor", get<C::Boolean>("interface:virtual_cursor")->value());

  output.write ("dialog_speed", get<C::Double>("text:dialog_speed")->value());
  output.write ("dialog_size", get<C::Double>("text:dialog_size")->value());

  output.write ("music_volume", get<C::Int>("music:volume")->value());
  output.write ("sounds_volume", get<C::Int>("sounds:volume")->value());

  output.write ("autosave", get<C::Boolean>("game:autosave")->value());
  output.write ("hints", get<C::Boolean>("game:hints_on")->value());

  output.write ("window",
                get<C::Int>("window:width")->value(),
                get<C::Int>("window:height")->value());
  output.write ("interface",
                get<C::Int>("interface:width")->value(),
                get<C::Int>("interface:height")->value());
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
  set<C::String>("game:name", game_name);

  std::string icon = input["icon"].string("images", "interface", "png");
  auto icon_img
    = set<C::Image>("icon:image", local_file_name(icon), 0);
  icon_img->on() = false;

  std::string cursor = input["cursor"].string("images", "interface", "png");
  auto cursor_img = C::make_handle<C::Image> ("cursor:image", local_file_name(cursor),
                                                              Config::cursor_depth);
  cursor_img->set_relative_origin(0.5, 0.5);

  auto status = get<C::Status>(GAME__STATUS);

  if constexpr (Config::android)
  {
    // Cursor displayed = NOT paused AND virtual
    set<C::Conditional>
        ("cursor:conditional",
         C::make_and
         (C::make_not
          (C::make_value_condition<Sosage::Status> (status, PAUSED)),
           get<C::Boolean>("interface:virtual_cursor")),
         cursor_img);
  }
  else
  {
    // Cursor displayed = NOT paused
    set<C::Conditional>
        ("cursor:conditional",
         C::make_not
         (C::make_value_condition<Sosage::Status> (status, PAUSED)),
         cursor_img);
  }

  set_fac<C::Position> (CURSOR__POSITION, "cursor:position",
                        (get<C::Boolean>("interface:virtual_cursor")->value()
                        ? Point(Config::world_width / 2, Config::world_height / 2) : Point(0,0)));

  std::string turnicon = input["turnicon"].string("images", "interface", "png");
  auto turnicon_img
    = set<C::Image>("turnicon:image", local_file_name(turnicon), 0);
  turnicon_img->on() = false;

  std::string loading_spin = input["loading_spin"][0].string("images", "interface", "png");
  int nb_img = input["loading_spin"][1].integer();
  auto loading_spin_img = set_fac<C::Animation> (LOADING_SPIN__IMAGE, "loading_spin:image", local_file_name(loading_spin),
                                                                   Config::loading_depth, nb_img, 1, true);
  loading_spin_img->on() = false;
  loading_spin_img->set_relative_origin(0.5, 0.5);
  set_fac<C::Position> (LOADING_SPIN__POSITION, "loading_spin:position", Point(Config::world_width / 2,
                                                                               Config::world_height / 2));

  std::string click_sound = input["click_sound"].string("sounds", "effects", "wav");
  set<C::Sound>("click:sound", local_file_name(click_sound));

  std::string debug_font = input["debug_font"].string("fonts", "ttf");
  set<C::Font> ("debug:font", local_file_name(debug_font), 40);

  std::string interface_font = input["interface_font"].string("fonts", "ttf");
  set<C::Font> ("interface:font", local_file_name(interface_font), 80);

  std::string interface_color = input["interface_color"].string();
  set<C::String> ("interface:color", interface_color);
  std::array<unsigned char, 3> color = color_from_string (interface_color);

  for (std::size_t i = 0; i < input["inventory_arrows"].size(); ++ i)
  {
    std::string id = input["inventory_arrows" ][i].string("images", "interface", "png");
    auto arrow
      = set<C::Image> ("inventory_arrow_" + std::to_string(i) + ":image",
                                         local_file_name(id),
                                         Config::inventory_front_depth);
    arrow->set_relative_origin(0.5, 0.5);
    auto arrow_background
      = set<C::Image> ("inventory_arrow_background_" + std::to_string(i)
                                         + ":image",
                                         arrow->width(), arrow->height(),
                                         color[0], color[1], color[2]);
    arrow_background->set_relative_origin(0.5, 0.5);
    arrow_background->z() = Config::inventory_back_depth;
  }

  for (std::size_t i = 0; i < input["actions"].size(); ++ i)
  {
    const Core::File_IO::Node& idefault = input["actions"][i];
    std::string id = idefault["id"].string();

    auto action = set<C::Random_conditional>("default:" + id);

    for (std::size_t j = 0; j < idefault["effect"].size(); ++ j)
    {
      const Core::File_IO::Node& iaction = idefault["effect"][j];
      std::string function = iaction.nstring();

      auto rnd_action = C::make_handle<C::Action>
        ("default:" + id + "_" + std::to_string(j));
      rnd_action->add ("look", {});
      rnd_action->add (function, iaction[function].string_array());
      action->add (1.0, rnd_action);
    }

  }

  std::string player = input["player"].string();
  set<C::String>("player:name", player);

  if (input.has("load_room"))
  {
    set<C::String>("game:new_room", input["load_room"][0].string());
    set<C::String>("game:new_room_origin", input["load_room"][1].string());
  }
  else
  {
    check (input.has("load_cutscene"), "Init should either load a room or a cutscene");
    set<C::String>("game:new_room", input["load_cutscene"].string());
  }
}

void File_IO::read_cutscene (const std::string& file_name)
{
  auto callback = get<C::Simple<std::function<void()> > >("game:loading_callback");
  callback->value()();

  clean_content();

  SOSAGE_TIMER_START(File_IO__read_cutscene);

  Core::File_IO input (local_file_name("data", "cutscenes", file_name, "yaml"));
  input.parse();

  callback->value()();

  std::string name = input["name"].string();

  auto interface_font = get<C::Font> ("interface:font");
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
    if (node.has("skin")) // Image
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
    img->on() = false;
    callback->value()();
  }

  std::unordered_map<std::string, double> map_id2begin;

  auto cutscene = set<C::Cutscene>("game:cutscene");
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
        map_id2begin.insert (std::make_pair (el, time));
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

  emit ("window:rescaled");
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
