/*
  [include/Sosage/System/File_IO.h]
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

#ifndef SOSAGE_SYSTEM_FILE_IO_H
#define SOSAGE_SYSTEM_FILE_IO_H

#include <Sosage/Component/Action.h>
#include <Sosage/Component/Font.h>
#include <Sosage/Component/Image.h>
#include <Sosage/Config/config.h>
#include <Sosage/Content.h>
#include <Sosage/Core/File_IO.h>
#include <Sosage/System/Base.h>

#include <unordered_set>

namespace Sosage::System
{

class File_IO : public Base
{
private:

  using Function = std::function<void(const std::string&, const Core::File_IO::Node&)>;
  std::unordered_map<std::string, Function> m_dispatcher;

public:

  File_IO (Content& content);

  virtual void run();

  void clean_content();

  void read_config();
  void write_config();

  void read_init ();
  void read_locale ();

  bool read_savefile(const std::string& save_id);
  void write_savefile();

private:

  void read_init_general (const Core::File_IO& input);
  void read_init_cursor (const Core::File_IO& input);
  void read_init_inventory (const Core::File_IO& input);
  void read_init_interface (const Core::File_IO& input);
  void read_init_functions (const Core::File_IO& input);
  void read_init_global_items (const Core::File_IO& input);
  void read_init_text_defaults (const Core::File_IO& input);

  void read_savefiles (const Core::File_IO& input);

  void parse_function (const std::vector<std::string>& args,
                       Component::Action_handle action);
  void create_locale_dependent_text (const std::string& id, Component::Font_handle font,
                                     const std::string& color, const std::string& text);
  void load_locale_dependent_image (const std::string& entity, const std::string& component,
                                    const std::string& filename,
                                    const std::function<Component::Image_handle(std::string)>& func);

  // Implemented in File_IO__read_room.cpp:
  void read_room (const std::string& file_name);

  void read_action (const std::string& id, const Core::File_IO::Node& input);
  void read_animation (const std::string& id, const Core::File_IO::Node& input);
  void read_character (const std::string& id, const Core::File_IO::Node& input);
  void read_character_skin (const std::string& id, const Core::File_IO::Node& input,
                            const std::string& default_state);
  void read_code (const std::string& id, const Core::File_IO::Node& input);
  void read_dialog (const std::string& id, const Core::File_IO::Node& input);
  void read_integer (const std::string& id, const Core::File_IO::Node& input);
  void read_music (const std::string& id, const Core::File_IO::Node& input);
  void read_object (const std::string& id, const Core::File_IO::Node& input);
  std::pair<Component::Handle, Component::Handle>
  read_object_action (const std::string& id, const std::string& action,
                      const Core::File_IO::Node& input);
  void read_scenery (const std::string& id, const Core::File_IO::Node& input);
  void read_sound (const std::string& id, const Core::File_IO::Node& input);
  void read_text (const std::string& id, const Core::File_IO::Node& input);
  void read_window (const std::string& id, const Core::File_IO::Node& input);

};

} // namespace Sosage::System

#endif // SOSAGE_SYSTEM_FILE_IO_H
