# the (library) modules to build
set(SERAPHIM_MODULES "core" "car" "face" "ipc" "object")

# the core module must always be built
if (NOT "core" IN_LIST SERAPHIM_MODULES)
  list(APPEND SERAPHIM_MODULES "core")
endif ()

foreach (module ${SERAPHIM_MODULES})
  if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${module}")
    add_subdirectory(${module})
  else ()
    message(WARNING "Module \"${module}\" does not exist.")
  endif ()
endforeach ()

set(SERAPHIM_MODULES ${SERAPHIM_MODULES} CACHE INTERNAL "List of modules" FORCE)
