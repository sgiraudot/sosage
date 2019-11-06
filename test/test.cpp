#include <Sosage/Engine.h>

int main (int argc, char** argv)
{
  Sosage::Engine sosage ("Sosage Test Game");

  if (argc > 1)
    sosage.read_file (argv[1]);
  else
    std::cerr << "No file provided" << std::endl;
  
  return EXIT_SUCCESS;
}
