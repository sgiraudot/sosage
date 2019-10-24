#ifndef SOSAGE_MODELS_INPUT_CORE_H
#define SOSAGE_MODELS_INPUT_CORE_H

namespace Sosage::Models
{

class Input_core
{
public:

  typedef int Event;
  
private:
  
public:

  Input_core()
  {
    debug ("Input_core ();");
  }

  ~Input_core()
  {
    debug ("~Input_core ();");
  }

  Event next_event()
  {
    debug ("Input_core::next_event ();");
    return 0;
  }

  bool is_exit (const Event&)
  {
    debug ("Input_core::is_exit (const Event&);");
    return true;
  }

  bool is_left_click (const Event&)
  {
    debug ("Input_core::is_left_click (const Event&);");
    return true;
  }

  std::pair<int, int> click_target (const Event&)
  {
    debug ("Input_core::click_target (const Event&);");
    return std::make_pair(0,0);
  }

};

} // namespace Sosage::Models

#endif // SOSAGE_MODELS_INPUT_CORE_H
