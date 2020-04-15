#include <Sosage/Engine.h>

int main (int argc, char** argv)
{
  Sosage::Engine sosage ("Superflu et le Mystère du Voleur de Pommes");

#ifdef SUPERFLU_INSTALL_DATA_FOLDER
  try
  {
    return sosage.run(SUPERFLU_DATA_FOLDER);
  }
  catch(std::runtime_error&) // todo: more precise exception
  {
    return sosage.run(SUPERFLU_INSTALL_DATA_FOLDER);
  }
#endif
  
  return sosage.run(SUPERFLU_DATA_FOLDER);
}

