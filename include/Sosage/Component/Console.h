#ifndef SOSAGE_COMPONENT_CONSOLE_H
#define SOSAGE_COMPONENT_CONSOLE_H

#include <Sosage/Component/Condition.h>
#include <Sosage/Utils/time.h>

#include <sstream>

#define SOSAGE_USE_STDCERR
#if defined(SOSAGE_USE_STDCERR)
#  define DBG_CERR std::cerr
#else
#  include <Sosage/Component/Console.h>
#  define DBG_CERR m_content.get<Component::Console>("game:console")->content()
#endif


namespace Sosage::Component
{

class Console : public Boolean
{
private:

  std::stringstream m_content;
  
public:

  Console (const std::string& id);
  virtual ~Console() { }
  std::string console_str();
  
  std::stringstream& content() { return m_content; }
};

typedef std::shared_ptr<Console> Console_handle;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_CONSOLE_H
