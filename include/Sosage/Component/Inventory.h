#ifndef SOSAGE_COMPONENT_INVENTORY_H
#define SOSAGE_COMPONENT_INVENTORY_H

#include <Sosage/Component/Handle.h>

#include <vector>

namespace Sosage::Component
{

class Inventory : public Base
{
  std::vector<std::string> m_data;
  std::size_t m_position;
  
public:

  Inventory (const std::string& id);

  void add (const std::string& entity);
  void remove (const std::string& entity);
  void next();
  void prev();

  std::size_t size() const;
  std::size_t position() const;
  std::string get (std::size_t i) const;
};

typedef std::shared_ptr<Inventory> Inventory_handle;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_INVENTORY_H
