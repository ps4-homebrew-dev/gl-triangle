#   Copyright (C) 2026 John Törnblom
#
# This file is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING. If not see
# <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.16)

project(gl-triangle C)

find_package(PkgConfig REQUIRED)

pkg_check_modules(EGL REQUIRED IMPORTED_TARGET egl)
pkg_check_modules(GLESV2 REQUIRED IMPORTED_TARGET glesv2)

add_executable(gl-triangle main.c)

target_compile_options(gl-triangle PRIVATE -Wall -Werror)

target_link_libraries(gl-triangle PRIVATE
    PkgConfig::EGL
    PkgConfig::GLESV2
)

if(ORBIS)
    orbis_add_pkg(gl-triangle
        TITLE "OpenGL Triangle"
        TITLE_ID BREW95439
        CONTENT_ID IV0000-BREW95439_00-GLTRIANGLE000000
        ASSETS sce_sys
        MODULES libEGL.so.1 libGLESv2.so.2 libgallium-${EGL_VERSION}.so libvulkan.so.1
    )
endif()
