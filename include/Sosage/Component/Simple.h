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

#include <Sosage/Component/Base.h>

#include <algorithm>
#include <array>
#include <tuple>
#include <unordered_set>
#include <vector>

namespace Sosage::Component
{

template <typename T>
std::string type_name (T*) { return typeid(T).name(); }
inline std::string type_name (int*) { return "int"; }
inline std::string type_name (double*) { return "double"; }
inline std::string type_name (std::string*) { return "std::string"; }

template <typename T>
class Simple : public Base
{
public:

  using const_reference = const T&;
  using value_type = T;

protected:

  T m_value;
  
public:

  Simple (const std::string& entity, const std::string& component, const T& value = T())
    : Base(entity, component), m_value(value) { }
  virtual ~Simple() { }
  virtual const T& value() const { return m_value; }
  void set (const T& value)
  {
    m_value = value;
    mark_as_altered();
  }

  STR_NAME("Simple<" + type_name((T*)(nullptr)) + ">");
};

template <typename T>
using Simple_handle = std::shared_ptr<Simple<T> >;

using Int = Simple<int>;
using Int_handle = std::shared_ptr<Int>;

using Double = Simple<double>;
using Double_handle = std::shared_ptr<Double>;

using String = Simple<std::string>;
using String_handle = std::shared_ptr<String>;

template <typename T, std::size_t S>
class Array : public Simple<std::array<T,S>>
{
  using Base = Simple<std::array<T,S>>;
public:

  template <typename ... T2>
  Array (const std::string& entity, const std::string& component, T2 ... t)
    : Base(entity, component, {t...}) { }

  T& operator[] (const std::size_t& idx)
  { return this->m_value[idx]; }

  const T& operator[] (const std::size_t& idx) const
  { return this->m_value[idx]; }

  STR_NAME("Array<" + type_name((T*)(nullptr)) + "," + std::to_string(S) + ">");
};

template <typename T, std::size_t S>
using Array_handle = std::shared_ptr<Array<T,S>>;

template <typename T>
class Vector : public Simple<std::vector<T> >
{
  using Base = Simple<std::vector<T> >;
public:

  Vector (const std::string& entity, const std::string& component, const std::vector<T>& value = std::vector<T>())
    : Base(entity, component, value) { }

  void push_back (const T& t) { this->m_value.push_back (t); }
  void remove (const T& t)
  {
    auto position = std::find(this->m_value.begin(), this->m_value.end(), t);
    if (position != this->m_value.end())
      this->m_value.erase(position);
  }
  STR_NAME("Vector<" + type_name((T*)(nullptr)) + ">");
};

template <typename T>
using Vector_handle = std::shared_ptr<Vector<T> >;


template <typename T>
class Set : public Simple<std::unordered_set<T> >
{
  using set_t = std::unordered_set<T>;
  using Base = Simple<set_t>;
public:

  Set(const std::string& entity, const std::string& component)
    : Base(entity, component) { }

  void insert (const T& t) { this->m_value.insert (t); }
  void erase (const T& t) { this->m_value.erase (t); }

  typename set_t::const_iterator begin() const { return this->m_value.begin(); }
  typename set_t::const_iterator end() const { return this->m_value.end(); }

  STR_NAME("Set<" + type_name((T*)(nullptr)) + ">");
};

template <typename T>
using Set_handle = std::shared_ptr<Set<T> >;

template <typename T1, typename T2>
class Pair : public Simple<std::pair<T1,T2>>
{
  using pair_t = std::pair<T1,T2>;
  using Base = Simple<pair_t>;

public:

  Pair (const std::string& entity, const std::string& component, const T1& t1, const T2& t2)
    : Base(entity, component, std::make_pair(t1, t2))
  { }

  T1& first() { return this->m_value.first(); }
  const T1& first() const { return this->m_value.first(); }
  T2& second() { return this->m_value.second(); }
  const T2& second() const { return this->m_value.second(); }

  STR_NAME("Pair<" + type_name((T1*)(nullptr)) + "," + type_name((T2*)(nullptr)) + ">");
};

template <typename T1, typename T2>
using Pair_handle = std::shared_ptr<Pair<T1,T2>>;

template <typename ... T>
class Tuple : public Simple<std::tuple<T...> >
{
  using tuple_t = std::tuple<T...>;
  using Base = Simple<tuple_t>;

  template <std::size_t I>
  using Element_type = typename std::tuple_element<I, tuple_t>::type;

public:

  Tuple (const std::string& entity, const std::string& component, T ... t)
    : Base(entity, component, std::make_tuple(t...))
  { }

  template <std::size_t I>
  Element_type<I>& get()
  {
    return std::get<I>(this->m_value);
  }

  template <std::size_t I>
  const Element_type<I>& get() const
  {
    return std::get<I>(this->m_value);
  }

  STR_NAME("Tuple<...>");
};

template <typename ... T>
using Tuple_handle = std::shared_ptr<Tuple<T...>>;


} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_SIMPLE_H

