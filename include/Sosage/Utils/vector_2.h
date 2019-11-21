#ifndef SOSAGE_UTILS_VECTOR_2_H
#define SOSAGE_UTILS_VECTOR_2_H

#include <Sosage/Utils/error.h>
#include <vector>

namespace Sosage
{

template <typename T>
class vector_2
{
  std::vector<T> m_data;
  std::size_t m_width;
  
public:

  vector_2 (std::size_t width = 0, std::size_t height = 0, const T& def = T())
    : m_data (width * height, def), m_width (width)
  { }

  std::size_t width() const { return m_width; }
  std::size_t height() const { return m_data.size() / m_width; }

  const T& operator() (std::size_t x, std::size_t y) const
  {
    dbg_check (x + m_width * y < m_data.size(), "Access to vector_2["
               + std::to_string(x) + ";" + std::to_string(y) + "] out of bounds ["
               + std::to_string(width()) + "; " + std::to_string(height()));
    return m_data[x + m_width * y];
  }
  T& operator() (std::size_t x, std::size_t y)
  {
    dbg_check (x + m_width * y < m_data.size(), "Access to vector_2["
               + std::to_string(x) + ";" + std::to_string(y) + "] out of bounds ["
               + std::to_string(width()) + "; " + std::to_string(height()));
    return m_data[x + m_width * y];
  }
};

} // namespace Sosage

#endif // SOSAGE_UTILS_VECTOR_2_H
