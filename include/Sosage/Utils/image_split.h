/*
    [include/Sosage/Utils/image_split.h]
    Handle splitting and recomposing images.

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

#ifndef SOSAGE_UTILS_IMAGE_SPLIT_H
#define SOSAGE_UTILS_IMAGE_SPLIT_H

#include <SDL.h>

#include <vector>
#include <iostream>

namespace Sosage::Splitter
{

constexpr Uint32 max_length = 1024;
Uint32 nb_sub (const Uint32& length);
SDL_Rect rect (const Uint32& width, const Uint32& height,
               const Uint32& x, const Uint32& y);
std::vector<SDL_Surface*> split_image (SDL_Surface* image);

} // namespace Sosage::Splitter

#endif // SOSAGE_UTILS_IMAGE_SPLIT_H
