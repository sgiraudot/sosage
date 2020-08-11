#include <Sosage/Utils/error.h>
#include <Sosage/Utils/file.h>
#include <Sosage/Engine.h>

int main (int argc, char** argv)
{
  Sosage::Engine sosage;

#ifdef SOSAGE_INSTALL_DATA_FOLDER
  try
  {
    return sosage.run(Sosage::base_path() + SOSAGE_INSTALL_DATA_FOLDER);
  }
  catch(Sosage::No_such_file&)
  {
    return sosage.run(SOSAGE_DATA_FOLDER);
  }
#endif

  return sosage.run(SOSAGE_DATA_FOLDER);
}
