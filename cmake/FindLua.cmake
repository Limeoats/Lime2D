set(FIND_LUA_PATHS
        C:/Lua)

find_path(LUA_INCLUDE_DIR lua.hpp
        PATH_SUFFIXES include
        PATHS ${FIND_LUA_PATHS})

find_library(LUA_LIBRARY
        NAMES lua5.1
        PATH_SUFFIXES lib
        PATHS ${FIND_LUA_PATHS})