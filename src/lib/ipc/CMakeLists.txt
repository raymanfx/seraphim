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

# Linux requires POSIX realtime extensions for threads, semaphores etc
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  target_link_libraries(${MODULE_NAME} PRIVATE rt)
endif ()

# protobuf generated classes
find_package(Protobuf REQUIRED)
target_link_libraries(${MODULE_NAME} PUBLIC ${PROTOBUF_LIBRARY})
target_link_libraries(${MODULE_NAME} PRIVATE seraphim_messages)

target_link_libraries(${MODULE_NAME} PUBLIC seraphim_core)

# Make sure the compiler can find include files for our library
# when other libraries or executables link to it
target_include_directories(${MODULE_NAME} PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/data>
                           $<INSTALL_INTERFACE:include>
                           $<INSTALL_INTERFACE:include/seraphim/ipc/data>)

# install shared lib and headers
install(FILES ${PROTO_HDRS} DESTINATION include/seraphim/ipc/data)
install(DIRECTORY include/seraphim DESTINATION include)
install(TARGETS ${MODULE_NAME} EXPORT ${MODULE_NAME} DESTINATION lib)

# add cmake install target
install(EXPORT ${MODULE_NAME} DESTINATION lib/seraphim)
