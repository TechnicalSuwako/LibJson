cmake_minimum_required(VERSION 3.14...3.28)
project(json LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "静的リンク" FORCE)

option(ENABLE_TEST "テストプロジェクト" ON)

if(MSVC) 
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  add_compile_options(/utf-8)
endif()

# 本ライブラリ
add_library(json STATIC)
file(GLOB_RECURSE LIBJSON_SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc")
file(GLOB_RECURSE LIBJSON_INC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hh")

target_sources(json
  PRIVATE "${LIBJSON_SRC}"
  PUBLIC "${LIBJSON_INC}"
)

target_include_directories(json PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# テスト用
if(ENABLE_TEST)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/webfinger.json
    ${CMAKE_CURRENT_BINARY_DIR}/webfinger.json
    COPYONLY
  )
  
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/activitypub.json
    ${CMAKE_CURRENT_BINARY_DIR}/activitypub.json
    COPYONLY
  )
  
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/gltf.json
    ${CMAKE_CURRENT_BINARY_DIR}/gltf.json
    COPYONLY
  )

  add_executable(json_test)
  file(GLOB_RECURSE TEST_SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cc")
  file(GLOB_RECURSE TEST_INC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.hh")
  target_sources(json_test PRIVATE ${TEST_SRC})
  target_link_libraries(json_test PRIVATE json)

  if(MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT json_test)
  endif()
endif()

# OS
if(UNIX)
  include(GNUInstallDirs)
  set(INSTALL_BINDIR ${CMAKE_INSTALL_LIBDIR})
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
  set(PRODUCTION_BUILD OFF CACHE BOOL "リリース版？" FORCE)
else()
  set(PRODUCTION_BUILD ON CACHE BOOL "リリース版？" FORCE)
endif()

install(TARGETS json
  RUNTIME DESTINATION ${INSTALL_BINDIR}
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
  COMPONENT Runtime
)

install(DIRECTORY DESTINATION ${INSTALL_DATADIR}
  COMPONENT Runtime
)

if(MSVC)
  target_compile_definitions(json PUBLIC _CRT_SECURE_NO_WARNINGS)
elseif(MINGW)
  if(PRODUCTION_BUILD)
    target_link_options(json PRIVATE -mwindows)
  endif()
elseif(APPLE OR UNIX)
  if(PRODUCTION_BUILD)
    target_link_options(json PRIVATE -s)
  endif()
endif()