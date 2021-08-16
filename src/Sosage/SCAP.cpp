#include <Sosage/Utils/asset_packager.h>

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
