cmake_minimum_required(VERSION 3.14)

set(CMAKE_SYSTEM_NAME WindowsStore CACHE STRING "Target UWP")
set(CMAKE_SYSTEM_VERSION 10.0.19041.0 CACHE STRING "Windows SDK version")

project(sonic3air-uwp LANGUAGES C CXX)

if(CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Supported build types" FORCE)
endif()

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

function(sonic3air_pick_existing_tree out_var marker_path)
	set(_picked "")
	foreach(_candidate IN LISTS ARGN)
		if(_candidate AND EXISTS "${_candidate}/${marker_path}")
			set(_picked "${_candidate}")
			break()
		endif()
	endforeach()
	set(${out_var} "${_picked}" PARENT_SCOPE)
endfunction()

sonic3air_pick_existing_tree(SONIC3AIR_UWP_DEPS_ROOT_DEFAULT "x64/lib/SDL2.lib"
	"${CMAKE_CURRENT_LIST_DIR}/third_party/uwp-deps"
)

set(SONIC3AIR_UWP_DEPS_ROOT "${SONIC3AIR_UWP_DEPS_ROOT_DEFAULT}" CACHE PATH "Root of the local UWP SDL and Mesa dependency bundle")

set(SONIC3AIR_UWP_X64_DIR "${SONIC3AIR_UWP_DEPS_ROOT}/x64")
set(SONIC3AIR_UWP_BIN_DIR "${SONIC3AIR_UWP_X64_DIR}/bin")
set(SONIC3AIR_UWP_LIB_DIR "${SONIC3AIR_UWP_X64_DIR}/lib")
set(SONIC3AIR_UWP_INCLUDE_DIR "${SONIC3AIR_UWP_X64_DIR}/include")
set(SONIC3AIR_UWP_GLAD_DIR "${CMAKE_CURRENT_LIST_DIR}/third_party/glad-compat")
set(SONIC3AIR_UWP_GLAD_INCLUDE_DIR "${SONIC3AIR_UWP_GLAD_DIR}/include")
set(SONIC3AIR_UWP_GLAD_SOURCE "${SONIC3AIR_UWP_GLAD_DIR}/src/gl.c")

if(NOT SONIC3AIR_UWP_DEPS_ROOT OR NOT EXISTS "${SONIC3AIR_UWP_DEPS_ROOT}")
	message(FATAL_ERROR
		"SONIC3AIR_UWP_DEPS_ROOT was not found. Put the UWP dependency bundle under '${CMAKE_CURRENT_LIST_DIR}/third_party/uwp-deps'."
	)
endif()

if(NOT EXISTS "${SONIC3AIR_UWP_LIB_DIR}/SDL2.lib")
	message(FATAL_ERROR "SDL2.lib was not found under ${SONIC3AIR_UWP_LIB_DIR}")
endif()

if(NOT EXISTS "${SONIC3AIR_UWP_GLAD_SOURCE}")
	message(FATAL_ERROR
		"Compatibility GLAD source was not found under '${SONIC3AIR_UWP_GLAD_DIR}'."
	)
endif()

foreach(required_runtime_file IN ITEMS
	"${SONIC3AIR_UWP_BIN_DIR}/SDL2.dll"
	"${SONIC3AIR_UWP_BIN_DIR}/opengl32.dll"
	"${SONIC3AIR_UWP_BIN_DIR}/dxil.dll"
	"${SONIC3AIR_UWP_BIN_DIR}/libgallium_wgl.dll"
	"${SONIC3AIR_UWP_BIN_DIR}/libglapi.dll"
	"${SONIC3AIR_UWP_BIN_DIR}/z-1.dll"
)
	if(NOT EXISTS "${required_runtime_file}")
		message(FATAL_ERROR "Required UWP runtime file was not found: ${required_runtime_file}")
	endif()
endforeach()

get_filename_component(SONIC3AIR_REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
set(SONIC3AIR_ZLIB_STAGE_ROOT "${CMAKE_CURRENT_BINARY_DIR}/third-party")
set(SONIC3AIR_ZLIB_SOURCE_DIR "${SONIC3AIR_ZLIB_STAGE_ROOT}/zlib")
set(SONIC3AIR_ZLIB_BUILD_DIR "${SONIC3AIR_ZLIB_STAGE_ROOT}/zlib-build")

set(SONIC3AIR_COMMON_INCLUDE_DIRS
	"${SONIC3AIR_UWP_INCLUDE_DIR}"
	"${SONIC3AIR_UWP_INCLUDE_DIR}/SDL2"
	"${SONIC3AIR_UWP_GLAD_INCLUDE_DIR}"
	"${SONIC3AIR_REPO_ROOT}/framework/external/ogg-vorbis/libogg/include"
	"${SONIC3AIR_REPO_ROOT}/framework/external/ogg-vorbis/libvorbis/include"
	"${SONIC3AIR_REPO_ROOT}/framework/external/ogg-vorbis/libvorbis/lib"
	"${SONIC3AIR_REPO_ROOT}/framework/external/zlib/zlib"
	"${SONIC3AIR_ZLIB_BUILD_DIR}"
	"${SONIC3AIR_REPO_ROOT}/framework/external/zlib/zlib/contrib/minizip"
	"${SONIC3AIR_REPO_ROOT}/framework/external/imgui/imgui"
	"${SONIC3AIR_REPO_ROOT}/librmx/source"
	"${SONIC3AIR_REPO_ROOT}/Oxygen/lemonscript/source"
	"${SONIC3AIR_REPO_ROOT}/Oxygen/oxygenengine/source"
	"${SONIC3AIR_REPO_ROOT}/Oxygen/sonic3air/source"
	"${SONIC3AIR_REPO_ROOT}/Oxygen/sonic3air/source/external"
)

function(sonic3air_apply_runtime_settings target_name)
	target_include_directories(${target_name} PRIVATE ${SONIC3AIR_COMMON_INCLUDE_DIRS})
	target_compile_definitions(${target_name} PRIVATE
		UWP_BUILD=1
		USE_IMGUI=1
		USE_DISCORD=0
		SDL_MAIN_HANDLED=1
		__WINRT__=1
		WINAPI_FAMILY=WINAPI_FAMILY_APP
		MAXPATHLEN=256
		_CRT_SECURE_NO_WARNINGS
		NOMINMAX
	)
	if(MSVC)
		target_compile_options(${target_name} PRIVATE /FS /MP1 /Z7)
	endif()
endfunction()

# Give zlib its own throwaway copy so configure does not scribble over the upstream source tree.
file(REMOVE_RECURSE "${SONIC3AIR_ZLIB_SOURCE_DIR}")
file(COPY "${SONIC3AIR_REPO_ROOT}/framework/external/zlib/zlib" DESTINATION "${SONIC3AIR_ZLIB_STAGE_ROOT}")
add_subdirectory("${SONIC3AIR_ZLIB_SOURCE_DIR}" "${SONIC3AIR_ZLIB_BUILD_DIR}")

file(GLOB OGGVORBIS_SOURCES
	"${SONIC3AIR_REPO_ROOT}/framework/external/ogg-vorbis/libogg/src/*.c"
	"${SONIC3AIR_REPO_ROOT}/framework/external/ogg-vorbis/libvorbis/lib/*.c"
)
add_library(sonic3air_oggvorbis STATIC ${OGGVORBIS_SOURCES})
sonic3air_apply_runtime_settings(sonic3air_oggvorbis)

file(GLOB MINIZIP_SOURCES
	"${SONIC3AIR_REPO_ROOT}/framework/external/zlib/zlib/contrib/minizip/ioapi.c"
	"${SONIC3AIR_REPO_ROOT}/framework/external/zlib/zlib/contrib/minizip/unzip.c"
	"${SONIC3AIR_REPO_ROOT}/framework/external/zlib/zlib/contrib/minizip/zip.c"
)
add_library(sonic3air_minizip STATIC ${MINIZIP_SOURCES})
sonic3air_apply_runtime_settings(sonic3air_minizip)
target_link_libraries(sonic3air_minizip PRIVATE zlibstatic)

file(GLOB IMGUI_SOURCES
	"${SONIC3AIR_REPO_ROOT}/framework/external/imgui/imgui/*.cpp"
	"${SONIC3AIR_REPO_ROOT}/framework/external/imgui/imgui/backends/imgui_impl_opengl3.cpp"
	"${SONIC3AIR_REPO_ROOT}/framework/external/imgui/imgui/backends/imgui_impl_sdl2.cpp"
)
add_library(sonic3air_imgui STATIC ${IMGUI_SOURCES})
sonic3air_apply_runtime_settings(sonic3air_imgui)
target_link_libraries(sonic3air_imgui PRIVATE
	"${SONIC3AIR_UWP_LIB_DIR}/SDL2.lib"
	opengl32.lib
)

file(GLOB_RECURSE RMXBASE_SOURCES
	"${SONIC3AIR_REPO_ROOT}/librmx/source/rmxbase/*.cpp"
)
add_library(sonic3air_rmxbase STATIC ${RMXBASE_SOURCES})
sonic3air_apply_runtime_settings(sonic3air_rmxbase)
target_link_libraries(sonic3air_rmxbase PRIVATE zlibstatic)

file(GLOB_RECURSE RMXMEDIA_SOURCES
	"${SONIC3AIR_REPO_ROOT}/librmx/source/rmxmedia/*.cpp"
)
list(APPEND RMXMEDIA_SOURCES "${SONIC3AIR_UWP_GLAD_SOURCE}")
add_library(sonic3air_rmxmedia STATIC ${RMXMEDIA_SOURCES})
sonic3air_apply_runtime_settings(sonic3air_rmxmedia)
target_link_libraries(sonic3air_rmxmedia PRIVATE
	sonic3air_rmxbase
	"${SONIC3AIR_UWP_LIB_DIR}/SDL2.lib"
	opengl32.lib
)

file(GLOB RMXEXT_OGGVORBIS_SOURCES
	"${SONIC3AIR_REPO_ROOT}/librmx/source/rmxext_oggvorbis/*.cpp"
)
add_library(sonic3air_rmxext_oggvorbis STATIC ${RMXEXT_OGGVORBIS_SOURCES})
sonic3air_apply_runtime_settings(sonic3air_rmxext_oggvorbis)
target_link_libraries(sonic3air_rmxext_oggvorbis PRIVATE
	sonic3air_rmxmedia
	sonic3air_oggvorbis
)

file(GLOB_RECURSE LEMONSCRIPT_SOURCES
	"${SONIC3AIR_REPO_ROOT}/Oxygen/lemonscript/source/lemon/*.cpp"
)
add_library(sonic3air_lemonscript STATIC ${LEMONSCRIPT_SOURCES})
sonic3air_apply_runtime_settings(sonic3air_lemonscript)
target_link_libraries(sonic3air_lemonscript PRIVATE sonic3air_rmxbase)

file(GLOB_RECURSE OXYGEN_NETCORE_SOURCES
	"${SONIC3AIR_REPO_ROOT}/Oxygen/oxygenengine/source/oxygen_netcore/*.cpp"
)
add_library(sonic3air_oxygen_netcore STATIC ${OXYGEN_NETCORE_SOURCES})
sonic3air_apply_runtime_settings(sonic3air_oxygen_netcore)
target_link_libraries(sonic3air_oxygen_netcore PRIVATE sonic3air_rmxbase)

file(GLOB_RECURSE OXYGEN_SOURCES
	"${SONIC3AIR_REPO_ROOT}/Oxygen/oxygenengine/source/oxygen/*.cpp"
)
add_library(sonic3air_oxygen STATIC ${OXYGEN_SOURCES})
sonic3air_apply_runtime_settings(sonic3air_oxygen)
target_link_libraries(sonic3air_oxygen PRIVATE
	sonic3air_minizip
	sonic3air_rmxmedia
	sonic3air_rmxext_oggvorbis
	sonic3air_lemonscript
	sonic3air_imgui
	sonic3air_oxygen_netcore
)

file(GLOB_RECURSE SONIC3AIR_RUNTIME_SOURCES
	"${SONIC3AIR_REPO_ROOT}/Oxygen/sonic3air/source/sonic3air/*.cpp"
)
add_library(sonic3air_runtime STATIC ${SONIC3AIR_RUNTIME_SOURCES})
sonic3air_apply_runtime_settings(sonic3air_runtime)
target_link_libraries(sonic3air_runtime PRIVATE sonic3air_oxygen)

add_executable(${PROJECT_NAME} WIN32
	uwp/main.cpp
	uwp/sonic3air-entry.cpp
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
	UWP_BUILD=1
	SDL_MAIN_HANDLED=1
	__WINRT__=1
	WINAPI_FAMILY=WINAPI_FAMILY_APP
	NOMINMAX
)

target_include_directories(${PROJECT_NAME} PRIVATE
	"${SONIC3AIR_UWP_INCLUDE_DIR}"
	"${SONIC3AIR_UWP_INCLUDE_DIR}/SDL2"
)
target_link_libraries(${PROJECT_NAME} PRIVATE sonic3air_runtime)

set(PACKAGE_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/uwp/Package.appxmanifest"
)

set(PACKAGE_ASSETS
	"${CMAKE_CURRENT_SOURCE_DIR}/uwp/Assets/LargeTile.png"
	"${CMAKE_CURRENT_SOURCE_DIR}/uwp/Assets/SmallTile.png"
	"${CMAKE_CURRENT_SOURCE_DIR}/uwp/Assets/SplashScreen.png"
	"${CMAKE_CURRENT_SOURCE_DIR}/uwp/Assets/Square150x150Logo.png"
	"${CMAKE_CURRENT_SOURCE_DIR}/uwp/Assets/Square44x44Logo.png"
	"${CMAKE_CURRENT_SOURCE_DIR}/uwp/Assets/StoreLogo.png"
	"${CMAKE_CURRENT_SOURCE_DIR}/uwp/Assets/WideTile.png"
)

set_source_files_properties(${PACKAGE_FILES} PROPERTIES
	VS_DEPLOYMENT_CONTENT TRUE
	VS_DEPLOYMENT_LOCATION "."
)

set_source_files_properties(${PACKAGE_ASSETS} PROPERTIES
	VS_DEPLOYMENT_CONTENT TRUE
	VS_DEPLOYMENT_LOCATION "Assets"
)

target_sources(${PROJECT_NAME} PRIVATE
	${PACKAGE_FILES}
	${PACKAGE_ASSETS}
)

function(sonic3air_deploy_file file_path deploy_location)
	if(NOT EXISTS "${file_path}")
		message(WARNING "Skipping missing deployment file: ${file_path}")
		return()
	endif()

	target_sources(${PROJECT_NAME} PRIVATE "${file_path}")
	set_source_files_properties("${file_path}" PROPERTIES
		VS_COPY_TO_OUT_DIR Always
		VS_DEPLOYMENT_CONTENT TRUE
		VS_DEPLOYMENT_LOCATION "${deploy_location}"
	)
endfunction()

function(sonic3air_deploy_tree source_root deploy_root)
	if(NOT EXISTS "${source_root}")
		message(WARNING "Skipping missing deployment tree: ${source_root}")
		return()
	endif()

	file(GLOB_RECURSE tree_files RELATIVE "${source_root}" "${source_root}/*")
	foreach(relative_path IN LISTS tree_files)
		set(full_path "${source_root}/${relative_path}")
		if(NOT IS_DIRECTORY "${full_path}")
			set(skip_file FALSE)
			foreach(excluded_path IN LISTS ARGN)
				if(relative_path MATCHES "^${excluded_path}(/|$)")
					set(skip_file TRUE)
					break()
				endif()
			endforeach()
			if(skip_file)
				continue()
			endif()

			get_filename_component(relative_directory "${relative_path}" DIRECTORY)
			if(relative_directory STREQUAL "")
				set(deploy_location "${deploy_root}")
			else()
				set(deploy_location "${deploy_root}/${relative_directory}")
			endif()
			sonic3air_deploy_file("${full_path}" "${deploy_location}")
		endif()
	endforeach()
endfunction()

sonic3air_deploy_tree("${SONIC3AIR_REPO_ROOT}/Oxygen/oxygenengine/data" "data")
sonic3air_deploy_tree("${SONIC3AIR_REPO_ROOT}/Oxygen/sonic3air/data" "data" "audio/remastered")
sonic3air_deploy_tree("${SONIC3AIR_REPO_ROOT}/Oxygen/sonic3air/scripts" "scripts")
sonic3air_deploy_file("${SONIC3AIR_REPO_ROOT}/Oxygen/sonic3air/_master_image_template/config.json" ".")
if(EXISTS "${SONIC3AIR_REPO_ROOT}/Oxygen/sonic3air/saves/scripts.bin")
	sonic3air_deploy_file("${SONIC3AIR_REPO_ROOT}/Oxygen/sonic3air/saves/scripts.bin" "data")
endif()

foreach(runtimeFile IN ITEMS
	"${SONIC3AIR_UWP_BIN_DIR}/SDL2.dll"
	"${SONIC3AIR_UWP_BIN_DIR}/opengl32.dll"
	"${SONIC3AIR_UWP_BIN_DIR}/dxil.dll"
	"${SONIC3AIR_UWP_BIN_DIR}/libgallium_wgl.dll"
	"${SONIC3AIR_UWP_BIN_DIR}/libglapi.dll"
	"${SONIC3AIR_UWP_BIN_DIR}/z-1.dll"
)
	sonic3air_deploy_file("${runtimeFile}" ".")
endforeach()

target_link_libraries(${PROJECT_NAME} PRIVATE
	"${SONIC3AIR_UWP_LIB_DIR}/SDL2.lib"
	WindowsApp.lib
)
