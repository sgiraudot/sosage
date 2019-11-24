#ifndef SOSAGE_THIRD_XML_H
#define SOSAGE_THIRD_XML_H

#include <libxml/parser.h>
#include <libxml/tree.h>

#include <memory>
#include <string>

namespace Sosage::Third_party
{

class XML
{
  class File
  {
    xmlDoc* m_file;
  public:
    File (const std::string& file_name)
    {
      m_file = xmlReadFile (file_name.c_str(), nullptr, 0);
    }
    ~File()
    {
      xmlFreeDoc(m_file);
      xmlCleanupParser();
    }

    xmlNode* root() { return xmlDocGetRootElement(m_file); }
  };
public:

  class Element
  {
    std::shared_ptr<File> m_file;
    xmlNode* m_node;
    
  public:

    Element (const std::string& file_name)
      : m_file (new File(file_name))
    {
      m_node = m_file->root();
    }
    Element (std::shared_ptr<File> file, xmlNode* node)
      : m_file (file), m_node (node)
    { }

    std::string name() { return std::string((const char*)(m_node->name)); }
    std::string property (const std::string& key)
    {
      xmlChar* c = xmlGetProp (m_node, (const xmlChar*)(key.c_str()));
      check (c != nullptr, "Cannot read property " + key);
      std::string out = (const char*)c;
      xmlFree(c);
      return out;
    }
    std::string property (const std::string& prefix, const std::string& key, const std::string& suffix)
    {
      return prefix + property(key) + suffix;
    }
    
    int int_property (const std::string& key, const int& def = -1)
    {
      xmlChar* c = xmlGetProp (m_node, (const xmlChar*)(key.c_str()));
      if (def != -1 && c == nullptr)
        return def;
      check (c != nullptr, "Cannot read int property " + key);
      int out = std::atoi ((const char*)c);
      xmlFree(c);
      return out;
    }
    double double_property (const std::string& key,
                            const double& def = std::numeric_limits<double>::quiet_NaN())
    {
      xmlChar* c = xmlGetProp (m_node, (const xmlChar*)(key.c_str()));
      if (def != std::numeric_limits<double>::quiet_NaN() && c == nullptr)
        return def;
      check (c != nullptr, "Cannot read double property " + key);
      double out = std::atof ((const char*)c);
      xmlFree(c);
      return out;
    }

    Element next_child() { return Element (m_file, m_node->children->next); }
    Element next() { return Element (m_file, m_node->next); }

    operator bool() const { return m_node; }
  };
};

} // namespace Sosage::Third_party
  

#endif // SOSAGE_THIRD_XML_H
