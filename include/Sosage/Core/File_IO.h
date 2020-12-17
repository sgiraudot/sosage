/*
  [include/Sosage/Core/File_IO.h]
  Abstraction file for third party library handling file IO.

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

#ifndef SOSAGE_CORE_FILE_IO_H
#define SOSAGE_CORE_FILE_IO_H

#include <Sosage/Third_party/Yaml.h>

namespace Sosage::Core
{
using File_IO = Third_party::Yaml;
}

#endif

