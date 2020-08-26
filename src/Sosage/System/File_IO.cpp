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

File_IO::File_IO (Content& content)
  : m_content (content)
{
}

void File_IO::run()
{
#ifdef SOSAGE_THREADS_ENABLED
  if (m_thread_state == STARTED)
    return;
  if (m_thread_state == FINISHED)
  {
    m_thread.join();
    m_thread_state = NO_THREAD;
  }
#endif

  if (auto new_room = m_content.request<Component::String>("game:new_room"))
  {
    auto status = m_content.get<Component::Status>(GAME__STATUS);

    // If new room must be loaded, first notify loading and restart
    // loop so that Graphic displays loading screen, then only load
    if (status->value() != LOADING)
    {
      status->push(LOADING);
      return;
    }
    else
    {
#ifdef SOSAGE_THREADS_ENABLED
      m_thread_state = STARTED;
      m_thread = std::thread (std::bind(&File_IO::read_room, this, new_room->value()));
#else
      read_room (new_room->value());
#endif
    }
  }
}

void File_IO::read_config()
{
  std::string file_name = Sosage::pref_path() + "config.yaml";

  // Default config values
  bool fullscreen = Config::android;
  int layout = int(Config::AUTO);
  bool virtual_cursor = Config::android;

  double char_per_second = 12.0;
  double dialog_size = 0.75;

  int music_volume = 64;
  int sounds_volume = 128;

  bool autosave = true;
  bool hints = true;

  int interface_width = 0;
  int interface_height = 200;

#ifdef SOSAGE_EMSCRIPTEN
  int window_width = 768;
  int window_height = 432;
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
    char_per_second = input["char_per_second"].floating();
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

  m_content.set<Component::Boolean>("window:fullscreen", fullscreen);
  m_content.set<Component::Int>("interface:layout", layout);
  m_content.set<Component::Boolean>("interface:virtual_cursor", virtual_cursor);

  m_content.set<Component::Double>("text:char_per_second", char_per_second);
  m_content.set<Component::Double>("text:dialog_size", dialog_size);

  m_content.set<Component::Int>("music:volume", music_volume);
  m_content.set<Component::Int>("sounds:volume", sounds_volume);

  m_content.set<Component::Boolean>("game:autosave", autosave);
  m_content.set<Component::Boolean>("game:hints_on", hints);

  m_content.set<Component::Int>("interface:width", interface_width);
  m_content.set<Component::Int>("interface:height", interface_height);
  m_content.set<Component::Int>("window:width", window_width);
  m_content.set<Component::Int>("window:height", window_height);
}

