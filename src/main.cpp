#include <Sosage/Engine.h>

int main (int argc, char** argv)
{
  Sosage::Engine sosage ("Superflu et le Mystère du Voleur de Pommes");

  return sosage.run(SUPERFLU_DATA_FOLDER);
}

