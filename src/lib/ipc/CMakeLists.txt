set(MODULE_NAME seraphim_ipc)

set(SOURCES
    src/net/socket.cpp
    src/net/tcp_socket.cpp
    src/net/tcp_stream.cpp
    src/shm_transport.cpp
    src/tcp_transport.cpp)

set(HEADERS
    include/seraphim/ipc/net/socket.h
    include/seraphim/ipc/net/tcp_socket.h
    include/seraphim/ipc/net/tcp_stream.h
    include/seraphim/ipc/transport.h
    include/seraphim/ipc/semaphore.h
    include/seraphim/ipc/shm_transport.h
    include/seraphim/ipc/tcp_transport.h)

add_library(${MODULE_NAME} OBJECT ${SOURCES} ${HEADERS})

# shared libraries need PIC
set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)

# shared and static libraries built from the same object files
add_library(${MODULE_NAME}_shared SHARED $<TARGET_OBJECTS:${MODULE_NAME}>)
#add_library(${MODULE_NAME}_static STATIC $<TARGET_OBJECTS:${MODULE_NAME}>)

# set the shared lib name
set_target_properties(${MODULE_NAME}_shared PROPERTIES OUTPUT_NAME ${MODULE_NAME})

# set the shared lib version
set_target_properties(${LIB_NAME} PROPERTIES SOVERSION 0.0.0)

# Linux requires POSIX realtime extensions for threads
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  target_link_libraries(${MODULE_NAME} PRIVATE rt)
endif ()

target_link_libraries(${MODULE_NAME} PRIVATE seraphim_messages)

# Make sure the compiler can find include files for our library
# when other libraries or executables link to it
target_include_directories(${MODULE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# install shared lib and headers
install(FILES ${HEADERS} DESTINATION include/seraphim/ipc)
install(TARGETS ${MODULE_NAME}_shared DESTINATION lib)
