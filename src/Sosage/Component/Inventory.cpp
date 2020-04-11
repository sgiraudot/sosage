#include <Sosage/Component/Inventory.h>
#include <Sosage/Config.h>

#include <iostream>

namespace Sosage::Component
{

Inventory::Inventory (const std::string& id)
  : Base (id), m_position(0)
{ }

void Inventory::add (const std::string& entity)
{
  m_data.push_back (entity);
}

void Inventory::remove (const std::string& entity)
{
  for (typename std::vector<std::string>::iterator
         it = m_data.begin(); it != m_data.end(); ++ it)
    if (*it == entity)
    {
      m_data.erase(it);
      break;
    }
}

void Inventory::next()
{
  if (m_position < m_data.size() - Sosage::displayed_inventory_size)
    ++ m_position;
}

void Inventory::prev()
{
  if (m_position != 0)
    -- m_position;
}

std::size_t Inventory::size() const
{
  return m_data.size();
}

std::size_t Inventory::position() const
{
  return m_position;
}
  

std::string Inventory::get (std::size_t i) const
{
  return m_data[i];
}


} // namespace Sosage::Component
