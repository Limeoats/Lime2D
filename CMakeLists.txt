cmake_minimum_required(VERSION 2.8)
project(Lime2D)

set(Lime2D_VERSION_MAJOR 1)
set(Lime2D_VERSION_MINOR 2)

if (WIN32)
    set(CPP_VERSION -std=gnu++14)
elseif(APPLE)
    set(CPP_VERSION -std=c++14)
else()
    set(CPP_VERSION -std=c++14)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-reorder ${CPP_VERSION}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

set(CMAKE_BUILD_TYPE Release)

set(LIB_FILES src/main.cpp src/lime2d.cpp src/lime2d_internal.cpp
        libext/imgui.cpp libext/imgui_demo.cpp libext/imgui_draw.cpp libext/imgui-SFML.cpp libext/tinyxml2.cpp)

include(FindPkgConfig)
find_package(SFML COMPONENTS system window graphics network audio REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Lua REQUIRED)

include_directories(${SFML_INCLUDE_DIR})
include_directories(${LUA_INCLUDE_DIR})

add_library(Lime2D ${LIB_FILES})
if (WIN32 OR APPLE)
    add_executable(Lime2DTest ${LIB_FILES})
endif()

if (WIN32)
    install(TARGETS Lime2D DESTINATION C:/Libs/Lime2D/lib)
    install(FILES src/lime2d.h src/lime2d_internal.h DESTINATION C:/Libs/Lime2D/include)
else()
    install(TARGETS Lime2D DESTINATION ~/Library/Frameworks/Lime2D/lib)
    install(FILES src/lime2d.h src/lime2d_internal.h DESTINATION ~/Library/Frameworks/Lime2D/include)
endif()
install(TARGETS Lime2D DESTINATION ${CMAKE_CURRENT_LIST_DIR}/bin/Lime2D/lib)
install(FILES src/lime2d.h src/lime2d_internal.h DESTINATION ${CMAKE_CURRENT_LIST_DIR}/bin/Lime2D/include)

add_custom_command(
        TARGET Lime2D
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target install
)

if(WIN32)
    target_link_libraries(Lime2D ${SFML_LIBRARIES} ${LUA_LIBRARY} -lopengl32 -lmingw32 -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lversion -luuid)
elseif(APPLE)
    target_link_libraries(Lime2D ${SFML_LIBRARIES} ${OPENGL_LIBRARY} ${LUA_LIBRARY})
else()
    target_link_libraries(Lime2D ${SFML_LIBRARIES} ${OPENGL_LIBRARY} ${LUA_LIBRARY})
endif()

if (WIN32 OR APPLE)
    if (WIN32)
        target_link_libraries(Lime2DTest ${SFML_LIBRARIES} ${LUA_LIBRARY} -lopengl32 -lmingw32 -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lversion -luuid -lktmw32 -lstdc++fs)
    elseif (APPLE)
        target_link_libraries(Lime2DTest ${SFML_LIBRARIES} ${OPENGL_LIBRARY} ${LUA_LIBRARY})
    endif()
endif()
