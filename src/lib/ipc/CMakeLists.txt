# protobuf messages
add_subdirectory(data)

set(MODULE_NAME ipc)

set(SOURCES
    net/socket.cpp
    net/tcp_socket.cpp
    semaphore.cpp
    shm_transport.cpp
    tcp_transport.cpp
    transport_factory.cpp)

set(HEADERS
    include/seraphim/ipc.h
    include/seraphim/ipc/net/socket.h
    include/seraphim/ipc/net/tcp_socket.h
    include/seraphim/ipc/except.h
    include/seraphim/ipc/transport.h
    include/seraphim/ipc/transport_factory.h
    include/seraphim/ipc/semaphore.h
    include/seraphim/ipc/shm_transport.h
    include/seraphim/ipc/tcp_transport.h)

add_library(${MODULE_NAME} STATIC ${SOURCES} ${HEADERS})
add_library(seraphim::${MODULE_NAME} ALIAS ${MODULE_NAME})

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

# shared libraries need PIC
set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)

# install headers
install(DIRECTORY include/seraphim DESTINATION include)
