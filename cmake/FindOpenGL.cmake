set(FIND_OPENGL_PATHS
        ~/Library/Frameworks)

find_library(OPENGL_LIBRARY
        NAMES OpenGL
        PATH_SUFFIXES lib
        PATHS ${FIND_OPENGL_PATHS})