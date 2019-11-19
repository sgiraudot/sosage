#ifndef SOSAGE_SYSTEM_IO_H
#define SOSAGE_SYSTEM_IO_H

#include <Sosage/Content.h>
#include <Sosage/Core/IO.h>

namespace Sosage::System
{

class IO
{
private:

  Content& m_content;

  std::string m_folder_name;

public:

  IO (Content& content);

  std::string read_init (const std::string& folder_name);
  std::string read_room (const std::string& file_name);
  std::string read_character (const std::string& file_name);

private:
  
  inline std::string local_file_name (const std::string& file_name)
  {
    return m_folder_name + file_name;
  }

};

} // namespace Sosage::System

#endif // SOSAGE_SYSTEM_IO_H
