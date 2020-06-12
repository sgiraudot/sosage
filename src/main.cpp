#include <Sosage/Utils/error.h>
#include <Sosage/Utils/file.h>
#include <Sosage/Engine.h>

int main (int argc, char** argv)
{
  Sosage::Engine sosage ("Superflu et le Mystère du Voleur de Pommes");

#ifdef SUPERFLU_INSTALL_DATA_FOLDER
  try
  {
    return sosage.run(Sosage::base_path() + SUPERFLU_INSTALL_DATA_FOLDER);
  }
  catch(Sosage::No_such_file&)
  {
    return sosage.run(SUPERFLU_DATA_FOLDER);
  }
#endif
  
  return sosage.run(SUPERFLU_DATA_FOLDER);
}

