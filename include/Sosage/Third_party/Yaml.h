/*
  [include/Sosage/Third_party/Yaml.h]
  Wrapper for libyaml library.

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

#ifndef SOSAGE_THIRD_YAML_H
#define SOSAGE_THIRD_YAML_H

#include <Sosage/Utils/conversions.h>
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
    typedef std::shared_ptr<Node> Ptr;
    
    bool sequence;
    std::map<std::string, Ptr> map;
    std::vector<Ptr> vec;
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

    const Node& operator[] (std::size_t idx) const
    {
      return *vec[idx];
    }

    const Node& operator[] (int idx) const
    {
      return *vec[std::size_t(idx)];
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

    std::string nstring () const
    {
      check(map.size() == 1, "Checking for nstring in non-singular node");
      return map.begin()->first;
    }

    std::string string (const std::string& folder,
                        const std::string& extension) const
    {
      return folder + Config::folder_separator + value + '.' + extension;
    }

    std::string string (const std::string& folder, const std::string& subfolder,
                        const std::string& extension) const
    {
      return folder + Config::folder_separator + subfolder + Config::folder_separator
        + value + '.' + extension;
    }

    int integer () const
    {
      return to_int(value);
    }

    double floating () const
    {
      return to_double(value);
    }

    bool boolean() const
    {
      return to_bool(value);
    }

    std::vector<std::string> string_array() const
    {
      std::vector<std::string> out;
      out.reserve(vec.size());
      for (Ptr n : vec)
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

  std::string m_filename;
  File m_file;
  Node::Ptr m_root;

public:

  Yaml (const std::string& filename, bool write = false)
    : m_filename (filename)
  {
    m_file = Sosage::open (m_filename.c_str(), write);
  }

  ~Yaml()
  {
    Sosage::close (m_file);
  }

  void parse()
  {
    unsigned char* buffer = new unsigned char[m_file.size + 1];
    std::size_t nb_read_total = 0, nb_read = 1;
    unsigned char* buf = buffer;
    while (nb_read_total < m_file.size && nb_read != 0) {
      nb_read = Sosage::read (m_file, buf, (m_file.size - nb_read_total));
      nb_read_total += nb_read;
      buf += nb_read;
    }

    check (nb_read_total == m_file.size, "Error while reading " + m_filename);
    buffer[nb_read_total] = '\0';

    yaml_parser_t parser;

    bool parser_initialized = yaml_parser_initialize(&parser);
    check (parser_initialized, "Failed initializing Yaml parser");

    yaml_parser_set_input_string(&parser, buffer, m_file.size);

    Node::Ptr n;
    std::string key = "";
    std::stack<Node::Ptr> nodes;

    yaml_event_t event;
    do
    {
      bool event_parsed = yaml_parser_parse(&parser, &event);
      check (event_parsed, "Failed parsing Yaml file (" + m_filename + ":" +
             std::to_string(parser.mark.line) + ")");

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
          n = std::make_shared<Node>();
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
            n = std::make_shared<Node>();
            if (!nodes.empty())
              nodes.top()->vec.push_back (n);
            nodes.push(n);
          }
          else
          {
            n = std::make_shared<Node>();
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
            Node::Ptr n = std::make_shared<Node>();
            n->value = v;
            nodes.top()->vec.push_back(n);
          }
          else if (key == "")
            key = v;
          else
          {
            Node::Ptr n = std::make_shared<Node>();
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

    delete[] buffer;
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

  template <typename T>
  void write (const std::string& key, const T& value)
  {
    Sosage::write (m_file, key + ": " + std::to_string(value) + "\n");
  }
  void write (const std::string& key, const bool& value)
  {
    Sosage::write (m_file, key + ": " + (value ? "true" : "false") + "\n");
  }
  void write (const std::string& key, const std::string& value)
  {
    Sosage::write (m_file, key + ": " + value + "\n");
  }
  void write (const std::string& key, const int& v0, const int& v1)
  {
    Sosage::write (m_file, key + ": [" + std::to_string(v0) + ", " + std::to_string(v1) + "]\n");
  }

};

} // namespace Sosage::Third_party
  

#endif // SOSAGE_THIRD_YAML_H
