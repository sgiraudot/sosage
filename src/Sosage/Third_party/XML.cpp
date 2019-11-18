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

std::string XML::read (const std::string& file_name, Engine& engine)
{
  std::string out;
  
  xmlDoc* file = xmlReadFile (file_name.c_str(), nullptr, 0);
  check (file != nullptr, "Cannot read " + file_name);

  xmlNode* node = xmlDocGetRootElement (file);
  if (get_name(node) == "sosage_init")
  {
    std::string v = get<std::string> (node, "version");
    check (version(v) == SOSAGE_VERSION,
           "Error: room version " + v + " incompatible with Sosage " + Sosage::version());

    std::string cursor = "sprites/" + get<std::string> (node, "cursor") + ".png";
    std::string debug_font = "fonts/" + get<std::string> (node, "debug_font") + ".ttf";
    std::string interface_font = "fonts/" + get<std::string> (node, "interface_font") + ".ttf";
    std::string interface_color = get<std::string> (node, "interface_color");
    engine.set_interface (cursor, debug_font, interface_font, interface_color);
    
    node = node->children->next;

    check(get_name(node) == "load", "Error: init file should load room");
    out = get<std::string> (node, "room") + ".xml";
  }
  else
  {
    check (get_name(node) == "sosage_room", file_name + " is not a Sosage room.");
  
    std::string name = get<std::string> (node, "name");
    std::string background = "backgrounds/" + get<std::string> (node, "background") + ".png";
    std::string ground_map = "backgrounds/" + get<std::string> (node, "ground_map") + ".png";
    engine.set_background (background, ground_map);

    std::string body = "sprites/" + get<std::string> (node, "body") + ".png";
    std::string head = "sprites/" + get<std::string> (node, "head") + ".png";
    int x = get<int> (node, "x");
    int y = get<int> (node, "y");
    engine.set_character (body, head, x, y);

    // Then, get room content
    node = node->children;
    while ((node = node->next))
    {
      if (get_name(node) == "object")
      {
        std::string id = get<std::string> (node, "id");
        std::string name = get<std::string> (node, "name");
        int x = get<int> (node, "x");
        int y = get<int> (node, "y");

        node = node->children->next;
        check (get_name(node) == "state", "Object " + id + " has no state.");
        std::string skin = "sprites/" + get<std::string>(node, "skin") + ".png";
        engine.add_object (id, skin, x, y);
      }
      else if (get_name(node) == "npc")
      {

      }
      else if (get_name(node) == "scenery")
      {

      }
    }
  }  

  xmlFreeDoc (file);
  xmlCleanupParser();

  return out;
}


} // namespace Sosage::Third_party
