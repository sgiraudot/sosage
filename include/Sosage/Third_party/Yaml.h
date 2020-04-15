#ifndef SOSAGE_THIRD_YAML_H
#define SOSAGE_THIRD_YAML_H

#include <Sosage/Utils/file.h>

#include <SDL.h>

#include <map>
#include <stack>
#include <string>
#include <vector>

#include <stdio.h>
#include <yaml.h>

namespace Sosage::Third_party
{

class Yaml
{
public:

  class Node
  {
  public:
    bool sequence;
    std::map<std::string, Node*> map;
    std::vector<Node*> vec;
    std::string value;

    Node() : sequence(false) { }

    const Node& operator[] (const std::string& key) const
    {
      check(has(key), "Value " + key + " not found in Yaml node " + value);
      return *(map.find(key)->second);
    }

    const Node& operator[] (const char* key) const
    {
      check(has(key), "Value " + std::string(key) + " not found in Yaml node " + value);
      return *(map.find(std::string(key))->second);
    }

    const Node& operator[] (int idx) const
    {
      return *vec[idx];
    }

    bool has (const std::string& key) const
    {
      return (map.find(key) != map.end());
    }

    std::size_t size() const { return vec.size(); }

    std::string string () const
    {
      return value;
    }
    
    std::string string (const std::string& folder,
                        const std::string& extension) const
    {
      return folder + Sosage::folder_separator + value + '.' + extension;
    }

    std::string string (const std::string& folder, const std::string& subfolder,
                        const std::string& extension) const
    {
      return folder + Sosage::folder_separator + subfolder + Sosage::folder_separator
        + value + '.' + extension;
    }

    int integer () const
    {
      return std::atoi(value.c_str());
    }

    bool boolean() const
    {
      return (value == "true");
    }

    std::vector<std::string> string_array() const
    {
      std::vector<std::string> out;
      out.reserve(vec.size());
      for (Node* n : vec)
        out.push_back (n->value);
      return out;
    }

    void print(std::string prefix = "")
    {
      if (value != "")
        std::cerr << prefix << " -> " << value << std::endl;
      else
      {
        for (const auto& m : map)
        {
          std::string new_prefix = prefix + ":" + m.first;
          m.second->print(new_prefix);
        }
        for (std::size_t j = 0; j < vec.size(); ++ j)
        {
          std::string new_prefix = prefix + ":" + std::to_string(j);
          vec[j]->print(new_prefix);
        }
      }

    }

  };

private:
  
  Node* m_root;

public:

  Yaml (const std::string& filename)
  {
    File file = Sosage::open (filename.c_str());

    unsigned char* buffer = new unsigned char[file.size + 1];
    std::size_t nb_read_total = 0, nb_read = 1;
    unsigned char* buf = buffer;
    while (nb_read_total < file.size && nb_read != 0) {
      nb_read = Sosage::read (file, buf, (file.size - nb_read_total));
      nb_read_total += nb_read;
      buf += nb_read;
    }
    Sosage::close (file);

    check (nb_read_total == file.size, "Error while reading " + filename);
    buffer[nb_read_total] = '\0';

    yaml_parser_t parser;

    bool parser_initialized = yaml_parser_initialize(&parser);
    check (parser_initialized, "Failed initializing Yaml parser");

    yaml_parser_set_input_string(&parser, buffer, file.size);

    Node* n;
    std::string key = "";
    std::stack<Node*> nodes;

    yaml_event_t event;
    do
    {
      bool event_parsed = yaml_parser_parse(&parser, &event);
      check (event_parsed, "Failed parsing Yaml event");

      switch(event.type)
      {
        // Ignored events
        case YAML_NO_EVENT: break;
        case YAML_STREAM_START_EVENT: break;
        case YAML_STREAM_END_EVENT: break;
        case YAML_DOCUMENT_START_EVENT: break;
        case YAML_DOCUMENT_END_EVENT: break;
        case YAML_ALIAS_EVENT: break;

        case YAML_SEQUENCE_START_EVENT:
          n = new Node;
          n->sequence = true;
          if (!nodes.empty())
          {
            if (nodes.top()->sequence)
              nodes.top()->vec.push_back (n);
            else
              nodes.top()->map.insert (std::make_pair (key, n));
          }
          key = "";
          nodes.push(n);
          break;
        
        case YAML_SEQUENCE_END_EVENT:
          nodes.pop();
          break;

        case YAML_MAPPING_START_EVENT:
          if (key == "")
          {
            n = new Node;
            if (!nodes.empty())
              nodes.top()->vec.push_back (n);
            nodes.push(n);
          }
          else
          {
            n = new Node;
            if (!nodes.empty())
              nodes.top()->map.insert (std::make_pair (key, n));
            key = "";
            nodes.push(n);
          }
          break;
        
        case YAML_MAPPING_END_EVENT:
          m_root = nodes.top();
          nodes.pop();
          break;

        case YAML_SCALAR_EVENT:
          std::string v = std::string(reinterpret_cast<const char*>(event.data.scalar.value));
          if (nodes.top()->sequence)
          {
            Node* n = new Node;
            n->value = v;
            nodes.top()->vec.push_back(n);
          }
          else if (key == "")
            key = v;
          else
          {
            Node* n = new Node;
            n->value = v;
            nodes.top()->map.insert (std::make_pair(key, n));
            key = "";
          }
          break;
      }
      if(event.type != YAML_STREAM_END_EVENT)
        yaml_event_delete(&event);
    }
    while(event.type != YAML_STREAM_END_EVENT);
    yaml_event_delete(&event);
    yaml_parser_delete(&parser);

//    m_root->print();
  }

  const Node& operator[] (const std::string& key) const
  {
    return (*m_root)[key];
  }

  const Node& operator[] (const char* key) const
  {
    return (*m_root)[key];
  }

  const Node& operator[] (int idx) const
  {
    return (*m_root)[idx];
  }
  
  bool has (const std::string& key) const
  {
    return m_root->has(key);
  }

};

} // namespace Sosage::Third_party
  

#endif // SOSAGE_THIRD_YAML_H
