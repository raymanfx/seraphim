# the samples to build
set(SERAPHIM_SAMPLES "core" "car" "face" "object")

# dependencies
set(SERAPHIM_SAMPLES_DEPENDENCIES_core "core")
set(SERAPHIM_SAMPLES_DEPENDENCIES_car "car")
set(SERAPHIM_SAMPLES_DEPENDENCIES_face "face" "object")
set(SERAPHIM_SAMPLES_DEPENDENCIES_object "object")

foreach (sample ${SERAPHIM_SAMPLES})
  set(DEPENDENCY_CHECK_SUCCESS TRUE)
  foreach (module ${SERAPHIM_SAMPLES_DEPENDENCIES_${sample}})
    if (NOT ${module} IN_LIST SERAPHIM_MODULES)
      message(WARNING "Module \"${module}\" deactivated, not building sample: \"${sample}\"")
      list(REMOVE_ITEM SERAPHIM_SAMPLES ${sample})
      set(DEPENDENCY_CHECK_SUCCESS FALSE)
      break()
    endif ()
  endforeach ()

  if (NOT DEPENDENCY_CHECK_SUCCESS)
    continue()
  endif ()

  if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${sample}")
    add_subdirectory(${sample})
  else ()
    message(WARNING "Sample \"${sample}\" does not exist.")
    list(REMOVE_ITEM SERAPHIM_SAMPLES ${sample})
  endif ()
endforeach ()

set(SERAPHIM_SAMPLES ${SERAPHIM_SAMPLES} CACHE INTERNAL "List of samples" FORCE)
