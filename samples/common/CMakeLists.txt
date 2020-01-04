set(SOURCES
    )

set(HEADERS
    optparse.hpp)

add_library(common STATIC ${SOURCES} ${HEADERS})

# cmake cannot detect the language for header only libs
set_target_properties(common PROPERTIES LINKER_LANGUAGE CXX)

# Make sure the compiler can find include files for our library
# when other libraries or executables link to it
target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Include threads
find_package(Threads REQUIRED)
target_link_libraries(common PUBLIC Threads::Threads)
