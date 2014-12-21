############################################################################################################
#                                                                                                          #
#    This file is part of Minor Planet Mayhem                                                              #
#                                                                                                          #
#    Copyright (c) 2014 Shachar Avni. All rights reserved.                                                 #
#                                                                                                          #
#    Use of this file is governed by a BSD-style license. See the accompanying LICENSE.txt for details     #
#                                                                                                          #
############################################################################################################

project(MPM)

cmake_minimum_required(VERSION 2.8)

set(LOCUS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Locus-Game-Engine")

include(${LOCUS_DIR}/cmake/GlobalProjectOptions.cmake)

include(${LOCUS_DIR}/cmake/Config.cmake)

add_subdirectory(${LOCUS_DIR} "${CMAKE_CURRENT_BINARY_DIR}/Locus")
add_subdirectory(src)

add_custom_target(MPM_Copy_Target_Files)

get_target_property(MPMTargetLocation MPM LOCATION)
get_filename_component(MPMTargetDir ${MPMTargetLocation} PATH)

set(Make_Copy_PDB_Files_Target OFF)

if(WINDOWS)
	if(ARCH_32BITS)
		set(OPENAL_DLL ${LOCUS_DIR}/third-party/OpenAL/bin/x86/openal32.dll)
	else()
		set(OPENAL_DLL ${LOCUS_DIR}/third-party/OpenAL/bin/x64/openal32.dll)
	endif()

	add_custom_command(TARGET MPM_Copy_Target_Files POST_BUILD
						COMMAND ${CMAKE_COMMAND} -E
						copy ${OPENAL_DLL} ${MPMTargetDir}/openal32.dll)

	if(BUILD_SHARED_LIBS)
		add_custom_target(MPM_Copy_PDB_Files)

		set(Make_Copy_PDB_Files_Target ON)

		list(APPEND DLL_ORIGIN_PATHS "${PROJECT_BINARY_DIR}/Locus/third-party/FreeType/$<CONFIGURATION>")
		list(APPEND DLL_NAMES "freetype")

		list(APPEND DLL_ORIGIN_PATHS "${PROJECT_BINARY_DIR}/Locus/third-party/GLEW/$<CONFIGURATION>")
		list(APPEND DLL_NAMES "glew")

		list(APPEND DLL_ORIGIN_PATHS "${PROJECT_BINARY_DIR}/Locus/third-party/GLFW/src/$<CONFIGURATION>")
		list(APPEND DLL_NAMES "glfw3")

		list(APPEND DLL_ORIGIN_PATHS "${PROJECT_BINARY_DIR}/Locus/third-party/OGG/$<CONFIGURATION>")
		list(APPEND DLL_NAMES "ogg")

		list(APPEND DLL_ORIGIN_PATHS "${PROJECT_BINARY_DIR}/Locus/third-party/PHYSFS/$<CONFIGURATION>")
		list(APPEND DLL_NAMES "physfs")

		list(APPEND DLL_ORIGIN_PATHS "${PROJECT_BINARY_DIR}/Locus/third-party/stb_image/src/$<CONFIGURATION>")
		list(APPEND DLL_NAMES "stb_image")

		list(APPEND DLL_ORIGIN_PATHS "${PROJECT_BINARY_DIR}/Locus/third-party/Vorbis/$<CONFIGURATION>")
		list(APPEND DLL_NAMES "vorbis")

		list(APPEND DLL_ORIGIN_PATHS "${PROJECT_BINARY_DIR}/Locus/third-party/Vorbis/$<CONFIGURATION>")
		list(APPEND DLL_NAMES "vorbisfile")

		list(APPEND DLL_ORIGIN_PATHS "${PROJECT_BINARY_DIR}/Locus/src/Audio/$<CONFIGURATION>")
		list(APPEND DLL_NAMES "Locus_Audio")

		list(APPEND DLL_ORIGIN_PATHS "${PROJECT_BINARY_DIR}/Locus/src/Common/$<CONFIGURATION>")
		list(APPEND DLL_NAMES "Locus_Common")

		list(APPEND DLL_ORIGIN_PATHS "${PROJECT_BINARY_DIR}/Locus/src/FileSystem/$<CONFIGURATION>")
		list(APPEND DLL_NAMES "Locus_FileSystem")

		list(APPEND DLL_ORIGIN_PATHS "${PROJECT_BINARY_DIR}/Locus/src/Geometry/$<CONFIGURATION>")
		list(APPEND DLL_NAMES "Locus_Geometry")

		list(APPEND DLL_ORIGIN_PATHS "${PROJECT_BINARY_DIR}/Locus/src/Math/$<CONFIGURATION>")
		list(APPEND DLL_NAMES "Locus_Math")

		list(APPEND DLL_ORIGIN_PATHS "${PROJECT_BINARY_DIR}/Locus/src/Rendering/$<CONFIGURATION>")
		list(APPEND DLL_NAMES "Locus_Rendering")

		list(APPEND DLL_ORIGIN_PATHS "${PROJECT_BINARY_DIR}/Locus/src/Simulation/$<CONFIGURATION>")
		list(APPEND DLL_NAMES "Locus_Simulation")

      list(APPEND DLL_ORIGIN_PATHS "${PROJECT_BINARY_DIR}/Locus/src/XML/$<CONFIGURATION>")
		list(APPEND DLL_NAMES "Locus_XML")

		list(LENGTH DLL_ORIGIN_PATHS NUM_DLLS)
		math(EXPR NUM_DLLS "${NUM_DLLS}-1")
		foreach(i RANGE ${NUM_DLLS})
			list(GET DLL_ORIGIN_PATHS ${i} DLL_PATH)
			list(GET DLL_NAMES ${i} DLL_NAME)

			add_custom_command(TARGET MPM_Copy_Target_Files POST_BUILD
								COMMAND ${CMAKE_COMMAND} -E copy_if_different
								"${DLL_PATH}/${DLL_NAME}.dll"
								"${MPMTargetDir}/${DLL_NAME}.dll")

			add_custom_command(TARGET MPM_Copy_PDB_Files POST_BUILD
								COMMAND ${CMAKE_COMMAND} -E copy_if_different
								"${DLL_PATH}/${DLL_NAME}.pdb"
								"${MPMTargetDir}/${DLL_NAME}.pdb")
		endforeach()
	endif()
endif()

if (MPM_USE_ARCHIVED_RESOURCES)
	add_custom_command(TARGET MPM_Copy_Target_Files POST_BUILD
						COMMAND ${CMAKE_COMMAND} -E
						copy "${PROJECT_SOURCE_DIR}/resources.zip" "${MPMTargetDir}/resources.zip")
else()
	add_custom_command(TARGET MPM_Copy_Target_Files POST_BUILD
						COMMAND ${CMAKE_COMMAND} -E
						copy_directory "${PROJECT_SOURCE_DIR}/resources" "${MPMTargetDir}/resources")
endif()

if(BUILD_SHARED_LIBS)
	set(PHYSFS_LIB_NAME physfs)
else()
    set(PHYSFS_LIB_NAME physfs-static)
endif()

set(MPM_ALL_DEPENDENCIES
    freetype
    glew
    glfw
    ogg
    ${PHYSFS_LIB_NAME}
    stb_image
    vorbis
    vorbisfile
    Locus_Common
    Locus_Audio
    Locus_FileSystem
    Locus_Math
    Locus_Geometry
    Locus_Rendering
    Locus_Simulation
    Locus_XML)

foreach(DEPENDENCY ${MPM_ALL_DEPENDENCIES})
	add_dependencies(MPM_Copy_Target_Files ${DEPENDENCY})
endforeach()

add_dependencies(MPM MPM_Copy_Target_Files)

if(${Make_Copy_PDB_Files_Target})
	add_dependencies(MPM_Copy_PDB_Files MPM)
endif()