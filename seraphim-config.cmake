# seraphim-config.cmake - package configuration file

get_filename_component(SELF_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(${SELF_DIR}/seraphim_core.cmake)
include(${SELF_DIR}/seraphim_ipc.cmake)
