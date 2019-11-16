project(Seraphim)

# macOS relies on homebrew to satisfy dependencies.
# Some headers like protobuf "port_def.inc" are not properly exposes at the
# moment, so we have to include them manually for the project.
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  include_directories("/usr/local/opt/protobuf/include")
endif ()

# the library modules are dependencies for other components (apps, samples), so
# we need to include those first to notify others about their build state
add_subdirectory(lib)
add_subdirectory(apps)

message(STATUS " ")
message(STATUS ">> Seraphim build configuration <<")
message(STATUS " ")
message(STATUS "  + Modules:")
foreach (module ${SERAPHIM_MODULES})
  message(STATUS "  |-- ${module}")
endforeach ()
message(STATUS " ")
message(STATUS "  + Apps:")
foreach (app ${SERAPHIM_APPS})
  message(STATUS "  |-- ${app}")
endforeach ()
message(STATUS " ")
message(STATUS "  + Samples:")
foreach (sample ${SERAPHIM_SAMPLES})
  message(STATUS "  |-- ${sample}")
endforeach ()
message(STATUS " ")
