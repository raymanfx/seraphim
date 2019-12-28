# the benchmarks to build
set(SERAPHIM_BENCHMARKS "core")

# dependencies
set(SERAPHIM_TESTS_DEPENDENCIES_core "core")

foreach (bmark ${SERAPHIM_BENCHMARKS})
  set(DEPENDENCY_CHECK_SUCCESS TRUE)
  foreach (module ${SERAPHIM_TESTS_DEPENDENCIES_${bmark}})
    if (NOT ${module} IN_LIST SERAPHIM_MODULES)
      message(WARNING "Module \"${module}\" deactivated, not building benchmark: \"${bmark}\"")
      list(REMOVE_ITEM SERAPHIM_BENCHMARKS ${bmark})
      set(DEPENDENCY_CHECK_SUCCESS FALSE)
      break()
    endif ()
  endforeach ()

  if (NOT DEPENDENCY_CHECK_SUCCESS)
    continue()
  endif ()

  if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${bmark}")
    add_subdirectory(${bmark})
  else ()
    message(WARNING "Benchmark \"${bmark}\" does not exist.")
    list(REMOVE_ITEM SERAPHIM_BENCHMARKS ${bmark})
  endif ()
endforeach ()

set(SERAPHIM_BENCHMARKS ${SERAPHIM_BENCHMARKS} CACHE INTERNAL "List of benchmarks" FORCE)
