#ifndef SOSAGE_SYSTEM_LOGIC_H
#define SOSAGE_SYSTEM_LOGIC_H

#include <Sosage/Content.h>
#include <Sosage/Utils/thread.h>

namespace Sosage::System
{

class Logic : public Threadable
{
private:

  Content& m_content;

public:

  Logic (Content& content);

  void main();
  
};

} // namespace Sosage::System

#endif // SOSAGE_SYSTEM_LOGIC_H
