#  CMake macros adapted from those written for Marlin, released under GPLv3:
#  https://github.com/ammonkey/marlin/blob/master/cmake/GSettings.cmake
#
#  Copyright (C) 2015-2022 Savoir-faire Linux Inc.
#  Author: Stepan Salenikovich <stepan.salenikovich@savoirfairelinux.com>
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.
#

option (GSETTINGS_LOCALCOMPILE "Compile GSettings schemas locally during build to the location of the binary (no need to run 'make install')" ON)

option (GSETTINGS_PREFIXINSTALL "Install GSettings Schemas relative to the location specified by the install prefix (instead of relative to where GLib is installed)" ON)

option (GSETTINGS_COMPILE "Compile GSettings Schemas after installation" ${GSETTINGS_LOCALCOMPILE})

if (GSETTINGS_LOCALCOMPILE)
    message(STATUS "GSettings schemas will be compiled to the build directory during the build.")
endif ()

if (GSETTINGS_PREFIXINSTALL)
    message (STATUS "GSettings schemas will be installed relative to the cmake install prefix.")
else ()
    message (STATUS "GSettings schemas will be installed relative to the GLib install location.")
endif ()

if (GSETTINGS_COMPILE)
    message (STATUS "GSettings shemas will be compiled after install.")
endif ()

macro (add_schema SCHEMA_NAME OUTPUT)

    set (PKG_CONFIG_EXECUTABLE pkg-config)

    if (GSETTINGS_PREFIXINSTALL)
        set (GSETTINGS_DIR "${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas/")
    else (GSETTINGS_PREFIXINSTALL)
        execute_process (COMMAND ${PKG_CONFIG_EXECUTABLE} glib-2.0 --variable prefix OUTPUT_VARIABLE _glib_prefix OUTPUT_STRIP_TRAILING_WHITESPACE)
        set (GSETTINGS_DIR "${_glib_prefix}/share/glib-2.0/schemas/")
    endif (GSETTINGS_PREFIXINSTALL)

    # Validate the schema
    execute_process (COMMAND ${PKG_CONFIG_EXECUTABLE} gio-2.0 --variable glib_compile_schemas  OUTPUT_VARIABLE _glib_comple_schemas OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process (COMMAND ${_glib_comple_schemas} --dry-run --schema-file=${CMAKE_CURRENT_SOURCE_DIR}/data/${SCHEMA_NAME} ERROR_VARIABLE _schemas_invalid OUTPUT_STRIP_TRAILING_WHITESPACE)

    if (_schemas_invalid)
      message (SEND_ERROR "Schema validation error: ${_schemas_invalid}")
    endif (_schemas_invalid)

    if (GSETTINGS_LOCALCOMPILE)
        # compile locally during build to not force the user to 'make install'
        # when running from the build dir
        add_custom_command(
            OUTPUT "${PROJECT_BINARY_DIR}/gschemas.compiled"
            WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/data"
            COMMAND
                "${_glib_comple_schemas}"
            ARGS
                "${PROJECT_SOURCE_DIR}/data"
                "--targetdir=${PROJECT_BINARY_DIR}"
            DEPENDS
                "${PROJECT_SOURCE_DIR}/data/${SCHEMA_NAME}"
            VERBATIM
        )

        set(${OUTPUT} "${PROJECT_BINARY_DIR}/gschemas.compiled")
    endif (GSETTINGS_LOCALCOMPILE)

    # Actually install and recompile schemas
    install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/${SCHEMA_NAME} DESTINATION ${GSETTINGS_DIR} OPTIONAL)

    if (GSETTINGS_COMPILE)
        install (CODE "message (STATUS \"Compiling GSettings schemas\")")
        install (CODE "execute_process (COMMAND ${_glib_comple_schemas} ${GSETTINGS_DIR})")
    endif ()
endmacro()
