#ifndef SOSAGE_SYSTEM_LOGIC_H
#define SOSAGE_SYSTEM_LOGIC_H

#include <Sosage/Component/Debug.h>
#include <Sosage/Component/Image.h>
#include <Sosage/Component/Position.h>
#include <Sosage/Component/Text.h>
#include <Sosage/Content.h>

namespace Sosage::System
{

class Logic
{
private:

  Content& m_content;

public:

  Logic (Content& content);

  void main();
  bool exit();
  bool paused();

private:
  void compute_path_from_target (Component::Position_handle target);
  void update_debug_info (Component::Debug_handle debug_info);
};

} // namespace Sosage::System

#endif // SOSAGE_SYSTEM_LOGIC_H
