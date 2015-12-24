
cmake_minimum_required (VERSION 2.6)
project (milton)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


add_executable(mla mla.c libstolen.c ui.c wiki.c)
add_executable(test-pcre test-pcre.c)
find_package(CURL)
find_package(LibXml2)
include_directories(${LIBXML2_INCLUDE_DIR})
set(CMAKE_C_FLAGS_RELEASE "--std=gnu11 -O2 -Wall -Wextra -pedantic")        ## Optimize
set(CMAKE_C_FLAGS_DEBUG "--std=gnu11 -ggdb -Wall -Wextra -pedantic")        ## Debug
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-s")  ## Strip binary
target_link_libraries(mla pthread curl ncursesw ${LIBXML2_LIBRARIES})
target_link_libraries(test-pcre pcre2-8)
