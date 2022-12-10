cmake_minimum_required(VERSION 3.22)
project(dntgenerator CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/externals/cage/cmake")
include(cage_all)
cage_default_init()
cage_assets_add_data_path("${CMAKE_CURRENT_SOURCE_DIR}/data")

add_subdirectory(externals/cage)

file(GLOB_RECURSE dntgenerator-sources "sources/*")
add_executable(dntgenerator ${dntgenerator-sources})
target_link_libraries(dntgenerator cage-core)
cage_ide_category(dntgenerator dntgenerator)
cage_ide_sort_files(dntgenerator)
cage_ide_working_dir_in_place(dntgenerator)
