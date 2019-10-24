#ifndef SOSAGE_SYSTEM_GRAPHIC_H
#define SOSAGE_SYSTEM_GRAPHIC_H

#include <Sosage/Content.h>
#include <Sosage/third_party_config.h>
#include <Sosage/Utils/thread.h>

namespace Sosage::System
{

class Graphic : public Threadable
{
private:

  Content& m_content;
  Graphic_core m_core;

public:

  Graphic (Content& content, const std::string& name);

  void main();
};

} // namespace Sosage::System

#endif // SOSAGE_SYSTEM_GRAPHIC_H
