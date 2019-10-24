#include <Sosage/Engine.h>

int main (int argc, char** argv)
{
  Sosage::Engine& sosage = Sosage::engine ("Sosage Test Game");

  if (argc > 1)
    sosage.run_file (argv[1]);
  else
  {
    sosage.set_image("background", "dummy.png", 0, 0, 0);
    sosage.main();
  }

  return EXIT_SUCCESS;
}
