
cmake_minimum_required (VERSION 2.6)
project (milton)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


add_executable(mla mla.c libstolen.c ui.c wiki.c)

find_package(CURL)
find_package(LibXml2)
set(CMAKE_C_FLAGS "--std=gnu11 -I/usr/include/libxml2 -O2 -Wall -Wextra -pedantic")        ## Optimize
##set(CMAKE_C_FLAGS "--std=gnu11 -I/usr/include/libxml2 -ggdb -Wall -Wextra -pedantic")        ## Debug
set(CMAKE_EXE_LINKER_FLAGS "-s")  ## Strip binary
target_link_libraries(mla pthread curl ncurses ${LIBXML2_LIBRARIES})
