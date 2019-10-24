#ifndef SOSAGE_UTILS_VECTOR_2_H
#define SOSAGE_UTILS_VECTOR_2_H

#include <cassert>
#include <vector>

namespace Sosage
{

template <typename T>
class vector_2
{
  std::vector<std::vector<T> > m_data;

public:

  vector_2 (std::size_t width = 0, std::size_t height = 0, const T& def = T())
  {
    m_data.resize(width);
    for (std::size_t i = 0; i < width; ++ i)
      m_data[i].resize(height, def);
  }

  std::size_t width() const { return m_data.size(); }
  std::size_t height() const { return m_data.front().size(); }

  const T& operator() (std::size_t x, std::size_t y) const
  {
    assert (x < width() && y < height());
    return m_data[x][y];
  }
  T& operator() (std::size_t x, std::size_t y)
  {
    assert (x < width() && y < height());
    return m_data[x][y];
  }
#if 0
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
    assert (x + m_width * y < m_data.size());
    return m_data[x + m_width * y];
  }
  T& operator() (std::size_t x, std::size_t y)
  {
    assert (x + m_width * y < m_data.size());
    return m_data[x + m_width * y];
  }
#endif
};

} // namespace Sosage

#endif // SOSAGE_UTILS_VECTOR_2_H
