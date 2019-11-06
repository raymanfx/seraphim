project(seraphim-lib VERSION 0.0.0)

# the (library) modules to build
set(SERAPHIM_MODULES "core")

set(MODULE_CAR ON CACHE BOOL "Car module")
set(MODULE_FACE ON CACHE BOOL "Face module")
set(MODULE_GUI ON CACHE BOOL "GUI module")
set(MODULE_IOP ON CACHE BOOL "Interoperability module")
set(MODULE_IPC ON CACHE BOOL "Inter-process communication module")
set(MODULE_OBJECT ON CACHE BOOL "Object module")

# build the list of enabled modules
if (MODULE_CAR)
    list(APPEND SERAPHIM_MODULES "car")
endif ()
if (MODULE_FACE)
    list(APPEND SERAPHIM_MODULES "face")
endif ()
if (MODULE_GUI)
    list(APPEND SERAPHIM_MODULES "gui")
endif ()
if (MODULE_IOP)
    list(APPEND SERAPHIM_MODULES "iop")
endif ()
if (MODULE_IPC)
    list(APPEND SERAPHIM_MODULES "ipc")
endif ()
if (MODULE_OBJECT)
    list(APPEND SERAPHIM_MODULES "object")
endif ()

foreach (module ${SERAPHIM_MODULES})
    add_subdirectory(${module})
endforeach ()

set(SERAPHIM_MODULES ${SERAPHIM_MODULES} CACHE INTERNAL "List of modules" FORCE)

# the main library
add_library(${PROJECT_NAME} SHARED dummy.cpp)

foreach (module ${SERAPHIM_MODULES})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${module})
endforeach ()

# set the shared lib version
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION
                      ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})
set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION
                      ${PROJECT_VERSION_MAJOR})

install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}-targets
        DESTINATION ${CMAKE_INSTALL_LIBDIR})

# add cmake install target
install(EXPORT ${PROJECT_NAME}-targets
        NAMESPACE seraphim::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/seraphim)
