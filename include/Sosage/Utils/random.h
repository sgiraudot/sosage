#ifndef SOSAGE_UTILS_RANDOM_H
#define SOSAGE_UTILS_RANDOM_H

namespace Sosage
{

inline int random_int (int min, int max)
{
  return min + (rand() % (max - min));
}

} // namespace Sosage

#endif // SOSAGE_UTILS_RANDOM_H
