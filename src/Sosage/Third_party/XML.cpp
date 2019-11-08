#include <Sosage/Third_party/XML.h>
#include <Sosage/Engine.h>
#include <Sosage/Utils/error.h>
#include <Sosage/version.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

namespace Sosage::Third_party
{

std::string get_name (xmlNode* node)
{
  return std::string((const char*)(node->name));
}

template <typename T>
T get (xmlNode* node, const std::string& id)
{
  check(false, "Error: unknown XML node type.");
}

template <>
std::string get<std::string> (xmlNode* node, const std::string& id)
{
  xmlChar* c = xmlGetProp (node, (const xmlChar*)(id.c_str()));
  std::string out = (const char*)c;
  xmlFree(c);
  return out;
}

template <>
int get<int> (xmlNode* node, const std::string& id)
{
  xmlChar* c = xmlGetProp (node, (const xmlChar*)(id.c_str()));
  if (!c)
    return -1;
  int out = std::atoi ((const char*)c);
  xmlFree(c);
  return out;
}

XML::XML () { }

XML::~XML () { }

void XML::read (const std::string& file_name, Engine& engine)
{
  xmlDoc* file = xmlReadFile (file_name.c_str(), nullptr, 0);
  check (file != nullptr, "Cannot read " + file_name);

  // First, get general room data
  xmlNode* node = xmlDocGetRootElement (file);
  check (get_name(node) == "sosage_room", file_name + " is not a Sosage room.");

  std::string v = get<std::string> (node, "version");
  check (version(v) == SOSAGE_VERSION,
         "Error: room version " + v + " incompatible with Sosage " + Sosage::version());
  
  std::string name = get<std::string> (node, "name");
  std::string background = "backgrounds/" + get<std::string> (node, "background") + ".png";
  std::string ground_map = "backgrounds/" + get<std::string> (node, "ground_map") + ".png";
  engine.set_background (background, ground_map);

  std::string cursor = "sprites/" + get<std::string> (node, "cursor") + ".png";
  std::string interface_font = "fonts/" + get<std::string> (node, "interface_font") + ".ttf";
  std::string interface_color = get<std::string> (node, "interface_color");
  engine.set_interface (cursor, interface_font, interface_color);

  std::string body = "sprites/" + get<std::string> (node, "body") + ".png";
  std::string head = "sprites/" + get<std::string> (node, "head") + ".png";
  int x = get<int> (node, "x");
  int y = get<int> (node, "y");
  engine.set_character (body, head, x, y);

  // Then, get room content
  node = node->xmlChildrenNode;
  do
  {
    if (get_name(node) == "object")
    {
      std::string id = get<std::string> (node, "id");
      std::string name = get<std::string> (node, "name");
      int x = get<int> (node, "x");
      int y = get<int> (node, "y");
    }
    else if (get_name(node) == "npc")
    {

    }
    else if (get_name(node) == "scenery")
    {

    }
  }
  while (node = node->next);
  

  xmlFreeDoc (file);
  xmlCleanupParser();
}


} // namespace Sosage::Third_party
