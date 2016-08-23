cmake_minimum_required(VERSION 3.5)
project(Lime2D)

if (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
endif()
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_BUILD_TYPE Debug)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake") #CMAKE_CURRENT_LIST_DIR is the directory holding the CMakeLists.txt we are currently using

set(SOURCE_FILES src/main.cpp src/lime2d.cpp
        libext/imgui.cpp libext/imgui_demo.cpp libext/imgui_draw.cpp libext/imgui-SFML.cpp libext/tinyxml2.cpp src/lime2d_internal.cpp)
add_executable(Lime2D ${SOURCE_FILES})

include(FindPkgConfig)
find_package(SFML COMPONENTS system window graphics network audio REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Lua REQUIRED)

include_directories(${SFML_INCLUDE_DIR})
include_directories(${LUA_INCLUDE_DIR})



if(WIN32)
    target_link_libraries(${PROJECT_NAME} ${SFML_LIBRARIES} ${LUA_LIBRARY} -lopengl32 -lmingw32 -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lversion -luuid)
else()
    target_link_libraries(${PROJECT_NAME} ${SFML_LIBRARIES} ${OPENGL_LIBRARY} ${LUA_LIBRARY} )
endif()
