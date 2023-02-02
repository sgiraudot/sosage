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

#include <Sosage/Utils/Asset_manager.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Sosage::Third_party
{

class Yaml
{
public:

  class Node
  {
  public:
    using Ptr = std::shared_ptr<Node>;
    using Map = std::map<std::string, Ptr>;
    using Vec = std::vector<Ptr>;
    using iterator = typename Map::const_iterator;

    bool sequence;
    Map map;
    Vec vec;
    std::string value;

    Node();
    iterator begin() const;
    iterator end() const;
    const Node& operator[] (const std::string& key) const;
    const Node& operator[] (const char* key) const;
    const Node& operator[] (std::size_t idx) const;
    const Node& operator[] (int idx) const;
    bool has (const std::string& key) const;
    std::size_t size() const;
    std::string string () const;
    std::string nstring () const;
    std::string string (const std::string& folder,
                        const std::string& extension) const;
    std::string string (const std::string& folder, const std::string& subfolder,
                        const std::string& extension) const;
    bool is_relative() const;
    int integer () const;
    double floating () const;
    bool boolean() const;
    std::vector<std::string> string_array() const;
    double time() const;
    void print(std::string prefix = "");
  };

private:

  std::string m_filename;
  Asset m_file;
  Node::Ptr m_root;
  std::size_t m_indent;

public:

  Yaml (const std::string& filename, bool pref_file = false, bool write = false);
  ~Yaml();
  bool parse();
  const Node& root() const;
  const Node& operator[] (const std::string& key) const;
  const Node& operator[] (const char* key) const;
  const Node& operator[] (int idx) const;
  bool has (const std::string& key) const;
  void indent();
  void start_section (const std::string& name);
  void end_section();

  template <typename T>
  void write (const std::string& key, const T& value)
  {
    indent();
    m_file.write (key + ": " + std::to_string(value) + "\n");
  }

  void write (const std::string& key, const bool& value);
  void write (const std::string& key, const std::string& value);
  void write (const std::string& key, const int& v0, const int& v1);
  void write (const std::string& key, const std::vector<std::string>& value);
  void write_list_item (const std::string& value);

  template <typename T>
  void write_list_item (const std::string& key1, const std::string& value1,
                        const std::string& key2, const T& value2)
  {
    indent();
    m_file.write ("- { " + key1 + ": \"" + value1 + "\", "
                   + key2 + ": " + std::to_string(value2) + " }\n");
  }

  void write_list_item (const std::string& key1, const std::string& value1,
                        const std::string& key2, const std::string& value2);
  void write_list_item (const std::string& key1, const std::string& value1,
                        const std::string& key2, const bool& value2);

  template <typename T>
  void write_list_item (const std::string& key1, const std::string& value1,
                        const std::string& key2, const std::initializer_list<T>& value2)
  {
    indent();
    m_file.write ("- { " + key1 + ": \"" + value1 + "\", "
                   + key2 + ": [");
    std::size_t idx = 0;
    for (const T& t : value2)
    {
      m_file.write (std::to_string(t));
      if (++ idx != value2.size())
        m_file.write (", ");
    }
    m_file.write ("] }\n");
  }

};

} // namespace Sosage::Third_party
  

#endif // SOSAGE_THIRD_YAML_H
