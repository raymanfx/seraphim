# the applications to build
set(SERAPHIM_APPS "frontend" "backend")

# dependencies
set(SERAPHIM_APPS_DEPENDENCIES_frontend "")
set(SERAPHIM_APPS_DEPENDENCIES_backend "core" "car" "face" "object")

foreach (app ${SERAPHIM_APPS})
  set(DEPENDENCY_CHECK_SUCCESS TRUE)
  foreach (module ${SERAPHIM_APPS_DEPENDENCIES_${app}})
    if (NOT ${module} IN_LIST SERAPHIM_MODULES)
      message(WARNING "Module \"${module}\" deactivated, not building app: \"${app}\"")
      list(REMOVE_ITEM SERAPHIM_APPS ${app})
      set(DEPENDENCY_CHECK_SUCCESS FALSE)
      break()
    endif ()
  endforeach ()

  if (NOT DEPENDENCY_CHECK_SUCCESS)
    continue()
  endif ()

  if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${app}")
    add_subdirectory(${app})
  else ()
    message(WARNING "App \"${app}\" does not exist.")
    list(REMOVE_ITEM SERAPHIM_APPS ${app})
  endif ()
endforeach ()

set(SERAPHIM_APPS ${SERAPHIM_APPS} CACHE INTERNAL "List of apps" FORCE)
