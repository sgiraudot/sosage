#include <Sosage/Utils/Asset_manager.h>
#include <Sosage/Utils/error.h>
#include <Sosage/Engine.h>

int main (int, char**)
{
  Sosage::Engine sosage;

#ifdef SOSAGE_INSTALL_DATA_FOLDER
  try
  {
    return sosage.run(Sosage::Third_party::SDL_file::base_path() + SOSAGE_INSTALL_DATA_FOLDER);
  }
  catch(Sosage::No_such_file&)
  {
    return sosage.run(SOSAGE_DATA_FOLDER);
  }
#endif

  return sosage.run(SOSAGE_DATA_FOLDER);
}
