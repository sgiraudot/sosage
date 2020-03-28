#ifndef SOSAGE_COMPONENT_CAST_H
#define SOSAGE_COMPONENT_CAST_H

#include <Sosage/Component/Handle.h>
#include <Sosage/Component/Conditional.h>

#include <memory>

namespace Sosage::Component
{

template <typename T>
inline std::shared_ptr<T> cast (Handle h)
{
  std::shared_ptr<T> out = std::dynamic_pointer_cast<T>(h);
  if (out)
    return out;

  Conditional_base_handle cond = std::dynamic_pointer_cast<Conditional_base>(h);
  if (cond)
    return std::dynamic_pointer_cast<T>(cond->get());

  return std::shared_ptr<T>();
}


}

#endif // SOSAGE_COMPONENT_CAST_H
