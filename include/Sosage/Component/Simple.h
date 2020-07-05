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

#include <string>
#include <vector>

namespace Sosage::Component
{

template <typename T>
class Simple : public Value<T>
{
  typedef Value<T> Base;
  
  T m_value;
  
public:

  Simple (const std::string& id, const T& value = T())
    : Base(id), m_value(value) { }
  virtual ~Simple() { }
  virtual T value() const { return m_value; }
  void set (const T& value) { m_value = value; }
};

template <typename T>
using Simple_handle = std::shared_ptr<Simple<T> >;

typedef Simple<int> Int;
typedef std::shared_ptr<Int> Int_handle;

typedef Simple<double> Double;
typedef std::shared_ptr<Double> Double_handle;

typedef Simple<std::string> String;
typedef std::shared_ptr<String> String_handle;

template <typename T>
using Vector = Simple<std::vector<T> >;
template <typename T>
using Vector_handle = std::shared_ptr<Vector<T> >;

} // namespace Sosage::Component


#endif // SOSAGE_COMPONENT_SIMPLE_H

