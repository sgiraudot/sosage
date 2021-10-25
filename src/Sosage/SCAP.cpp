#ifdef SOSAGE_SCAP // avoid main() conflicts with Sosage

#include <Sosage/Utils/asset_packager.h>

std::string Sosage::Asset_manager::folder_name = "";
std::vector<Sosage::Buffer> Sosage::Asset_manager::buffers;
Sosage::Package_asset_map Sosage::Asset_manager::package_asset_map;

int main (int argc, char** argv)
{
  if (argc != 3)
  {
    std::cerr << "Usage: " << argv[0] << " [input_folder] [output_folder]" << std::endl;
    return EXIT_SUCCESS;
  }

  std::string root = argv[1];
  std::string out = argv[2];
    if (root.find(".data") != std::string::npos)
    Sosage::SCAP::decompile_package (root, out);
  else
    Sosage::SCAP::compile_package (root, out);

  return EXIT_SUCCESS;
}

#endif
