#ifndef SOSAGE_SYSTEM_INPUT_H
#define SOSAGE_SYSTEM_INPUT_H

#include <Sosage/Content.h>
#include <Sosage/Utils/thread.h>
#include <Sosage/third_party_config.h>

namespace Sosage::System
{

class Input : public Threadable
{
  Content& m_content;
  Input_core m_core;

public:

  Input (Content& content);

  void main ();
  
};

} // namespace Sosage::System

#endif // SOSAGE_SYSTEM_INPUT_H
