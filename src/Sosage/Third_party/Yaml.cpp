/*
  [src/Sosage/Third_party/Yaml.cpp]
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

#include <Sosage/Third_party/Yaml.h>
#include <Sosage/Utils/conversions.h>
#include <Sosage/Utils/error.h>

#include <stack>

#include <yaml.h>

namespace Sosage::Third_party
{

Yaml::Node::Node()
  : sequence(false)
{ }

Yaml::Node::iterator Yaml::Node::begin() const
{
  return map.begin();
}

Yaml::Node::iterator Yaml::Node::end() const
{
  return map.end();
}

const Yaml::Node& Yaml::Node::operator[] (const std::string& key) const
{
  check(has(key), "Value " + key + " not found in Yaml input " + value);
  return *(map.find(key)->second);
}

const Yaml::Node& Yaml::Node::operator[] (const char* key) const
{
  check(has(key), "Value " + std::string(key) + " not found in Yaml input " + value);
  return *(map.find(std::string(key))->second);
}

const Yaml::Node& Yaml::Node::operator[] (std::size_t idx) const
{
  return *vec[idx];
}

const Yaml::Node& Yaml::Node::operator[] (int idx) const
{
  return *vec[std::size_t(idx)];
}

bool Yaml::Node::has (const std::string& key) const
{
  return contains (map, key);
}

std::size_t Yaml::Node::size() const
{
  return vec.size();
}

std::string Yaml::Node::string () const
{
  return value;
}

std::string Yaml::Node::nstring () const
{
  check(map.size() == 1, "Checking for nstring in non-singular input");
  return map.begin()->first;
}

std::string Yaml::Node::string (const std::string& folder,
                                const std::string& extension) const
{
  return folder + "/" + value + "." + extension;

}

std::string Yaml::Node::string (const std::string& folder, const std::string& subfolder,
                                const std::string& extension) const
{
  return folder + "/" + subfolder + "/" + value + "." + extension;
}

bool Yaml::Node::is_relative () const
{
  return value[0] == '+' || value[0] == '-';
}

int Yaml::Node::integer () const
{
  return to_int(value);
}

double Yaml::Node::floating () const
{
  return to_double(value);
}

bool Yaml::Node::boolean() const
{
  return to_bool(value);
}

std::vector<std::string> Yaml::Node::string_array() const
{
  std::vector<std::string> out;
  out.reserve(vec.size());
  for (Ptr n : vec)
    out.push_back (n->value);
  return out;
}

double Yaml::Node::time() const
{
  std::size_t sep0 = value.find(':');
  check (sep0 != std::string::npos, "Time string should have ':' character");
  std::size_t sep1 = value.find('.');
  check (sep0 != std::string::npos, "Time string should have '.' character");

  std::string minutes_str (value.begin(), value.begin() + sep0);
  std::string seconds_str (value.begin() + sep0 + 1, value.begin() + sep1);
  std::string milliseconds_str (value.begin() + sep1 + 1, value.end());

  int minutes = to_int(minutes_str);
  int seconds = to_int(seconds_str);
  int milliseconds = to_int(milliseconds_str);

  return 60. * minutes + seconds + (milliseconds / 1000.);
}


void Yaml::Node::print(std::string prefix)
{
  if (value != "")
  {
    debug << prefix << " -> " << value << std::endl;
  }
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

Yaml::Yaml (const std::string& filename, bool pref_file, bool write)
  : m_filename (filename), m_indent(0)
{
  if (pref_file)
    m_file = Asset_manager::open_pref (m_filename.c_str(), write);
  else
    m_file = Asset_manager::open (m_filename.c_str());
}

Yaml::~Yaml()
{
  if (m_file)
    m_file.close();
}

bool Yaml::parse()
{
  if (!m_file)
    return false;

  unsigned char* buffer = new unsigned char[m_file.size() + 1];
  std::size_t nb_read_total = 0, nb_read = 1;
  unsigned char* buf = buffer;
  while (nb_read_total < m_file.size() && nb_read != 0) {
    nb_read = m_file.read (buf, (m_file.size() - nb_read_total));
    nb_read_total += nb_read;
    buf += nb_read;
  }

  check (nb_read_total == m_file.size(), "Error while reading " + m_filename);
  buffer[nb_read_total] = '\0';

  yaml_parser_t parser;

  bool parser_initialized = yaml_parser_initialize(&parser);
  check (parser_initialized, "Failed initializing Yaml parser");

  yaml_parser_set_input_string(&parser, buffer, m_file.size());

  Node::Ptr n;
  std::string key = "";
  std::stack<Node::Ptr> inputs;

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
        if (!inputs.empty())
        {
          if (inputs.top()->sequence)
            inputs.top()->vec.push_back (n);
          else
            inputs.top()->map.insert (std::make_pair (key, n));
        }
        key = "";
        inputs.push(n);
        break;

      case YAML_SEQUENCE_END_EVENT:
        inputs.pop();
        break;

      case YAML_MAPPING_START_EVENT:
        if (key == "")
        {
          n = std::make_shared<Node>();
          if (!inputs.empty())
            inputs.top()->vec.push_back (n);
          inputs.push(n);
        }
        else
        {
          n = std::make_shared<Node>();
          if (!inputs.empty())
            inputs.top()->map.insert (std::make_pair (key, n));
          key = "";
          inputs.push(n);
        }
        break;

      case YAML_MAPPING_END_EVENT:
        m_root = inputs.top();
        inputs.pop();
        break;

      case YAML_SCALAR_EVENT:
        std::string v = std::string(reinterpret_cast<const char*>(event.data.scalar.value));
        if (inputs.top()->sequence)
        {
          Node::Ptr n = std::make_shared<Node>();
          n->value = v;
          inputs.top()->vec.push_back(n);
        }
        else if (key == "")
          key = v;
        else
        {
          Node::Ptr n = std::make_shared<Node>();
          n->value = v;
          inputs.top()->map.insert (std::make_pair(key, n));
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

  return true;
}

const Yaml::Node& Yaml::root() const
{
  return *m_root;
}

const Yaml::Node& Yaml::operator[] (const std::string& key) const
{
  return (*m_root)[key];
}

const Yaml::Node& Yaml::operator[] (const char* key) const
{
  return (*m_root)[key];
}

const Yaml::Node& Yaml::operator[] (int idx) const
{
  return (*m_root)[idx];
}

bool Yaml::has (const std::string& key) const
{
  return m_root->has(key);
}

void Yaml::indent()
{
  if (m_indent != 0)
    m_file.write (std::string(m_indent, ' '));
}

void Yaml::start_section (const std::string& name)
{
  indent();
  m_file.write (name + ":\n");
  m_indent += 2;
}

void Yaml::end_section()
{
  m_indent -= 2;
}

void Yaml::write (const std::string& key, const bool& value)
{
  indent();
  m_file.write (key + ": " + (value ? "true" : "false") + "\n");
}

void Yaml::write (const std::string& key, const std::string& value)
{
  indent();
  m_file.write (key + ": \"" + value + "\"\n");
}

void Yaml::write (const std::string& key, const int& v0, const int& v1)
{
  indent();
  m_file.write (key + ": [" + std::to_string(v0) + ", " + std::to_string(v1) + "]\n");
}

void Yaml::write (const std::string& key, const std::vector<std::string>& value)
{
  indent();
  m_file.write (key + ":\n");
  m_indent += 2;
  for (const std::string& v : value)
  {
    indent();
    m_file.write ("- \"" + v + "\"\n");
  }
  m_indent -= 2;
}

void Yaml::write_list_item (const std::string& value)
{
  indent();
  m_file.write ("- \"" + value + "\"\n");
}

void Yaml::write_list_item (const std::string& key1, const std::string& value1,
                            const std::string& key2, const std::string& value2)
{
  indent();
  m_file.write ("- { " + key1 + ": \"" + value1 + "\", "
                 + key2 + ": \"" + value2 + "\" }\n");
}

void Yaml::write_list_item (const std::string& key1, const std::string& value1,
                            const std::string& key2, const bool& value2)
{
  indent();
  m_file.write ("- { " + key1 + ": \"" + value1 + "\", "
                 + key2 + ": \"" + (value2 ? "true" : "false") + "\" }\n");
}

} // namespace Sosage::Third_party
