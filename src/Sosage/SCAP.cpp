#ifdef SOSAGE_SCAP // avoid main() conflicts with Sosage

#include <Sosage/Utils/asset_packager.h>

std::string Sosage::Asset_manager::folder_name = "";
std::vector<Sosage::Buffer> Sosage::Asset_manager::buffers;
Sosage::Package_asset_map Sosage::Asset_manager::package_asset_map;

int main (int argc, char** argv)
{
  if (argc != 2)
  {
    std::cerr << "Usage: " << argv[0] << " [root]" << std::endl;
    return EXIT_SUCCESS;
  }

  std::string root = argv[1];
  if (root.find(".data") != std::string::npos)
    Sosage::SCAP::decompile_package (root);
  else
    Sosage::SCAP::compile_package (root);

  return EXIT_SUCCESS;
}

#endif
