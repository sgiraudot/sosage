#include <Sosage/Component/Console.h>

#include <algorithm>

namespace Sosage::Component
{

Console::Console (const std::string& id)
  : Boolean(id, false)
{
  m_content << "SOSAGE CONSOLE" << std::endl;
}

std::string Console::console_str()
{
  std::string content = m_content.str();

  std::size_t nb_lines = std::count (content.begin(), content.end(), '\n');

  std::size_t start = 0;
  while (nb_lines > 20)
  {
    start = content.find ('\n', start + 1);
    -- nb_lines;
  }

  std::string out = "";

  std::size_t begin = start;
  while (begin != content.size())
  {
    std::size_t end = content.find ('\n', begin + 1);
    if (end == std::string::npos)
      end = content.size();

    out += "> " + std::string (content.begin() + begin + 1, content.begin() + end) + "\n";

    begin = end;
  }
  
  m_content = std::stringstream();
  m_content << std::string (content.begin() + start, content.end());

  return out;
}

} // namespace Sosage::Component
