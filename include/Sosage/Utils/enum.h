/*
  [include/Sosage/Utils/enum.h]
  All enums used by Sosage.

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

#ifndef SOSAGE_UTILS_ENUM_H
#define SOSAGE_UTILS_ENUM_H

namespace Sosage
{

enum Status
{
  IDLE,
  PAUSED,
  LOCKED,
  DIALOG_CHOICE,
  IN_MENU,
  IN_WINDOW,
  IN_CODE
};


enum Collision_type
{
  PIXEL_PERFECT,
  BOX,
  UNCLICKABLE
};

enum Event_type
{
  EMPTY,

  WINDOW,

  CURSOR_DOWN,
  CURSOR_MOVE,
  CURSOR_UP,

  KEY_DOWN,
  KEY_UP
};

enum Event_value
{
  NONE,

  // Window
  QUIT,
  RESIZED,

  // Cursor
  LEFT,
  RIGHT,

  // Keys
  ALT,
  ANDROID_BACK,
  ENTER,
  EXIT,
  SPACE,

  A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z,
  F1,F2,F3,F4,F5,F6,F7,F8,F9,F10,F11,F12
};

enum Fast_access_component
{
  CAMERA__POSITION,
  CURSOR__POSITION,
  CLOCK__TIME,
  GAME__DEBUG,
  GAME__STATUS,
  LOADING_SPIN__IMAGE,
  LOADING_SPIN__POSITION,

  NUMBER_OF_KEYS
};


}

#endif //  SOSAGE_UTILS_ENUM_H