void File_IO::write_config()
{
  Core::File_IO output (Sosage::pref_path() + "config.yaml", true);

  output.write ("fullscreen", m_content.get<Component::Boolean>("window:fullscreen")->value());
  output.write ("layout", m_content.get<Component::Int>("interface:layout")->value());
  output.write ("virtual_cursor", m_content.get<Component::Boolean>("interface:virtual_cursor")->value());

  output.write ("char_per_second", m_content.get<Component::Double>("text:char_per_second")->value());
  output.write ("dialog_size", m_content.get<Component::Double>("text:dialog_size")->value());

  output.write ("music_volume", m_content.get<Component::Int>("music:volume")->value());
  output.write ("sounds_volume", m_content.get<Component::Int>("sounds:volume")->value());

  output.write ("autosave", m_content.get<Component::Boolean>("game:autosave")->value());
  output.write ("hints", m_content.get<Component::Boolean>("game:hints_on")->value());

  output.write ("window",
                m_content.get<Component::Int>("window:width")->value(),
                m_content.get<Component::Int>("window:height")->value());
  output.write ("interface",
                m_content.get<Component::Int>("interface:width")->value(),
                m_content.get<Component::Int>("interface:height")->value());
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
  m_content.set<Component::String>("game:name", game_name);

  std::string icon = input["icon"].string("images", "interface", "png");
  auto icon_img
    = m_content.set<Component::Image>("icon:image", local_file_name(icon), 0);
  icon_img->on() = false;

  std::string cursor = input["cursor"].string("images", "interface", "png");
  auto cursor_img = Component::make_handle<Component::Image> ("cursor:image", local_file_name(cursor),
                                                              Config::cursor_depth);
  cursor_img->set_relative_origin(0.5, 0.5);

  auto status = m_content.get<Component::Status>(GAME__STATUS);

  if constexpr (Config::android)
  {
    // Cursor displayed = NOT paused AND virtual
    m_content.set<Component::Conditional>
        ("cursor:conditional",
         Component::make_and
         (Component::make_not
          (Component::make_value_condition<Sosage::Status> (status, PAUSED)),
           m_content.get<Component::Boolean>("interface:virtual_cursor")),
         cursor_img);
  }
  else
  {
    // Cursor displayed = NOT paused
    m_content.set<Component::Conditional>
        ("cursor:conditional",
         Component::make_not
         (Component::make_value_condition<Sosage::Status> (status, PAUSED)),
         cursor_img);
  }

  m_content.set_fac<Component::Position> (CURSOR__POSITION, "cursor:position", Point(0,0));

  std::string turnicon = input["turnicon"].string("images", "interface", "png");
  auto turnicon_img
    = m_content.set<Component::Image>("turnicon:image", local_file_name(turnicon), 0);
  turnicon_img->on() = false;

  std::string loading = input["loading"].string("images", "interface", "png");
  auto loading_img = Component::make_handle<Component::Image> ("loading:image", local_file_name(loading),
                                                               Config::cursor_depth);
  loading_img->set_relative_origin(0.5, 0.5);
  m_content.set<Component::Position> ("loading:position", Point(Config::world_width / 2,
                                                                Config::world_height / 2));

#ifdef SOSAGE_THREADS_ENABLED
  std::string loading_spin = input["loading_spin"][0].string("images", "interface", "png");
  int nb_img = input["loading_spin"][1].integer();
  auto loading_spin_img = m_content.set_fac<Component::Animation> (LOADING_SPIN__IMAGE, "loading_spin:image", local_file_name(loading_spin),
                                                                   Config::cursor_depth, nb_img, 1, true);
  loading_spin_img->on() = false;
  loading_spin_img->set_relative_origin(0.5, 0.5);
  m_content.set_fac<Component::Position> (LOADING_SPIN__POSITION, "loading_spin:position", Point(Config::world_width / 2,
                                                                                                 Config::world_height / 2));
#endif

  m_content.set<Component::Conditional>
  ("loading:conditional",
   Component::make_value_condition<Sosage::Status> (status, LOADING),
   loading_img);

  std::string click_sound = input["click_sound"].string("sounds", "effects", "wav");
  m_content.set<Component::Sound>("click:sound", local_file_name(click_sound));

  std::string debug_font = input["debug_font"].string("fonts", "ttf");
  m_content.set<Component::Font> ("debug:font", local_file_name(debug_font), 40);

  std::string interface_font = input["interface_font"].string("fonts", "ttf");
  m_content.set<Component::Font> ("interface:font", local_file_name(interface_font), 80);

  std::string interface_color = input["interface_color"].string();
  m_content.set<Component::String> ("interface:color", interface_color);

  std::array<unsigned char, 3> color = color_from_string (interface_color);

  for (std::size_t i = 0; i < input["inventory_arrows"].size(); ++ i)
  {
    std::string id = input["inventory_arrows" ][i].string("images", "interface", "png");
    auto arrow
      = m_content.set<Component::Image> ("inventory_arrow_" + std::to_string(i) + ":image",
                                         local_file_name(id),
                                         Config::inventory_front_depth);
    arrow->set_relative_origin(0.5, 0.5);
    auto arrow_background
      = m_content.set<Component::Image> ("inventory_arrow_background_" + std::to_string(i)
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

  m_content.set<Component::String>("game:new_room", input["load_room"].string());
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
