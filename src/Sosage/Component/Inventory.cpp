#include <Sosage/Component/Inventory.h>
#include <Sosage/Config.h>

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
  if (m_position < m_data.size() - config().displayed_inventory_size)
    ++ m_position;
}

void Inventory::prev()
{
  if (m_position != 0)
    -- m_position;
}

std::string Inventory::get (std::size_t i) const
{
  if (i + m_position >= m_data.size() ||
      i < m_position || 
      i - m_position >= config().displayed_inventory_size)
    return "";
  return m_data[i + m_position];
}


} // namespace Sosage::Component
