/*
  [include/Sosage/Component/Locale.h]
  Handle translations of text.

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

#ifndef SOSAGE_COMPONENT_LOCALE_H
#define SOSAGE_COMPONENT_LOCALE_H

#include <Sosage/Component/Base.h>

#include <unordered_map>

namespace Sosage::Component
{

class Locale : public Base
{
  using Dictionary = std::unordered_map<std::string, std::string>;
  Dictionary m_dict;

public:

  Locale (const std::string& entity, const std::string& component);
  void add (const std::string& line, const std::string& translation);
  const std::string& get (const std::string& line);

  STR_NAME("Locale");
};

using Locale_handle = std::shared_ptr<Locale>;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_LOCALE_H
