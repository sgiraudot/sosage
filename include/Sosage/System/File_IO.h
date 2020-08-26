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

#include <Sosage/Config/config.h>
#include <Sosage/Content.h>
#include <Sosage/Core/File_IO.h>
#include <Sosage/System/Handle.h>

#include <unordered_set>
#ifdef SOSAGE_THREADS_ENABLED
#include <atomic>
#include <thread>
#endif

namespace Sosage::System
{

class File_IO : public Base
{
private:

  Content& m_content;
  std::string m_folder_name;
  std::unordered_set<std::string> m_latest_room_entities;
#ifdef SOSAGE_THREADS_ENABLED
  std::atomic<Thread_state> m_thread_state = NO_THREAD;
  std::thread m_thread;
#endif

public:

  File_IO (Content& content);

  virtual void run();

  void read_config();
  void write_config();

  void test_init_folder (const std::string& folder_name)
  { Core::File_IO input (folder_name + "data" + Config::folder_separator + "init.yaml"); }

  void read_init (const std::string& folder_name);

private:

  std::string local_file_name (const std::string& file_name) const;
  std::string local_file_name (const std::string& folder, const std::string& subfolder,
                               const std::string& file_name, const std::string& extension) const;

  // Implemented in File_IO__read_room.cpp:
public:
  void read_room (const std::string& file_name);
private:
  void read_animation (const Core::File_IO::Node& node, const std::string& id);
  void read_character (const Core::File_IO::Node& node, const std::string& id);
  void read_code (const Core::File_IO::Node& node, const std::string& id);
  void read_object (const Core::File_IO::Node& node, const std::string& id);
  void read_scenery (const Core::File_IO::Node& node, const std::string& id);
  void read_window (const Core::File_IO::Node& node, const std::string& id);

};

} // namespace Sosage::System

#endif // SOSAGE_SYSTEM_FILE_IO_H
