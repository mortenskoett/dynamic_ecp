# -- Shared Library --
# Not super good and only for CMake 3.12 or later:
file(GLOB_RECURSE SHARED_SOURCE_FILES CONFIGURE_DEPENDS
  ${ECP_SOURCE_DIR}/src/eCP/index/shared/*.cpp
  ${ECP_SOURCE_DIR}/src/eCP/index/shared/*.hpp
)

add_library(sharedLib
  SHARED
  ${SHARED_SOURCE_FILES}
)

target_include_directories(sharedLib
  PUBLIC
    ./
)

# -- eCP Library --
# Not super good and only for CMake 3.12 or later:
file(GLOB SOURCE_LIST CONFIGURE_DEPENDS "${ECP_SOURCE_DIR}/src/eCP/index/*.cpp")
file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${ECP_SOURCE_DIR}/include/eCP/index/*.hpp")

add_library(eCPLib
  SHARED
  ${SOURCE_LIST}
  ${HEADER_LIST}
)

target_include_directories(eCPLib
  PUBLIC
    ../include/
    ./
)

target_link_libraries(eCPLib
  sharedLib
  utilLib
)

# -- Utilities Library --
add_library(utilLib
  ./eCP/utilities/utilities.cpp
)

target_compile_options(utilLib
  PRIVATE
    "-fPIC"
)

target_link_libraries(utilLib
  PRIVATE
    HDF5::HDF5
)

target_include_directories(utilLib
  PUBLIC
    ./
)

# -- Debugging Library --
add_library(debugLib
  ./eCP/debugging/debug_tools.cpp
)

target_include_directories(debugLib
  PUBLIC
    ./
)

target_link_libraries(debugLib
  sharedLib
)
