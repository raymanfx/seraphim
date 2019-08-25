# the tests to build
set(SERAPHIM_TESTS "core")

# dependencies
set(SERAPHIM_TESTS_DEPENDENCIES_core "core")

foreach (test ${SERAPHIM_TESTS})
  set(DEPENDENCY_CHECK_SUCCESS TRUE)
  foreach (module ${SERAPHIM_TESTS_DEPENDENCIES_${sample}})
    if (NOT ${module} IN_LIST SERAPHIM_MODULES)
      message(WARNING "Module \"${module}\" deactivated, not building test: \"${test}\"")
      list(REMOVE_ITEM SERAPHIM_TESTS ${test})
      set(DEPENDENCY_CHECK_SUCCESS FALSE)
      break()
    endif ()
  endforeach ()

  if (NOT DEPENDENCY_CHECK_SUCCESS)
    continue()
  endif ()

  if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${test}")
    add_subdirectory(${test})
  else ()
    message(WARNING "Test \"${test}\" does not exist.")
    list(REMOVE_ITEM SERAPHIM_TESTS ${test})
  endif ()
endforeach ()

set(SERAPHIM_TESTS ${SERAPHIM_TESTS} CACHE INTERNAL "List of tests" FORCE)
