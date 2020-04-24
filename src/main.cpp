#include <Sosage/Engine.h>

int main (int argc, char** argv)
{
  Sosage::Engine sosage ("Superflu et le Mystère du Voleur de Pommes");

  try
  {
    return sosage.run(SUPERFLU_DATA_FOLDER);
  }
  catch(Sosage::Invalid_data_folder&) // todo: more precise exception
  {
#ifdef SUPERFLU_INSTALL_DATA_FOLDER
    try
    {
      return sosage.run(SUPERFLU_INSTALL_DATA_FOLDER);
    }
    catch(Sosage::Invalid_data_folder&)
    {

    }
#endif
  }
  
  return EXIT_FAILURE;
}

