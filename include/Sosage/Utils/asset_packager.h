/*
    [include/Sosage/Utils/asset_packager.h]
    Package (and depackage) assets.

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

#ifndef SOSAGE_UTILS_ASSET_PACKAGER_H
#define SOSAGE_UTILS_ASSET_PACKAGER_H

#include <Sosage/Utils/Asset_manager.h>

#include <memory>

namespace Sosage::SCAP
{

constexpr unsigned int surface_format = SDL_PIXELFORMAT_ABGR8888;

std::string package (const std::string& filename);

using Output_file = std::shared_ptr<std::ofstream>;
using Package_files = std::unordered_map<std::string, Output_file>;

Package_files open_packages (const std::string& root);
void write_file (std::ofstream& ofile, const std::string& filename, bool compressed);
void write_image (std::ofstream& ofile, const std::string& filename);
void compile_package (const std::string& input_folder, const std::string& output_folder);
void decompile_package (const std::string& filename, const std::string& folder);

} // namespace Sosage::SCAP

#endif // SOSAGE_UTILS_ASSET_PACKAGER_H
