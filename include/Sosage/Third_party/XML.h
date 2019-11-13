#ifndef SOSAGE_THIRD_XML_H
#define SOSAGE_THIRD_XML_H

#include <libxml/parser.h>
#include <libxml/tree.h>

#include <string>

namespace Sosage
{
class Engine;
}

namespace Sosage::Third_party
{

class XML
{
  
public:

  XML ();
  ~XML ();

  std::string read (const std::string& file_name, Engine& engine);
};

} // namespace Sosage::Third_party
  

#endif // SOSAGE_THIRD_XML_H
