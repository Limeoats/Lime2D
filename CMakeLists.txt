cmake_minimum_required(VERSION 2.8)
project(Lime2D)

set(Lime2D_VERSION_MAJOR 1)
set(Lime2D_VERSION_MINOR 0)

if (WIN32)
    set(CPP_VERSION -std=gnu++14)
else()
    set(CPP_VERSION -std=c++14)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall ${CPP_VERSION}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_BUILD_TYPE Release)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

set(SOURCE_FILES src/main.cpp src/lime2d.cpp src/lime2d_internal.cpp
        libext/imgui.cpp libext/imgui_demo.cpp libext/imgui_draw.cpp libext/imgui-SFML.cpp libext/tinyxml2.cpp)

add_library(Lime2D ${SOURCE_FILES})

include(FindPkgConfig)
find_package(SFML COMPONENTS system window graphics network audio REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Lua REQUIRED)

include_directories(${SFML_INCLUDE_DIR})
include_directories(${LUA_INCLUDE_DIR})

if(WIN32)
    target_link_libraries(${PROJECT_NAME} ${SFML_LIBRARIES} ${LUA_LIBRARY} -lopengl32 -lmingw32 -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lversion -luuid)
elseif(APPLE)
    target_link_libraries(${PROJECT_NAME} ${SFML_LIBRARIES} ${OPENGL_LIBRARY} ${LUA_LIBRARY} )
elseif(LINUX)
    target_link_libraries(Lime2DTest ${SFML_LIBRARIES} ${LUA_LIBRARY})
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


#Lime2D TEST Program configuration
add_executable(Lime2DTest ${SOURCE_FILES})
include(FindPkgConfig)
find_package(SFML COMPONENTS system window graphics network audio REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Lua REQUIRED)

include_directories(${SFML_INCLUDE_DIR})
include_directories(${LUA_INCLUDE_DIR})

if(WIN32)
    target_link_libraries(Lime2DTest ${SFML_LIBRARIES} ${LUA_LIBRARY} -lopengl32 -lmingw32 -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lversion -luuid)
elseif(APPLE)
    target_link_libraries(Lime2DTest ${SFML_LIBRARIES} ${OPENGL_LIBRARY} ${LUA_LIBRARY} )
elseif(LINUX)
    target_link_libraries(Lime2DTest ${SFML_LIBRARIES} ${LUA_LIBRARY})
endif()
