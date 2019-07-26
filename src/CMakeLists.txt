set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(Seraphim)

# macOS relies on homebrew to satisfy dependencies.
# Some headers like protobuf "port_def.inc" are not properly exposes at the
# moment, so we have to include them manually for the project.
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  include_directories("/usr/local/include")
endif ()

add_subdirectory(lib)
