/*
  [include/Sosage/Component/Simple.h]
  Components holding one unique type of object.

  =====================================================================

  This file is part of SOSAGE.

  SOSAGE is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  SOSAGE is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with SOSAGE.  If not, see <https://www.gnu.org/licenses/>.

  =====================================================================

  Author(s): Simon Giraudot <sosage@ptilouk.net>
*/

#ifndef SOSAGE_COMPONENT_SIMPLE_H
#define SOSAGE_COMPONENT_SIMPLE_H

#include <Sosage/Component/Handle.h>

#include <string>
#include <unordered_set>
#include <vector>

namespace Sosage::Component
{

template <typename T>
class Simple : public Base
{
protected:

  T m_value;
  
public:

  Simple (const std::string& id, const T& value = T())
    : Base(id), m_value(value) { }
  virtual ~Simple() { }
  virtual const T& value() const { return m_value; }
  void set (const T& value) { m_value = value; }
};

template <typename T>
using Simple_handle = std::shared_ptr<Simple<T> >;

using Int = Simple<int>;
using Int_handle = std::shared_ptr<Int>;

using Double = Simple<double>;
using Double_handle = std::shared_ptr<Double>;

using String = Simple<std::string>;
using String_handle = std::shared_ptr<String>;

template <typename T>
class Vector : public Simple<std::vector<T> >
{
  using Base = Simple<std::vector<T> >;
public:

  Vector (const std::string& id, const std::vector<T>& value = std::vector<T>())
    : Base(id, value) { }

  void push_back (const T& t) { this->m_value.push_back (t); }
};

template <typename T>
using Vector_handle = std::shared_ptr<Vector<T> >;

template <typename T>
class Set : public Simple<std::unordered_set<T> >
{
  using set_t = std::unordered_set<T>;
  using Base = Simple<set_t>;
public:

  Set(const std::string& id)
    : Base(id) { }

  void insert (const T& t) { this->m_value.insert (t); }

  typename set_t::const_iterator begin() const { return this->m_value.begin(); }
  typename set_t::const_iterator end() const { return this->m_value.end(); }
};

template <typename T>
using Set_handle = std::shared_ptr<Set<T> >;


} // namespace Sosage::Component


#endif // SOSAGE_COMPONENT_SIMPLE_H

