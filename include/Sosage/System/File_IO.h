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

#include <Sosage/Component/Font.h>
#include <Sosage/Component/Image.h>
#include <Sosage/Config/config.h>
#include <Sosage/Content.h>
#include <Sosage/Core/File_IO.h>
#include <Sosage/System/Handle.h>

#include <unordered_set>

namespace Sosage::System
{

class File_IO : public Base
{
private:

  std::unordered_set<std::string> m_latest_room_entities;

public:

  File_IO (Content& content);

  virtual void run();

  void clean_content();

  void read_config();
  void write_config();

  void read_init ();
  void read_locale ();

  void read_cutscene (const std::string& file_name);

  void read_savefile();
  void write_savefile();

private:

  void create_locale_dependent_text (const std::string& id, Component::Font_handle font,
                                     const std::string& color, const std::string& text);
  void load_locale_dependent_image (const std::string& id, const std::string& filename,
                                    const std::function<Component::Image_handle(std::string)>& func);

  // Implemented in File_IO__read_room.cpp:
  void read_room (const std::string& file_name);

  void read_action (const Core::File_IO::Node& input);
  void read_animation (const Core::File_IO::Node& input);
  void read_character (const Core::File_IO::Node& input);
  void read_code (const std::string& id);
  void read_dialog (const std::string& id);
  void read_integer (const Core::File_IO::Node& input);
  void read_music (const Core::File_IO::Node& input);
  void read_object (const std::string& id);
  std::pair<Component::Handle, Component::Handle>
  read_object_action (const std::string& id, const std::string& action,
                      const Core::File_IO::Node& input);
  void read_origin (const Core::File_IO::Node& input);
  void read_scenery (const Core::File_IO::Node& input);
  void read_sound (const Core::File_IO::Node& input);
  void read_window (const Core::File_IO::Node& input);

};

} // namespace Sosage::System

#endif // SOSAGE_SYSTEM_FILE_IO_H
