#include <Sosage/Utils/Asset_manager.h>
#include <Sosage/Utils/error.h>
#include <Sosage/Engine.h>

int main (int argc, char** argv)
{
  Sosage::Engine sosage(argc, argv);

#ifdef SOSAGE_INSTALL_DATA_FOLDER
  if (sosage.run(Sosage::Third_party::SDL_file::base_path() + SOSAGE_INSTALL_DATA_FOLDER))
  {
    std::cerr << "Okay" << std::endl;
    return EXIT_SUCCESS;
  }
#endif
  std::cerr << "Not okay" << std::endl;
  return (sosage.run(SOSAGE_DATA_FOLDER) ? EXIT_SUCCESS : EXIT_FAILURE);
}
