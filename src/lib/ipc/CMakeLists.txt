# protobuf messages
add_subdirectory(data)

set(MODULE_NAME seraphim_ipc)
set(MODULE_VERSION_MAJOR 0)
set(MODULE_VERSION_MINOR 0)
set(MODULE_VERSION_PATCH 0)

set(SOURCES
    src/module.cpp
    src/net/socket.cpp
    src/net/tcp_socket.cpp
    src/net/tcp_stream.cpp
    src/semaphore.cpp
    src/shm_transport.cpp
    src/tcp_transport.cpp
    src/transport_factory.cpp)

set(HEADERS
    include/seraphim/ipc.h
    include/seraphim/ipc/module.h
    include/seraphim/ipc/net/socket.h
    include/seraphim/ipc/net/tcp_socket.h
    include/seraphim/ipc/net/tcp_stream.h
    include/seraphim/ipc/transport.h
    include/seraphim/ipc/transport_factory.h
    include/seraphim/ipc/semaphore.h
    include/seraphim/ipc/shm_transport.h
    include/seraphim/ipc/tcp_transport.h)

add_library(${MODULE_NAME} SHARED ${SOURCES} ${HEADERS})

# set the shared lib name
set_target_properties(${MODULE_NAME} PROPERTIES OUTPUT_NAME ${MODULE_NAME})

# set the shared lib version
set_target_properties(${MODULE_NAME} PROPERTIES VERSION
                      ${MODULE_VERSION_MAJOR}.${MODULE_VERSION_MINOR}.${MODULE_VERSION_PATCH})
set_target_properties(${MODULE_NAME} PROPERTIES SOVERSION
                      ${MODULE_VERSION_MAJOR})

# Linux requires POSIX realtime extensions for threads
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  target_link_libraries(${MODULE_NAME} PRIVATE rt)
endif ()

target_link_libraries(${MODULE_NAME} PUBLIC seraphim_core)
target_link_libraries(${MODULE_NAME} PUBLIC seraphim_messages)

# Make sure the compiler can find include files for our library
# when other libraries or executables link to it
target_include_directories(${MODULE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# install shared lib and headers
install(FILES ${HEADERS} DESTINATION include/seraphim/ipc)
install(TARGETS ${MODULE_NAME} DESTINATION lib)
