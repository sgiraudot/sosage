#include <Sosage/Core/File_IO.h>

#include <unordered_map>

int main()
{
  std::unordered_map<std::string, std::vector<std::string> > map_id_to_room;

  std::string data_files = SOSAGE_DATA_FILES;
  std::size_t begin = 0;
  while (begin < data_files.size())
  {
    std::size_t end = data_files.find(';', begin+1);
    if (end == std::string::npos)
      end = data_files.size();

    std::string filename (data_files.begin() + begin, data_files.begin() + end);
    std::cerr << "Parsing " << filename << "... ";
    begin = end + 1;

    Sosage::Core::File_IO input (filename);
    input.parse();

    std::string name = input["name"].string();

    if (input.has("content"))
      for (std::size_t i = 0; i < input["content"].size(); ++ i)
      {
        Sosage::Core::File_IO::Node node = input["content"][i];
        if (node.has("type"))
        {
          std::string type = node["type"].string();
          // actions and characters are allowed to be duplicated
          if (type == "action" || type == "character")
            continue;
        }
        auto inserted = map_id_to_room.insert
                        (std::make_pair (node["id"].string(),
                         std::vector<std::string>()));
        inserted.first->second.emplace_back(name);
      }
    std::cerr << "done" << std::endl;
  }

  bool okay = true;
  std::cerr << "[Checking unicity of IDs accross yaml files]" << std::endl;
  for (const auto& id : map_id_to_room)
    if (id.second.size() > 1)
    {
      std::cerr << " * " << id.first << " in [";
      for (const std::string& room : id.second)
        std::cerr << " " << room;
      std::cerr << " ]" << std::endl;
      okay = false;
    }
  if (okay)
    std::cerr << " -> All IDs are unique, all good." << std::endl;

  return EXIT_SUCCESS;
}
