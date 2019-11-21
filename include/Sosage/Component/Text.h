#ifndef SOSAGE_COMPONENT_TEXT_H
#define SOSAGE_COMPONENT_TEXT_H

#include <Sosage/Component/Handle.h>

namespace Sosage::Component
{

class Text : public Base
{
private:
  std::string m_value;
  
public:

  Text (const std::string& id, const std::string& value);
  const std::string& value() const { return m_value; }
};

typedef std::shared_ptr<Text> Text_handle;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_TEXT_H
