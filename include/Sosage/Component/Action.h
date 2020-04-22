#ifndef SOSAGE_COMPONENT_ACTION_H
#define SOSAGE_COMPONENT_ACTION_H

#include <Sosage/Component/Handle.h>

#include <vector>

namespace Sosage::Component
{

class Action : public Base
{
public:
  
  class Step
  {
  private:

    std::vector<std::string> m_content;

  public:

    Step (const std::vector<std::string>& content)
      : m_content (content)
    { }

    std::size_t size() const { return m_content.size(); }
    const std::string& get (const std::size_t& i) const { return m_content[i]; }
    int get_int (const std::size_t& i) const { return std::atoi(m_content[i].c_str()); }
    double get_double (const std::size_t& i) const { return std::atof(m_content[i].c_str()); }

  };

private:

  std::vector<Step> m_steps;
  
public:

  Action (const std::string& id);
  void add (const std::vector<std::string>& content);
  std::string str() const;

  std::size_t size() const { return m_steps.size(); }
  std::vector<Step>::const_iterator begin() const { return m_steps.begin(); }
  std::vector<Step>::const_iterator end() const { return m_steps.end(); }
  const Step& operator[] (const std::size_t& idx) const { return m_steps[idx]; }

};

typedef std::shared_ptr<Action> Action_handle;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_ACTION_H