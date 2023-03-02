/*
    [src/Sosage/Utils/image_split.cpp]
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

#include <Sosage/Utils/image_split.h>

#include <vector>
#include <iostream>

namespace Sosage::Splitter
{

Uint32 nb_sub (const Uint32& length)
{
  return 1 + (length / max_length);
}

SDL_Rect rect (const Uint32& width, const Uint32& height,
               const Uint32& x, const Uint32& y)
{
  Uint32 nb_x = nb_sub(width);
  Uint32 nb_y = nb_sub(height);
  Uint32 w = 1 + width / nb_x;
  Uint32 h = 1 + height / nb_y;

  SDL_Rect out;
  out.x = w * x;
  out.y = h * y;
  out.w = ((x == nb_x - 1) ? (width - out.x) : w);
  out.h = ((y == nb_y - 1) ? (height - out.y) : h);
  return out;
}

std::vector<SDL_Surface*> split_image (SDL_Surface* image)
{
  SDL_SetSurfaceBlendMode(image, SDL_BLENDMODE_NONE);
  std::vector<SDL_Surface*> out;
  for (Uint32 x = 0; x < nb_sub(image->w); ++ x)
  {
    for (Uint32 y = 0; y < nb_sub(image->h); ++ y)
    {
      SDL_Rect source = rect (image->w, image->h, x, y);
      SDL_Rect target;
      target.x = 0;
      target.y = 0;
      target.w = source.w;
      target.h = source.h;

      SDL_Surface* tile = SDL_CreateRGBSurfaceWithFormat
        (image->flags, source.w, source.h, 32, image->format->format);

      SDL_BlitSurface (image, &source, tile, &target);

      out.push_back(tile);
    }
  }
  return out;
}


} // namespace Sosage::Splitter
