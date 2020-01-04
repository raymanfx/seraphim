# protobuf messages
add_subdirectory(data)

set(MODULE_NAME ipc)
set(MODULE_VERSION_MAJOR 0)
set(MODULE_VERSION_MINOR 0)
set(MODULE_VERSION_PATCH 0)

set(SOURCES
    net/socket.cpp
    net/tcp_socket.cpp
    net/udp_socket.cpp
    semaphore.cpp
    shm_transport.cpp
    tcp_transport.cpp
    transport_factory.cpp)

set(HEADERS
    include/seraphim/ipc.hpp
    include/seraphim/ipc/net/socket.hpp
    include/seraphim/ipc/net/tcp_socket.hpp
    include/seraphim/ipc/net/udp_socket.hpp
    include/seraphim/ipc/except.hpp
    include/seraphim/ipc/transport.hpp
    include/seraphim/ipc/transport_factory.hpp
    include/seraphim/ipc/semaphore.hpp
    include/seraphim/ipc/shm_transport.hpp
    include/seraphim/ipc/tcp_transport.hpp)

add_library(${MODULE_NAME} SHARED ${SOURCES} ${HEADERS})
add_library(seraphim::${MODULE_NAME} ALIAS ${MODULE_NAME})

# set the shared lib name
set_target_properties(${MODULE_NAME} PROPERTIES OUTPUT_NAME seraphim_${MODULE_NAME})

# set the shared lib version
set_target_properties(${MODULE_NAME} PROPERTIES VERSION
                      ${MODULE_VERSION_MAJOR}.${MODULE_VERSION_MINOR}.${MODULE_VERSION_PATCH})
set_target_properties(${MODULE_NAME} PROPERTIES SOVERSION
                      ${MODULE_VERSION_MAJOR})

# Linux requires POSIX realtime extensions for threads, semaphores etc
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  target_link_libraries(${MODULE_NAME} PRIVATE rt)
endif ()

target_link_libraries(${MODULE_NAME} PUBLIC seraphim::core)
target_link_libraries(${MODULE_NAME} PUBLIC seraphim::ipc_messages)

# Make sure the compiler can find include files for our library
# when other libraries or executables link to it
target_include_directories(${MODULE_NAME} PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# install shared lib and headers
install(DIRECTORY include/seraphim
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT headers)
install(TARGETS ${MODULE_NAME} EXPORT ${MODULE_NAME}
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT libraries)

# see https://gitlab.kitware.com/cmake/cmake/issues/17357
install(TARGETS ipc_messages EXPORT ${MODULE_NAME})

# add cmake install target
install(EXPORT ${MODULE_NAME} NAMESPACE seraphim::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/seraphim
    COMPONENT libraries)
