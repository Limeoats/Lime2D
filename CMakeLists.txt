cmake_minimum_required(VERSION 3.5)
project(Lime2D)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-sign-compare -Wno-unused-parameter -Wno-unused-private-field  -std=c++11")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

set(SOURCE_FILES src/main.cpp src/lime2d.cpp
        imgui/imgui.cpp imgui/imgui_demo.cpp imgui/imgui_draw.cpp src/lime2d_internal.cpp src/lime2d_internal.h)
add_executable(Lime2D ${SOURCE_FILES})

include(FindPkgConfig)
find_package(SFML REQUIRED system window graphics network audio)
include_directories(${SFML_INCLUDE_DIR})

find_package(OpenGL REQUIRED)



if(WIN32)
    target_link_libraries(${PROJECT_NAME} ${SFML_LIBRARIES} -lopengl32 -lmingw32 -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lversion -luuid)
else()
    target_link_libraries(${PROJECT_NAME} ${SFML_LIBRARIES} ${OPENGL_LIBRARY} )
endif()