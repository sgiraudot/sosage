#include <Sosage/Content.h>

namespace Sosage
{

Content::Content()
  : m_data ()
{

}

void Content::remove (const std::string& key)
{
  Component::Handle_set::iterator iter = m_data.find(std::make_shared<Component::Base>(key));
  check (iter != m_data.end(), "Entity " + key + " doesn't exist");
  m_data.erase(iter);
}


} // namespace Sosage
