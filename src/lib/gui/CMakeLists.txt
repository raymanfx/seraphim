set(MODULE_NAME gui)

set(SOURCES
    gl_window.cpp
    glad.c)

set(HEADERS
    include/KHR/khrplatform.h
    include/glad/glad.h
    include/seraphim/gui.h
    include/seraphim/gui/window.h
    include/seraphim/gui/glfw.h
    include/seraphim/gui/gl_window.h)

add_library(${MODULE_NAME} STATIC ${SOURCES} ${HEADERS})
add_library(seraphim::${MODULE_NAME} ALIAS ${MODULE_NAME})

# GLFW dependency
find_package(glfw3 REQUIRED)
target_link_libraries(${MODULE_NAME} PUBLIC glfw)

target_link_libraries(${MODULE_NAME} PUBLIC seraphim::core)

# Include threads
find_package(Threads REQUIRED)
target_link_libraries(${MODULE_NAME} PUBLIC Threads::Threads)

# Make sure the compiler can find include files for our library
# when other libraries or executables link to it
target_include_directories(${MODULE_NAME} PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# shared libraries need PIC
set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)

# install headers
install(DIRECTORY include/seraphim DESTINATION include)
