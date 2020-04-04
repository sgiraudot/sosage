#ifndef SOSAGE_SYSTEM_INPUT_H
#define SOSAGE_SYSTEM_INPUT_H

#include <Sosage/Content.h>
#include <Sosage/Core/Input.h>

namespace Sosage::System
{

class Input
{
  Content& m_content;
  Core::Input m_core;

public:

  Input (Content& content);

  void run ();
  
};

} // namespace Sosage::System

#endif // SOSAGE_SYSTEM_INPUT_H
