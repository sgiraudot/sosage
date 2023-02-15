/*
  [include/Sosage/Utils/helpers.h]
  Useful macros.

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

#ifndef SOSAGE_UTILS_HELPERS_H
#define SOSAGE_UTILS_HELPERS_H

#define is_looking_right(x) \
  (get<C::Animation>(x + "_head", "image")->frames().front().y == 0)

#define is_walking_right(x) \
  (get<C::Animation>(x + "_body", "image")->frames().front().y != 2)

#define is_character(id) \
  request<C::String>((id), "color")

#define get_ground_map(id) \
  (value<C::Boolean>(id, "uses_2nd_map", false) ? \
  request<C::Ground_map>("background", "2nd_ground_map") : \
  request<C::Ground_map>("background", "ground_map"))

#endif // SOSAGE_UTILS_HELPERS_H
