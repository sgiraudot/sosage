#ifndef SOSAGE_COMPONENT_HANDLE_H
#define SOSAGE_COMPONENT_HANDLE_H

#include <Sosage/Utils/thread.h>

namespace Sosage::Component
{

class Free_base
{
public:
  Free_base() { }
  virtual ~Free_base() { }
};

typedef Lockable<Free_base> Base;

typedef std::shared_ptr<Base> Handle;

template <typename T>
inline std::shared_ptr<T> component_cast (Handle h)
{
  return std::dynamic_pointer_cast<T>(h);
}

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_HANDLE_H
