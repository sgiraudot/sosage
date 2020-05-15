#include <Sosage/Config/version.h>
#include <Sosage/Engine.h>

int main (int argc, char** argv)
{
  std::cerr << "Running Sosage " << Sosage::Version::str() << std::endl;
  
  Sosage::Engine sosage ("Superflu et le Mystère du Voleur de Pommes");

#ifdef SUPERFLU_INSTALL_DATA_FOLDER
  try
  {
    // TODO: fix memory leak of SDL_GetBasePath
    return sosage.run(std::string(SDL_GetBasePath()) + SUPERFLU_INSTALL_DATA_FOLDER);
  }
  catch(Sosage::Invalid_data_folder&)
  {
    return sosage.run(SUPERFLU_DATA_FOLDER);
  }
#endif
  
  return sosage.run(SUPERFLU_DATA_FOLDER);
}

