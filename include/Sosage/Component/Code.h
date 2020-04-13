#ifndef SOSAGE_COMPONENT_CODE_H
#define SOSAGE_COMPONENT_CODE_H

#include <Sosage/Component/Handle.h>

#include <vector>

namespace Sosage::Component
{

class Code : public Base
{

  struct Button
  {
    std::string value;
    int x;
    int y;
    int w;
    int l;

    Button (const std::string value, int x, int y, int w, int l)
      : value (value), x(x), y(y), w(w), l(l)
    { }
  };

  std::vector<Button> m_buttons;
  std::vector<std::string> m_answer;
  std::vector<std::string> m_proposal;
  
public:

  Code (const std::string& id);

  void add_button (const std::string& value, int x, int y, int w, int l);
  void add_answer_item (const std::string& value);
  bool click (int x, int y);
  void reset();
  bool failure();
  bool success();
  
  
};

typedef std::shared_ptr<Code> Code_handle;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_INVENTORY_H
